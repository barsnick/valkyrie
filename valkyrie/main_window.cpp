/* ---------------------------------------------------------------------
 * Implementation of MainWindow                          main_window.cpp
 * Application's top-level window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qobjectlist.h>

#include "main_window.h"
#include "tool_object.h"
#include "valkyrie_object.h"
#include "valkyrie_xpm.h"
#include "tb_mainwin_icons.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "vk_messages.h"
#include "context_help.h"
#include "html_urls.h"

/*
   &File:      ALT+Key_F
   O&ptions:   ALT+Key_P
   &Tools:     ALT+Key_T
   &Help:      ALT+Key_H
   handBook:   Key_F1

   R&estart:   CTRL+Key_E
   &Run:       CTRL+Key_R
   &Save:      CTRL+Key_S
   S&top:      CTRL+Key_T
   &Close:     CTRL+Key_W
   E&xit:      CTRL+Key_X

   Memcheck    SHIFT+Key_M
   Cachegrind: SHIFT+Key_C 
   Massif:     SHIFT+Key_S
*/



/* class MainWindow ---------------------------------------------------- */
MainWindow::~MainWindow()
{
   /* Write window attributes to config on exit.
      Want to ignore any outstanding dirty config vals, but vkConfig
      doesn't hold originals... simplest just to use a clean vkConfig... */

   VkConfig cfg;
   if ( cfg.initCfg( m_valkyrie ) ) {
      cfg.wrInt( this->height(), "height", "MainWin" );
      cfg.wrInt( this->width(),  "width",  "MainWin" );
      cfg.wrInt( this->x(),      "x-pos",  "MainWin" );
      cfg.wrInt( this->y(),      "y-pos",  "MainWin" );

      // save opts to disk... too bad if it fails
      cfg.sync( m_valkyrie );
   }
}

MainWindow::MainWindow( Valkyrie* vk ) : QMainWindow( 0, "mainWindow" )
{
   m_valkyrie = vk;

   setCaption( vkConfig->vkName() );
   setIcon( QPixmap(valkyrie_xpm) );
   statusBar()->setSizeGripEnabled( false );

   m_viewStack = new ToolViewStack( this, "view_stack" );
   m_viewStack->setMargin( 0 );
   m_viewStack->setLineWidth( 0 );

   setCentralWidget( m_viewStack );

   /* handbook: init before menubar / toolbar */
   m_handBook = new HandBook();

   /* create menubar, status bar + flags widget */
   mkMenuBar();
   mkStatusBar();

   /* en/disable tooltips */
   m_showToolTips = !vkConfig->rdBool("show-tooltips", "valkyrie");
   toggleToolTips();

   /* init the options dialog */
   m_optionsWin = new OptionsWindow( this );
   /* let the flags widget know that flags may have been modified */
   connect( m_optionsWin, SIGNAL(flagsChanged()),
            this,           SLOT(updateVgFlags()) );
}


void MainWindow::showToolView( int tvid )
{
   if ( m_viewStack->visible() != 0 ) {
      /* already loaded and visible */
      if ( m_viewStack->id( m_viewStack->visible() ) == tvid ) {
         return;
      } 
   }

   /* set up next view */
   ToolObject* nextTool = valkyrie()->valgrind()->toolObj( tvid );
   vk_assert( nextTool != 0 );

   /* view already created and on stack? */
   ToolView* nextView = m_viewStack->view( tvid );
   if ( nextView == 0 ) {
      { // new toolview
         nextView = nextTool->createView( m_viewStack );
         vk_assert( nextView != 0 );

         m_viewStack->addView( nextView, tvid );

         connect( nextTool, SIGNAL(running(bool)), 
                  this,       SLOT(updateButtons(bool)) );
         connect( nextTool, SIGNAL(message(QString)),
                  this,       SLOT(setStatus(QString)) );
         connect( this,     SIGNAL(toolbarLabelsToggled(bool)),
                  nextView,   SLOT(toggleToolbarLabels(bool)) );

         /* view starts tool processes via this signal */
         connect( nextView, SIGNAL(run(VkRunState::State)),
                  this,       SLOT(run(VkRunState::State)) );
      }
   }

   m_viewStack->raiseView( tvid );
   setToggles( tvid );
   updateVgFlags();
}


void MainWindow::stop()
{
   /* don't come in here if there's no current view */
   if ( m_viewStack->visible() == 0 )
      return;

   valkyrie()->stopTool( m_viewStack->visibleId() );
}

