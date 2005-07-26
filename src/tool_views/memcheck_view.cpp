/* ---------------------------------------------------------------------
 * Implementation of MemcheckView                      memcheck_view.cpp
 * Memcheck's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "memcheck_view.h"
#include "memcheck_object.h"

#include "vk_config.h"
#include "memcheck_icons.h"
#include "async_process.h"
#include "vk_messages.h"
#include "vk_file_utils.h"
#include "context_help.h"
#include "html_urls.h"

#include <qcursor.h>
#include <qfiledialog.h>
#include <qheader.h>
#include <qtoolbar.h>
#include <qmotifstyle.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtabwidget.h>


void MemcheckView::showSuppEditor()
{ printf("TODO: Memcheck::showSuppEditor()\n"); }



/* class MemcheckView -------------------------------------------------- */
MemcheckView::~MemcheckView() { }


MemcheckView::MemcheckView( QWidget* parent, Memcheck* mc )
  : ToolView( parent, mc )
{
  mkToolBar();

  QVBoxLayout* vLayout = new QVBoxLayout( central, 0, -1, "vLayout" );
  vLayout->setResizeMode( QLayout::FreeResize );

  /* create a tabwidget */
  QTabWidget* tabwidget = new QTabWidget( central, "mc_tabwidget" );
  vLayout->addWidget( tabwidget );
  /* some tweaks to prettify the tabwidget */
  tabwidget->setTabPosition( QTabWidget::Bottom  );
  QFont fnt = tabwidget->font();
  fnt.setPointSize( fnt.pointSize() - 2 );
  tabwidget->setFont( fnt );

  /* first tab: the listview */
  lView = new QListView( tabwidget, "lview" );
  tabwidget->addTab( lView, "valgrind output" );
  lView->setShowToolTips( false );
  lView->setSorting( -1 );
  lView->setMargin( 5 );
  lView->addColumn( "" );
  lView->header()->setStretchEnabled( true, 0 );
  lView->header()->hide();
  QFont lview_fnt( "Adobe Courier", 9, QFont::Normal, false );
  lview_fnt.setStyleHint( QFont::TypeWriter );
  lView->setFont( lview_fnt );

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


  QFont clientout_fnt( "Adobe Courier", 9, QFont::Normal, false );
  clientout_fnt.setStyleHint( QFont::TypeWriter );

  /* second tab: stdout */
  stdoutTedit = new QTextEdit( tabwidget, "stdout_tedit" );
  tabwidget->addTab( stdoutTedit, "client stdout" );
  stdoutTedit->setMargin( 5 );
  stdoutTedit->setFont( clientout_fnt );
  //setTextFormat(plain|rich|LogText(=lotsoftext)|auto)
  stdoutTedit->setReadOnly( true );
  stdoutTedit->setText( "" );

  /* third tab: stderr */
  stderrTedit = new QTextEdit(tabwidget, "stderr_tedit");
  tabwidget->addTab( stderrTedit, "client stderr" );
  stderrTedit->setMargin( 5 );
  stderrTedit->setFont( clientout_fnt );
  //setTextFormat(plain|rich|LogText(=lotsoftext)|auto)
  stderrTedit->setReadOnly( true );
  stderrTedit->setText( "" );
}


/* clear and reset the listview for a new run */
void MemcheckView::clear()
{
  lView->clear();
  stdoutTedit->clear();
  stderrTedit->clear();
}


/* called by memcheck: set state for buttons; set cursor state */
void MemcheckView::setState( bool run )
{
  openlogButton->setEnabled( !run );
  // TODO: suppedButton->setEnabled( !run );
  if ( run ) {       /* startup */
    savelogButton->setEnabled( false );
    setCursor( QCursor(Qt::WaitCursor) );
  } else {           /* finished */
    unsetCursor();
    savelogButton->setEnabled( lView->childCount() != 0 );
  }
}


/* slot: connected to MainWindow::toggleToolbarLabels(). 
   called when user toggles 'show-butt-text' in Options page */
void MemcheckView::toggleToolbarLabels( bool state )
{
  openlogButton->setUsesTextLabel( state );
  savelogButton->setUsesTextLabel( state );
  suppedButton->setUsesTextLabel( state );
}


/* slot: called from logMenu. parse and load a single logfile.
   setting the open-file-dialog's 'start-with' dir to null causes it
   to start up in whatever the user's current dir happens to be. */
