/* ---------------------------------------------------------------------
 * implementation of MemcheckView                      memcheck_view.cpp
 * ---------------------------------------------------------------------
 */

#include "memcheck_view.h"
#include "vk_config.h"
#include "tb_memcheck_icons.h"
#include "vk_utils.h"
#include "async_process.h"
#include "vk_msgbox.h"

#include <qcursor.h>
#include <qfiledialog.h>
#include <qheader.h>
#include <qmenubar.h>
#include <qmotifstyle.h>
#include <qtoolbutton.h>
#include <qtooltip.h>


void MemcheckView::showSuppEditor()
{ printf("TODO: Memcheck::showSuppEditor()\n"); }



/* class MemcheckView -------------------------------------------------- */
MemcheckView::~MemcheckView() 
{ 
  delete lView;
  lView = 0;

  if ( xmlParser ) {
    delete xmlParser;
    xmlParser = 0;
  }
  if ( proc ) {
    killProc();
    //delete proc;
    //proc = 0;
  }
}



MemcheckView::MemcheckView( QWidget* parent, VkObject* obj )
  : ToolView( parent, obj )
{
  valkyrie = (Valkyrie*)vkConfig->vkObject( "valkyrie" );

  mkMenuBar();

  /* create the listview */
  lView = new QListView( this, "lview" );
  lView->setShowToolTips( false );
  lView->setSorting( -1 );
  lView->setMargin( 5 );
  lView->addColumn( "" );
  lView->header()->setStretchEnabled( true, 0 );
  lView->header()->hide();
  QFont fnt( "Adobe Courier", 12, QFont::Normal, false );
  fnt.setStyleHint( QFont::TypeWriter );
  lView->setFont( fnt );

  setFocusProxy( lView );
  setCentralWidget( lView );

	/* create the xml parser */
	xmlParser = new XMLParser( this );
	reader.setContentHandler( xmlParser );
	reader.setErrorHandler( xmlParser );

  savelogButton->setEnabled( false );
  openOneButton->setEnabled( false );
  openAllButton->setEnabled( false );
  srcPathButton->setEnabled( false );

	toggleRunning( false );

  /* enable | disable show*Item buttons */
  connect( lView, SIGNAL(selectionChanged()),
           this,  SLOT(itemSelected()) );
  /* launch editor with src file loaded */
  connect( lView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
           this,  SLOT(launchEditor(QListViewItem*, const QPoint&, int)) );
}


void MemcheckView::clear()
{ lView->clear(); }


void MemcheckView::stop() 
{ killProc(); }


/* set state for MainWin's run, restart, stop buttons, as well as our own */
void MemcheckView::toggleRunning( bool run )
{
	is_Running = run;
  emit running( is_Running );
	openlogButton->setEnabled( !is_Running );
	savelogButton->setEnabled( !is_Running );

  if ( is_Running ) {  /* startup */
		setCursor( QCursor(Qt::WaitCursor) );
	} else {             /* finished */
		unsetCursor();
	}
}


/* Called by MainWin when user toggles 'show-butt-text' via Options page */
void MemcheckView::toggleToolbarLabels( bool state )
{
  openlogButton->setUsesTextLabel( state );
  savelogButton->setUsesTextLabel( state );
  suppedButton->setUsesTextLabel( state );
}


/* Choices of what we are supposed to be doing are:
   o Valkyrie::RunMode == modeParseLog:
     - user specified --view-log on the cmd-line: MainWin calls run()
     - user clicked openLogFile menu:             openLogFile() calls run()
   o Valkyrie::RunMode == modeMergeLogs:
     - user specified --merge on the cmd-line:    MainWin calls run()
     - user clicked mergeLogFile menu:            openMergeFile() calls run()
   o Valkyrie::RunMode == modeParseOutput:
     - just run valgrind with all args, incl. --xml=yes
*/
bool MemcheckView::run()
{
  /* already doing stuff */
  if ( is_Running ) {
    vkInfo( this, "Memcheck Run", 
            "<p>Currently engaged. Try again later.</p>" );
    return is_Running;
  }

	/* start setting things up */
	bool ok = true;
	currentPid  = -1;
	toggleRunning( true );

	/* reset, clear and initialise the parser and the input source */
	xmlParser->reset();
	source.reset();

	switch ( valkyrie->runMode ) {

		/* what-to-do wasn't specified on the cmd-line, so find out */
		case Valkyrie::modeNotSet: {
			ok = setup();
		} break;

		case Valkyrie::modeParseLog: {
			ok = parseLog();
		} break;

		case Valkyrie::modeMergeLogs: {
			ok = mergeLogs();
		} break;

		case Valkyrie::modeParseOutput: {
			ok = initParseOutput();
		} break;

	}   /* end switch ( valkyrie->runMode ) */

	toggleRunning( false );
	return is_Running;
}


