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

#include "vk_config.h"
#include "memcheck_icons.h"
#include "async_process.h"
#include "vk_messages.h"
#include "vk_file_utils.h"
#include "context_help.h"
#include "html_urls.h"
#include "tool_object.h"       // VkRunState
#include "vk_utils.h"

#include <qcursor.h>
#include <qfiledialog.h>
#include <qheader.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtabwidget.h>


void MemcheckView::showSuppEditor()
{ vkPrint("TODO: Memcheck::showSuppEditor()"); }



/* class MemcheckView -------------------------------------------------- */
MemcheckView::~MemcheckView()
{
   if (logview != 0) {
      delete logview;
      logview = 0;
   }
}


MemcheckView::MemcheckView( QWidget* parent, const char* name )
   : ToolView( parent, name )
{
   mkToolBar();

   QVBoxLayout* vLayout = new QVBoxLayout( central, 0, -1, "vLayout" );
   vLayout->setResizeMode( QLayout::FreeResize );

#if 0
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
#endif

   lView = new QListView( central, "lview" );
   vLayout->addWidget( lView );

   logview = new VgLogView( lView );

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
   openAllButton->setEnabled( true );  /* TODO: enable only if lview populated */
   srcPathButton->setEnabled( true );  /* TODO: enable only if lview populated */

   /* enable | disable show*Item buttons */
   connect( lView, SIGNAL(selectionChanged()),
            this,  SLOT(itemSelected()) );
   /* launch editor with src file loaded */
   connect( lView, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
            this,  SLOT(launchEditor(QListViewItem*, const QPoint&, int)) );


   QFont clientout_fnt( "Adobe Courier", 9, QFont::Normal, false );
   clientout_fnt.setStyleHint( QFont::TypeWriter );

   /* other tabs:
      - suppression editor?
   */
}


/* called by memcheck: set state for buttons; set cursor state */
void MemcheckView::setState( bool run )
{
   loadlogButton->setEnabled( !run );
   mrglogButton->setEnabled( !run );
   // TODO: suppedButton->setEnabled( !run );
   if ( run ) {       /* startup */
      savelogButton->setEnabled( false );
      setCursor( QCursor(Qt::WaitCursor) );
      lView->clear();
   } else {           /* finished */
      unsetCursor();
      savelogButton->setEnabled( lView->childCount() != 0 );
   }
}


/* slot: connected to MainWindow::toggleToolbarLabels(). 
   called when user toggles 'show-butt-text' in Options page */
void MemcheckView::toggleToolbarLabels( bool state )
{
   loadlogButton->setUsesTextLabel( state );
   mrglogButton->setUsesTextLabel( state );
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
   emit run( VkRunState::TOOL1 );
#endif
}


/* slot: called from logMenu.  Open a file which contains a list of
   logfiles-to-be-merged, each on a separate line, with a minimum of
   two logfiles. */
void MemcheckView::openMergeFile()
{
   QString merge_file;
   /* TODO: Multiple xml files instead of single list file
      QStringList files = QFileDialog::getOpenFileNames() */
   merge_file = QFileDialog::getOpenFileName( QString::null,
                                              "XML Log Lists (*.loglst);;All Files (*)", 
                                              this, "fdlg", "Select Log File" );
   /* user might have clicked Cancel */
   if ( merge_file.isEmpty() )
      return;

   vkConfig->wrEntry( merge_file, "merge", "valkyrie" );
   /* returns the filename the merge has been saved to */
   emit run( VkRunState::TOOL2 );
}


void MemcheckView::mkToolBar()
{
   mcToolBar = new QToolBar( this, "mc_toolbar" );
   mcToolBar->setLabel( "Memcheck ToolBar" );

   bool show_text = vkConfig->rdBool( "show-butt-text", "valkyrie" );

   /* open-all items button --------------------------------------------- */
   openAllButton = new QToolButton( mcToolBar, "tb_open_all" );
   openAllButton->setIconSet( QPixmap( open_all_items_xpm ) );
   openAllButton->setAutoRaise( true );
   openAllButton->setToggleButton( true );
   connect( openAllButton, SIGNAL( toggled(bool) ), 
            this,          SLOT( openAllItems(bool) ) );
   QToolTip::add( openAllButton, 
                  "Open / Close all errors" );
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
                  "Show source paths" );
   ContextHelp::add( srcPathButton, urlValkyrie::srcPathButton );

   /* fake motif-style separator ---------------------------------------- */
   QLabel* sep_lbl = new QLabel( mcToolBar, "lbl_sep" );
   mcToolBar->setStretchableWidget( sep_lbl );

   /* load-log(s) button ------------------------------------------------ */
   loadlogButton = new QToolButton( mcToolBar, "tb_load_log" );
   loadlogButton->setIconSet( QPixmap( open_log_xpm ) );
   loadlogButton->setTextLabel( "Load Log" );
#if (QT_VERSION-0 >= 0x030200)
   loadlogButton->setTextPosition( QToolButton::BesideIcon );
#else // QT_VERSION < 3.2
   loadlogButton->setTextPosition( QToolButton::Right );
