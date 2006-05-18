/* ---------------------------------------------------------------------
 * Implementation of HandBook                              hand_book.cpp
 * Context-sensitive help browser
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "hand_book.h"
#include "tb_handbook_icons.h"
#include "vk_config.h"

#include <qfiledialog.h>
#include <qmenubar.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qstatusbar.h>
#include <qsimplerichtext.h>
#include <qtoolbutton.h>



HandBook::~HandBook() { }


HandBook::HandBook( QWidget* parent, const char* name )
   : QMainWindow( parent, name, WDestructiveClose )
{
   caption.sprintf( "%s HandBook", vkConfig->vkName() );
   setCaption( caption );
   setIcon( QPixmap(help_xpm) );

   setRightJustification( true );
   setDockEnabled( DockLeft, false );
   setDockEnabled( DockRight, false );

   browser = new QTextBrowser( this );
   setCentralWidget( browser );

   mkMenuToolBars();

   browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
   browser->setTextFormat( Qt::RichText );
   browser->setLinkUnderline( true );

   /* set the list of dirs to search when files are requested */
   QStringList paths;
   paths << vkConfig->vkdocDir() << vkConfig->vgdocDir();
   browser->mimeSourceFactory()->setFilePath( paths );

   /* default startup is with the index page loaded */
   browser->setSource( "index.html" );

   connect( browser, SIGNAL( textChanged() ),
            this,    SLOT( textChanged() ) );
   connect( browser,     SIGNAL( highlighted( const QString&) ),
            statusBar(), SLOT( message( const QString&)) );

   resize( 500, 600 );
   hide();
}


void HandBook::setBackwardAvailable( bool b)
{ mainMenu->setItemEnabled( BACKWARD, b ); }


void HandBook::setForwardAvailable( bool b)
{ mainMenu->setItemEnabled( FORWARD, b ); }


void HandBook::historyChosen( int i )
{
   if ( !mapHistory.contains(i) )
      return;

   browser->setSource( mapHistory[i] );
}


void HandBook::bookmarkChosen( int i )
{
   if ( !mapBookmarks.contains(i) )
      return;

   QString url = mapBookmarks[i];
   int len = url.length() - 1;
   int pos = url.find('[', 0, false ) + 1;
   url = url.mid( pos, len-pos );
   browser->setSource( url );
}


void HandBook::addBookmark()
{ 
   /* eg. /home/.../.../.../manual/licenses.html */
   QString url   = browser->context();
   QString title = browser->documentTitle();
   /* just show the page title in the menu */
   int id = bookmarkMenu->insertItem( title );
   /* stash title[url] in the map */
   mapBookmarks[id] = title + "[" + url + "]";
}


/* the handbook is explicitly killed by MainWindow on exit. */
void HandBook::closeEvent( QCloseEvent* )
{ hide(); }


void HandBook::showYourself()
{
   if ( !isVisible() ) {
      show();
   } else if ( isMinimized() ) {
      showNormal();
   } else {
      raise();
   }
}


/* Sets the name of the displayed document to url */
void HandBook::openUrl( const QString& url )
{ browser->setSource( url ); }


void HandBook::openUrl()
{
   QString fn = QFileDialog::getOpenFileName( vkConfig->vkdocDir(), 
                                              "Html Files (*.html *.htm);;All Files (*)", this );
   if ( !fn.isEmpty() ) {
      browser->setSource( fn );
   }
}


void HandBook::textChanged()
{
   if ( browser->documentTitle().isNull() ) {
      setCaption( caption + " - " + browser->context() );
   } else {
      setCaption( caption + " - " + browser->documentTitle() ) ;
   }

   selectedURL = browser->context();

   if ( !selectedURL.isEmpty() && pathCombo ) {
      bool exists = false;
      int i;
      for ( i=0; i<pathCombo->count(); ++i ) {
         if ( pathCombo->text(i) == selectedURL ) {
            exists = true;
            break;
         }
      }

      if ( !exists ) {
         pathCombo->insertItem( selectedURL, 0 );
         pathCombo->setCurrentItem( 0 );
         mapHistory[ historyMenu->insertItem(selectedURL) ] = selectedURL;
      } else {
         pathCombo->setCurrentItem(i);
      }

      selectedURL = QString::null;
   }
}