/* start appropriate process for given runState */
void MainWindow::run( VkRunState::State runState )
{
   vk_assert( runState != VkRunState::STOPPED );

   /* don't come in here if there's no current view */
   if ( m_viewStack->visible() == 0 )
      return;

   /* last process might not be done ... */
   if ( !valkyrie()->queryToolDone( m_viewStack->visibleId() ) )
      return;

   /* if running valgrind, make sure there's a valid binary
      just 'cos it made it into the config doesn't mean it's valid */
   if( runState == VkRunState::VALGRIND ) {
      int errval  = PARSED_OK;
      QString bin = vkConfig->rdEntry( "binary", "valkyrie" );
      binaryCheck( &errval, bin );
      if ( errval != PARSED_OK ) {
         vkError( this, "Run Tool", "Invalid Binary: Please set a valid binary in Options::Valkyrie." );
         return;
      }
   }

   if ( !valkyrie()->runTool( m_viewStack->visibleId(), runState ) ) {
      vkError( this, "Run Tool",
               "Failed to complete execution." );
   }
}

/* run valgrind --tool=<current_tool> + flags + executable */
void MainWindow::runValgrind()
{
   /* don't come in here if there's no current view */
   if ( m_viewStack->visible() == 0 )
      return;

   /* valkyrie may have been started with no executable
      specified. if so, show prefsWindow + msgbox */
   if ( vkConfig->rdEntry("binary","valkyrie").isEmpty() ) {
      showOptionsWindow( Valkyrie::ID_VALKYRIE );
      vkInfo( m_optionsWin, "Run Valgrind",
              "Please enter the path to the executable "
              "you wish to run, together with any arguments");
      return;
   }

   run( VkRunState::VALGRIND );
}


void MainWindow::showAboutInfo( int id )
{
   HelpAbout::TabId tabid = (HelpAbout::TabId)id;
   HelpAbout * dlg = new HelpAbout( this, tabid );
   dlg->exec();
   delete dlg;
}


void MainWindow::showOptionsWindow( int view_id )
{ 
   m_optionsWin->showPage( view_id ); 
   m_optionsWin->raise();
}


/* slot, connected to a tool object's signal running(bool) */
void MainWindow::updateButtons( bool running )
{
   m_runButton->setEnabled( !running );
   m_stopButton->setEnabled( running );
}


void MainWindow::setToggles( int tview_id )
{
   int obj_id = -1;
   for ( uint index=0; index < m_toolsMenu->count(); index++ ) {
      obj_id = m_toolsMenu->idAt( index );
      /* set the view menu item on || off */
      m_toolsMenu->setItemEnabled( obj_id, obj_id != tview_id );
   }

   QToolButton* tbutt;
   if ( tview_id == -1 ) {
      tbutt = (QToolButton*)m_viewButtGroup->selected();
      tbutt->setOn( false );
   } else {
      tbutt = (QToolButton*)m_viewButtGroup->find( tview_id );
      tbutt->setOn( true );
   }

   if ( tview_id == -1 ) {
      /* no more tool views */
      m_fileMenu->setItemEnabled( FILE_RUN,   false );
      m_fileMenu->setItemEnabled( FILE_STOP,  false );
      m_fileMenu->setItemEnabled( FILE_CLOSE, false );

      m_runButton->setEnabled( false );
      m_stopButton->setEnabled( false );

      if ( m_flagsButton->isOn() ) {
         m_flagsLabel->hide();
         m_flagsButton->setDown( false );
      }
      m_flagsButton->setEnabled( false );
   } else {
      /* someone is hanging around ... */
      m_fileMenu->setItemEnabled( FILE_CLOSE,   true );
      bool is_running = false;
      if (m_viewStack->visible() != 0) {
         ToolObject* tool = valkyrie()->valgrind()->toolObj( m_viewStack->visibleId() );
         is_running = tool->isRunning();
      }
      m_fileMenu->setItemEnabled( FILE_RUN,  !is_running );
      m_fileMenu->setItemEnabled( FILE_STOP, is_running );

      m_runButton->setEnabled( !is_running );
      m_stopButton->setEnabled( is_running );

      if ( m_flagsButton->isOn() ) {
         showFlagsWidget( true );
      }
      m_flagsButton->setEnabled( true );
   }

}


/* slot: connected to a tool object's signal message(QString) */
void MainWindow::setStatus( QString msg )
{ m_statusMsg->setText( msg ); }


/* shows / hides the label which contains the flags relevant to 
   the current toolview */
void MainWindow::showFlagsWidget( bool show )
{
   if ( !show ) {
      m_flagsLabel->hide();
   } else {
      if ( m_viewStack->visible() != 0 ) {
         QString flags = valkyrie()->getDisplayFlags();
         m_flagsLabel->setText( flags );
         if ( !m_flagsLabel->isVisible() ) {
            m_flagsLabel->show();
         }
      }
   }
}