#endif
   loadlogButton->setUsesTextLabel( show_text );
   loadlogButton->setAutoRaise( true );
   connect( loadlogButton, SIGNAL( clicked() ), 
            this,          SLOT( openLogFile() ) );
   QToolTip::add( loadlogButton, "Load log file" );
   ContextHelp::add( loadlogButton, urlValkyrie::loadLogButton );

   /* merge-log(s) button ------------------------------------------------ */
   mrglogButton = new QToolButton( mcToolBar, "tb_merge_log" );
   mrglogButton->setIconSet( QPixmap( tool_run_xpm ) );
   mrglogButton->setTextLabel( "Merge Logs" );
#if (QT_VERSION-0 >= 0x030200)
   mrglogButton->setTextPosition( QToolButton::BesideIcon );
#else // QT_VERSION < 3.2
   mrglogButton->setTextPosition( QToolButton::Right );
#endif
   mrglogButton->setUsesTextLabel( show_text );
   mrglogButton->setAutoRaise( true );
   connect( mrglogButton, SIGNAL( clicked() ), 
            this,         SLOT( openMergeFile() ) );
   QToolTip::add( mrglogButton, "Merge multiple log files" );
   ContextHelp::add( mrglogButton, urlValkyrie::mrgLogButton );

   /* save-log button --------------------------------------------------- */
   savelogButton = new QToolButton( mcToolBar, "tb_save_log" );
   savelogButton->setIconSet( QPixmap( save_log_xpm ) );
   savelogButton->setTextLabel( "Save Log" );
#if (QT_VERSION-0 >= 0x030200)
   savelogButton->setTextPosition( QToolButton::BesideIcon );
#else // QT_VERSION < 3.2
   savelogButton->setTextPosition( QToolButton::Right );
#endif
   savelogButton->setUsesTextLabel( show_text );
   savelogButton->setAutoRaise( true );
   connect( savelogButton, SIGNAL( clicked() ), 
            this,          SIGNAL( saveLogFile() ) );
   QToolTip::add( savelogButton, "Save output to a log file" );
   ContextHelp::add( savelogButton, urlValkyrie::saveLogButton );

   /* suppressions editor button ---------------------------------------- */
   suppedButton = new QToolButton( mcToolBar, "tb_supp_ed" );
   suppedButton->setIconSet( QPixmap( supp_editor_xpm ) );
   suppedButton->setTextLabel( "Supp'n Editor" );
#if (QT_VERSION-0 >= 0x030200)
   suppedButton->setTextPosition( QToolButton::BesideIcon );
#else // QT_VERSION < 3.2
   suppedButton->setTextPosition( QToolButton::Right );
#endif
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
//void MemcheckView::launchEditor( QListViewItem* lv_item, const QPoint&, int )
void MemcheckView::launchEditor( QListViewItem*, const QPoint&, int )
{
#if 0
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
#endif
}


/* opens all error items plus their children. 
   ignores the status, preamble, et al. items. */
void MemcheckView::openAllItems( bool open )
{
   if (!lView->firstChild()) /* listview populated at all? */
      return;

   VgOutputItem* op_item = (VgOutputItem*)lView->firstChild()->firstChild();
   vk_assert(op_item);

   /* move down till we get to the first error */
   while ( op_item && op_item->elemType() != VgElement::ERROR ) {
      op_item = op_item->nextSibling();
   }

   QListViewItemIterator it( op_item );
   while ( it.current() ) {
      if ( ((VgOutputItem*)it.current())->elemType() == VgElement::SUPPCOUNTS )
         break;                        /* stop when we hit suppressions */
      it.current()->setOpen( open );
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
                   

/* shows the file paths for all frames under current item */
void MemcheckView::showSrcPath()
{
   QListViewItem* lv_item = lView->currentItem();
   if ( !lv_item ) return;

   VgOutputItem* curr_item = (VgOutputItem*)lv_item;

   /* iterate over items up to nextSibling */
   QListViewItem* sibling = curr_item->nextSibling();
   VgOutputItem* item;
   QListViewItemIterator it( curr_item );
   for(; (item = (VgOutputItem*)it.current()); ++it ) {
      if (item == sibling)
         break;
      if ( item->elemType() == VgElement::FRAME ) {
         /* show path in this frame */
         FrameItem* frame = (FrameItem*)item;
         frame->setText( ((VgFrame*)&frame->elem)->describe_IP(true) );
      }
   }
}


void MemcheckView::itemSelected()
{
   QListViewItem* lv_item = lView->currentItem();
   if ( !lv_item ) {
      openOneButton->setEnabled( false );
   } else {
      /* if item openable and within an error: enable openOneButton */
      bool isOpenable = (lv_item->firstChild() != 0);
      bool withinError = false;
      VgOutputItem* curr_item = (VgOutputItem*)lv_item;
      for (; curr_item; curr_item = curr_item->parent() ) {
         if ( curr_item->elemType() == VgElement::ERROR ) {
            withinError = true;
            break;
         }
      }
      openOneButton->setEnabled( isOpenable && withinError );
   }
}