void HandBook::print()
{
   QPrinter printer;
   printer.setFullPage(true);
   if ( !printer.setup( this ) )
      return;

   QPainter p( &printer );
   QPaintDeviceMetrics metrics(p.device());
   int dpix = metrics.logicalDpiX();
   int dpiy = metrics.logicalDpiY();
   const int margin = 72; // pt
   QRect body(margin*dpix/72, margin*dpiy/72,
              metrics.width()-margin*dpix/72*2,
              metrics.height()-margin*dpiy/72*2 );
   QSimpleRichText richText( browser->text(), QFont(), 
                             browser->context(), browser->styleSheet(),
                             browser->mimeSourceFactory(), body.height() );
   richText.setWidth( &p, body.width() );
   QRect view( body );
   int page = 1;
   QString pnum;
   do {
      pnum.setNum( page );
      richText.draw( &p, body.left(), body.top(), view, colorGroup() );
      view.moveBy( 0,   body.height() );
      p.translate( 0 , -body.height() );
      p.drawText( view.right() - p.fontMetrics().width( pnum ),
                  view.bottom() + p.fontMetrics().ascent() + 5, pnum );
      if ( view.top()  >= richText.height() )
         break;
      printer.newPage();
      page++;
   } while (true);

}


void HandBook::save()
{
   QStringList aList;

   /* save the history */
   QMap<int, QString>::Iterator it1 = mapHistory.begin();
   for ( ; it1 != mapHistory.end(); ++it1 ) {
      aList.append( *it1 );
   }
   QFile aFile( vkConfig->rcDir() + "help.history" );
   if ( aFile.open( IO_WriteOnly ) ) {
      QTextStream stream( &aFile );
      for ( QStringList::Iterator it = aList.begin(); 
            it != aList.end(); ++it )
         stream << *it << "\n";
      aFile.close();
   }

   /* save the bookmarks */
   aList.clear();
   QMap<int, QString>::Iterator it2 = mapBookmarks.begin();
   for ( ; it2 != mapBookmarks.end(); ++it2 ) {
      aList.append( *it2 );
   }
   aFile.setName( vkConfig->rcDir() + "help.bookmarks" );
   if ( aFile.open( IO_WriteOnly ) ) {
      QTextStream stream( &aFile );
      for ( QStringList::Iterator it = aList.begin(); 
            it != aList.end(); ++it )
         stream << *it << "\n";
      aFile.close();
   }

   /* be tidy */
   aList.clear();
}


void HandBook::pathSelected( const QString &path )
{
   browser->setSource( path );
   QMap<int, QString>::Iterator it = mapHistory.begin();
   bool exists = false;
   for ( ; it != mapHistory.end(); ++it ) {
      if ( *it == path ) {
         exists = true;
         break;
      }
   }
   if ( !exists ) {
      mapHistory[ historyMenu->insertItem(path) ] = path;
   }

}