void MemcheckView::openLogFile()
{ 
#if 0
  /* testing new file dialog stuff */
  QString fname = "";  
  FileDialog* fd = new FileDialog( this, "log_file_fd" );
  fd->exec();
#else
  QString log_file;

  QFileDialog dlg;
  dlg.setShowHiddenFiles( true );
  
  log_file = dlg.getOpenFileName( QString::null,
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Select Log File" );
  /* user might have clicked Cancel */
  if ( log_file.isEmpty() )
    return;

  vkConfig->wrEntry( log_file, "view-log", "valkyrie" );
  tool()->parseLogFile( false );
#endif
}


/* slot: called from logMenu.  Open a file which contains a list of
   logfiles-to-be-merged, each on a separate line, with a minimum of
   two logfiles. */
void MemcheckView::openMergeFile()
{
  QString merge_file;
  merge_file = QFileDialog::getOpenFileName( QString::null,
                "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                this, "fdlg", "Select Log File" );
  /* user might have clicked Cancel */
  if ( merge_file.isEmpty() )
    return;

  vkConfig->wrEntry( merge_file, "merge", "valkyrie" );
  /* returns the filename the merge has been saved to */
  tool()->mergeLogFiles();
}


/* slot: called from savelogButton. Opens a dialog so user can choose
   a (different) filename to save the currently loaded logfile to. */
void MemcheckView::saveLogFile()
{
  logFilename = vkConfig->rdEntry( "view-log", "valkyrie" );
  QFileInfo fi( logFilename );
  QString fname = QFileDialog::getSaveFileName( fi.dirPath(), 
                  "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)", 
                  this, "fdlg", "Save Log File As" );

  tool()->saveParsedOutput( fname );
}


void MemcheckView::mkToolBar()
{
  mcToolBar = new QToolBar( "Memcheck ToolBar", this,
                            DockTop, false, "mc_toolbar" );

  mcToolBar->setStyle( new QMotifStyle() );
  bool show_text = vkConfig->rdBool( "show-butt-text", "valkyrie" );

  /* open-all items button --------------------------------------------- */
  openAllButton = new QToolButton( mcToolBar, "tb_open_all" );
  openAllButton->setIconSet( QPixmap( open_all_items_xpm ) );
  openAllButton->setAutoRaise( true );
  openAllButton->setToggleButton( true );
  connect( openAllButton, SIGNAL( toggled(bool) ), 
           this,          SLOT( openAllItems(bool) ) );
  QToolTip::add( openAllButton, 
                 "Open / Close all errors (and their call chains)" );
  ContextHelp::add( openAllButton, urlValkyrie::openAllButton );

  /* open-one item button ---------------------------------------------- */
  openOneButton = new QToolButton( mcToolBar, "tb_open_one" );
  openOneButton->setIconSet( QPixmap( open_one_item_xpm ) );
  openOneButton->setAutoRaise( true );
  connect( openOneButton, SIGNAL( clicked() ), 
           this,          SLOT( openOneItem() ) );
  QToolTip::add( openOneButton, 
                 "Open / Close the selected item" );
  ContextHelp::add( openOneButton, urlValkyrie::openOneButton );

  /* show src path button ---------------------------------------------- */
  srcPathButton = new QToolButton( mcToolBar, "tb_src_path" );
  srcPathButton->setIconSet( QPixmap( src_path_xpm ) );
  srcPathButton->setAutoRaise( true );
  connect( srcPathButton, SIGNAL( clicked() ), 
           this,          SLOT( showSrcPath() ) );
  QToolTip::add( srcPathButton, 
                 "Show file paths (for current frame)" );
  ContextHelp::add( srcPathButton, urlValkyrie::srcPathButton );

  /* fake motif-style separator ---------------------------------------- */
  QLabel* sep_lbl = new QLabel( mcToolBar, "lbl_sep" );
  mcToolBar->setStretchableWidget( sep_lbl );

  /* open-log(s) button ------------------------------------------------ */
  openlogButton = new QToolButton( mcToolBar, "tb_open_log" );
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
  ContextHelp::add( openlogButton, urlValkyrie::openLogButton );

  /* save-log button --------------------------------------------------- */
  savelogButton = new QToolButton( mcToolBar, "tb_save_log" );
  savelogButton->setIconSet( QPixmap( save_log_xpm ) );
  savelogButton->setTextLabel( "Save Log" );
  savelogButton->setTextPosition( QToolButton::BesideIcon );
  savelogButton->setUsesTextLabel( show_text );
  savelogButton->setAutoRaise( true );
  connect( savelogButton, SIGNAL( clicked() ), 
           this,          SLOT( saveLogFile() ) );
  QToolTip::add( savelogButton, "Save output to a log file" );
  ContextHelp::add( savelogButton, urlValkyrie::saveLogButton );

  /* suppressions editor button ---------------------------------------- */
  suppedButton = new QToolButton( mcToolBar, "tb_supp_ed" );
  suppedButton->setIconSet( QPixmap( supp_editor_xpm ) );
  suppedButton->setTextLabel( "Supp'n Editor" );
  suppedButton->setTextPosition( QToolButton::BesideIcon );
  suppedButton->setUsesTextLabel( show_text );
  suppedButton->setAutoRaise( true );
  connect( suppedButton, SIGNAL( clicked() ), 
           this,         SLOT( showSuppEditor() ) );
  QToolTip::add( suppedButton, "Open the Suppressions Editor" );
  ContextHelp::add( suppedButton, urlValkyrie::suppEdButton );
  suppedButton->setEnabled( false );
  // TODO: implement suppressionsEditor
}



/* ---------------------------------------------------------------------
 * All functions below are related to interacting with the listView
 * --------------------------------------------------------------------- */

/* checks if itemType() == SRC.  if true, and item isReadable or
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


/* opens all error items plus their children. 
   ignores the status, preamble, et al. items. */
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
  vk_assert( op_item->itemType() == XmlOutput::ERROR ||
             op_item->itemType() == XmlOutput::LEAK_ERROR );

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
    bool state = ( op_item->itemType() == XmlOutput::ERROR      || 
                   op_item->itemType() == XmlOutput::LEAK_ERROR ||
                   op_item->itemType() == XmlOutput::STACK      ||
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
  top_status->printDisplay();
  top_item->setText( top_status->displayString() );
}


void MemcheckView::loadItem( XmlOutput * output )
{
  vk_assert( output );

  /* xmlParser emits loadItem() for ErrCounts: used when merging
     to file, but not when displaying stuff in the listview. */
  if ( output->itemType == XmlOutput::ERR_COUNTS )
    return;

  OutputItem* new_item;
  OutputItem* last_child;
  static OutputItem* parent = 0;

  /* ensure we've got something to display */
  switch ( output->itemType ) {
    case XmlOutput::INFO:
    case XmlOutput::STATUS:
    case XmlOutput::SUPP_COUNTS:
    case XmlOutput::ERROR:
    case XmlOutput::LEAK_ERROR:
    case XmlOutput::FRAME:
      output->printDisplay();
      break;
    default: break;
  }

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

  /* grab current pid while the going is good (used for creating
     <filename>.pid<pid> when required); set relevant items expandable */
  switch ( output->itemType ) {
    case XmlOutput::INFO:
    case XmlOutput::PREAMBLE:
    case XmlOutput::ERROR:
    case XmlOutput::LEAK_ERROR:
    case XmlOutput::SUPP_COUNTS:
      new_item->setExpandable( true );
      break;
    default: break;
  }

}


/* iterate over all errors in the listview, looking for a match on
   error->unique with ecounts->pairList->unique.  if we find a match,
   update the error's num_times value */
void MemcheckView::updateErrors( ErrCounts * ecounts )
{
  OutputItem* myChild = (OutputItem*)lView->firstChild()->firstChild();
  while ( myChild ) {
    if ( myChild->itemType() == XmlOutput::ERROR ) {
      Error* error = (Error*)myChild->xmlOutput;
      int num = ecounts->findUnique( error->unique );
      if ( num != -1 ) {  /* found this unique id in the list */
        error->num_times = num;
        error->printDisplay();
        myChild->setText( error->displayString() );
      }
    }
    myChild = myChild->nextSibling();
  }
}


void MemcheckView::loadClientOutput( const QString& client_output, int log_fd )
{
  if (log_fd == 1) {
    stdoutTedit->append( client_output );  
  } else if (log_fd == 2) {
    stderrTedit->append( client_output );  
  }
}


/* base class for SrcItem and OutputItem ------------------------------- */
XmlOutputItem::XmlOutputItem( QListView* parent ) 
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

void SrcItem::setPixmap( const char* pix_xpm[] ) 
{
  pix = new QPixmap( pix_xpm );
  setup();
  widthChanged( 0 );
  invalidateHeight();
  repaint();
}

XmlOutput::ItemType SrcItem::itemType()
{ return XmlOutput::SRC; }

const QPixmap* SrcItem::pixmap( int i ) const 
{ return ( i ) ? 0 : pix; }

void SrcItem::setReadWrite( bool read, bool write )
{
  isReadable  = read;
  isWriteable = write;
  if ( isWriteable ) {
    setPixmap( write_xpm );
  } else if ( isReadable ) {
    setPixmap( read_xpm );
  }
}



/* class OutputItem ---------------------------------------------------- */

/* top-level status item */
OutputItem::OutputItem( QListView* parent, XmlOutput* output ) 
  : XmlOutputItem( parent ) 
{
  xmlOutput = output;
  setMultiLinesEnabled( true );
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
        Info* info = (Info*)xmlOutput;
        OutputItem* after = this;
        OutputItem* item;
        /* may / may not have log-file-qualifier */
        if ( ! info->logQualList.isEmpty() ) {
          OutputItem* qual_item = new OutputItem(this, after, "logfilequalifier");
          for ( unsigned int i=0; i<info->logQualList.count(); i++ ) {
            item = new OutputItem( qual_item, after, info->logQualList[i] );
            after = item;
          }
          qual_item->setOpen( true );
          after = qual_item;
        }
        /* may / may not have a user comment */
        if ( ! info->userComment.isEmpty() ) {
          OutputItem* comment_item = new OutputItem( this, after, "usercomment" );
          item = new OutputItem( comment_item, after, info->userComment );
          comment_item->setOpen( true );
          after = comment_item;
        }
        /* args and friends */
        OutputItem* args_item = new OutputItem( this, after, "args" );
        for ( unsigned int i=0; i<info->vgInfoList.count(); i++ ) {
          item = new OutputItem( args_item, after, info->vgInfoList[i] );
          after = item;
        }
        for ( unsigned int i=0; i<info->exInfoList.count(); i++ ) {
          item = new OutputItem( args_item, after, info->exInfoList[i] );
          after = item;
        }
        args_item->setOpen( true );
      } break;

      /* 'J is the biz' stuff */
      case XmlOutput::PREAMBLE: {
        Preamble* preamble = (Preamble*)xmlOutput;
        OutputItem* child_item = 0;
        for ( uint i=0; i<preamble->lines.count(); i++ ) {
          child_item = new OutputItem(this, child_item, preamble->lines[i]);
        } 
      } break;

      case XmlOutput::ERROR:
      case XmlOutput::LEAK_ERROR: {
        OutputItem* after = this;
        Error* error = (Error*)xmlOutput;
        Stack* stack1 = error->stackList.first();
        OutputItem* stack_item1 = new OutputItem( this, after, stack1 );
        stack_item1->setText( "stack" );
        after = stack_item1;
        if ( error->haveAux ) {
          OutputItem* aux_item = new OutputItem( this, after,
                                                 error->auxwhat );
          after = aux_item;
        }
        Stack* stack2 = error->stackList.next();
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
          frame->printDisplay();
          frame_item = new OutputItem( stack_item1, after, frame );
          frame_item->setText( frame->displayString() );
          frame_item->setExpandable( frame->readable || frame->writeable );
          after = frame_item;
        }
        stack_item1->setOpen( true );
        /* J sez there may be more than two stacks in the future .. */
        vk_assert( error->stackList.count() <= 2 );
      } break;

      case XmlOutput::STACK: {
        OutputItem* after = this;
        Stack* stack2 = (Stack*)xmlOutput;
        OutputItem* frame_item;
        Frame* frame;
        for ( frame=stack2->frameList.first(); 
              frame; frame=stack2->frameList.next() ) {
          frame->printDisplay();
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
            /* add a couple of spaces to look pretty */
            src_lines << "  " + aline;
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
      case XmlOutput::SUPP_COUNTS: {
        SuppCounts* scounts = (SuppCounts*)xmlOutput;
        OutputItem* child_item = 0;
        for ( uint i=0; i<scounts->supps.count(); i++ ) {
          child_item = new OutputItem(this, child_item, scounts->supps[i]);
        }
      } break;
 
      default: 
        break;
    }
    
    listView()->setUpdatesEnabled( true );
  }

  QListViewItem::setOpen( open );
}
