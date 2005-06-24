/* ---------------------------------------------------------------------
 * implementation of MainWindow                          main_window.cpp
 * ---------------------------------------------------------------------
 */

#include <qapplication.h>
#include <qlayout.h>
#include <qmotifstyle.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <qvbox.h>

#include "main_window.h"
#include "tb_mainwin_icons.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "vk_msgbox.h"
#include "context_help.h"
#include "html_urls.h"

#include "memcheck_view.h"
#include "cachegrind_view.h"
#include "massif_view.h"


/*
  &File:    ALT+Key_F
  O&ptions: ALT+Key_P
  &Tools:   ALT+Key_T
  &Help:    ALT+Key_H
  handBook: Key_F1

  R&estart: CTRL+Key_E
  &Run:     CTRL+Key_R
  &Save:    CTRL+Key_S
  S&top:    CTRL+Key_T
  &Close:   CTRL+Key_W
  E&xit:    CTRL+Key_X

  Memcheck    SHIFT+Key_M
  Cachegrind: SHIFT+Key_C 
  Massif:     SHIFT+Key_S
*/



/* class MainWindow ---------------------------------------------------- */
MainWindow::~MainWindow() 
{  }


MainWindow::MainWindow() : QMainWindow( 0, "mainWindow" )
{
  optionsWin = 0;
  activeView = 0;
  valkyrie   = (Valkyrie*)vkConfig->vkObject( "valkyrie" );
  vk_assert( valkyrie != 0 );

  setCaption( vkConfig->vkName() );
  setIcon( vkConfig->pixmap( "valkyrie.xpm" ) );
  statusBar()->setSizeGripEnabled( false );

  /* workspace */
  QVBox* vbox = new QVBox( this );
  vbox->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  vbox->setMargin( 1 );
  vbox->setLineWidth( 1 );
  setCentralWidget( vbox );
  wSpace = new WorkSpace( vbox );
  wSpace->setBackgroundMode( QWidget::PaletteBase );
  wSpace->setScrollBarsEnabled( true );

  /* handbook: init before menubar / toolbar */
  handBook = new HandBook();

  /* create menubar, status bar + flags widget */
  mkMenuBar();
  mkStatusBar();

  /* startup with the last tool set in config */
  showToolView( vkConfig->defaultToolId() );
}


// FIXME: this should use VkObject::ObjectId
void MainWindow::showToolView( int tvid )
{
  if ( activeView != 0 ) {
    /* already loaded and visible */
    if ( activeView->id() == tvid ) {
      return;
    } 
  }

  activeView = wSpace->findView( tvid );

  if ( activeView == 0 ) {

    // tools: MEMCHECK, CACHEGRIND, MASSIF
    VkObject* obj = vkConfig->vkObject( tvid, true );
    switch ( tvid ) {
      case VkObject::MEMCHECK:
        activeView = new MemcheckView( wSpace, obj );
        break;
      case VkObject::CACHEGRIND:
        activeView = new CachegrindView( wSpace, obj );
        break;
      case VkObject::MASSIF:
        activeView = new MassifView( wSpace, obj );
        break;
      default:
        vk_assert_never_reached();
        break;

    }

    connect( activeView, SIGNAL(running(bool)), 
             this,       SLOT(updateButtons(bool)) );
    connect( activeView, SIGNAL(message(QString)),
             this,       SLOT(setStatus(QString)) );

    /* if what-to-do was specified on the cmd-line, do it.
       otherwise, hang around and look boo'ful */
    if ( valkyrie->runMode != Valkyrie::NOT_SET && 
         vkConfig->rdEntry("tool", "valgrind") == activeView->name() ) {
      activeView->run();
    }

  }

  activeView->showMaximized();
  activeView->setFocus();
  setToggles( tvid );
}


void MainWindow::stop()
{
  /* don't come in here if there's no current view */
  if ( wSpace->numViews() == 0 )
    return;

  activeView->stop();
}


/* run executable, after clearing previous output */
void MainWindow::run()
{
  /* don't come in here if there's no current view */
  if ( wSpace->numViews() == 0 )
    return;

  /* valkyrie may have been started with no executable
     specified. if so, show prefsWindow + msgbox */
  if ( vkConfig->rdEntry("binary","valkyrie").isEmpty() ) {
    printf("TODO: showOptionsWin( VkObject::VALKYRIE );\n");
    vkInfo( this/*optionsWin*/, "Run Executable",
            "Please enter the path to the executable "
            "you wish to run, together with any arguments");
    return;
  }

  if ( !activeView->run() ) {
    vkError( this, "Run Executable",
             "Failed to start the executable running." );
  } 

}