void HandBook::mkMenuToolBars()
{
   mainMenu = new QMenuBar( this, "help_menubar" );

   /* file menu --------------------------------------------------------- */
   QPopupMenu* fileMenu = new QPopupMenu( this );
   fileMenu->insertItem( "Open File",  this, SLOT( openUrl()  ) );
   fileMenu->insertItem( "Print",      this, SLOT( print()     ) );
   fileMenu->insertSeparator();
   fileMenu->insertItem( "Close",      this, SLOT( close()     ) );
   mainMenu->insertItem( "&File", fileMenu );

   /* go menu ----------------------------------------------------------- */
   QPopupMenu* goMenu = new QPopupMenu( this );
   goMenu->insertItem( QPixmap(back_xpm), "Backward", 
                       browser, SLOT( backward() ), 0, BACKWARD );
   goMenu->insertItem( QPixmap(forward_xpm), "Forward", 
                       browser, SLOT( forward() ),  0, FORWARD );
   goMenu->insertItem( QPixmap(home_xpm),    "Home", 
                       browser, SLOT( home() ),     0, HOME );
   mainMenu->insertItem( "&Go", goMenu );

   /* history menu ------------------------------------------------------ */
   QStringList aList;
   historyMenu = new QPopupMenu( this );
   /* load the history from file into the list */
   QFile aFile( vkConfig->rcDir() + "help.history" );
   if ( aFile.open( IO_ReadOnly ) ) {
      QTextStream stream( &aFile );
      while ( !stream.atEnd() ) {
         aList += stream.readLine();
      }
      aFile.close();
      while ( aList.count() > 20 ) {
         aList.remove( aList.begin() );
      }
   }
   /* put each string into the map */
   for ( unsigned int id=0; id<aList.count(); id++ ) {
      historyMenu->insertItem( aList[id], id );
      mapHistory[id] = aList[id];
   }
   connect( historyMenu, SIGNAL( activated(int) ),
            this,        SLOT( historyChosen(int) ) );
   mainMenu->insertItem( "History", historyMenu );

   /* bookmarks menu ---------------------------------------------------- */
   aList.clear();
   bookmarkMenu = new QPopupMenu( this );
   bookmarkMenu->insertItem( "Add Bookmark", this, SLOT( addBookmark() ) );
   bookmarkMenu->insertSeparator();
   /* load the bookmarks from file into the list */
   aFile.setName( vkConfig->rcDir() + "help.bookmarks" );
   if ( aFile.open( IO_ReadOnly ) ) {
      QTextStream stream( &aFile );
      while ( !stream.atEnd() ) {
         aList += stream.readLine();
      }
      aFile.close();
   }
   /* put the page titles in the menu, and the entire string in the map */
   for ( unsigned int id=0; id<aList.count(); id++ ) {
      QString url_title = aList[id];
      int pos = url_title.find('[', 0, false );
      QString title = url_title.left( pos );
      bookmarkMenu->insertItem( title, id );
      mapBookmarks[id] = url_title;
   }
   aList.clear();
   connect( bookmarkMenu, SIGNAL( activated(int) ),
            this,         SLOT( bookmarkChosen(int) ) );
   mainMenu->insertItem( "Bookmarks", bookmarkMenu );


   mainMenu->setItemEnabled( FORWARD,  false);
   mainMenu->setItemEnabled( BACKWARD, false);
   connect( browser, SIGNAL( backwardAvailable( bool ) ),
            this,    SLOT( setBackwardAvailable( bool ) ) );
   connect( browser, SIGNAL( forwardAvailable( bool ) ),
            this,    SLOT( setForwardAvailable( bool ) ) );


   /* dismiss 'button' -------------------------------------------------- */
   mainMenu->insertSeparator();
   QToolButton* dismissButton = new QToolButton( this, "tb_dismiss" );
   dismissButton->setText( "Dismiss" );
   dismissButton->setAutoRaise( true );
   connect( dismissButton, SIGNAL( clicked() ), 
            this,          SLOT( close() ) );
   mainMenu->insertItem( dismissButton );


   /* toolbar ----------------------------------------------------------- */
   QToolBar* toolbar = new QToolBar( this, "handbook_toolbar" );
   addToolBar( toolbar, "Toolbar");

   QToolButton* button;
   button = new QToolButton( QPixmap(back_xpm), "Backward", "", 
                             browser, SLOT(backward()), toolbar );
   connect( browser, SIGNAL( backwardAvailable(bool) ), 
            button,  SLOT( setEnabled(bool) ) );
   button->setEnabled( false );

   button = new QToolButton( QPixmap(forward_xpm), "Forward", "", 
                             browser, SLOT(forward()), toolbar );
   connect( browser, SIGNAL( forwardAvailable(bool) ), 
            button,  SLOT( setEnabled(bool) ) );
   button->setEnabled( false );

   button = new QToolButton( QPixmap(home_xpm), "Home", "", 
                             browser, SLOT(home()), toolbar );

   button = new QToolButton( QPixmap(reload_xpm), "Reload", "", 
                             browser, SLOT(reload()), toolbar );

   toolbar->addSeparator();

   pathCombo = new QComboBox( true, toolbar );
   pathCombo->insertItem( vkConfig->vkdocDir() );
   connect( pathCombo, SIGNAL( activated(const QString &) ),
            this,      SLOT( pathSelected(const QString &) ) );

   toolbar->setStretchableWidget( pathCombo );
}

