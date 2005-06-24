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
  xmlParser    = 0;
  valkyrie     = (Valkyrie*)vkConfig->vkObject( "valkyrie" );
  inputFormat  = NOT_SET;

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

  savelogButton->setEnabled( false );
  openOneButton->setEnabled( false );
  openAllButton->setEnabled( false );
  srcPathButton->setEnabled( false );

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
{ printf("TODO: MemcheckView::stop()\n"); }


void MemcheckView::setRunning( bool run )
{
	if ( run ) {    // startup
		is_Running = true;
		emit running( is_Running );
		setCursor( QCursor(Qt::WaitCursor) );
		openlogButton->setEnabled( false );
		savelogButton->setEnabled( false );
	} else {        // finished
		is_Running = false;
		emit running( is_Running ); 
		emit message( "Loaded: " +logFilename );
		savelogButton->setEnabled( is_Edited );
		openlogButton->setEnabled( true );
		unsetCursor();
	}
}


/* Choices of what we are supposed to be doing are:
   1. if Valkyrie::RunMode == PARSE_XXX_LOG
      - user specified a log-file on the cmd-line: constructor calls run()
      - user opened a log-file: openLogfile() calls run()
      - user wants to view the same log-file again: clicks runButton

   2. if Valkyrie::RunMode == PARSE_TEXT_OUTPUT
      - run valgrind with all args, but --xml=no
      - run valgrind with all args, but --xml=yes
*/
bool MemcheckView::run()
{
  /* already doing stuff */
  if ( is_Running ) {
    vkInfo( this, "Memcheck Run", 
            "<p>Currently engaged. Try again later.</p>" );
    return is_Running;
  }

  inputData = "";

  switch ( valkyrie->runMode ) {

    case Valkyrie::NOT_SET: {
      /* ?? no flags were set on the cmd-line, so just do what we were
         doing the last time user clicked run() */
      printf("TODO: void Memcheck::run() case Valkyrie::NOT_SET\n");
      vk_assert_never_reached();
    } break;


    case Valkyrie::PARSE_LOG: {

      switch ( inputFormat ) {
        case INVALID:
          return false;
        case NOT_SET:
          inputFormat = validateLogFile();
          run();
          break;
        case TEXT:
          vkInfo( this, "Coming Soon ...",
                  "<p>Parsing/viewing of valgrind text logs "
                  "is not yet implemented.</p>" );
          break;
        case XML:
          //is_Running = true;
          //emit running( is_Running );
					//setCursor( QCursor(Qt::WaitCursor) );
					//openlogButton->setEnabled( false );
					//savelogButton->setEnabled( false );
          /* create the default handler if necessary */
          if ( xmlParser == 0 ) {
            xmlParser = new XMLParser( this );
            reader.setContentHandler( xmlParser );
          }
          xmlParser->reset();
          /* reset and clear the input source */
          source.reset();
          parseXmlLog();
          break;
      }

    } break;

    case Valkyrie::PARSE_OUTPUT: {
      
      switch ( inputFormat ) {
        case INVALID:
          return false;
        case NOT_SET:
          inputFormat = validateOutput();
          run();
          break;
        case TEXT:
          vkInfo( this, "Coming Soon ...",
                  "<p>Parsing/viewing of valgrind text output "
                  "is not yet implemented.</p>" );
          break;
        case XML:
          //is_Running = true;
          //emit running( is_Running );
					//setCursor( QCursor(Qt::WaitCursor) );
					//openlogButton->setEnabled( false );
					//savelogButton->setEnabled( false );
					/* setup auto-save valgrind's xml output to file */
					logFilename = vk_mkstemp( "output-log", vkConfig->logsDir() );
					logFile.setName( logFilename );
					if ( logFile.open( IO_WriteOnly ) ) { 
						logStream.setDevice( &logFile );
					} else {
						VK_DEBUG("failed to open logfile '%s' for writing", 
										 logFilename.ascii() );
						return false;
					}
          /* create the default handler if necessary */
          if ( xmlParser == 0 ) {
            xmlParser = new XMLParser( this );
            reader.setContentHandler( xmlParser );
            reader.setErrorHandler( xmlParser );
          }
          /* reset, clear and initialise the input source */
          xmlParser->reset();
          source.reset();
          source.setData( inputData );
          /* tell the parser we are parsing incrementally */
          reader.parse( &source, true );
          /* fork a new process in non-blocking mode */
          if ( proc == 0 ) {
            proc = new QProcess( this );
          }
          /* stuff the cmd-line flags || non-default options into the
             process */
          proc->setArguments( flags );
          /* connect the pipe to this toolview */
          connect( proc, SIGNAL(readyReadStderr()),
                   this, SLOT(parseXmlOutput()) );
          /* tell MainWin when valgrind has exited */
          connect( proc, SIGNAL(processExited()),
                   this, SLOT(processExited()) );
          /* set state for MainWin's run, restart, stop buttons */
          setRunning( proc->start() );
          break;
      }

    } break;   /* end case Valkyrie::PARSE_OUTPUT: */

  }            /* end switch ( valkyrie->runMode ) */

  return is_Running;
}


