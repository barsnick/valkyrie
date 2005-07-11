/* ---------------------------------------------------------------------
 * Implementation of MainWindow                          main_window.cpp
 * Application's top-level window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qlayout.h>
#include <qmotifstyle.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qobjectlist.h>

#include "main_window.h"
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
{ /* handBook + optionsWin are deleted in closeEvent() */ }


MainWindow::MainWindow( Valkyrie* valk ) : QMainWindow( 0, "mainWindow" )
{
  valkyrie = valk;

  setCaption( vkConfig->vkName() );
  setIcon( vkConfig->pixmap( "valkyrie.xpm" ) );
  statusBar()->setSizeGripEnabled( false );

  /* setup tool space */
  QVBox* vbox = new QVBox( this );
  //  vbox->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  vbox->setMargin( 0 );
  vbox->setLineWidth( 0 );
  setCentralWidget( vbox );

  viewStack = new ToolViewStack( vbox );

  /* handbook: init before menubar / toolbar */
  handBook = new HandBook();

  /* create menubar, status bar + flags widget */
  mkMenuBar();
  mkStatusBar();

  /* en/disable tooltips */
  showToolTips = !vkConfig->rdBool("show-tooltips", "valkyrie");
  toggleToolTips();

  /* init the options dialog */
  optionsWin = new OptionsWindow( this );
  /* let the flags widget know that flags may have been modified */
  connect( optionsWin, SIGNAL(flagsChanged()),
           this,       SLOT(updateFlagsWidget()) );
}


void MainWindow::showToolView( int tvid )
{
  if ( viewStack->visibleView() != 0 ) {
    /* already loaded and visible */
    if ( viewStack->visibleView()->id() == tvid ) {
      return;
    } 
  }

  /* Setup new view */
  bool set_running = false;
  ToolView*   nextView = viewStack->view( tvid );
  ToolObject* nextTool = vkConfig->vkToolObj( tvid );

  if ( nextView == 0 ) {
    { // newToolView()
      nextView = nextTool->createToolView( viewStack );
      vk_assert( nextView != 0 );

      viewStack->addView( nextView, tvid );

      connect( nextTool, SIGNAL(running(bool)), 
               this,       SLOT(updateButtons(bool)) );
      connect( nextTool, SIGNAL(message(QString)),
               this,       SLOT(setStatus(QString)) );
      connect( this,       SIGNAL(toolbarLabelsToggled(bool)),
               nextView, SLOT(toggleToolbarLabels(bool)) );
    }

#if 1 
/* CAB: This ain't right, I think...
   if we close the view, then open it again...
   do we want an 'on-open' flag?
*/
    /* if what-to-do was specified on the cmd-line, do it.
       otherwise, hang around and look boo'ful */
    if ( vkConfig->currentToolName() == nextView->name() ) {
      set_running = true;
    }
#endif
  }

  viewStack->raiseView( tvid );

  /* ensure the toolview is visible before we start doing stuff */
  if ( set_running ) {
    qApp->processEvents();
    /* don't do anything if runMode == modeNotSet */
    valkyrie->runTool( nextTool );
  }

  setToggles( tvid );
}


void MainWindow::stop()
{
  /* don't come in here if there's no current view */
  if ( viewStack->visibleView() == 0 )
    return;

  ToolObject* tool = vkConfig->vkToolObj( viewStack->visibleView()->id() );
  valkyrie->stopTool( tool );
}


/* run valgrind --tool=<current_tool> + flags + executable */
void MainWindow::run()
{
  /* don't come in here if there's no current view */
  if ( viewStack->visibleView() == 0 )
    return;

  /* valkyrie may have been started with no executable
     specified. if so, show prefsWindow + msgbox */
  if ( vkConfig->rdEntry("binary","valkyrie").isEmpty() ) {
    showOptionsWindow( VkObject::VALKYRIE );
    vkInfo( optionsWin, "Run Valgrind",
            "Please enter the path to the executable "
            "you wish to run, together with any arguments");
    return;
  }

  valkyrie->setRunMode( Valkyrie::modeParseOutput );
  ToolObject* tool = vkConfig->vkToolObj( viewStack->visibleView()->id() );
  if ( valkyrie->runTool( tool ) ) {
    printf("TODO: toggle buttons to reflect running state\n");
  } else {
    vkError( this, "Run Valgrind",
             "Failed to start the executable running." );
  }

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
	optionsWin->showPage( view_id ); 
	optionsWin->raise();
}


