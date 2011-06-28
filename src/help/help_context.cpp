/****************************************************************************
** ContextHelp implementation
**  - context-sensitive help
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
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
#include <QDesktopWidget>
#include <QMouseEvent>


#include "help/help_context.h"
#include "help/help_handbook.h"
#include "utils/vk_utils.h"              // VK_DEBUG


// There can be only one.
static ContextHelp* ctxt = 0;



/*!
  class ContextHelpAction
*/
ContextHelpAction::~ContextHelpAction()
{
   if ( ctxt ) {
      ctxt->actions.removeAll( this ); // remove widget from list
      // TODO: check return value - num removed should only ever be 1.
   }
}


ContextHelpAction::ContextHelpAction( QWidget* parent, HandBook* book )
   : QAction( parent )
{
   setObjectName( QString::fromUtf8( "ctxt_help_tb" ) );
   
   ContextHelp::setupSingleton();
   
   ctxt->actions.append( this );
   ctxt->hbook = book;
   
   setIcon( QPixmap( QString::fromUtf8( ":/vk_icons/icons/context_help.xpm" ) ) );
   setCheckable( true );
   setIconVisibleInMenu( true );
#if 0
   setAutoRaise( true );
   setFocusPolicy( Qt::NoFocus );
#endif
   this->setToolTip( "This is a <b>Context Help</b> button. Clicking on a widget will open the relevant manual help page" );
   
   connect( this, SIGNAL( triggered( bool ) ),
            this, SLOT( startListening( bool ) ) );
}


/*!
  Action triggered
   - start global listen for a left mouse click on a widget
*/
void ContextHelpAction::startListening( bool checked )
{
   // if not already active && this button is on...
   if ( !ctxt->listeningForEvent && checked ) {
      QApplication::setOverrideCursor( Qt::WhatsThisCursor );
      ctxt->listeningForEvent = true;
      qApp->installEventFilter( ctxt );
   }
}





/*!
 class ContextHelp
*/
static void qContextHelpCleanup()
{
   if ( ctxt ) {
      delete ctxt;
      ctxt = 0;
   }
}


ContextHelp::ContextHelp()
   : QObject( 0 )
{
   setObjectName( QString::fromUtf8( "ctxt_help_tb" ) );
   ctxt = this;
   listeningForEvent = false;
}


ContextHelp::~ContextHelp()
{
   if ( listeningForEvent == true && qApp ) {
      QApplication::restoreOverrideCursor();
   }
   
   actions.clear();
   wdict.clear();
   
   ctxt = 0;
}


/*!
  removes the Context help associated with the widget.
  this happens automatically if the widget is destroyed.
*/
void ContextHelp::remove( QWidget* widget )
{
   vk_assert( widget != NULL );
   
   wdict.remove( widget );
   // TODO: check return value - num removed should only ever be 1.
}


bool ContextHelp::eventFilter( QObject* obj, QEvent* ev )
{
   if ( listeningForEvent ) {
   
      if ( ev->type() == QEvent::MouseButtonPress && obj->isWidgetType() ) {
         QWidget* widg = ( QWidget* ) obj;
         
         if ((( QMouseEvent* )ev )->button() == Qt::RightButton ) {
            return false;   // ignore right mouse button
         }
         
         QString url;
         
         while ( widg && url.isEmpty() ) {
            if ( widg->inherits( "QMenuBar" ) ) {
               // If we're a qmenubar, allow event to pass on so menus work...
               // TODO: find what menuitem we're sitting on, if any, and get that widget...
               return false;
            }
            
            url = wdict.value( widg );
            
            if ( url.isEmpty() ) {
               //             pos += widg->pos();
               widg = widg->parentWidget();  // 0 if no parent
            }
         }
         
         cancelHelpEvent();
         
         if ( !widg || url.isEmpty() ) {
            //TODO: vkMsg?
            cerr << "No help found for this widget" << endl;
            return true;
         }
         
         showHelp( url );
         return true;
      }
      else if ( ev->type() == QEvent::MouseButtonRelease ) {
         if ((( QMouseEvent* )ev )->button() == Qt::RightButton ) {
            return false;   // ignore right mouse button
         }
         
         return !obj->isWidgetType();
      }
      else if ( ev->type() == QEvent::MouseMove ) {
         return !obj->isWidgetType();
      }
      else if ( ev->type() == QEvent::KeyPress ) {
         QKeyEvent* kev = ( QKeyEvent* )ev;
         
         if ( kev->key() == Qt::Key_Escape ) {
            cancelHelpEvent();
            return true;
         }
         else if ( kev->key() == Qt::Key_Menu ||
                   ( kev->key() == Qt::Key_F10 &&
                     ( kev->modifiers() & Qt::ShiftModifier ) ) ) {
            //TODO: test shift-F10. modifiers() may not be trustworthy.
            
            // don't react to these keys: they are used for context menus
            return false;
         }
         
#if 0 // TODO: how to do this in Qt4?
         else if ( kev->state() == kev->stateAfter() &&
                   kev->key() != Qt::Key_Meta ) {
            // not a modifier key
            cancelHelpEvent();
         }
         
#endif
      }
      else if ( ev->type() == QEvent::MouseButtonDblClick ) {
         return true;
      }
   }
   
   return false;
}