/* get a log-file to open via the QFileDialog::getOpenFileName() */
void MemcheckView::openLogfile()
{ 
  QString logfile;
  logfile = QFileDialog::getOpenFileName( vkConfig->logsDir(), 
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Select Log File" );
  /* user might have clicked Cancel */
  if ( logfile.isEmpty() )
    return;

  vkConfig->wrEntry( logfile, "view-log", "valkyrie" );
  inputFormat = validateLogFile();
  switch ( inputFormat ) {
    case NOT_SET:
    case INVALID:
      vkConfig->wrEntry( "", "view-log", "valkyrie" );
      break;
    case TEXT:
    case XML:
      vkConfig->wrEntry( logfile, "view-log", "valkyrie" );
      valkyrie->runMode = Valkyrie::PARSE_LOG;
      run();
      break;
  }

}


/* get a log-file name via the QFileDialog::getSaveFileName() */
void MemcheckView::getSaveFilename()
{
  QString fname;
  fname = QFileDialog::getSaveFileName( QString::null, 
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Save Log File" );
  if ( fname.isEmpty() ) 
    return;
  else
    saveLogfile( fname );
}


void MemcheckView::saveLogfile( QString /*fname*/ )
{
  printf("TODO: how to save output to logfile in xml format\n");

  //QFile outF( fname );
  //if ( outF.open( IO_WriteOnly ) ) { 
  //  QTextStream aStream( &outF );
  //  aStream << header;
      // ... ... 
  //  outF.close();
  //  emit message("Logfile saved to: " + fname );
  //  savelogButton->setEnabled( false );
  //}
}


/* default is to have --xml=yes, but allow user to override this on
   the cmd-line.  so just check the flags passed on the cmd-line */
MemcheckView::ParseFormat MemcheckView::validateOutput()
{
  bool xml = vkConfig->rdBool( "xml", "memcheck" );
  ParseFormat format = ( xml ) ? XML : TEXT;

  return format;
}


/* Checks the log-file currently in vkConfig re existence, 
   user perms, etc.
   Also tries to figure out the type of log-file { TEXT | XML } 
   If the file doesn't comply, deletes the vkConfig entry. */
MemcheckView::ParseFormat MemcheckView::validateLogFile()
{
  ParseFormat format = INVALID;

  QString logfile = vkConfig->rdEntry( "view-log", "valkyrie" );
  QFile file( logfile );
  QFileInfo fi( file );
  if ( ! (fi.exists() && fi.isFile() && 
          !fi.isSymLink() && fi.isReadable()) ) {
    vkError( this, "Error", "Invalid file '%s'", logfile.ascii() );
    return format;
  }

  if ( !file.open( IO_ReadOnly ) ) {
    vkError( this, "Error",
             "Failed to open file '%s'", logfile.ascii() );
    return format;
  }

  /* find out what type of input file we are looking at by reading a
     few lines of text */
  format = TEXT;    /* let's be on the safe side */
  QTextStream stream( &file );
  QString aline;
  int pos;
  int n = 0;
  while ( !stream.atEnd() && n < 10 ) {
    aline = stream.readLine().simplifyWhiteSpace();
    if ( !aline.isEmpty() ) {      /* found something */
      pos = aline.find( "<valgrindoutput>", 0, false );
      if ( pos != -1 ) {
        format = XML;
        break;
      }
    }
    n++;
  }
  file.close();

  return format;
}


/* Tell MainWin we are done.
   If user passed either 'log-file' or 'log-file-exactly' on the
   cmd-line, save the output immediately to whatever they
   specified.
   log-file == file.pid || log-file-exactly == file.name */
void MemcheckView::processExited()
{ 
	is_Edited = true;
  //is_Running = false;
  //emit running( is_Running ); 

	/* un-setup auto-save log stuff */
  logFile.close();
	logStream.unsetDevice();

	/* try for --log-file-exactly first */
	QString cmd_fname = vkConfig->rdEntry( "log-file-exactly","memcheck" );
	if ( cmd_fname.isEmpty() ) {    
		/* try with --log-file */
		cmd_fname = vkConfig->rdEntry( "log-file","memcheck" );
		if ( !cmd_fname.isEmpty() ) {
			/* tack the pid on the end */
			OutputItem* myChild = (OutputItem*)lView->firstChild()->firstChild();
			if ( myChild ) {
				Info* info = (Info*)myChild->xmlOutput;
				cmd_fname += "." + QString::number( info->pid );
			}
		}
	}

	if ( !cmd_fname.isEmpty() ) {

		QFileInfo fi( cmd_fname );
		if ( fi.dirPath() == "." ) {
			/* no filepath given, so save in default dir */
			cmd_fname = vkConfig->logsDir() + cmd_fname;
		} else {
			/* found a path: make sure it's the absolute version */
			cmd_fname = fi.dirPath( true ) + "/" + fi.fileName();
		}
		fi.setFile( cmd_fname );
		/* if this filename already exists, check if we should over-write it */
		if ( fi.exists() ) {
			int ok = vkQuery( this, 2, "Overwrite File",
							 "<p>Over-write existing logfile '%s' ?</p>", 
												cmd_fname.ascii() );
			if ( ok == MsgBox::vkNo ) {
				goto done;
			}
		}
		/* move (rename, actually) the auto-named log-file */
		fi.setFile( logFilename );
		QDir dir( fi.dir() );
		if ( dir.rename( fi.fileName(), cmd_fname ) ) {
			logFilename = cmd_fname;
			is_Edited = false;
		} else {
			VK_DEBUG("Failed to rename logfile '%s'", cmd_fname.ascii() );
		}
	}

 done:
	//emit message( "Saved: " +logFilename );
	//savelogButton->setEnabled( is_Edited );
	//openlogButton->setEnabled( true );
  //unsetCursor();
	setRunning( false );
}


/* Read and process the data - which might be output in chunks.  
	 We auto-save the xmll output to a logfile in
	 ~/.valkie-X.X.X/logs/files/ */
void MemcheckView::parseXmlOutput()
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


void MemcheckView::parseXmlLog()
{
  QString logfile = vkConfig->rdEntry( "view-log", "valkyrie" );
  QFile xmlFile( logfile );
  if ( !xmlFile.open( IO_ReadOnly ) ) {
    vkError( this, "Open File Error", 
             "<p>Unable to open xml log '%s'</p>", logfile.ascii() );
    return;
  }

	setRunning( true );

  QTextStream stream( &xmlFile );
  stream.setEncoding( QTextStream::UnicodeUTF8 );
  inputData = stream.readLine();
  source.setData( inputData );
  bool ok = reader.parse( &source, true );

  while ( !stream.atEnd() ) {
    inputData = stream.readLine();
    source.setData( inputData );
    ok = reader.parseContinue();
    if ( !ok ) {
      vkError( this, "Parse Error", 
               "<p>Parsing failed on line: '%s'</p>", inputData.ascii() );
      break;
    }

  }

	//emit message( "Loaded: " +logFilename );
	//savelogButton->setEnabled( false );
	//openlogButton->setEnabled( true );
  //unsetCursor();
	setRunning( false );
}


/* checks if itemType() == SRC.  If true, and item isReadable ||
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
    QString editor = vkConfig->rdEntry( "editor","Prefs" );
    QStringList args;
    args << editor;                               /* emacs, nedit, ...*/
    args << "+"+ QString::number(frame->lineno);  /* +43  */
    args << fpath;                                /* /home/../file.c  */

    AsyncProcess::spawn( args, AsyncProcess::SEARCH_PATH );
  }

}