/* what-to-do wasn't specified on the cmd-line, so find out, or make
   some arbitrary decision :) */
bool MemcheckView::setup()
{
	printf("TODO: setup()\n");
	return false;
}


/* Called from run().  Either a logfile was specified on the cmd-line,
   or has been set via the open-file-dialog.  Either way, the value in
   [valkyrie:view-log] is what we need to know. */
bool MemcheckView::parseLog()
{
	bool ok = false;
  logFilename = vkConfig->rdEntry( "view-log", "valkyrie" );
  logFile.setName( logFilename );
  if ( !logFile.open( IO_ReadOnly ) ) {
    vkError( this, "Open File Error", 
             "<p>Unable to open logfile '%s'</p>", logFilename.ascii() );
    return ok;
  }
	QFileInfo fi( logFilename );
	emit message( "Parsing: " + fi.fileName() );

  int lineNumber = 1;
  QTextStream stream( &logFile );
  stream.setEncoding( QTextStream::UnicodeUTF8 );
  inputData = stream.readLine();
  source.setData( inputData );
  ok = reader.parse( &source, true );

  while ( !stream.atEnd() ) {
    lineNumber++;
    inputData = stream.readLine();
    source.setData( inputData );
    ok = reader.parseContinue();
    if ( !ok ) {
      vkError( this, "Parse Error", 
               "<p>Parsing failed on line no %d: '%s'</p>", 
               lineNumber, inputData.ascii() );
      break;
    }
  }

	logFile.close();
	if ( !ok )
		emit message( "Failed: " + fi.fileName() );
	else
		emit message( "Finished: " + fi.fileName() );
	return ok;
}


/* Called from run().  Initialises the auto-save filename and stream,
   resets the parser and its reader, connects proc to a specified fd */
bool MemcheckView::initParseOutput()
{
	bool ok = false;

	/* double-check the --xml flag hasn't been set to no */
  if ( !vkConfig->rdBool( "xml", "valgrind" ) ) {
		vkInfo( this, "Invalid Format", 
						"<p>valkyrie cannot parse text output.</p>"
						"<p>Reset the flag to --xml=yes.</p>" );
		return ok;
	}

	/* setup auto-save valgrind's xml output to file */
	logFilename = vk_mkstemp( "output-log", vkConfig->logsDir() );
	logFile.setName( logFilename );
	if ( ! logFile.open( IO_WriteOnly ) ) { 
		VK_DEBUG("failed to open logfile '%s' for writing", 
						 logFilename.ascii() );
		ok = false;
	} else {
		logStream.setDevice( &logFile );
		inputData = "";
		source.setData( inputData );
		/* tell the parser we are parsing incrementally */
		reader.parse( &source, true );

		/* fork a new process in non-blocking mode */
		if ( proc == 0 ) proc = new QProcess( this );
		/* stuff the cmd-line flags/non-default opts into the process */
		proc->setArguments( flags );

		/* connect the pipe to this toolview on stdout || stderr */
		int fd_out = vkConfig->rdInt( "log-fd", "valgrind" );
		if ( fd_out == 1 ) {           /* stdout */
			connect( proc, SIGNAL( readyReadStdout() ),
							 this, SLOT( parseOutput() ) );
		} else if ( fd_out == 2 ) {    /* stderr */
			connect( proc, SIGNAL( readyReadStderr() ),
							 this, SLOT( parseOutput() ) );
		} else {
			vk_assert_never_reached();
		}

		/* tell MainWin when valgrind has exited */
		connect( proc, SIGNAL( processExited() ),
						 this, SLOT( saveParsedOutput() ) );
		ok = proc->start();
	}

	return ok;
}


/* Slot, connected to proc's signal readyReadStd***().
   Read and process the data, which might be output in chunks.
   Xml output is auto-saveed to a logfile in ~/.valkyrie-X.X.X/logs/ */