void ContextHelp::setupSingleton()
{
   if ( !ctxt ) {
      ctxt = new ContextHelp();
      
      
      //TODO: this really necessary?
      // not better to setup a parent, so gets auto-deleted at app close?
      // or just create and delete in main()?
      
      /* it is necessary to use a post routine, because the destructor
         deletes pixmaps and other stuff that needs a working X
         connection under X11. */
      qAddPostRoutine( qContextHelpCleanup );
   }
}


/*!
  Cancel the context help
   - reset actions, cursor, remove eventfilter
*/
void ContextHelp::cancelHelpEvent()
{
   if ( listeningForEvent ) {
      // set all actions off.
      foreach( ContextHelpAction * act, ctxt->actions ) {
         act->setChecked( false );
      }
      
      QApplication::restoreOverrideCursor();
      listeningForEvent = false;
      qApp->removeEventFilter( this );
   }
}


/*!
  Open help at url
*/
void ContextHelp::showHelp( const QString& text )
{
   cerr << "ContextHelp::showHelp: '" << text.toLatin1().constData() << "'" << endl;
   
   if ( text.isEmpty() ) {
      return;
   }
   
   if ( !hbook->isVisible() ) {
   
      // find out where MainWindow is, and park up beside it
      QWidget* mw = qApp->activeWindow();
      int scr = QApplication::desktop()->screenNumber( mw );
      QRect screen = QApplication::desktop()->screenGeometry( scr );
      
      int x;
      int hw = hbook->width();
      
      // get the global co-ords of the top-left pixel of MainWin
      QPoint pos = mw->mapToGlobal( QPoint( 0, 0 ) );
      
      if ( hw < ( pos.x() - screen.x() ) ) {
         x = pos.x() - hw;
      }
      else {
         x = pos.x() + mw->width();
      }
      
      hbook->move( x, pos.y() );
      hbook->show();
   }
   
   hbook->raise();
   hbook->openUrl( text );
}


/*!
  Only of our registed widgets died: remove it from the list
*/
void ContextHelp::cleanupWidget()
{
   const QObject* obj = sender();
   
   if ( !obj->isWidgetType() ) {   // sanity check
      cerr << "Error: ContextHelp::cleanupWidget(): "
           << "signal received from non-widget: "
           << qPrintable( obj->objectName() ) << endl;
   }
   else {
      remove(( QWidget* )obj );
   }
}


/*!
  Add help url to widget
*/
void ContextHelp::newItem( QWidget* widg, const QString& url )
{
   // in case widg already in our lists, replace it.
   if ( wdict.contains( widg ) ) {
      cerr << "ContextHelp::newItem(): widg ("
           << qPrintable( widg->objectName() ) << ") was registered to: '"
           << qPrintable( wdict.value( widg ) ) << "'" << endl
           << " - Replacing with: '" << qPrintable( url ) << "'" << endl;
      wdict.remove( widg );
   }
   
   // add to our list
   wdict.insert( widg, url );
   
   // make sure to remove mappings that no longer exist.
   connect( widg, SIGNAL( destroyed() ),
            this, SLOT( cleanupWidget() ) );
}


/*!
 Static function:
  - Initialise context help singleton if necessary
  - Add help url to given widget
*/
void ContextHelp::addHelp( QWidget* widg, const QString& url )
{
   vk_assert( widg != NULL );
   
   if ( !url.isEmpty() ) {
      setupSingleton();
      ctxt->newItem( widg, url );
   }
}

