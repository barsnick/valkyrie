/* ---------------------------------------------------------------------
 * Implementation of HandBook                              hand_book.cpp
 * Context-sensitive help browser
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "hand_book.h"
#include "tb_handbook_icons.h"
#include "vk_config.h"
#include "vk_messages.h"
#include "vk_utils.h"

#include <qcursor.h>
#include <qfiledialog.h>
#include <qmenubar.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qprocess.h>
#include <qstatusbar.h>
#include <qsimplerichtext.h>
#include <qtoolbutton.h>

#include <stdlib.h>           // getenv


/* class VkTextBrowser ---------------------------------------------- */
/* Only used in order to catch 'linkClicked' signal and open a browser
   for external links. */
VkTextBrowser::VkTextBrowser ( QWidget* parent, const char* name )
   : QTextBrowser( parent, name )
{
   connect( this, SIGNAL(linkClicked(const QString&)),
            this, SLOT(doLinkClicked(const QString&)) );
}

void VkTextBrowser::doLinkClicked ( const QString& link )
{
   //vkPrint("link: '%s'", link.latin1());
   if (link.startsWith("http://")) {
      /* just reload same page */
      reload();

      /* try to launch a browser */
      if ( !launch_browser( link ) ) {
         vkPrintErr("Error: failed to startup a browser.");
         vkError( this, "Browser Startup",
                  "<p>Failed to startup a browser.<br> \
                   Please set a browser in Options::Valkyrie::Browser</p>" );
      }      
   } else if (link.startsWith("mailto:")) {
      reload();   /* just reload same page */

      /* try to launch a mail client */
      // TODO
   }
}


bool VkTextBrowser::try_launch_browser(QString browser, const QString& link)
{
   browser = browser.simplifyWhiteSpace();
   QStringList args;
   if (browser.find("%s") != -1) {
      /* As per http://www.catb.org/~esr/BROWSER/ */
      browser = browser.replace("%s", link);
      browser = browser.replace("%%", "%");

      /* TODO: this is necessary, but rather inadequate...
         - what's the general solution? */
      browser = browser.replace("\"", "");
      args = QStringList::split(" ", browser );
   } else {
      args << browser << link;
   }
//   vkPrint(" args: '%s'\n", args.join(" |").latin1());
   QProcess proc( args );
   return proc.start();
}


bool VkTextBrowser::launch_browser(const QString& link)
{
   bool ok = false;
   QApplication::setOverrideCursor(Qt::WaitCursor);
   
   /* try config::vk::browser */
   QString rc_browser = vkConfig->rdEntry( "browser", "valkyrie" );
   if (!rc_browser.isEmpty()) {
      ok = try_launch_browser(rc_browser, link);
      if (!ok) {
         vkPrintErr("Error: Failed to launch browser '%s'",
                    rc_browser.latin1());
         vkPrintErr(" - trying other browsers.");
         vkError( this, "Browser Startup",
                  "<p>Failed to startup configured default browser '%s'.<br> \
                   Please verify Options::Valkyrie::Browser.<br><br> \
                   Going on to try other browsers... </p>",
                  rc_browser.latin1() );
      }
   }

   /* either no config val set, or it failed...
      try env var $BROWSER */
   if (!ok) {
      QString env_browser = getenv("BROWSER");
      if (!env_browser.isEmpty()) {
         QStringList browsers = QStringList::split( ':', env_browser );
         QStringList::iterator it;
         for (it=browsers.begin(); it != browsers.end(); ++it ) {
            if (ok = try_launch_browser(*it, link))
               break;
         }
         if (!ok) {
            vkPrintErr("Error: Failed to launch any browser in env var $BROWSER '%s'",
                       env_browser.latin1());
            vkPrintErr(" - trying other browsers.");
         }
      }
   }

   /* still nogo?
      last resort: try preset list */
   if (!ok) {
      QStringList browsers;
      browsers << "firefox" << "mozilla" << "konqueror" << "netscape";
      QStringList::iterator it;
      for (it=browsers.begin(); it != browsers.end(); ++it ) {
         if (ok = try_launch_browser(*it, link))
            break;
      }
   }

   QApplication::restoreOverrideCursor();
   return ok;
} 


/* overloaded to reload _with_ #mark */
void VkTextBrowser::reload()
{ setSource( source() ); }





/* class HandBook --------------------------------------------------- */
HandBook::~HandBook()
{
   /* save history + bookmarks */
   save();
}


HandBook::HandBook( QWidget* parent, const char* name )
   : QMainWindow( parent, name, WDestructiveClose )
{
   caption.sprintf( "%s HandBook", vkConfig->vkName() );
   setCaption( caption );
   setIcon( QPixmap(help_xpm) );

   setRightJustification( true );
   setDockEnabled( DockLeft, false );
   setDockEnabled( DockRight, false );

   browser = new VkTextBrowser( this );
   setCentralWidget( browser );

   mkMenuToolBars();

   browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
   browser->setTextFormat( Qt::RichText );
   browser->setLinkUnderline( true );

   /* set the list of dirs to search when files are requested */
   QStringList paths;
   paths << vkConfig->vkdocDir();
   browser->mimeSourceFactory()->setFilePath( paths );

   connect( browser, SIGNAL( sourceChanged(const QString&) ),
            this,    SLOT( sourceChanged(const QString&) ) );
   connect( browser,     SIGNAL( highlighted( const QString&) ),
            statusBar(), SLOT( message( const QString&)) );

   /* default startup is with the index page loaded */
   QString home = vkConfig->vkdocDir() + "index.html";
   browser->setSource( home );

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


void HandBook::openFile()
{
   QString fn = QFileDialog::getOpenFileName( vkConfig->vkdocDir(), 
                                              "Html Files (*.html *.htm);;All Files (*)", this );
   if ( !fn.isEmpty() ) {
      browser->setSource( fn );
   }
}


void HandBook::sourceChanged( const QString& url )
{
   if ( browser->documentTitle().isNull() )
      setCaption( "Qt Example - Helpviewer - " + url );
   else
      setCaption( "Qt Example - Helpviewer - " + browser->documentTitle() ) ;
   
   if ( !url.isEmpty() && pathCombo ) {
      bool exists = FALSE;
      int i;
      for ( i = 0; i < pathCombo->count(); ++i ) {
         if ( pathCombo->text( i ) == url ) {
            exists = TRUE;
            break;
         }
      }
      if ( !exists ) {
         pathCombo->insertItem( url, 0 );
         pathCombo->setCurrentItem( 0 );
         mapHistory[ historyMenu->insertItem( url ) ] = url;
      } else
         pathCombo->setCurrentItem( i );
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
   fileMenu->insertItem( "Open File",  this, SLOT( openFile()  ) );
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