/* Called by this->showToolView(),
   m_optionsWin::flagsChanged() signal
*/
void MainWindow::updateVgFlags()
{
   /* update valkyrie's flags: if there's a visible ToolView */
   if ( m_viewStack->visible() != 0 )
      valkyrie()->updateVgFlags( m_viewStack->visibleId() );

   /* update flags display */
   if ( m_flagsLabel->isVisible() ) {
      showFlagsWidget( true );
   }
}


/* the stack owns the toolviews, and MainWin owns the stack, so we
   don't need to explicitly delete the toolviews as they will be
   auto-deleted when MainWin closes */
void MainWindow::closeEvent( QCloseEvent *ce )
{
   ToolObjList tools = valkyrie()->valgrind()->toolObjList();
   for ( ToolObject* tool = tools.first(); tool; tool = tools.next() ) {
      if ( tool->view() != 0 && !tool->queryDone() ) {
         ce->ignore();
         return;
      }
   }

   /* handbook is a top-level parent-less widget, so we have to
      explicitly delete it */
   delete m_handBook;
   m_handBook = 0;

   QMainWindow::closeEvent( ce );
}


void MainWindow::closeToolView()
{
   int currViewId = m_viewStack->visibleId();

   /* don't come in here if there's no current view */
   if ( currViewId == -1 ) return;

   ToolObject* currTool = valkyrie()->valgrind()->toolObj( currViewId );

   vk_assert(currTool->view() != 0);
   vk_assert(currTool->view() == m_viewStack->visible());

   /* process might still be running, or whatever ... */
   if ( !currTool->queryDone() ) return;

   /* remove view from stack (doesn't delete) */
   m_viewStack->removeView( m_viewStack->widget( currViewId ) );

   /* delete view, and zero tool's view ptr */
   currTool->deleteView();

   /* find the next view to be shown, if exists, and show it */
   ToolView* nextView = m_viewStack->nextView();
   if ( nextView != 0 ) {  // Found another view in stack
      m_viewStack->raiseView( nextView );
   }

   setToggles( m_viewStack->id( nextView ) );
}


void MainWindow::toggleToolTips()
{
   m_showToolTips = !m_showToolTips;
   QToolTip::setGloballyEnabled( m_showToolTips );
}


void MainWindow::toggleToolbarLabels()
{
   m_showToolbarLabels = !m_showToolbarLabels;
   m_runButton->setUsesTextLabel( m_showToolbarLabels );
   m_stopButton->setUsesTextLabel( m_showToolbarLabels );
   /* tell the toolviews to follow suit */
   emit toolbarLabelsToggled( m_showToolbarLabels );
}