/* slot, connected to a tool object's signal running(bool) */
void MainWindow::updateButtons( bool running )
{
  runButton->setEnabled( !running );
  stopButton->setEnabled( running );
}


void MainWindow::setToggles( int tview_id )
{
  int obj_id = -1;
  for ( uint index=0; index<toolsMenu->count(); index++ ) {
    obj_id = toolsMenu->idAt( index );
    /* set the view menu item on || off */
    toolsMenu->setItemEnabled( obj_id, obj_id != tview_id );
  }

  QToolButton* tbutt;
  if ( tview_id == -1 ) {
    tbutt = (QToolButton*)viewButtGroup->selected();
    tbutt->setOn( false );
  } else {
    tbutt = (QToolButton*)viewButtGroup->find( tview_id );
    tbutt->setOn( true );
  }

  if ( tview_id == -1 ) {
    /* no more tool views */
    fileMenu->setItemEnabled( FILE_RUN,   false );
    fileMenu->setItemEnabled( FILE_STOP,  false );
    fileMenu->setItemEnabled( FILE_CLOSE, false );

    runButton->setEnabled( false );
    stopButton->setEnabled( false );

    if ( flagsButton->isOn() ) {
      flagsLabel->hide();
      flagsButton->setDown( false );
    }
    flagsButton->setEnabled( false );
  } else {
    /* someone is hanging around ... */
    fileMenu->setItemEnabled( FILE_CLOSE,   true );
    bool is_running = false;
    if (viewStack->visibleView() != 0) {
      ToolObject* tool = vkConfig->vkToolObj( viewStack->visibleView()->id() );
      is_running = tool->isRunning();
    }
    fileMenu->setItemEnabled( FILE_RUN,  !is_running );
    fileMenu->setItemEnabled( FILE_STOP, is_running );

    runButton->setEnabled( !is_running );
    stopButton->setEnabled( is_running );

    if ( flagsButton->isOn() ) {
      showFlagsWidget( true );
    }
    flagsButton->setEnabled( true );
  }

}


/* slot: connected to a tool object's signal message(QString) */
void MainWindow::setStatus( QString msg )
{ statusMsg->setText( msg ); }


/* shows / hides the label which contains the flags relevant to 
   the current toolview */
void MainWindow::showFlagsWidget( bool show )
{
  if ( !show ) {
    flagsLabel->hide();
  } else {
    if ( viewStack->visibleView() != 0 ) {
      ToolObject* tool = vkConfig->vkToolObj( viewStack->visibleView()->id() );
      QString flags = valkyrie->currentFlags( tool );
      flagsLabel->setText( flags );
      if ( !flagsLabel->isVisible() ) {
        flagsLabel->show();
      }
    }
  }
}


/* slot: connected to optionsWin signal flagsChanged() */
void MainWindow::updateFlagsWidget()
{
  if ( flagsLabel->isVisible() ) {
    showFlagsWidget( true );
  }
}


void MainWindow::resizeEvent( QResizeEvent *re )
{
  QWidget::resizeEvent( re );
#if 0
  QSize sz = size();
  vkConfig->wrInt( sz.width(),  "width",  "MainWin" );
  vkConfig->wrInt( sz.height(), "height", "MainWin" );
#else
  vkConfig->wrInt( frameGeometry().width(),  "width",  "MainWin" );
  vkConfig->wrInt( frameGeometry().height(), "height", "MainWin" );
#endif
}