void MainWindow::showOptionsWindow( int view_id )
{
  if ( optionsWin == NULL ) {
    optionsWin = new OptionsWindow( this );
  }
  
  optionsWin->showPage( view_id );
}


/* connected to a toolview's signal running( bool ) */
void MainWindow::updateButtons( bool tv_running )
{
  runButton->setEnabled( !tv_running );
  stopButton->setEnabled( tv_running );
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
    fileMenu->setItemEnabled( FILE_RUN,     false );
    fileMenu->setItemEnabled( FILE_STOP,    false );
    fileMenu->setItemEnabled( FILE_CLOSE,   false );

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
    bool is_running = (activeView == 0) ? false : activeView->isRunning();
    fileMenu->setItemEnabled( FILE_RUN,     !is_running );
    fileMenu->setItemEnabled( FILE_STOP,    is_running );

    runButton->setEnabled( !is_running );
    stopButton->setEnabled( is_running );

    if ( flagsButton->isOn() ) {
      showFlagsWidget( true );
    }
    flagsButton->setEnabled( true );
  }

}


void MainWindow::clearStatus()
{ statusMsg->setText( "" );  }
void MainWindow::setStatus( QString msg )
{ statusMsg->setText( msg ); }


/* shows|hides the label which contains the flags relevant to the
   current toolview */
void MainWindow::showFlagsWidget( bool show )
{
  if ( !show ) {
    flagsLabel->hide();
  } else {
    if ( activeView != 0 ) {
      QString flags = activeView->currFlags();
      flagsLabel->setText( flags );
      if ( !flagsLabel->isVisible() ) {
        flagsLabel->show();
      }
    }
  }
}


/* HelpInfo + HandBook ----------------------------------------------- */
void MainWindow::helpInfo( int id )
{
  HelpInfo::TabId tabid = (HelpInfo::TabId)id;
  HelpInfo * dlg = new HelpInfo( this, tabid );
  dlg->exec();
  delete dlg;
}


void MainWindow::resizeEvent( QResizeEvent *re )
{
  QWidget::resizeEvent( re );
#if 1
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
#if 1
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
  QPtrList<ToolView> views = wSpace->viewList();
  for ( ToolView* tView=views.first(); tView; tView=views.next() ) {
    if ( !tView->close() ) {
      ce->ignore();
      return;
    }
  }

  if ( optionsWin != 0 ) {
    delete optionsWin;
    optionsWin = 0;
  }

  if ( handBook != 0 ) { 
    handBook->save();
    delete handBook;
    handBook = 0;
  }

  QMainWindow::closeEvent( ce );
}


void MainWindow::closeToolView()
{
  /* you can never be too careful */
  if ( activeView == 0 ) return;

  /* try to deliver the coup de grace */
  if ( !activeView->close() ) return;

  /* find out who is now the active window */
  activeView = wSpace->activeView();

  int id = ( activeView == 0 ) ? -1 : activeView->id();
  setToggles( id );
}


