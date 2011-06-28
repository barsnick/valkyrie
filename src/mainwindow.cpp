/****************************************************************************
** MainWindow implementation
**  - the top-level application window
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QColor>
#include <QColorGroup>
#include <QEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

#include "mainwindow.h"
#include "toolview/memcheckview.h"
#include "toolview/helgrindview.h"

#include "help/help_about.h"
#include "help/help_context.h"
#include "help/help_urls.h"
#include "options/vk_option.h"
#include "objects/tool_object.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"


/***************************************************************************/
/*!
    \class MainWindow
    \brief This provides the core QMainWindow class for the application.

    This class provides the basis for the application, and defines the
    layout which is extended upon by the ToolViews.

    The centralWidget is a ToolViewStack, which provides a stack of ToolViews.
    Multiple ToolViews (one for each valgrind tool-type) can thus be opened,
    without interfering with each other.

    Basic functionality is provided in the menus and toolbars, and this is
    extended upon by the (valgrind tool) ToolView interfaces.

    \sa ToolViewStack, ToolView
*/

/*!
    Constructs a MainWindow with the given parent.
*/
MainWindow::MainWindow( Valkyrie* vk )
   : QMainWindow(),
     valkyrie( vk ), toolViewStack( 0 ), statusLabel( 0 ),
     handBook( 0 ), optionsDialog( 0 )
{
   setObjectName( QString::fromUtf8( "MainWindowClass" ) );
   QString title = VkCfg::appName();
   title.replace( 0, 1, title[0].toUpper() );
   setWindowTitle( title );
   
   lastAppFont = qApp->font();
   lastPalette = qApp->palette();
   
   QIcon icon_vk;
   icon_vk.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/valkyrie.xpm" ) ) );
   setWindowIcon( icon_vk );
   
   // handbook: init before menubar / toolbar
   handBook = new HandBook();
   
   // interface setup
   setupLayout();
   setupActions();
   setupMenus();
   setupToolBars();
   setupStatusBar();
   
   // functions for dealing with config updates
   VkOption* opt = valkyrie->getOption( VALKYRIE::ICONTXT );
   connect( opt, SIGNAL( valueChanged() ), this, SLOT( showLabels() ) );
   opt = valkyrie->getOption( VALKYRIE::TOOLTIP );
   connect( opt, SIGNAL( valueChanged() ), this, SLOT( showToolTips() ) );
   opt = valkyrie->getOption( VALKYRIE::FNT_GEN_SYS );
   connect( opt, SIGNAL( valueChanged() ), this, SLOT( setGenFont() ) );
   opt = valkyrie->getOption( VALKYRIE::FNT_GEN_USR );
   connect( opt, SIGNAL( valueChanged() ), this, SLOT( setGenFont() ) );
   opt = valkyrie->getOption( VALKYRIE::FNT_TOOL_USR );
   connect( opt, SIGNAL( valueChanged() ), this, SLOT( setToolFont() ) );
   opt = valkyrie->getOption( VALKYRIE::PALETTE );
   connect( opt, SIGNAL( valueChanged() ), this, SLOT( setPalette() ) );
   
   showLabels();
   showToolTips();
   setGenFont();
   setToolFont();
   setPalette();
   
   updateEventFilters( this );
   updateEventFilters( handBook );

#if 0
   // CAB: Handy shortcut for testing: load last project
   QString proj_fname = actFile_RecentProjs[0]->data().toString();
   vkCfgProj->openProject( proj_fname );
   setCurrentProject( proj_fname );
#endif
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
MainWindow::~MainWindow()
{
   // cleanup toolviews
   delete toolViewStack;
   
   // Save window position to config.
   vkCfgGlbl->setValue( "mainwindow_size", size() );
   vkCfgGlbl->setValue( "mainwindow_pos", pos() );
   vkCfgGlbl->sync();
   
   // handbook has no parent, so have to delete it.
   delete handBook;
   handBook = 0;
}


/*!
   Allow a tool to insert a menu into the main menuBar.
   A tool can't do this directly via it's parent pointer, as it needs access
   to private vars for positioning within the menu.

   \sa removeToolMenuAction()
*/
void MainWindow::insertToolMenuAction( QAction* action )
{
   menuBar->insertAction( menuHelp->menuAction(), action );
}


/*!
    Allow a tool to remove a (previously inserted) menu from the main menuBar.
    A tool could do this directly via 'parent', but insert can't be
    done directly, so use this function for consistency.

    \sa insertToolMenuAction()
*/
void MainWindow::removeToolMenuAction( QAction* action )
{
   menuBar->removeAction( action );
}


/*!
    Setup the basic interface layout.
*/
void MainWindow::setupLayout()
{
   resize( vkCfgGlbl->value( "mainwindow_size", QSize( 600, 600 ) ).toSize() );
   move( vkCfgGlbl->value( "mainwindow_pos", QPoint( 400, 0 ) ).toPoint() );
   
   toolViewStack  = new ToolViewStack( this );
   setCentralWidget( toolViewStack );
}


/*!
    Setup the top-level actions.
*/
void MainWindow::setupActions()
{
   // TODO: shortcuts
   // act->setShortcut( tr("Ctrl+XXX") );
   
   actFile_NewProj = new QAction( this );
   actFile_NewProj->setObjectName( QString::fromUtf8( "actFile_NewProj" ) );
   actFile_NewProj->setText( tr( "&New Project..." ) );
   actFile_NewProj->setToolTip( tr( "Create a project to save your configuration" ) );
   QIcon icon_newproj;
   icon_newproj.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/filenew.png" ) ) );
   actFile_NewProj->setIcon( icon_newproj );
   actFile_NewProj->setIconVisibleInMenu( true );
   connect( actFile_NewProj, SIGNAL( triggered() ), this, SLOT( createNewProject() ) );
   
   actFile_OpenProj = new QAction( this );
   actFile_OpenProj->setObjectName( QString::fromUtf8( "actFile_OpenProj" ) );
   actFile_OpenProj->setText( tr( "&Open Project..." ) );
   actFile_OpenProj->setToolTip( tr( "Open an existing project to load a saved configuration" ) );
   QIcon icon_openproj;
   icon_openproj.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/folder_blue.png" ) ) );
   actFile_OpenProj->setIcon( icon_openproj );
   actFile_OpenProj->setIconVisibleInMenu( true );
   connect( actFile_OpenProj, SIGNAL( triggered() ), this, SLOT( openProject() ) );
   
   for (int i = 0; i < MaxRecentProjs; ++i) {
      actFile_RecentProjs[i] = new QAction(this);
      actFile_RecentProjs[i]->setVisible(false);
      connect(actFile_RecentProjs[i], SIGNAL(triggered()), this, SLOT( openRecentProject() ));
   }

   actFile_SaveAs = new QAction( this );
   actFile_SaveAs->setObjectName( QString::fromUtf8( "actFile_SaveAs" ) );
   actFile_SaveAs->setText( tr( "Save &As..." ) );
   actFile_SaveAs->setToolTip( tr( "Save current configuration to a new project" ) );
   QIcon icon_saveas;
   icon_saveas.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/filesaveas.png" ) ) );
   actFile_SaveAs->setIcon( icon_saveas );
   actFile_SaveAs->setIconVisibleInMenu( true );
   connect( actFile_SaveAs, SIGNAL( triggered() ), this, SLOT( saveAsProject() ) );

   actFile_Close = new QAction( this );
   actFile_Close->setObjectName( QString::fromUtf8( "actFile_Close" ) );
   actFile_Close->setToolTip( tr( "Close the currently active tool" ) );
   actFile_Close->setText( tr( "&Close Tool" ) );
   connect( actFile_Close, SIGNAL( triggered() ), this, SLOT( closeToolView() ) );
   
   actFile_Exit = new QAction( this );
   actFile_Exit->setObjectName( QString::fromUtf8( "actFile_Exit" ) );
   actFile_Exit->setText( tr( "E&xit" ) );
   actFile_Exit->setToolTip( tr( "Exit Valkyrie" ) );
   QIcon icon_exit;
   icon_exit.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/exit.png" ) ) );
   actFile_Exit->setIcon( icon_exit );
   actFile_Exit->setIconVisibleInMenu( true );
   connect( actFile_Exit, SIGNAL( triggered() ), qApp, SLOT( closeAllWindows() ) );
   
   actEdit_Options = new QAction( this );
   actEdit_Options->setObjectName( QString::fromUtf8( "actEdit_Options" ) );
   actEdit_Options->setText( tr( "O&ptions" ) );
   actEdit_Options->setToolTip( tr( "Open the options-editing window" ) );
   QIcon icon_options;
   icon_options.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/gear.png" ) ) );
   actEdit_Options->setIcon( icon_options );
   actEdit_Options->setIconVisibleInMenu( true );
   connect( actEdit_Options, SIGNAL( triggered() ), this, SLOT( openOptions() ) );
   
   actProcess_Run = new QAction( this );
   actProcess_Run->setObjectName( QString::fromUtf8( "actProcess_Run" ) );
   actProcess_Run->setText( tr( "&Run" ) );
   actProcess_Run->setToolTip( tr( "Run Valgrind with the currently active tool" ) );
   actProcess_Run->setShortcut( QString::fromUtf8( "Ctrl+R" ) );
   QIcon icon_run;
   icon_run.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/valgrind_run.png" ) ) );
   actProcess_Run->setIcon( icon_run );
   actProcess_Run->setIconVisibleInMenu( true );
   connect( actProcess_Run, SIGNAL( triggered() ), this, SLOT( runValgrind() ) );
   
   actProcess_Stop = new QAction( this );
   actProcess_Stop->setObjectName( QString::fromUtf8( "actProcess_Stop" ) );
   actProcess_Stop->setText( tr( "S&top" ) );
   actProcess_Stop->setToolTip( tr( "Stop Valgrind" ) );
   QIcon icon_stop;
   icon_stop.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/valgrind_stop.png" ) ) );
   actProcess_Stop->setIcon( icon_stop );
   actProcess_Stop->setIconVisibleInMenu( true );
   connect( actProcess_Stop, SIGNAL( triggered() ), this, SLOT( stopTool() ) );
   
   actHelp_Handbook = new QAction( this );
   actHelp_Handbook->setObjectName( QString::fromUtf8( "actHelp_Handbook" ) );
   actHelp_Handbook->setText( tr( "Handbook" ) );
   actHelp_Handbook->setToolTip( tr( "Open the Valkyrie Handbook" ) );
   actHelp_Handbook->setShortcut( QString::fromUtf8( "F1" ) );
   QIcon icon_handbook;
   icon_handbook.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_handbook_help.xpm" ) ) );
   actHelp_Handbook->setIcon( icon_handbook );
   actHelp_Handbook->setIconVisibleInMenu( true );
   connect( actHelp_Handbook, SIGNAL( triggered() ), this, SLOT( openHandBook() ) );
   
   actHelp_About_Valkyrie = new QAction( this );
   actHelp_About_Valkyrie->setObjectName( QString::fromUtf8( "actHelp_About_Valkyrie" ) );
   actHelp_About_Valkyrie->setText( tr( "About Valkyrie" ) );
   //   actHelp_About_Valkyrie->setMenuRole( QAction::AboutRole );
   connect( actHelp_About_Valkyrie, SIGNAL( triggered() ), this, SLOT( openAboutVk() ) );
   
   actHelp_About_Qt = new QAction( this );
   actHelp_About_Qt->setObjectName( QString::fromUtf8( "actHelp_About_Qt" ) );
   actHelp_About_Qt->setText( tr( "About Qt" ) );
   //   actHelp_About_Qt->setMenuRole( QAction::AboutQtRole );
   connect( actHelp_About_Qt, SIGNAL( triggered() ), qApp, SLOT( aboutQt() ) );
   
   actHelp_License = new QAction( this );
   actHelp_License->setObjectName( QString::fromUtf8( "actHelp_License" ) );
   actHelp_License->setText( tr( "License" ) );
   connect( actHelp_License, SIGNAL( triggered() ), this, SLOT( openAboutLicense() ) );
   
   actHelp_Support = new QAction( this );
   actHelp_Support->setObjectName( QString::fromUtf8( "actHelp_Support" ) );
   actHelp_Support->setText( tr( "Support" ) );
   connect( actHelp_Support, SIGNAL( triggered() ), this, SLOT( openAboutSupport() ) );
   
   
   // ------------------------------------------------------------
   // Tool actions - exclusive selection group
   toolActionGroup = new QActionGroup( this );
   //TODO: if we make it exclusive, then we can't close all toolviews and have none selected... can we?
   //toolActionGroup->setExclusive( true );
   connect( toolActionGroup, SIGNAL( triggered( QAction* ) ),
            this,              SLOT( toolGroupTriggered( QAction* ) ) );
   
   QIcon icon_bullet;
   icon_bullet.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_mainwin_blackbullet.xpm" ) ) );
   
   ToolObjList tools = valkyrie->valgrind()->getToolObjList();
   vk_assert( tools.size() > 0 );
   foreach( ToolObject * tool, tools ) {
      QString toolname = tool->objectName();
      toolname[0] = toolname[0].toUpper();
      
      QAction* actTool = new QAction( this );
      actTool->setObjectName( "actTool_" + toolname );
      actTool->setCheckable( true );
      actTool->setIcon( icon_bullet );
      actTool->setIconVisibleInMenu( true );
      actTool->setText( toolname );
      actTool->setProperty( "toolId", tool->getToolId() );
      
      toolActionGroup->addAction( actTool );
   }
   
   // ------------------------------------------------------------
   // initial enables/disables
   updateVgButtons( false );
}


/*!
    Setup the main menus
*/
void MainWindow::setupMenus()
{
   // ------------------------------------------------------------
   // Basic menu setup
   menuBar = new QMenuBar( this );
   menuBar->setObjectName( QString::fromUtf8( "menuBar" ) );
   menuBar->setGeometry( QRect( 0, 0, 496, 25 ) );
   this->setMenuBar( menuBar );
   
   menuFile = new QMenu( menuBar );
   menuFile->setObjectName( QString::fromUtf8( "menuFile" ) );
   menuFile->setTitle( tr( "&File" ) );
   menuRecentProjs = new QMenu( menuBar );
   menuRecentProjs->setObjectName( QString::fromUtf8( "menuRecentProjs" ) );
   menuRecentProjs->setTitle( tr( "Recent Projects" ) );
   menuEdit = new QMenu( menuBar );
   menuEdit->setObjectName( QString::fromUtf8( "menuEdit" ) );
   menuEdit->setTitle( tr( "&Edit" ) );
   menuProcess = new QMenu( menuBar );
   menuProcess->setObjectName( QString::fromUtf8( "menuProcess" ) );
   menuProcess->setTitle( tr( "&Process" ) );
   menuTools = new QMenu( menuBar );
   menuTools->setObjectName( QString::fromUtf8( "menuTools" ) );
   menuTools->setTitle( tr( "&Tools" ) );
   menuHelp = new QMenu( menuBar );
   menuHelp->setObjectName( QString::fromUtf8( "menuHelp" ) );
   menuHelp->setTitle( tr( "Help" ) );
   
   // application-wide context help button
   ContextHelpAction* ctxtHlpAction = new ContextHelpAction( this, handBook );
   ctxtHlpAction->setText( tr( "Context Help" ) );
   
   
   // ------------------------------------------------------------
   // Add actions to menus
   menuBar->addAction( menuFile->menuAction() );
   menuBar->addAction( menuEdit->menuAction() );
   menuBar->addAction( menuProcess->menuAction() );
   menuBar->addAction( menuTools->menuAction() );
   menuBar->addAction( menuHelp->menuAction() );
   
   menuFile->addAction( actFile_NewProj );
   menuFile->addAction( actFile_OpenProj );
   menuFile->addMenu( menuRecentProjs );
   menuFile->addAction( actFile_SaveAs );
   menuFile->addSeparator();
   menuFile->addAction( actFile_Close );
   menuFile->addSeparator();
   menuFile->addAction( actFile_Exit );

   for (int i = 0; i < MaxRecentProjs; ++i) {
      menuRecentProjs->addAction( actFile_RecentProjs[i]);
   }
   updateActionsRecentProjs();
   
   menuEdit->addAction( actEdit_Options );
   
   menuProcess->addAction( actProcess_Run );
   menuProcess->addAction( actProcess_Stop );
   
   foreach( QAction * actTool, toolActionGroup->actions() )
   menuTools->addAction( actTool );
   
   menuHelp->addAction( ctxtHlpAction );
   menuHelp->addSeparator();
   menuHelp->addAction( actHelp_Handbook );
   menuHelp->addSeparator();
   menuHelp->addAction( actHelp_About_Valkyrie );
   menuHelp->addAction( actHelp_About_Qt );
   menuHelp->addSeparator();
   menuHelp->addAction( actHelp_License );
   menuHelp->addAction( actHelp_Support );
}


/*!
    Setup the main toolbars
*/
void MainWindow::setupToolBars()
{
   // ------------------------------------------------------------
   // Basic toolbar setup
   mainToolBar = new QToolBar( this );
   mainToolBar->setObjectName( QString::fromUtf8( "mainToolBar" ) );
   this->addToolBar( Qt::TopToolBarArea, mainToolBar );
   
   // ------------------------------------------------------------
   // Add actions to toolbar
   mainToolBar->addAction( actProcess_Run );
   mainToolBar->addAction( actProcess_Stop );
   
   // Ensures further toolbars are added underneath.
   // TODO: hmm. if add & remove & add toolbars, the toolbar gets added to the side, not under.
   // addToolBarBreak();
}


/*!
    Setup the bottom status bar.
    This shows status messages from all interfaces: both the main interface
    and tool-specific interfaces
*/
void MainWindow::setupStatusBar()
{
   // ------------------------------------------------------------
   // Basic statusbar setup
   mainStatusBar = this->statusBar();
   mainStatusBar->setObjectName( QString::fromUtf8( "mainStatusBar " ) );

#if 0 //TODO: make use of this...
   QLabel* permanentLabel = new QLabel( mainStatusBar );
   permanentLabel->setObjectName( QString::fromUtf8( "permanentLabel " ) );
#endif
   
   statusLabel = new QLabel( mainStatusBar );
   statusLabel->setObjectName( QString::fromUtf8( "statusLabel " ) );
   
   mainStatusBar->addWidget( statusLabel, 1 );
//   mainStatusBar->addPermanentWidget( permanentLabel, 0 );
}


/*!
  Install our eventFilter for given widgets and all its children.
  Just to support tooltips suppression - apparently the only way :-(
*/
void MainWindow::updateEventFilters( QObject* obj )
{
   foreach( QObject * obj, obj->children() ) {
      obj->installEventFilter( this );
      updateEventFilters( obj );
   }
}


/*!
  EventFilter: installed for all widgets.
  If configure option given, suppress all ToolTips
*/
bool MainWindow::eventFilter( QObject* obj, QEvent* e )
{
   if ( !fShowToolTips && e->type() == QEvent::ToolTip ) {
      // eat tooltip event
      return true;
   }
   else {
      // pass the event on to the parent class
      return QMainWindow::eventFilter( obj, e );
   }
}


/*!
    Show a requested ToolView, from the given \a toolId.

    The ToolViews are created and shown on demand.
*/
void MainWindow::showToolView( VGTOOL::ToolID toolId )
{
   vk_assert( toolId > VGTOOL::ID_NULL );
   
   if ( toolViewStack->currentToolId() == toolId ) {
      // already loaded and visible.
      return;
   }
   
   // else: toolview may still be loaded, but not visible...
   
   ToolView* nextView = toolViewStack->findView( toolId );
   
   if ( nextView == 0 ) {
      // toolview not loaded => load it.
      
      // set up next view
      ToolObject* nextTool = valkyrie->valgrind()->getToolObj( toolId );
      vk_assert( nextTool != 0 );
      
      // Factory Method to create views
      nextView = nextTool->createView( this );
      vk_assert( nextView != 0 );
      
      connect( nextTool, SIGNAL( running( bool ) ),
               this,       SLOT( updateVgButtons( bool ) ) );
      connect( nextTool, SIGNAL( message( QString ) ),
               statusLabel,       SLOT( setText( QString ) ) );
               
      // Set a vg logfile. Loading done by tool_object
      connect( nextView, SIGNAL( logFileChosen( QString ) ),
               this,       SLOT( setLogFile( QString ) ) );
               
      //TODO: perhaps bring ToolObject::fileSaveDialog() here too...
      // + ToolObject::saveParsedOutput()...
      
      // view starts tool processes via this signal
      connect( nextView, SIGNAL( run( VGTOOL::ToolProcessId ) ),
               this,       SLOT( runTool( VGTOOL::ToolProcessId ) ) );
               
      // add view to the stack
      toolViewStack->addView( nextView );
      
      // widgets (menubar, toolbar) have been added: update event filters
      updateEventFilters( this );
   }
   
   // make sure the toolview is made visible:
   
   toolViewStack->raiseView( nextView );
   setToggles( toolId );
}


/*!
  slot, connected to a tool object's signal running(bool)
*/
void MainWindow::updateVgButtons( bool running )
{
   actProcess_Run->setEnabled( !running );
   actProcess_Stop->setEnabled( running );
}


/*!
    Toggles the various actions depending on the current toolview and internal state.

    For example, if a ToolView is closed, and another on the ToolViewStack
    is brought forward, the various actions must be updated to correspond
    with the state of the new ToolView.
*/
void MainWindow::setToggles( VGTOOL::ToolID toolId )
{
   if ( toolId  == VGTOOL::ID_NULL ) {
      // no more tool views
      
      // disable all actions in the tool actiongroup
      // TODO: nicer way to do this? - maybe connect sig/slot toolview to action?
      foreach( QAction * actTool, toolActionGroup->actions() )
      actTool->setChecked( false );
      
      actFile_Close->setEnabled( false );
      actProcess_Run->setEnabled( false );
      actProcess_Stop->setEnabled( false );
   }
   else {
      // at least one toolview found: update state
      
      // enable the relevant action in the tool actiongroup
      // TODO: nicer way to do this? - maybe connect sig/slot toolview to action?
      foreach( QAction * actTool, toolActionGroup->actions() ) {
         VGTOOL::ToolID toolId_action =
            ( VGTOOL::ToolID )actTool->property( "toolId" ).toInt();
            
         if ( toolId_action == toolId ) {
            actTool->setChecked( true );
         }
         else {
            actTool->setChecked( false );
         }
         
         //TODO: review toggle functionality.
      }
      
      actFile_Close->setEnabled( true );
      updateVgButtons( false );
   }
}


void MainWindow::showLabels()
{
   VkOption* opt = valkyrie->getOption( VALKYRIE::ICONTXT );
   bool show = vkCfgProj->value( opt->configKey() ).toBool();
   
   if ( show ) {
      setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
   }
   else {
      setToolButtonStyle( Qt::ToolButtonIconOnly );
   }
}


void MainWindow::showToolTips()
{
   VkOption* opt = valkyrie->getOption( VALKYRIE::TOOLTIP );
   fShowToolTips = vkCfgProj->value( opt->configKey() ).toBool();
}


void MainWindow::setGenFont()
{
   // TODO: qApp->setFont will be called twice if FNT_GEN_USR && FNT_GEN_SYS
   // are both modified together - do we care?
   
   QFont fnt;
   VkOption* opt = valkyrie->getOption( VALKYRIE::FNT_GEN_SYS );
   bool useVkSysFont = vkCfgProj->value( opt->configKey() ).toBool();
   
   if ( useVkSysFont ) {
      fnt = lastAppFont;
   }
   else {
      lastAppFont = qApp->font();
      VkOption* opt = valkyrie->getOption( VALKYRIE::FNT_GEN_USR );
      QString str_fnt = vkCfgProj->value( opt->configKey() ).toString();
      fnt.fromString( str_fnt );
   }
   
   if ( qApp->font() != fnt ) {
      qApp->setFont( fnt );
   }
}

void MainWindow::setToolFont()
{
   QFont fnt;
   VkOption* opt = valkyrie->getOption( VALKYRIE::FNT_TOOL_USR );
   QString str = vkCfgProj->value( opt->configKey() ).toString();
   fnt.fromString( str );
   
   // set font for all tool views
   foreach( ToolObject * tool, valkyrie->valgrind()->getToolObjList() ) {
      ToolView* tv = tool->view();
      
      if ( tv != NULL ) {
         tv->setToolFont( fnt );
      }
   }
}

void MainWindow::setPalette()
{
   QPalette pal;
   VkOption* opt = valkyrie->getOption( VALKYRIE::PALETTE );
   bool useVkPalette = vkCfgProj->value( opt->configKey() ).toBool();
   
   if ( !useVkPalette ) {
      pal = lastPalette;
   }
   else {
      lastPalette = qApp->palette();
      
      QColor bg     = vkCfgGlbl->value( "colour_background" ).value<QColor>();
      QColor base   = vkCfgGlbl->value( "colour_base"       ).value<QColor>();
      QColor text   = vkCfgGlbl->value( "colour_text"       ).value<QColor>();
      QColor dkgray = vkCfgGlbl->value( "colour_dkgray"     ).value<QColor>();
      QColor hilite = vkCfgGlbl->value( "colour_highlight"  ).value<QColor>();
      
      // anything not ok -> return default qApp palette:
      if ( bg.isValid() && base.isValid() && text.isValid() &&
           dkgray.isValid() && hilite.isValid() ) {
           
         pal = QPalette( bg, bg );
         // 3 colour groups: active, inactive, disabled
         // bg colour for text entry widgets
         pal.setColor( QPalette::Active,   QPalette::Base, base );
         pal.setColor( QPalette::Inactive, QPalette::Base, base );
         pal.setColor( QPalette::Disabled, QPalette::Base, base );
         // general bg colour
         pal.setColor( QPalette::Active,   QPalette::Window, bg );
         pal.setColor( QPalette::Inactive, QPalette::Window, bg );
         pal.setColor( QPalette::Disabled, QPalette::Window, bg );
         // same as bg
         pal.setColor( QPalette::Active,   QPalette::Button, bg );
         pal.setColor( QPalette::Inactive, QPalette::Button, bg );
         pal.setColor( QPalette::Disabled, QPalette::Button, bg );
         // general fg colour - same as Text
         pal.setColor( QPalette::Active,   QPalette::WindowText, text );
         pal.setColor( QPalette::Inactive, QPalette::WindowText, text );
         pal.setColor( QPalette::Disabled, QPalette::WindowText, dkgray );
         // same as fg
         pal.setColor( QPalette::Active,   QPalette::Text, text );
         pal.setColor( QPalette::Inactive, QPalette::Text, text );
         pal.setColor( QPalette::Disabled, QPalette::Text, dkgray );
         // same as text and fg
         pal.setColor( QPalette::Active,   QPalette::ButtonText, text );
         pal.setColor( QPalette::Inactive, QPalette::ButtonText, text );
         pal.setColor( QPalette::Disabled, QPalette::ButtonText, dkgray );
         // highlight
         pal.setColor( QPalette::Active,   QPalette::Highlight, hilite );
         pal.setColor( QPalette::Inactive, QPalette::Highlight, hilite );
         pal.setColor( QPalette::Disabled, QPalette::Highlight, hilite );
         // contrast with highlight
         pal.setColor( QPalette::Active,   QPalette::HighlightedText, base );
         pal.setColor( QPalette::Inactive, QPalette::HighlightedText, base );
         pal.setColor( QPalette::Disabled, QPalette::HighlightedText, base );
      }
   }
   
   if ( qApp->palette() != pal ) {
      qApp->setPalette( pal );
   }
}


/*!
  Set logfile name to be loaded
  Used from toolviews 'cos no access to vk from there
  Necessary to set cfg as does cmd line --view-cfg...
  TODO: this is just plain nasty...
*/
void MainWindow::setLogFile( QString logFilename )
{
   VkOption* opt = valkyrie->getOption( VALKYRIE::VIEW_LOG );
   opt->updateConfig( logFilename );
}


/*!
    Create a new project based on the default-project settings.
    Save configuration in a new project file.
    Previous settings are discarded.
*/
void MainWindow::createNewProject()
{
   // TODO: put dir & name choice in one dialog.
   
   // Choose project directory
   QString dir = QFileDialog::getExistingDirectory( this, "Choose Project Directory", "./",
                 QFileDialog::ShowDirsOnly |
                 QFileDialog::DontResolveSymlinks );
                 
   if ( dir.isEmpty() || dir.isNull() ) {
      return;
   }
   
   // Choose project name
   QString proj_name;
   
   while ( true ) {
      bool ok = true;
      proj_name = QInputDialog::getText( this, "Choose New Project Name",
                                         "Project Name:", QLineEdit::Normal,
                                         "", &ok );
                                         
      if ( !ok ) {                  // User chaged their minds.
         return;
      }
      
      if ( !proj_name.isEmpty() ) { // loop if empty name.
         break;
      }
   }
   
   // TODO: check if exists, may overwrite, etc...

   QString proj_fname = dir + "/" + proj_name + "." + VkCfg::filetype();
   vkCfgProj->createNewProject( proj_fname );

   // TODO: ok/cancel?

   setCurrentProject( proj_fname );
}


/*!
    Open an existing project.
*/
void MainWindow::openProject()
{
   QString filter = QString("*.") + VkCfg::filetype();
   QString proj_fname = QFileDialog::getOpenFileName( this, "Open Valkyrie Project",
                           "./", "Valkyrie Projects (" + filter + ")" );
                           
   if ( proj_fname.isEmpty() || proj_fname.isNull() ) {
      // Cancelled
      return;
   }
   
   vkCfgProj->openProject( proj_fname );

   // TODO: ok/cancel?

   setCurrentProject( proj_fname );
}


/*!
    Open a recent existing project.
*/
void MainWindow::openRecentProject()
{
   QAction *action = qobject_cast<QAction *>(sender());
   if (action) {
      QString proj_fname = action->data().toString();
      vkCfgProj->openProject( proj_fname );

      // TODO: ok/cancel?

      setCurrentProject( proj_fname );
   }
}


/*!
    Save current settings to a new project.
*/
void MainWindow::saveAsProject()
{
   // TODO: put dir & name choice in one dialog.

   // Choose project directory
   QString dir = QFileDialog::getExistingDirectory( this, "Choose Project Directory", "./",
                 QFileDialog::ShowDirsOnly |
                 QFileDialog::DontResolveSymlinks );

   if ( dir.isEmpty() || dir.isNull() ) {
      return;
   }

   // Choose project name
   QString proj_name;

   while ( true ) {
      bool ok = true;
      proj_name = QInputDialog::getText( this, "Enter Project Name",
                                         "Project Name:", QLineEdit::Normal,
                                         "", &ok );

      if ( !ok ) {                  // User chaged their minds.
         return;
      }

      if ( !proj_name.isEmpty() ) { // loop if empty name.
         break;
      }
   }


   // TODO: check if exists may overwrite, etc...
   QString proj_fname = dir + "/" + proj_name + "." + VkCfg::filetype();
   vkCfgProj->saveProjectAs( proj_fname );

   // TODO: ok/cancel?

   setCurrentProject( proj_fname );
}


/*!
    Set current project
     - update the config with the new project name
     - update the actions to keep in sync.
*/
void MainWindow::setCurrentProject(const QString &projName)
{
#if 0 // TODO: maybe future, but then do consistently everywhere...
   curFile = fileName;
   if (curFile.isEmpty())
      setWindowTitle(tr("Recent Files"));
   else
      setWindowTitle(tr("%1 - %2").arg(strippedName(curFile))
                     .arg(tr("Recent Files")));
#endif

   QStringList files = vkCfgGlbl->value( "recent_projects" )
                       .toString().split( VkCfg::sepChar(), QString::SkipEmptyParts );
   files.removeAll( projName );
   files.prepend( projName );
   while (files.size() > MaxRecentProjs) {
      files.removeLast();
   }

   vkCfgGlbl->setValue( "recent_projects", files.join( VkCfg::sepChar() ) );
   vkCfgGlbl->sync();

   updateActionsRecentProjs();
}


/*!
  Update the actions for the list of recent projects
*/
void MainWindow::updateActionsRecentProjs()
{
   QStringList files = vkCfgGlbl->value( "recent_projects" )
                       .toString().split( VkCfg::sepChar(), QString::SkipEmptyParts );
   int numRecentProjs = qMin(files.size(), (int)MaxRecentProjs);

   for (int i = 0; i < numRecentProjs; ++i) {
      QString text = QFileInfo( files[i] ).fileName();
      actFile_RecentProjs[i]->setText(text);
      actFile_RecentProjs[i]->setData(files[i]);
      actFile_RecentProjs[i]->setVisible(true);
   }
   for (int j = numRecentProjs; j < MaxRecentProjs; ++j) {
      actFile_RecentProjs[j]->setVisible(false);
   }
}


/*!
    Close the currently-shown ToolView.

    Calls the ToolViewStack to remove the toolview from the stack,
    and clean up after it (update the menus, etc).
*/
void MainWindow::closeToolView()
{
   ToolView* tv = toolViewStack->currentView();
   
   // if there ain't no toolview, we cain't do much
   if ( tv == 0 ) {
      vkPrintErr( "MainWindow::closeToolView(): No toolview. "
                  "This shouldn't happen!" );
      return;
   }
   
   vkDebug( "MainWindow::closeToolView(): %s", qPrintable( tv->objectName() ) );
        
   // last process might not be done ...
   if ( !valkyrie->queryToolDone( toolViewStack->currentToolId() ) ) {
      vkPrintErr( "Warning: Last process not finished" );
      return;
   }

   toolViewStack->removeView( tv );
   
   // current toolview will now have changed (maybe to NULL)
   setToggles( toolViewStack->currentToolId() );
}



/*!
    Calls showToolView() for a chosen valgrind tool.

    This slot is called by a trigger of the tool actionGroup, which is
    setup for the "Tools" menu.
    We make use of the QAction::property to store the toolId,
    and when one of the tools is selected, this id is used to call up
    the corresponding ToolView on the TooObject.
*/
void MainWindow::toolGroupTriggered( QAction* action )
{
   VGTOOL::ToolID toolId = ( VGTOOL::ToolID )action->property( "toolId" ).toInt();
   showToolView( toolId );
}


/*!
    Open the Options dialog.
*/
void MainWindow::openOptions()
{
   // TODO: decide whether really want a modeless dialog: is this functionality useful/used?
   // if non-modal, rem to reinit all opt widgets when a project is created/opened
#if 0
   if ( !optionsDialog ) {
      optionsDialog = new VkOptionsDialog( this );
   }
   
   optionsDialog->show();
   optionsDialog->raise();
   optionsDialog->activateWindow();
#else
   VkOptionsDialog optionsDlg( this );
   
   updateEventFilters( &optionsDlg );
   
   optionsDlg.exec();
#endif
}


/*!
    Run the valgrind tool process.
*/
void MainWindow::runTool( VGTOOL::ToolProcessId procId )
{
   VGTOOL::ToolID tId = toolViewStack->currentToolId();   

   vk_assert( procId > VGTOOL::PROC_NONE );

   // don't come in here if there's no current view
   if ( !toolViewStack->isVisible() ) {
      //This should never happen... assert?
      vkPrintErr( "Error: No toolview visible (procId=%d)."
                  "This shouldn't happen!", procId );
      return;
   }
   
   if ( procId == VGTOOL::PROC_VALGRIND ) {
      // Valkyrie may have been started with no executable
      // specified. If so, show msgbox, then options dialog
      if ( vkCfgProj->value( "valkyrie/binary" ).toString().isEmpty() ) {

         vkInfo( this, "Run Valgrind: No program specified",
                 "Please specify (via Options->Valkyrie->Binary)<br>"
                 "the path to the program you wish to run, along<br>"
                 "with any arguments required" );
         openOptions();

         return;
      }
   }

   // last process might not be done ...
   if ( !valkyrie->queryToolDone( tId ) ) {
      vkPrintErr( "Warning: Last process not finished" );
      return;
   }
   
   if ( !valkyrie->runTool( tId, procId ) ) {
      //TODO: make sure all fail cases have given a message to the user already
      
      VK_DEBUG( "Failed to complete execution for toolId (%d), procId (%d)",
                tId, procId );
   }
   
}

/*!
  run valgrind --tool=<current_tool> + flags + executable
*/
void MainWindow::runValgrind()
{
   runTool( VGTOOL::PROC_VALGRIND );
}



/*!
    Stop the valgrind tool process.
*/
void MainWindow::stopTool()
{
   valkyrie->stopTool( toolViewStack->currentToolId() );
}


/*!
    Open the application handbook.
*/
void MainWindow::openHandBook()
{
   handBook->showYourself();
}


/*!
    Open the application About dialog.
*/
void MainWindow::openAboutVk()
{
   HelpAbout dlg( this, HELPABOUT::ABOUT_VK );
   dlg.exec();
}


/*!
    Open the About-License dialog.
*/
void MainWindow::openAboutLicense()
{
   HelpAbout dlg( this, HELPABOUT::LICENSE );
   dlg.exec();
}


/*!
    Open the About-Support dialog.
*/
void MainWindow::openAboutSupport()
{
   HelpAbout dlg( this, HELPABOUT::SUPPORT );
   dlg.exec();
}