void MemcheckView::parseOutput()
{
  bool ok;
  while ( proc->canReadLineStderr() ) {
    inputData = proc->readLineStderr();
    /* auto-save output to a tmp unique logfile */
    logStream << inputData << "\n";
    source.setData( inputData );
    ok = reader.parseContinue();
    if ( !ok ) {
      vkError( this, "Parse Error", 
               "<p>Parsing failed on line: '%s'</p>", inputData.ascii() );
      break;
    }
  }
}


/* Slot, connected to proc's signal processExited().
   Parsing of valgrind's output is finished, successfully or otherwise.
   If the user passed either 'log-file' or 'log-file-exactly' on the
   cmd-line, save the output immediately to whatever they specified.
   log-file == file.pid || log-file-exactly == file.name */
void MemcheckView::saveParsedOutput()
{ 
	is_Edited = true;
	/* un-setup auto-save log stuff */
  logFile.close();
  logStream.unsetDevice();

  /* try for --log-file-exactly first */
  QString save_fname = vkConfig->rdEntry( "log-file-exactly","memcheck" );
  if ( save_fname.isEmpty() ) {
    /* try with --log-file */
    save_fname = vkConfig->rdEntry( "log-file","memcheck" );
    if ( !save_fname.isEmpty() && currentPid != -1 ) {
      /* tack the pid on the end */
			save_fname += "." + QString::number( currentPid );
		}
	}

	/* there's nothing else we can do; let the user decide */
  if ( save_fname.isEmpty() ) {
		return;
	}

	QFileInfo fi( save_fname );
	if ( fi.dirPath() == "." ) {
		/* no filepath given, so save in default dir */
		save_fname = vkConfig->logsDir() + save_fname;
	} else {
		/* found a path: make sure it's the absolute version */
		save_fname = fi.dirPath( true ) + "/" + fi.fileName();
	}
	fi.setFile( save_fname );
	/* if this filename already exists, check if we should over-write it */
	if ( fi.exists() ) {
		int ok = vkQuery( this, 2, "Overwrite File",
											"<p>Over-write existing file '%s' ?</p>", 
											save_fname.ascii() );
		if ( ok == MsgBox::vkNo ) {
			return;
		}
	}

	/* move (rename, actually) the auto-named log-file */
	fi.setFile( logFilename );
	QDir dir( fi.dir() );
	if ( dir.rename( fi.fileName(), save_fname ) ) {
		logFilename = save_fname;
		is_Edited = false;
	} else {
		VK_DEBUG("Failed to rename logfile '%s'", save_fname.ascii() );
	}

}


/* Slot: called from logMenu. Parse and load a single logfile.
	 Setting the open-file-dialog's 'start-with' dir to null causes it
	 to start up in whatever the user's current dir happens to be. */