void MainWindow::mkMenuBar()
{
  QMenuBar* mainMenu = new QMenuBar( this, "main menubar" );
  mainMenu->setStyle( new QMotifStyle() );
  bool show_text = vkConfig->rdBool( "show-butt-text", "Prefs" );
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
  //QIconSet bulletSet( QPixmap(black_bullet_xpm) );
  QPixmap bulletSet(black_bullet_xpm);
  toolsMenu = new QPopupMenu( this );

  VkObjectList objList = vkConfig->vkObjList();
  VkObject* obj;
  for ( obj = objList.first(); obj; obj = objList.next() ) {
    if ( obj->isTool() ) {
      toolsMenu->insertItem( bulletSet, obj->accelTitle(), 
                             this, SLOT( showToolView(int) ), 
                             obj->accelKey(), obj->id() );
    }
  }
  id = mainMenu->insertItem( "&Tools", toolsMenu, -1, index );
  mainMenu->setAccel( ALT+Key_T, id );
  ContextHelp::add( toolsMenu, urlValkyrie::Dummy );

  /* options / preferences et al --------------------------------------- */
  index++;
  QPopupMenu* prefsMenu = new QPopupMenu( this );

  for ( obj = objList.first(); obj; obj = objList.next() ) {
    prefsMenu->insertItem( obj->title(), this, 
                           SLOT(showOptionsWindow(int)), 0, obj->id() );
  }
  id = mainMenu->insertItem( "O&ptions", prefsMenu, -1, index );
  mainMenu->setAccel( ALT+Key_P, id );
  ContextHelp::add( prefsMenu, urlValkyrie::Dummy );

  /* run button -------------------------------------------------------- */
  index++;
  runButton = new QToolButton( this, "tb_rerun" );
  runButton->setIconSet( QPixmap(run_xpm) );
  runButton->setTextLabel( "&Run" );
  runButton->setTextPosition( QToolButton::BesideIcon );
  runButton->setUsesTextLabel( show_text );
  runButton->setAutoRaise( true );
  runButton->setAccel( CTRL+Key_R );
  connect( runButton, SIGNAL( clicked() ), 
           this,      SLOT( run() ) );
  QToolTip::add( runButton, "Run executable, after clearing output" );
  mainMenu->insertItem( runButton, -1, index );
  ContextHelp::add( runButton, urlValkyrie::Dummy );

  /* stop button ------------------------------------------------------- */
  index++;
  stopButton = new QToolButton( this, "tb_stop" );
  stopButton->setIconSet( QPixmap(stop_xpm) );
  stopButton->setTextLabel( "S&top" );
  stopButton->setTextPosition( QToolButton::BesideIcon );
  stopButton->setUsesTextLabel( show_text );
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
  helpButton->setUsesTextLabel( show_text );
  helpButton->setAutoRaise( true );
  helpButton->setAccel( ALT+Key_H );
  QPopupMenu* helpMenu = new QPopupMenu( helpButton );
  helpMenu->insertItem( "Handbook", handBook, SLOT(show()), Key_F1 );
  helpMenu->insertSeparator();
  helpMenu->insertItem( "About Valkyrie", this, 
                        SLOT(helpInfo(int)), 0, HelpInfo::ABOUT_VK );
  helpMenu->insertItem( "About Qt", this, 
                        SLOT(helpInfo(int)), 0, HelpInfo::ABOUT_QT );
  helpMenu->insertSeparator();
  helpMenu->insertItem( "Licence", this, 
                        SLOT(helpInfo(int)), 0, HelpInfo::LICENCE );
  helpMenu->insertItem( "Support", this, 
                        SLOT(helpInfo(int)), 0, HelpInfo::SUPPORT );
  helpButton->setPopup( helpMenu );
  helpButton->setPopupDelay( 1 );
  QToolTip::add( helpButton, "Show help manual / information" );
  mainMenu->insertItem( helpButton, -1, index );

  /* application-wide context help button ------------------------------ */
  index++;
  QToolButton* ctxtButton = new ContextHelpButton( this, handBook );
  QToolTip::add( ctxtButton, "This is a <b>Context Help</b> button. "
                 "It enables the user to ask for help on the screen.");
  mainMenu->insertItem( ctxtButton, -1, index );
}


/* 'status bar' with 2 rows: label to show non-default flags on top,
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
  VkObjectList objList = vkConfig->vkObjList();
  VkObject* obj;
  for ( obj = objList.first(); obj; obj = objList.next() ) {
    if ( obj->isTool() ) {
      int len = obj->accelTitle().length();
      butt_width = ( len > butt_width ) ? len : butt_width;
    }
  }

  QToolButton* tvButton;
  for ( obj = objList.first(); obj; obj = objList.next() ) {
    if ( obj->isTool() ) {
      tvButton = new QToolButton( statusFrame );
      tvButton->setToggleButton( true );
      tvButton->setEnabled( true );
      tvButton->setText( obj->accelTitle() );
      tvButton->setAccel( obj->accelKey() );
      tvButton->setMinimumWidth( butt_width );
      viewButtGroup->insert( tvButton, obj->id() );
      bot_row->addWidget( tvButton );
    }
  }

  /* frame+layout for messages */
  QFrame* msgFrame = new QFrame( statusFrame );
  msgFrame->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
  bot_row->addWidget( msgFrame, 20 );
  QBoxLayout* msgLayout = new QHBoxLayout( msgFrame, 5 );
  statusMsg = new QLabel( "", msgFrame );
  //statusMsg->setPaletteBackgroundColor( Qt::white );
  statusMsg->setAlignment( AlignLeft );
  statusMsg->setTextFormat( Qt::PlainText );
  msgLayout->addWidget( statusMsg );
  ContextHelp::add( statusMsg, urlValkyrie::Dummy );

  /* frame+layout for the view valgrind flags icon */
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