void MainWindow::mkMenuBar()
{
   /* show toolbutton text labels (or not) */
   m_showToolbarLabels = vkConfig->rdBool("show-butt-text","valkyrie");

   /* --------------------------------------------------------------- */
   /* Main Menu ----------------------------------------------------- */
   QMenuBar* mainMenu = new QMenuBar( this, "main menubar" );
   ContextHelp::add( mainMenu, urlValkyrie::menuBar );

   int index = -1;

   /* file menu --------------------------------------------------------- */
   index++;
   m_fileMenu = new QPopupMenu( this, "file_menu" );
   m_fileMenu->insertItem( "&Run Valgrind", this, SLOT(runValgrind()), 
                           CTRL+Key_R, FILE_RUN );
   m_fileMenu->insertItem( "S&top", this, SLOT(stop()), 
                           CTRL+Key_T, FILE_STOP );
   m_fileMenu->insertSeparator();
   m_fileMenu->insertItem( "&Close", this, SLOT(closeToolView()),  
                           CTRL+Key_W, FILE_CLOSE );
   m_fileMenu->insertItem( "E&xit", qApp, 
                           SLOT(closeAllWindows()), CTRL+Key_X );
   mainMenu->insertItem( "&File", m_fileMenu, -1, index );
   ContextHelp::add( m_fileMenu, urlValkyrie::fileMenu );

   /* toolview menu ----------------------------------------------------- */
   index++;
   QPixmap bulletSet(black_bullet_xpm);
   m_toolsMenu = new QPopupMenu( this, "tools_menu" );
   ToolObjList tools = valkyrie()->valgrind()->toolObjList();
   for ( ToolObject* tool = tools.first(); tool; tool = tools.next() ) {
      m_toolsMenu->insertItem( bulletSet, tool->accelTitle(), 
                               this, SLOT( showToolView(int) ),
                               tool->accelKey(), tool->objId() );
   }
   mainMenu->insertItem( "&Tools", m_toolsMenu, -1, index );
   ContextHelp::add( m_toolsMenu, urlValkyrie::toolMenu );

   /* options / preferences et al --------------------------------------- */
   index++;
   m_optsMenu = new QPopupMenu( this, "options_menu" );
   VkObjectList objList = valkyrie()->vkObjList();
   for ( VkObject* obj = objList.first(); obj; obj = objList.next() ) {
      m_optsMenu->insertItem( obj->title(), this, 
                              SLOT(showOptionsWindow(int)),
                              0, obj->objId() );
   }
   mainMenu->insertItem( "O&ptions", m_optsMenu, -1, index );
   ContextHelp::add( m_optsMenu, urlValkyrie::optionsMenu );


   /* help menu ----------------------------------------------------- */
   index++;
   m_helpMenu = new QPopupMenu( this, "help_menu" );
   m_helpMenu->insertItem( "Handbook", m_handBook, SLOT(showYourself()), Key_F1 );
   m_helpMenu->insertSeparator();
   m_helpMenu->insertItem( "About Valkyrie", this, 
                           SLOT(showAboutInfo(int)), 0, HelpAbout::ABOUT_VK );
   m_helpMenu->insertItem( "About Qt", this, 
                           SLOT(showAboutInfo(int)), 0, HelpAbout::ABOUT_QT );
   m_helpMenu->insertSeparator();
   m_helpMenu->insertItem( "License", this, 
                           SLOT(showAboutInfo(int)), 0, HelpAbout::LICENSE );
   m_helpMenu->insertItem( "Support", this, 
                           SLOT(showAboutInfo(int)), 0, HelpAbout::SUPPORT );
   mainMenu->insertItem( "&Help", m_helpMenu, -1, index );
   QToolTip::add( m_helpMenu, "Show help manual / information" );
   ContextHelp::add( m_helpMenu, urlValkyrie::helpMenu );

   /* Note: 'windows' style horizontal menu's seem to right-justify anything
      after a non-QPopupMenu */

   /* application-wide context help button ------------------------------ */
   index++;
   QToolButton* ctxtButton = new ContextHelpButton( this, m_handBook );
   QToolTip::add( ctxtButton, "This is a <b>Context Help</b> button. "
                  "Clicking on a widget will open the relevant manual help page");
   mainMenu->insertItem( ctxtButton, -1, index );


   /* --------------------------------------------------------------- */
   /* Valgrind Control Toolbar -------------------------------------- */
   QToolBar* vgCtlToolBar = new QToolBar( this, "vg_ctl_toolbar" );
   vgCtlToolBar->setLabel( "Valgrind Control Toolbar" );

   /* run button ---------------------------------------------------- */
   index++;
   m_runButton = new QToolButton( vgCtlToolBar, "tb_rerun" );
   m_runButton->setIconSet( QPixmap(run_xpm) );
   m_runButton->setTextLabel( "&Run Valgrind" );
#if (QT_VERSION-0 >= 0x030200)
   m_runButton->setTextPosition( QToolButton::BesideIcon );
#else // QT_VERSION < 3.2
   m_runButton->setTextPosition( QToolButton::Right );
#endif
   m_runButton->setUsesTextLabel( m_showToolbarLabels );
   m_runButton->setAutoRaise( true );
   m_runButton->setAccel( CTRL+Key_R );
   connect( m_runButton, SIGNAL( clicked() ), 
            this,          SLOT( runValgrind() ) );
   QToolTip::add( m_runButton, "Run valgrind with specified executable" );
   ContextHelp::add( m_runButton, urlValkyrie::runButton );

   /* stop button --------------------------------------------------- */
   index++;
   m_stopButton = new QToolButton( vgCtlToolBar, "tb_stop" );
   m_stopButton->setIconSet( QPixmap(stop_xpm) );
   m_stopButton->setTextLabel( "S&top" );
#if (QT_VERSION-0 >= 0x030200)
   m_stopButton->setTextPosition( QToolButton::BesideIcon );
#else // QT_VERSION < 3.2
   m_stopButton->setTextPosition( QToolButton::Right );
#endif
   m_stopButton->setUsesTextLabel( m_showToolbarLabels );
   m_stopButton->setAutoRaise( true );
   m_stopButton->setAccel( CTRL+Key_T );
   connect( m_stopButton, SIGNAL( clicked() ),
            this,           SLOT( stop() ) );
   QToolTip::add( m_stopButton, "Terminate program execution immediately" );
   ContextHelp::add( m_stopButton, urlValkyrie::stopButton );


   /* strech toolbar to right --------------------------------------- */
   QLabel* lbl_extend = new QLabel( vgCtlToolBar, "lbl_extend" );
   vgCtlToolBar->setStretchableWidget( lbl_extend );
}


