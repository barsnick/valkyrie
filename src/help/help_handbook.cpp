/****************************************************************************
** HandBook implementation
**  - context-sensitive help browser
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

#include "help/help_handbook.h"
#include "utils/vk_config.h"
//#include "vk_messages.h"
#include "utils/vk_utils.h"


#include "QApplication"
#include "QDockWidget"
#include "QFile"
#include "QFileDialog"
#include "QMenu"
#include "QPrinter"
#include "QProcess"
#include "QString"
#include "QStringList"
#include "QTextStream"
#include "QToolBar"



/*!
  class HandBook
*/
HandBook::~HandBook()
{
   // save history + bookmarks
   save();
}


HandBook::HandBook( QWidget* parent ) //, const char* name )
   : QMainWindow( parent )
   //TODO: ?
   //, name, WDestructiveClose )
   //? Qt::WA_DeleteOnClose
{
   setObjectName( QString::fromUtf8( "handbook" ) );
   
   QString VkName = vkConfig->vkName;
   VkName.replace( 0, VkName[0].toUpper() );
   
   caption = VkName + " HandBook";
   this->setWindowTitle( caption );
   setWindowIcon( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_handbook_help.xpm" ) ) );
   
   browser = new QTextBrowser( this );
   setCentralWidget( browser );
   
   mkMenuToolBars();
   
   browser->setOpenExternalLinks( true );
   browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
   
   // set the list of dirs to search when files are requested
   QStringList paths;
   paths << vkConfig->vkDocPath;
   browser->setSearchPaths( paths );
   
   connect( browser, SIGNAL( sourceChanged( const QUrl& ) ),
            this,      SLOT( sourceChanged( const QUrl& ) ) );
   //   connect( browser,     SIGNAL( highlighted(const QString&) ),
   //            statusBar(),   SLOT( showMessage(const QString&)) );
   connect( bookmarkMenu, SIGNAL( hovered( QAction* ) ),
            this,           SLOT( bookmarkHighlighted( QAction* ) ) );
            
            
   // default startup is with the index page loaded
   QString home = vkConfig->vkDocPath + "/index.html";
   browser->setSource( home );
   
   //TODO: vkErrors?  hmm. in a constructor...
   
   //TODO: vkConfig
   resize( 560, 600 );
   hide();
}


void HandBook::historyChosen( QAction* act )
{
   browser->setSource( act->text() );
}


void HandBook::bookmarkChosen( QAction* act )
{
   if ( !bookmarkMenu->actions().contains( act ) ) {
      cerr << "Error: HandBook::bookmarkChosen: act not in bookmarks!" << endl;
      //TODO: shouldn't ever happen: vkError
      return;
   }
   
   QString url = act->data().toString();
   //   browser->setSource( url );
}


void HandBook::bookmarkHighlighted( QAction* act )
{
   if ( !bookmarkMenu->actions().contains( act ) ) {
      cerr << "Error: HandBook::bookmarkHighlighted: act not in bookmarks!" << endl;
      //TODO: shouldn't ever happen: vkError
      return;
   }
   
   if ( act->objectName() != QString::fromUtf8( "handbook_actBookmark" ) ) {
      // other actions besides bookmarks are in the menu -> ignore.
      return;
   }
   
   QString link = act->data().toString();
   //TODO: why doesn't this work?!
   // appears _very_ briefly and gets cleared => flickers.
   statusBar()->showMessage( link, 1000 );
}



void HandBook::addBookmark()
{
   QString url = browser->source().toString();
   QString title = browser->documentTitle();
   
   if ( browser->documentTitle().isNull() ) {
      title = url;
   }
   
   // just show the page title in the menu, but hold onto the url
   QAction* actNew = new QAction( this );
   actNew->setObjectName( QString::fromUtf8( "handbook_actBookmark" ) );
   actNew->setText( title );
   actNew->setData( url );
   bookmarkMenu->addAction( actNew );
   
   // Don't let bookmarks grow endlessly
   //  - find first bookmark action
   //  - count how many actions from there
   //  - remove that first action if necessary
   int nActsBefore = 0;
   QAction* actFirst = 0;
   foreach( actFirst, bookmarkMenu->actions() ) {
      // skip non-bookmark actions
      if ( actFirst->objectName() == QString::fromUtf8( "handbook_actBookmark" ) ) {
         break;
      }
      
      nActsBefore++;
   }
   
   if (( bookmarkMenu->actions().count() - nActsBefore ) > max_bookmarks ) {
      vk_assert( actFirst );
      bookmarkMenu->removeAction( actFirst );
      delete actFirst;
   }
}


// the handbook is explicitly killed by MainWindow on exit.
void HandBook::closeEvent( QCloseEvent* )
{
   hide();
}


void HandBook::showYourself()
{
   if ( !isVisible() ) {
      show();
   }
   else if ( isMinimized() ) {
      showNormal();
   }
   else {
      raise();
   }
}


/*!
  Sets the name of the displayed document to url
*/
void HandBook::openUrl( const QString& url )
{
   browser->setSource( url );
}


/*!
  Open a user-specified html file
*/
void HandBook::openFile()
{
   QString fn = QFileDialog::getOpenFileName(
                   this, "Open File", vkConfig->vkDocPath,
                   "Html Files (*.html *.htm);;All Files (*)" );
                   
   if ( !fn.isEmpty() ) {
      browser->setSource( fn );
   }
}


/*!
  source changed (not from pathCombo)
   - update pathCombo
*/
void HandBook::sourceChanged( const QUrl& url )
{
   QString link = url.toString();
   //   vkPrint("CHECKME: sourceChanged() link: '%s'", qPrintable( link ) );
   
   //TODO: if link.isEmpty -> 404
   
   if ( browser->documentTitle().isNull() ) {
      setWindowTitle( "VkHelp - " + link );
   }
   else {
      setWindowTitle( "VkHelp - " + browser->documentTitle() ) ;
   }
   
   if ( !link.isEmpty() && pathCombo ) {
   
      // pathCombo is not kept in-sync with history
      int idx = pathCombo->findText( link, Qt::MatchFixedString ); // not case-sensitive
      
      if ( idx != -1 ) {
         pathCombo->setCurrentIndex( idx );
      }
      else {
         // combobox
         pathCombo->insertItem( 0, link );
         pathCombo->setCurrentIndex( 0 );
         
         if ( pathCombo->count() > max_history ) {
            pathCombo->removeItem( max_history );
         }
      }
      
      // history menu: if already in history, move to top.
      bool found = false;
      foreach( QAction * act, historyMenu->actions() ) {
         if ( act->text() == link ) {
            historyMenu->removeAction( act );
            
            if ( found ) {
               // found more than once!
               VK_DEBUG( "found double entry in historyMenu: %s", qPrintable( act->text() ) );
            }
            else {
               if ( ! historyMenu->actions().isEmpty() ) {
                  historyMenu->insertAction( historyMenu->actions().first(), act );
               }
               else {
                  historyMenu->addAction( act );
               }
               
               found = true;
               
               // continue looking though the list, to check for double entries.
            }
         }
      }
      
      if ( !found ) {
         // not in history: prepend, and remove last if necessary
         QAction* act = new QAction( this );
         act->setObjectName( QString::fromUtf8( "handbook_actHistory" ) );
         act->setText( link );
         
         if ( ! historyMenu->actions().isEmpty() ) {
            historyMenu->insertAction( historyMenu->actions().first(), act );
         }
         else {
            historyMenu->addAction( act );
         }
         
         if ( historyMenu->actions().count() > max_history ) {
            QAction* act = historyMenu->actions().last();
            historyMenu->removeAction( act );
            delete act;
         }
      }
   }
}


void HandBook::mkMenuToolBars()
{
   menuBar = new QMenuBar( this );
   menuBar->setObjectName( QString::fromUtf8( "help_menubar" ) );
   this->setMenuBar( menuBar );
   
   // ------------------------------------------------------------
   // file menu
   QMenu* fileMenu = new QMenu( menuBar );
   fileMenu->setObjectName( QString::fromUtf8( "handbook_fileMenu" ) );
   fileMenu->setTitle( tr( "&File" ) );
   menuBar->addAction( fileMenu->menuAction() );
   
   QAction* actFile_Open = new QAction( this );
   actFile_Open->setObjectName( QString::fromUtf8( "handbook_actFile_Open" ) );
   actFile_Open->setText( tr( "Open File" ) );
   connect( actFile_Open, SIGNAL( triggered() ), this, SLOT( openFile() ) );
   
   QAction* actFile_Close = new QAction( this );
   actFile_Close->setObjectName( QString::fromUtf8( "handbook_actFile_Close" ) );
   actFile_Close->setText( tr( "Close" ) );
   connect( actFile_Close, SIGNAL( triggered() ), this, SLOT( close() ) );
   
   fileMenu->addAction( actFile_Open );
   fileMenu->addSeparator();
   fileMenu->addAction( actFile_Close );
   
   // ------------------------------------------------------------
   // go menu
   QMenu* goMenu = new QMenu( menuBar );
   goMenu->setObjectName( QString::fromUtf8( "handbook_goMenu" ) );
   goMenu->setTitle( tr( "&Go" ) );
   menuBar->addAction( goMenu->menuAction() );
   
   QAction* actGo_Backward = new QAction( this );
   actGo_Backward->setObjectName( QString::fromUtf8( "handbook_actGo_Backward" ) );
   actGo_Backward->setText( tr( "Backward" ) );
   actGo_Backward->setIcon( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_handbook_back.xpm" ) ) );
   connect( actGo_Backward, SIGNAL( triggered() ), browser, SLOT( backward() ) );
   
   QAction* actGo_Forward = new QAction( this );
   actGo_Forward->setObjectName( QString::fromUtf8( "handbook_actGo_Forward" ) );
   actGo_Forward->setText( tr( "Forward" ) );
   actGo_Forward->setIcon( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_handbook_forward.xpm" ) ) );
   connect( actGo_Forward, SIGNAL( triggered() ), browser, SLOT( forward() ) );
   
   QAction* actGo_Home = new QAction( this );
   actGo_Home->setObjectName( QString::fromUtf8( "handbook_actGo_Home" ) );
   actGo_Home->setText( tr( "Home" ) );
   actGo_Home->setIcon( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_handbook_home.xpm" ) ) );
   connect( actGo_Home, SIGNAL( triggered() ), browser, SLOT( home() ) );
   
   QAction* actGo_Reload = new QAction( this );
   actGo_Reload->setObjectName( QString::fromUtf8( "handbook_actGo_Reload" ) );
   actGo_Reload->setText( tr( "Reload" ) );
   actGo_Reload->setIcon( QPixmap( QString::fromUtf8( ":/vk_icons/icons/tb_handbook_reload.xpm" ) ) );
   connect( actGo_Reload, SIGNAL( triggered() ), browser, SLOT( reload() ) );
   
   goMenu->addAction( actGo_Backward );
   goMenu->addAction( actGo_Forward );
   goMenu->addAction( actGo_Home );
   // Dont add Reload here.
   
   actGo_Backward->setEnabled( false );
   actGo_Forward->setEnabled( false );
   connect( browser,        SIGNAL( backwardAvailable( bool ) ),
            actGo_Backward,   SLOT( setEnabled( bool ) ) );
   connect( browser,        SIGNAL( forwardAvailable( bool ) ),
            actGo_Forward,    SLOT( setEnabled( bool ) ) );
            
   // ------------------------------------------------------------
   // history menu
   historyMenu = new QMenu( menuBar );
   historyMenu->setObjectName( QString::fromUtf8( "handbook_historyMenu" ) );
   historyMenu->setTitle( tr( "History" ) );
   menuBar->addAction( historyMenu->menuAction() );
   connect( historyMenu, SIGNAL( triggered( QAction* ) ),
            this,          SLOT( historyChosen( QAction* ) ) );
   readHistory();
   
   // ------------------------------------------------------------
   // bookmarks menu
   bookmarkMenu = new QMenu( menuBar );
   bookmarkMenu->setObjectName( QString::fromUtf8( "handbook_bookmarkMenu" ) );
   bookmarkMenu->setTitle( tr( "Bookmark" ) );
   menuBar->addAction( bookmarkMenu->menuAction() );
   
   QAction* actBM_AddBM = new QAction( this );
   actBM_AddBM->setObjectName( QString::fromUtf8( "handbook_actBM_AddBM" ) );
   actBM_AddBM->setText( tr( "Add Bookmark" ) );
   connect( actBM_AddBM, SIGNAL( triggered() ), this, SLOT( addBookmark() ) );
   
   bookmarkMenu->addAction( actBM_AddBM );
   bookmarkMenu->addSeparator();
   connect( bookmarkMenu, SIGNAL( triggered( QAction* ) ),
            this,           SLOT( bookmarkChosen( QAction* ) ) );
   readBookmarks();
   
   
   // ------------------------------------------------------------
   // dismiss button
   QAction* act_dismiss = new QAction( this );
   act_dismiss->setObjectName( QString::fromUtf8( "act_dismiss" ) );
   act_dismiss->setText( tr( "Dismiss" ) );
   connect( act_dismiss, SIGNAL( triggered() ),
            this,          SLOT( close() ) );
            
   menuBar->addSeparator();
   menuBar->addAction( act_dismiss );
   
   
   // ------------------------------------------------------------
   // toolbar
   QToolBar* toolbar = new QToolBar( this );
   toolbar->setObjectName( QString::fromUtf8( "handbook_toolbar" ) );
   this->addToolBar( Qt::TopToolBarArea, toolbar );
   toolbar->setAllowedAreas( Qt::TopToolBarArea | Qt::BottomToolBarArea );
   
   toolbar->addAction( actGo_Backward );
   toolbar->addAction( actGo_Forward );
   toolbar->addAction( actGo_Home );
   toolbar->addAction( actGo_Reload );
   toolbar->addSeparator();
   
   pathCombo = new QComboBox( toolbar );
   pathCombo->setEditable( true );
   pathCombo->addItem( vkConfig->vkDocPath );
   QSizePolicy sp = pathCombo->sizePolicy();
   sp.setHorizontalPolicy( QSizePolicy::MinimumExpanding );
   pathCombo->setSizePolicy( sp );
   connect( pathCombo, SIGNAL( activated( const QString& ) ),
            this,        SLOT( openUrl( const QString& ) ) );
   toolbar->addWidget( pathCombo );
   
   
   // ------------------------------------------------------------
   // Basic statusbar setup
   statusBar()->setObjectName( QString::fromUtf8( "helpStatusBar " ) );
   //TODO: why disable?
   //   statusBar()->setSizeGripEnabled( false );
}



void HandBook::save()
{
   vk_assert( historyMenu->actions().count() <= max_history );
   
   //TODO: do this via vkConfig!
#if 0
   //TODO: need to write to _global_ config, not project/temp config...
   QStringList history;
   foreach( QAction * act, historyMenu->actions() ) {
      history << act->text();
   }
   vkConfig->setValue( "valkyrie/handbook_history", history );
   
   int nBookmarks = 0;
   QStringList bookmarks;
   foreach( QAction * act, historyMenu->actions() ) {
      // skip non-bookmark actions
      if ( act->objectName() == QString::fromUtf8( "handbook_actBookmark" ) ) {
         bookmarks << act->text() + vkConfig->vkSepChar + act->data().toString();
         nBookmarks++;
      }
   }
   vkConfig->setValue( "valkyrie/handbook_bookmarks", bookmarks );
   
   vk_assert( nBookmarks <= max_bookmarks );
   
#else
   // save the history
   QFile aFile( vkConfig->vkRcDir + "/help.history" );
   
   if ( aFile.open( QIODevice::WriteOnly ) ) {
      QTextStream stream( &aFile );
   
      foreach( QAction * act, historyMenu->actions() ) {
         stream << act->text() << "\n";
      }
      aFile.close();
   }
   
   // save the bookmarks
   aFile.setFileName( vkConfig->vkRcDir + "/help.bookmarks" );
   
   if ( aFile.open( QIODevice::WriteOnly ) ) {
      QTextStream stream( &aFile );
   
      int nBookmarks = 0;
      foreach( QAction * act, bookmarkMenu->actions() ) {
         // skip non-bookmark actions
         if ( act->objectName() == QString::fromUtf8( "handbook_actBookmark" ) ) {
            stream << act->text()
                   << vkConfig->vkSepChar
                   << act->data().toString() << "\n";
            nBookmarks++;
         }
      }
      aFile.close();
   
      vk_assert( nBookmarks <= max_bookmarks );
   }
   
#endif
}


/*!
  load the history from file into the menu
*/
void HandBook::readHistory()
{
   bool ok = false;
   max_history   = vkConfig->value( "valkyrie/handbook_max_history", 20 ).toInt( &ok );
   
   if ( !ok ) {
      cerr << "Error: bad value for config::valkyrie/handbook_max_history" << endl;
   }
   
   // TODO: do this via vkConfig!
#if 0
   //TODO: need to write to _global_ config, not project/temp config...
   QStringList history = vkConfig->value( "valkyrie/handbook_history" ).toStringList();
   int len = history.count() > max_history ? max_history : history.count();
   
   for ( int idx = 0; idx < len; idx++ ) {
      QString link = history.at( idx );
      
      QAction* act = new QAction( this );
      act->setObjectName( QString::fromUtf8( "handbook_actHistory" ) );
      act->setText( link );
      historyMenu->addAction( act );
   }
   
#else
   QFile aFile( vkConfig->vkRcDir + "/help.history" );
   
   if ( aFile.open( QIODevice::ReadOnly ) ) {
      // read in max_history links
      QTextStream stream( &aFile );
   
      for ( int idx = 0; idx < max_history && !stream.atEnd(); idx++ ) {
         QString link = stream.readLine();
   
         QAction* act = new QAction( this );
         act->setObjectName( QString::fromUtf8( "handbook_actHistory" ) );
         act->setText( link );
         historyMenu->addAction( act );
      }
   
      aFile.close();
   }
   
#endif
}



/*!
  load the bookmarks from file into the menu
*/
void HandBook::readBookmarks()
{
   bool ok = false;
   max_bookmarks = vkConfig->value( "valkyrie/handbook_max_bookmarks", 20 ).toInt( &ok );
   
   if ( !ok ) {
      cerr << "Error: bad value for config::valkyrie/handbook_max_bookmarks" << endl;
   }
   
   // TODO: do this via vkConfig!
#if 0
   //TODO: need to write to _global_ config, not project/temp config...
   QStringList bookmarks = vkConfig->value( "valkyrie/handbook_bookmarks" ).toStringList();
   int len = bookmarks.count() > max_bookmarks ? max_bookmarks : bookmarks.count();
   
   for ( int idx = 0; idx < len; idx++ ) {
      QString str = bookmarks.at( idx );
      
      QStringList sl = str.split( vkConfig->vkSepChar );  //, QString::SkipEmptyParts
      QString title = sl.first();
      QString url   = sl.last();
      
      // menu list
      QAction* act = new QAction( this );
      act->setObjectName( QString::fromUtf8( "handbook_actBookmark" ) );
      act->setText( title );
      act->setData( url );
      bookmarkMenu->addAction( act );
   }
   
#else
   QFile aFile( vkConfig->vkRcDir + "/help.bookmarks" );
   
   if ( aFile.open( QIODevice::ReadOnly ) ) {
      // read in max_bookmarks links
      QTextStream stream( &aFile );
   
      for ( int idx = 0; idx < max_bookmarks && !stream.atEnd(); idx++ ) {
         // read in one line at a time, and split on vkSepChar
         QStringList sl = stream.readLine().split( vkConfig->vkSepChar, QString::SkipEmptyParts );
         QString title = sl.first();
         QString url   = sl.last();
   
         // menu list
         QAction* act = new QAction( this );
         act->setObjectName( QString::fromUtf8( "handbook_actBookmark" ) );
         act->setText( title );
         act->setData( url );
         bookmarkMenu->addAction( act );
      }
   
      aFile.close();
   }
   
#endif
}