void MemcheckView::openLogFile()
{ 
	QString log_file;
	log_file = QFileDialog::getOpenFileName( QString::null,
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Select Log File" );
  /* user might have clicked Cancel */
  if ( log_file.isEmpty() )
    return;

	valkyrie->runMode = Valkyrie::modeNotSet;
	if ( validateLogFile( log_file ) ) {
		run();
  }

}


/* Slot: called from savelogButton. Opens a dialog so user can choose
   a filename to save the currently loaded logfile to. */
void MemcheckView::saveLogFile()
{
	QString fname;
  fname = QFileDialog::getSaveFileName( QString::null, 
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Save Log File" );
  if ( fname.isEmpty() ) 
    return;

	printf("TODO: save currently loaded output to file\n" );
}


/* Checks logfile 'log_file' re existence, user perms, xml format.
	 Uses vkObject::checkOptArg(); if checks are passed, sets
	 Valkyrie::RunMode to VIEW_LOG */
bool MemcheckView::validateLogFile( const QString& log_file )
{
	const char* argval = log_file.latin1();
	int errval = valkyrie->checkOptArg( Valkyrie::VIEW_LOG, argval );
	if ( errval != PARSED_OK ) {
		vkError( this, "Invalid File", "%s:\n\"%s\"", 
						 parseErrString(errval), argval );
		return false;
	}

  return true;
}


/* Called from run().  Either --merge=file-list was specified on the
   cmd-line, or has been set via the open-file-dialog.  Either way,
   the value in [valkyrie:merge] is what we need to know */
bool MemcheckView::mergeLogs()
{
	bool ok = false;

  logFilename = vkConfig->rdEntry( "merge", "valkyrie" );
  logFile.setName( logFilename );
  if ( !logFile.open( IO_ReadOnly ) ) {
    vkError( this, "Open File Error", 
             "<p>Unable to open logfile '%s'</p>", logFilename.ascii() );
    return ok;
  }

	QTextStream stream( &logFile );
	QStringList files;
	while ( !stream.atEnd() ) {
		QString tmp = stream.readLine().simplifyWhiteSpace();
		if ( !tmp.isEmpty() ) 
			files << tmp;
	}
	logFile.close();
	/* check there is a minimum of two files-to-merge */
	if ( files.count() < 2 ) {
		vkError( this, "Merge LogFiles Error", 
						 "<p>The minimum number of files required is 2;"
						 " the file '%s' contains only %d.</p>", 
						 logFilename.ascii(), files.count() );
		return ok;
	}

	/* as we iterate through the list, check each file format */
  for ( unsigned int i=0; i<files.count(); i++ ) {
		printf("--> ok = validateLogFile( files[i];\n");
	}

	return ok;
}


/* Slot: called from logMenu.  Open a file which contains a list of
	 logfiles-to-be-merged, each on a separate line, with a minimum of
	 two logfiles. All filechecks are done by mergeLogs(), as we will
	 then merely skip any files which can't be read for some reason. */
void MemcheckView::openMergeFile()
{
	QString merge_file;
	merge_file = QFileDialog::getOpenFileName( QString::null,
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Select Log File" );
  /* user might have clicked Cancel */
  if ( merge_file.isEmpty() )
    return;

	valkyrie->runMode = Valkyrie::modeMergeLogs;
  vkConfig->wrEntry( merge_file, "merge", "valkyrie" );
	run();
}


void MemcheckView::mkMenuBar()
{
  QMenuBar* mcMenu = new QMenuBar( this, "mc_menubar" );
  mcMenu->setStyle( new QMotifStyle() );
  bool show_text = vkConfig->rdBool( "show-butt-text", "valkyrie" );
  int index = -1;

  /* open-all items button --------------------------------------------- */
  index++;
  openAllButton = new QToolButton( this, "tb_open_all" );
  openAllButton->setIconSet( QPixmap( open_all_items_xpm ) );
  openAllButton->setAutoRaise( true );
  openAllButton->setToggleButton( true );
  connect( openAllButton, SIGNAL( toggled(bool) ), 
           this,          SLOT( openAllItems(bool) ) );
  QToolTip::add( openAllButton, 
                 "Open / Close all errors (and their call chains)" );
  mcMenu->insertItem( openAllButton, -1, index );

  /* open-one item button ---------------------------------------------- */
  index++;
  openOneButton = new QToolButton( this, "tb_open_one" );
  openOneButton->setIconSet( QPixmap( open_one_item_xpm ) );
  openOneButton->setAutoRaise( true );
  connect( openOneButton, SIGNAL( clicked() ), 
           this,          SLOT( openOneItem() ) );
  QToolTip::add( openOneButton, 
                 "Open / Close the selected error" );
  mcMenu->insertItem( openOneButton, -1, index );

  /* show src path button ---------------------------------------------- */
  index++;
  srcPathButton = new QToolButton( this, "tb_src_path" );
  srcPathButton->setIconSet( QPixmap( src_path_xpm ) );
  srcPathButton->setAutoRaise( true );
  connect( srcPathButton, SIGNAL( clicked() ), 
           this,          SLOT( showSrcPath() ) );
  QToolTip::add( srcPathButton, 
                 "Show file paths (for current stack)" );
  mcMenu->insertItem( srcPathButton, -1, index );

  /* separator --------------------------------------------------------- */
  index++;
  mcMenu->insertSeparator( index );

  /* open-log(s) button ------------------------------------------------ */
  index++;
  openlogButton = new QToolButton( this, "tb_open_log" );
  openlogButton->setIconSet( QPixmap( open_log_xpm ) );
  openlogButton->setTextLabel( "Log File" );
  openlogButton->setTextPosition( QToolButton::BesideIcon );
  openlogButton->setUsesTextLabel( show_text );
  openlogButton->setAutoRaise( true );
  QPopupMenu* logMenu = new QPopupMenu( openlogButton );
  logMenu->insertItem( "Parse Single",   this, SLOT(openLogFile()) );
  logMenu->insertItem( "Merge Multiple", this, SLOT(openMergeFile()) );
  openlogButton->setPopup( logMenu );
  openlogButton->setPopupDelay( 1 );
  QToolTip::add( openlogButton, "Parse and view log file(s)" );
  mcMenu->insertItem( openlogButton, -1, index );

  /* save-log button --------------------------------------------------- */
  index++;
  savelogButton = new QToolButton( this, "tb_save_log" );
  savelogButton->setIconSet( QPixmap( save_log_xpm ) );
  savelogButton->setTextLabel( "Save Log" );
  savelogButton->setTextPosition( QToolButton::BesideIcon );
  savelogButton->setUsesTextLabel( show_text );
  savelogButton->setAutoRaise( true );
  connect( savelogButton, SIGNAL( clicked() ), 
           this,          SLOT( saveLogFile() ) );
  QToolTip::add( savelogButton, "Save output to a log file" );
  mcMenu->insertItem( savelogButton, -1, index );

  /* suppressions editor button ---------------------------------------- */
  index++;
  suppedButton = new QToolButton( this, "tb_supp_ed" );
  suppedButton->setIconSet( QPixmap( supp_editor_xpm ) );
  suppedButton->setTextLabel( "Supp'n Editor" );
  suppedButton->setTextPosition( QToolButton::BesideIcon );
  suppedButton->setUsesTextLabel( show_text );
  suppedButton->setAutoRaise( true );
  connect( suppedButton, SIGNAL( clicked() ), 
           this,         SLOT( showSuppEditor() ) );
  QToolTip::add( suppedButton, "Open the Suppressions Editor" );
  mcMenu->insertItem( suppedButton, -1, index );
}



/* ---------------------------------------------------------------------
 * All functions below this line are directly related to interacting 
 * with the listView
 * --------------------------------------------------------------------- */

/* Checks if itemType() == SRC.  If true, and item isReadable ||
   isWriteable, launches an editor with the source file loaded */
void MemcheckView::launchEditor( QListViewItem* lv_item, const QPoint&, int )
{
  if ( !lv_item ) return;
  XmlOutputItem* op_item = (XmlOutputItem*)lv_item;
  if ( op_item->itemType() != XmlOutput::SRC ) return;

  /* get the path to the source file */
  if ( op_item->isReadable || op_item->isWriteable ) {
    /* the Frame has the file path */
    OutputItem* frame_item = (OutputItem*)op_item->parent();
    Frame* frame = (Frame*)frame_item->xmlOutput;
    vk_assert( frame != 0 );
    QString fpath = frame->filepath;
    vk_assert( !fpath.isEmpty() );

    /* this will never be empty. if the user hasn't specified a
       preference, use the default. There Can Be Only One. */
    QString editor = vkConfig->rdEntry( "src-editor","valkyrie" );
    QStringList args;
    args << editor;                               /* emacs, nedit, ...*/
    args << "+"+ QString::number(frame->lineno);  /* +43  */
    args << fpath;                                /* /home/../file.c  */

    AsyncProcess::spawn( args, AsyncProcess::SEARCH_PATH );
  }

}


/* Opens all error items plus their children. 
   Ignores the status, preamble, et al. items. */
void MemcheckView::openAllItems( bool state )
{ 
  XmlOutputItem* op_item = (XmlOutputItem*)lView->firstChild()->firstChild();
  if ( !op_item ) return;

  /* move down till we get to the first error */
  while ( op_item && op_item->itemType() != XmlOutput::ERROR ) {
    op_item = op_item->nextSibling();
  }

  QListViewItemIterator it( op_item );
  while ( it.current() ) {
    it.current()->setOpen( state );
    ++it;
  }

}


/* opens/closes the current item. when opening, all its children are
   also set open.  when closing, only the selected item is closed */
void MemcheckView::openOneItem()
{
  QListViewItem* lv_item = lView->currentItem();
  if ( !lv_item ) return;

  if ( lv_item->isOpen() )
    lv_item->setOpen( false );
  else {
    QListViewItem* sib_item = lv_item->nextSibling();
    if ( sib_item == 0 ) {
      /* this item hath no brethren */
      sib_item = lv_item->itemBelow();
    }
    QListViewItemIterator it( lv_item );
    while ( it.current() ) {
      it.current()->setOpen( true );
      ++it;
      if ( it.current() == sib_item ) {
        break;
      }
    }
  }
}
                   

/* shows the file paths in the currently selected error */
void MemcheckView::showSrcPath()
{
  QListViewItem* lv_item = lView->currentItem();
  if ( !lv_item ) return;
  
  XmlOutputItem* op_item = (XmlOutputItem*)lv_item;
  if ( op_item->itemType() == XmlOutput::ERROR )
    ;
  else if ( op_item->itemType() == XmlOutput::STACK )
    op_item = op_item->parent();
  else if ( op_item->itemType() == XmlOutput::FRAME )
    op_item = op_item->parent()->parent();
  else if ( op_item->itemType() == XmlOutput::AUX )
    op_item = op_item->parent();
  else if ( op_item->itemType() == XmlOutput::SRC )
    op_item = op_item->parent()->parent()->parent();
  vk_assert( op_item != 0 );
  vk_assert( op_item->itemType() == XmlOutput::ERROR );

  /* iterate over all the frames in this error,
     stopping when we get to the error below */
  QListViewItem* sib_error = op_item->nextSibling();
  XmlOutputItem* next_item;
  QListViewItemIterator it( op_item );
  while ( it.current() ) {
    next_item = (XmlOutputItem*)it.current();
    if ( next_item->itemType() == XmlOutput::FRAME ) {
      Frame* frame = (Frame*)next_item->xmlOutput;
      if ( !frame->pathPrinted ) {
        frame->printPath();
        next_item->setText( frame->displayString() );
      }
    }
    ++it;
    if ( it.current() == sib_error ) {
      break;
    }
  }

}


void MemcheckView::itemSelected()
{
QListViewItem* lv_item = lView->currentItem();
  if ( !lv_item ) 
    openOneButton->setEnabled( false );
  else {
    XmlOutputItem* op_item = (XmlOutputItem*)lv_item;
    bool state = ( op_item->itemType() == XmlOutput::ERROR || 
                   op_item->itemType() == XmlOutput::STACK ||
                   op_item->itemType() == XmlOutput::FRAME );
    openOneButton->setEnabled( state );

    state = ( state && op_item->isReadable || op_item->isWriteable );
    srcPathButton->setEnabled( state );

    state = !( op_item->itemType() == XmlOutput::PREAMBLE ||
               op_item->itemType() == XmlOutput::STATUS   ||
               op_item->itemType() == XmlOutput::INFO );
    openAllButton->setEnabled( state );
  }
}


/* update the top status item */
void MemcheckView::updateStatus()
{
  OutputItem* top_item = (OutputItem*)lView->firstChild();
  TopStatus* top_status = (TopStatus*)top_item->xmlOutput;
  top_status->print();
  top_item->setText( top_status->displayString() );
}


void MemcheckView::loadItem( XmlOutput * output )
{
  OutputItem* new_item;
  OutputItem* last_child;
  static OutputItem* parent = 0;

  if ( lView->firstChild() == 0 ) {
    new_item = new OutputItem( lView, output );
    new_item->setOpen( true );
    parent = new_item;
  } else {
    last_child = parent->firstChild();
    if ( last_child ) {
      while ( last_child->nextSibling() ) {
        last_child = last_child->nextSibling();
      }
    }
    new_item = new OutputItem( parent, last_child, output );
    new_item->setOpen( false );
  }

  switch ( output->itemType ) {
    case XmlOutput::INFO:
			currentPid = ((Info*)output)->pid;
    case XmlOutput::PREAMBLE:
    case XmlOutput::ERROR:
    case XmlOutput::COUNTS:
      new_item->setExpandable( true );
      break;
    default:
      break;
  }

}


/* Traverse all errors in the listview.  If we find an error:unique in
   the counts->map, update the error's num_times value */
void MemcheckView::updateErrors( Counts * counts )
{
  OutputItem* myChild = (OutputItem*)lView->firstChild()->firstChild();
  while ( myChild ) {
    if ( myChild->itemType() == XmlOutput::ERROR ) {
      Error* error = (Error*)myChild->xmlOutput;
      QString id = error->unique;
      bool ok = counts->contains( id );
      if ( ok ) {   /* found this unique id in the map */
        error->num_times = counts->find( id );
        error->print();
        myChild->setText( error->displayString() );
      }
    }
    myChild = myChild->nextSibling();
  }
}


/* base class for SrcItem and OutputItem ------------------------------- */
XmlOutputItem::XmlOutputItem( QListView * parent ) 
  : QListViewItem( parent ) 
{ initialise(); }

XmlOutputItem::XmlOutputItem( QListViewItem* parent )
  : QListViewItem( parent ) 
{ initialise(); }

XmlOutputItem::XmlOutputItem( QListViewItem* parent, QListViewItem* after )
  : QListViewItem( parent, after ) 
{ initialise(); }

void XmlOutputItem::initialise() 
{
  xmlOutput  = 0;
  isReadable = isWriteable = false;
}

void XmlOutputItem::setText( QString str ) 
{ QListViewItem::setText( 0, str ); }

XmlOutputItem * XmlOutputItem::firstChild() 
{ return (XmlOutputItem*)QListViewItem::firstChild(); }

XmlOutputItem * XmlOutputItem::nextSibling() 
{ return (XmlOutputItem*)QListViewItem::nextSibling(); }

XmlOutputItem * XmlOutputItem::parent()
{ return (XmlOutputItem*)QListViewItem::parent(); }


/* class SrcItem ------------------------------------------------------- */
SrcItem::~SrcItem() {
  if ( pix ) {
    delete pix;
    pix = 0;
  }
}

SrcItem::SrcItem( OutputItem* parent, QString txt )
  : XmlOutputItem( parent ) 
{ 
  pix = 0;
  setMultiLinesEnabled( true );
  setText( txt );
}

void SrcItem::paintCell( QPainter* p, const QColorGroup& cg,
                         int col, int width, int align ) 
{
  QColor bg( 240, 240, 240 );    // very pale gray
  QColorGroup cgrp( cg );        // copy the original
  cgrp.setColor( QColorGroup::Base, bg );
  QListViewItem::paintCell( p, cgrp, col, width, align );
}

void SrcItem::setPixmap( QString pix_file ) 
{
  pix = new QPixmap( vkConfig->imgDir() + pix_file );
  setup();
  widthChanged( 0 );
  invalidateHeight();
  repaint();
}

XmlOutput::ItemType SrcItem::itemType()
{  return XmlOutput::SRC; }

const QPixmap* SrcItem::pixmap( int i ) const 
{ return ( i ) ? 0 : pix; }

void SrcItem::setReadWrite( bool read, bool write )
{
  isReadable  = read;
  isWriteable = write;
  if ( isWriteable )
    setPixmap( "write.xpm" );
  else if ( isReadable )
    setPixmap( "read.xpm" );
}



/* class OutputItem ---------------------------------------------------- */

/* top-level status item */
OutputItem::OutputItem( QListView * parent, XmlOutput * output ) 
  : XmlOutputItem( parent ) 
{
  xmlOutput = output;
  setText( xmlOutput->displayString() );
}

/* used for: preamble:line, info:argv + args, supp counts */
OutputItem::OutputItem( OutputItem* parent, OutputItem* after, QString txt )
  : XmlOutputItem( parent, after ) 
{ setText( txt ); }

/* everybody else */
OutputItem::OutputItem( OutputItem* parent, OutputItem* after, 
                        XmlOutput * output )
  : XmlOutputItem( parent, after ) 
{ 
  xmlOutput  = output;
  setText( xmlOutput->displayString() );
  
  if ( xmlOutput->itemType == XmlOutput::FRAME ) {
    isReadable  = ((Frame*)xmlOutput)->readable;
    isWriteable = ((Frame*)xmlOutput)->writeable;
  }
}

OutputItem * OutputItem::firstChild() 
{ return (OutputItem*)QListViewItem::firstChild(); }

OutputItem * OutputItem::nextSibling() 
{ return (OutputItem*)QListViewItem::nextSibling(); }

OutputItem * OutputItem::parent()
{ return (OutputItem*)QListViewItem::parent(); }


XmlOutput::ItemType OutputItem::itemType()
{
  if ( xmlOutput != 0 ) { 
    return xmlOutput->itemType;
  } else {
    if ( parent() && parent()->itemType() == XmlOutput::ERROR ) {
      return XmlOutput::AUX;
    } else {
      return XmlOutput::INFO;
    }
  }
}

void OutputItem::paintCell( QPainter* p, const QColorGroup& cg,
                            int col, int width, int align ) 
{
  if ( ! (isReadable || isWriteable ) ) {
    QListViewItem::paintCell( p, cg, col, width, align );
  } else {
    QColorGroup cgrp( cg );
    cgrp.setColor( QColorGroup::Text, Qt::blue );
    QListViewItem::paintCell( p, cgrp, col, width, align );
  }
}

void OutputItem::setOpen( bool open ) 
{
  if ( !xmlOutput ) {
    QListViewItem::setOpen( open );
    return;
  }

  if ( open && !childCount() ) {
    listView()->setUpdatesEnabled( false );

    switch ( xmlOutput->itemType ) {

      case XmlOutput::INFO: {
        OutputItem* after = this;
        OutputItem* argv_item = new OutputItem( this, after, "argv" );
        Info* info = (Info*)xmlOutput;
        for ( uint i=0; i<info->infoList.count(); i++ ) {
          OutputItem*item = new OutputItem( argv_item, after,
                                            info->infoList[i] );
          after = item;
        }
        argv_item->setOpen( true );
      } break;

      /* 'j is the biz' stuff */
      case XmlOutput::PREAMBLE: {
        Preamble* preamble = (Preamble*)xmlOutput;
        OutputItem* child_item = 0;
        for ( uint i=0; i<preamble->lines.count(); i++ ) {
          child_item = new OutputItem(this, child_item, preamble->lines[i]);
        } 
      } break;

      case XmlOutput::ERROR: {
        OutputItem* after = this;
        Error* error = (Error*)xmlOutput;
        Stack* stack1 = error->stack1;
        OutputItem* stack_item1 = new OutputItem( this, after, stack1 );
        stack_item1->setText( "stack" );
        after = stack_item1;
        if ( !error->auxwhat.isEmpty() ) {
          OutputItem* aux_item = new OutputItem( this, after,
                                                 error->auxwhat );
          after = aux_item;
        }
        Stack* stack2 = error->stack2;
        if ( stack2 != 0 ) {
          OutputItem* stack_item2 = new OutputItem( this, after, stack2 );
          stack_item2->setText( "stack" );
          stack_item2->setExpandable( true );
        }
        /* populate the first stack, and set it open by default */
        after = stack_item1;
        OutputItem* frame_item;
        Frame* frame;
        for ( frame=stack1->frameList.first(); 
              frame; frame=stack1->frameList.next() ) {
          frame_item = new OutputItem( stack_item1, after, frame );
          frame_item->setText( frame->displayString() );
          frame_item->setExpandable( frame->readable || frame->writeable );
          after = frame_item;
        }
        stack_item1->setOpen( true );
      } break;

      case XmlOutput::STACK: {
        OutputItem* after = this;
        Stack* stack2 = (Stack*)xmlOutput;
        OutputItem* frame_item;
        Frame* frame;
        for ( frame=stack2->frameList.first(); 
              frame; frame=stack2->frameList.next() ) {
          frame_item = new OutputItem( this, after, frame );
          frame_item->setText( frame->displayString() );
          frame_item->setExpandable( frame->readable || frame->writeable );
          after = frame_item;
        }
      } break;

      case XmlOutput::FRAME: {
        if ( !isReadable ) 
          break;

        Frame* frame = (Frame*)xmlOutput;
        int target_line = frame->lineno;
        /* num lines to show above / below the target line */
        int extra_lines = vkConfig->rdInt( "src-lines", "valkyrie" );
        /* figure out where to start showing src lines */
        int top_line = 1;
        if ( target_line > extra_lines+1 )
          top_line = target_line - extra_lines;
        int bot_line = target_line + extra_lines;
        int current_line = 1;

        QFile srcfile( frame->filepath );
        if ( !srcfile.open( IO_ReadOnly ) )
          return;

        QString aline;
        QStringList src_lines;
        QTextStream stream( &srcfile );
        while ( !stream.atEnd() && ( current_line <= bot_line ) ) {
          aline = stream.readLine();
          if ( current_line >= top_line && current_line <= bot_line ) {
            /* replace all tabs with 2 spaces to look pretty */
            aline = aline.replace( '\t', "  ", false );
            src_lines << aline;
          }
          current_line++;
        }
        srcfile.close();
        aline = src_lines.join( "\n" );

        /* create the item for the src lines */
        SrcItem* src_item = new SrcItem( this, aline );
        src_item->setReadWrite(frame->readable, frame->writeable );
        src_item->setOpen( true );
      } break;


      /* suppression count */
      case XmlOutput::COUNTS: {
        Counts* counts = (Counts*)xmlOutput;
        OutputItem* child_item = 0;
        for ( uint i=0; i<counts->supps.count(); i++ ) {
          child_item = new OutputItem(this, child_item, counts->supps[i]);
        }
      } break;
 
      default: 
        break;
    }
    
    listView()->setUpdatesEnabled( true );
  }

  QListViewItem::setOpen( open );
}