/* 'status bar' with 3 rows: label to show non-default flags on top,
   message text in the middle
   toolview buttons on the bottom */
void MainWindow::mkStatusBar()
{
   QFrame* statusFrame = new QFrame( statusBar() );
   statusBar()->addWidget( statusFrame, 10, true );
   QVBoxLayout* statusLayout = new QVBoxLayout( statusFrame );
   statusLayout->setSpacing( 2 );

   /* hbox for top row */
   QHBoxLayout* top_row = new QHBoxLayout( 2, "top_row" );
   statusLayout->addLayout( top_row, 20 );
  
   m_flagsLabel = new QLabel( statusFrame, "flags_label" );
   top_row->addWidget( m_flagsLabel );
   m_flagsLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
   m_flagsLabel->setPaletteBackgroundColor( Qt::white );
   m_flagsLabel->setAlignment( AlignLeft );
   m_flagsLabel->setTextFormat( Qt::PlainText );
   ContextHelp::add( m_flagsLabel, urlValkyrie::flagsWidget );
   m_flagsLabel->hide();


   /* hbox for middle row */
   QHBoxLayout* mid_row = new QHBoxLayout( 5, "middle_row" );
   statusLayout->addLayout( mid_row );

   /* frame+layout for messages */
   QFrame* msgFrame = new QFrame( statusFrame );
   msgFrame->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
   mid_row->addWidget( msgFrame );
   QBoxLayout* msgLayout = new QHBoxLayout( msgFrame, 2 );
   m_statusMsg = new QLabel( "", msgFrame, "status_msg" );
   m_statusMsg->setAlignment( AlignLeft );
   m_statusMsg->setTextFormat( Qt::PlainText );
   int status_height = fontMetrics().height();
   m_statusMsg->setFixedHeight( status_height );
   msgLayout->addWidget( m_statusMsg );
   ContextHelp::add( m_statusMsg, urlValkyrie::statusMsg );


   /* hbox for the bottom row */
   QHBoxLayout* bot_row = new QHBoxLayout( 5, "bot_row" );
   statusLayout->addLayout( bot_row, 0 );

   /* the toolview buttons */
   m_viewButtGroup = new QButtonGroup( statusFrame );
   m_viewButtGroup->setExclusive( true );
   m_viewButtGroup->hide();
   connect( m_viewButtGroup, SIGNAL(clicked(int)), 
            this,              SLOT(showToolView(int)) );
   //RM: ContextHelp::add( m_viewButtGroup, urlValkyrie::tviewButtons );

   /* set the buttons to all be the same width */
   int butt_width = fontMetrics().width( "XMemcheckX" );
   ToolObjList tools = valkyrie()->valgrind()->toolObjList();
   for ( ToolObject* tool = tools.first(); tool; tool = tools.next() ) {
      int len = tool->accelTitle().length();
      butt_width = ( len > butt_width ) ? len : butt_width;
   }
   int butt_height = fontMetrics().height() + 12;
   QToolButton* tvButton;
   for ( ToolObject* tool = tools.first(); tool; tool = tools.next() ) {
      tvButton = new QToolButton( statusFrame, tool->name() + "_button" );
      tvButton->setToggleButton( true );
      tvButton->setEnabled( true );
      tvButton->setText( tool->accelTitle() );
      tvButton->setAccel( tool->accelKey() );
      tvButton->setMinimumWidth( butt_width );
      tvButton->setFixedHeight( butt_height );
      m_viewButtGroup->insert( tvButton, tool->objId() );
      bot_row->addWidget( tvButton );
      ContextHelp::add( tvButton, urlValkyrie::toolMenu );
   }

   bot_row->addStretch(1);

   /* frame+layout for the view-flags icon */
   m_flagsButton = new QToolButton( statusFrame, "flags_button" );
   bot_row->addWidget( m_flagsButton );
   m_flagsButton->setToggleButton( true );
   m_flagsButton->setEnabled( true );
   m_flagsButton->setPixmap( QPixmap(view_flags_xpm) );
   connect( m_flagsButton, SIGNAL( toggled(bool) ), 
            this,            SLOT( showFlagsWidget(bool) ) );
   QToolTip::add( m_flagsButton, "Show non-default valgrind flags for current tool" );
   ContextHelp::add( m_flagsButton, urlValkyrie::flagsWidget );
}