void MainWindow::moveEvent( QMoveEvent* )
{ 
#if 0
  QPoint pt = pos();
  vkConfig->wrInt( pt.x(), "x-pos", "MainWin" );
  vkConfig->wrInt( pt.y(), "y-pos", "MainWin" );
#else
  vkConfig->wrInt( frameGeometry().x(), "x-pos", "MainWin" );
  vkConfig->wrInt( frameGeometry().y(), "y-pos", "MainWin" );
#endif
}


void MainWindow::closeEvent( QCloseEvent *ce )
{
  const ToolViewList* views = viewStack->viewList();
  ToolViewListIter it( *views );
  ToolView* view;
  for (; ((view = it.current()) != 0); ++it ) {
    if ( !(view->close()) ) {
      ce->ignore();
      return;
    }
  }
  delete views; // delete the list, not the objects

  delete optionsWin;
  optionsWin = 0;

  /* save history + bookmarks */
  handBook->save();
  delete handBook;
  handBook = 0;

  QMainWindow::closeEvent( ce );
}


void MainWindow::closeToolView()
{
  ToolView* currView = viewStack->visibleView();
  /* don't come in here if there's no current view */
  if ( currView == 0 ) return;

  ToolObject* currTool = vkConfig->vkToolObj( currView->id() );

  /* try to deliver the coup de grace */
  if ( !currTool->closeView() ) return;

  /* remove from stack (doesn't delete) */
  viewStack->removeView( currView );

  /* delete toolview, and set ToolObject's ptr to 0 */
  currTool->deleteView();

  /* find the next view to be shown, if exists, and show it */
  ToolView* nextView = viewStack->nextView();
  if ( nextView != 0 ) {  // Found another view in stack
    viewStack->raiseView( nextView );
  }

  setToggles( ( nextView == 0 ) ? -1 : nextView->id() );
}


void MainWindow::toggleToolTips()
{
  showToolTips = !showToolTips;
  QToolTip::setGloballyEnabled( showToolTips );
}


void MainWindow::toggleToolbarLabels()
{
  showToolbarLabels = !showToolbarLabels;
  runButton->setUsesTextLabel( showToolbarLabels );
  stopButton->setUsesTextLabel( showToolbarLabels );
  helpButton->setUsesTextLabel( showToolbarLabels );
  /* tell the toolviews to follow suit */
  emit toolbarLabelsToggled( showToolbarLabels );
}