/* opens all error items plus their children. 
   ignores the status item, preamble et al. items. */
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
    case XmlOutput::PREAMBLE:
    case XmlOutput::INFO:
    case XmlOutput::ERROR:
    case XmlOutput::COUNTS:
      new_item->setExpandable( true );
      break;
    default:
      break;
  }

}


/* Traverse all errors in the listview.  If we find a error:unique in
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


void MemcheckView::mkMenuBar()
{
  QMenuBar* mcMenu = new QMenuBar( this, "mc_menubar" );
  mcMenu->setStyle( new QMotifStyle() );
  bool show_text = vkConfig->rdBool( "show-butt-text", "Prefs" );
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

  /* open-log button --------------------------------------------------- */
  index++;
  openlogButton = new QToolButton( this, "tb_open_log" );
  openlogButton->setIconSet( QPixmap( open_log_xpm ) );
  openlogButton->setTextLabel( "Open Log" );
  openlogButton->setTextPosition( QToolButton::BesideIcon );
  openlogButton->setUsesTextLabel( show_text );
  openlogButton->setAutoRaise( true );
  connect( openlogButton, SIGNAL( clicked() ), 
           this,          SLOT( openLogfile() ) );
  QToolTip::add( openlogButton, "Open and load a log file" );
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
           this,          SLOT( getSaveFilename() ) );
  QToolTip::add( savelogButton, "Save output to a log file" );
  mcMenu->insertItem( savelogButton, -1, index );

  /* suppressions editor button ---------------------------------------- */
  index++;
  QToolButton* suppedButton = new QToolButton( this, "tb_supp_ed" );
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
        int extra_lines = vkConfig->rdInt( "extra-src-lines", "Prefs" );
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