void MainWindow::mkMenuBar()
{
  /* show toolbutton text labels (or not) */
  showToolbarLabels = vkConfig->rdBool("show-butt-text","valkyrie");

  QMenuBar* mainMenu = new QMenuBar( this, "main menubar" );
  mainMenu->setStyle( new QMotifStyle() );
  int index = -1;

  /* file menu --------------------------------------------------------- */
  index++;
  fileMenu = new QPopupMenu( this );
  fileMenu->insertItem( "&Run", this, SLOT(run()), 
                        CTRL+Key_R, FILE_RUN );
  fileMenu->insertItem( "S&top", this, SLOT(stop()), 
                        CTRL+Key_T, FILE_STOP );
  fileMenu->insertSeparator();
  fileMenu->insertItem( "&Close", this, SLOT(closeToolView()),  
                        CTRL+Key_W, FILE_CLOSE );
  fileMenu->insertItem( "E&xit", qApp, 
                        SLOT(closeAllWindows()), CTRL+Key_X );
  int id = mainMenu->insertItem( "&File", fileMenu, -1, index );
  mainMenu->setAccel( ALT+Key_F, id );
  ContextHelp::add( fileMenu, urlValkyrie::Dummy );

  /* toolview menu ----------------------------------------------------- */
  index++;
  QPixmap bulletSet(black_bullet_xpm);
  toolsMenu = new QPopupMenu( this );
  ToolList tools = valkyrie->toolList();
  for ( ToolObject* tool = tools.first(); tool; tool = tools.next() ) {
    toolsMenu->insertItem( bulletSet, tool->accelTitle(), 
                           this, SLOT( showToolView(int) ),
                           tool->accelKey(), tool->id() );
  }
  tools.clear();
  id = mainMenu->insertItem( "&Tools", toolsMenu, -1, index );
  mainMenu->setAccel( ALT+Key_T, id );
  ContextHelp::add( toolsMenu, urlValkyrie::Dummy );

  /* options / preferences et al --------------------------------------- */
  index++;
  QPopupMenu* prefsMenu = new QPopupMenu( this );
  VkObjectList objList = vkConfig->vkObjList();
  for ( VkObject* obj = objList.first(); obj; obj = objList.next() ) {
    prefsMenu->insertItem( obj->title(), this, 
                           SLOT(showOptionsWindow(int)), 0, obj->id() );
  }
  id = mainMenu->insertItem( "O&ptions", prefsMenu, -1, index );
  mainMenu->setAccel( ALT+Key_P, id );
  ContextHelp::add( prefsMenu, urlValkyrie::Dummy );

  /* spacer between popup menus and tool-buttons ----------------------- */
  index++;
  QLabel* lbl = new QLabel( this, "lbl_spacer" );
  lbl->setText( "     " );
  mainMenu->insertItem( lbl, -1, index );

  /* run button -------------------------------------------------------- */
  index++;
  runButton = new QToolButton( this, "tb_rerun" );
  runButton->setIconSet( QPixmap(run_xpm) );
  runButton->setTextLabel( "&Run" );
  runButton->setTextPosition( QToolButton::BesideIcon );
  runButton->setUsesTextLabel( showToolbarLabels );
  runButton->setAutoRaise( true );
  runButton->setAccel( CTRL+Key_R );
  connect( runButton, SIGNAL( clicked() ), 
           this,      SLOT( run() ) );
  QToolTip::add( runButton, "Run valgrind with specified executable" );
  mainMenu->insertItem( runButton, -1, index );
  ContextHelp::add( runButton, urlValkyrie::Dummy );

  /* stop button ------------------------------------------------------- */
  index++;
  stopButton = new QToolButton( this, "tb_stop" );
  stopButton->setIconSet( QPixmap(stop_xpm) );
  stopButton->setTextLabel( "S&top" );
  stopButton->setTextPosition( QToolButton::BesideIcon );
  stopButton->setUsesTextLabel( showToolbarLabels );
  stopButton->setAutoRaise( true );
  stopButton->setAccel( CTRL+Key_T );
  connect( stopButton, SIGNAL( clicked() ), this,
           SLOT( stop() ) );
  QToolTip::add( stopButton, "Terminate program execution immediately" );
  mainMenu->insertItem( stopButton, -1, index );
  ContextHelp::add( stopButton, urlValkyrie::Dummy );

  index++;
  mainMenu->insertSeparator( index );

  /* help button ------------------------------------------------------- */
  index++;
  helpButton = new QToolButton( this );
  helpButton->setIconSet( QPixmap(help_xpm) );
  helpButton->setTextLabel( "&Help" );
  helpButton->setTextPosition( QToolButton::BesideIcon );
  helpButton->setUsesTextLabel( showToolbarLabels );
  helpButton->setAutoRaise( true );
  helpButton->setAccel( ALT+Key_H );
  QPopupMenu* helpMenu = new QPopupMenu( helpButton );
  helpMenu->insertItem( "Handbook", handBook, SLOT(show()), Key_F1 );
  helpMenu->insertSeparator();
  helpMenu->insertItem( "About Valkyrie", this, 
                        SLOT(showAboutInfo(int)), 0, HelpAbout::ABOUT_VK );
  helpMenu->insertItem( "About Qt", this, 
                        SLOT(showAboutInfo(int)), 0, HelpAbout::ABOUT_QT );
  helpMenu->insertSeparator();
  helpMenu->insertItem( "Licence", this, 
                        SLOT(showAboutInfo(int)), 0, HelpAbout::LICENCE );
  helpMenu->insertItem( "Support", this, 
                        SLOT(showAboutInfo(int)), 0, HelpAbout::SUPPORT );
  helpButton->setPopup( helpMenu );
  helpButton->setPopupDelay( 1 );
  QToolTip::add( helpButton, "Show help manual / information" );
  mainMenu->insertItem( helpButton, -1, index );

  /* application-wide context help button ------------------------------ */
  index++;
  QToolButton* ctxtButton = new ContextHelpButton( this, handBook );
  QToolTip::add( ctxtButton, "This is a <b>Context Help</b> button. "
           "Clicking on a widget will open the relevant manual help page");
  mainMenu->insertItem( ctxtButton, -1, index );
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
  
  flagsLabel = new QLabel( statusFrame, "flags_label" );
  top_row->addWidget( flagsLabel );
  flagsLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  flagsLabel->setPaletteBackgroundColor( Qt::white );
  flagsLabel->setAlignment( AlignLeft );
  flagsLabel->setTextFormat( Qt::PlainText );
  flagsLabel->setText( vkConfig->rdEntry( "vg-exec", "valkyrie" ) );
  flagsLabel->hide();


  /* hbox for middle row */
  QHBoxLayout* mid_row = new QHBoxLayout( 5, "middle_row" );
  statusLayout->addLayout( mid_row );

  /* frame+layout for messages */
  QFrame* msgFrame = new QFrame( statusFrame );
  msgFrame->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  mid_row->addWidget( msgFrame );
  QBoxLayout* msgLayout = new QHBoxLayout( msgFrame, 2 );
  statusMsg = new QLabel( "", msgFrame );
  statusMsg->setAlignment( AlignLeft );
  statusMsg->setTextFormat( Qt::PlainText );
  int status_height = fontMetrics().height();
  statusMsg->setFixedHeight( status_height );
  msgLayout->addWidget( statusMsg );
  ContextHelp::add( statusMsg, urlValkyrie::Dummy );


  /* hbox for the bottom row */
  QHBoxLayout* bot_row = new QHBoxLayout( 5, "bot_row" );
  statusLayout->addLayout( bot_row, 0 );

  /* the toolview buttons */
  viewButtGroup = new QButtonGroup( statusFrame );
  viewButtGroup->setExclusive( true );
  viewButtGroup->hide();
  connect( viewButtGroup, SIGNAL(clicked(int)), 
           this,          SLOT(showToolView(int)) );
  ContextHelp::add( viewButtGroup, urlValkyrie::Dummy );

  /* set the buttons to all be the same width */
  int butt_width = fontMetrics().width( "XMemcheckX" );
  ToolList tools = valkyrie->toolList();
  ToolObject* tool;
  for ( tool = tools.first(); tool; tool = tools.next() ) {
    int len = tool->accelTitle().length();
    butt_width = ( len > butt_width ) ? len : butt_width;
  }
  int butt_height = fontMetrics().height() + 12;

  QToolButton* tvButton;
  for ( tool = tools.first(); tool; tool = tools.next() ) {
    tvButton = new QToolButton( statusFrame );
    tvButton->setToggleButton( true );
    tvButton->setEnabled( true );
    tvButton->setText( tool->accelTitle() );
    tvButton->setAccel( tool->accelKey() );
    tvButton->setMinimumWidth( butt_width );
    tvButton->setFixedHeight( butt_height );
    viewButtGroup->insert( tvButton, tool->id() );
    bot_row->addWidget( tvButton );
  }
  tools.clear();

  bot_row->addStretch(1);

  /* frame+layout for the view-flags icon */
  flagsButton = new QToolButton( statusFrame );
  bot_row->addWidget( flagsButton );
  flagsButton->setToggleButton( true );
  flagsButton->setEnabled( true );
  flagsButton->setPixmap( QPixmap(view_flags_xpm) );
  connect( flagsButton, SIGNAL( toggled(bool) ), 
           this,        SLOT( showFlagsWidget(bool) ) );
  QToolTip::add( flagsButton, "Show non-default flags for current tool" );
  ContextHelp::add( flagsButton, urlValkyrie::FlagsButton );
}

