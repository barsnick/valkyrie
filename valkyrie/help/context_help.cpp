/* ---------------------------------------------------------------------
 * Implementation of ContextHelp                        context_help.cpp
 * Context-sensitive help 
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "context_help.h"
#include "hand_book.h"
#include "vk_utils.h"              // VK_DEBUG

#include <qapplication.h>


static const char * const context_help_xpm[] = {
   "16 16 3 1",
   "   c None",
   "+  c #000000",
   "*  c #000080",
   "+        *****  ",
   "++      *** *** ",
   "+++    ***   ***",
   "++++   **     **",
   "+++++  **     **",
   "++++++  *    ***",
   "+++++++     *** ",
   "++++++++   ***  ",
   "+++++++++ ***   ",
   "+++++     ***   ",
   "++ +++          ",
   "+  +++    ***   ",
   "    +++   ***   ",
   "    +++         ",
   "     +++        ",
   "     +++        "};


/* static, but static the less-typing way */
static ContextHelp * ctxt = 0;


/* class ContextHelpButton --------------------------------------------- */
ContextHelpButton::~ContextHelpButton()
{
   if ( ctxt && ctxt->buttons ) {
      ctxt->buttons->take( (void*)this );
   }
}


ContextHelpButton::ContextHelpButton( QWidget* parent, HandBook* book )
   : QToolButton( parent, "ctxt_help_tb" )
{
   ContextHelp::setUp();

   ctxt->buttons->insert( (void*)this, this );
   ctxt->hbook = book;

   QPixmap p( (const char**)context_help_xpm );
   setIconSet( p );
   setToggleButton( true );
   setAutoRaise( true );
   setFocusPolicy( NoFocus );
   setTextLabel( "Context Help" );

   connect( this, SIGNAL( released() ),
            this, SLOT( mouseReleased() ) );
}


void ContextHelpButton::mouseReleased()
{
   if ( ctxt->state == ContextHelp::Inactive && isOn() ) {
      ContextHelp::setUp();
      QApplication::setOverrideCursor( whatsThisCursor, false );
      ctxt->state = ContextHelp::Waiting;
      qApp->installEventFilter( ctxt );
   }
}




/* class ContextHelp --------------------------------------------------- */
static void qContextHelpCleanup()
{
   if ( ctxt ) {
      delete ctxt;
      ctxt = 0;
   }
}


ContextHelp::UrlItem::~UrlItem()
{
   if ( count ) {
      VK_DEBUG("Internal error (%d)", count);
   }
}


ContextHelp::ContextHelp()
   : QObject( 0, "global context help" )
{
   ctxt    = this;
   state   = Inactive;
   wdict   = new QPtrDict<ContextHelp::UrlItem>;
   tlw     = new QPtrDict<QWidget>;
   buttons = new QPtrDict<ContextHelpButton>;
}


ContextHelp::~ContextHelp()
{
   if ( state == Waiting && qApp )
      QApplication::restoreOverrideCursor();

   /* delete the two straight-and-simple dicts */
   delete tlw;
   delete buttons;
  
   /* then delete the complex one. */
   QPtrDictIterator<UrlItem> it( *wdict );
   UrlItem * item;
   QWidget * w;
   while( ( item = it.current() ) != 0 ) {
      w = (QWidget *)it.currentKey();
      ++it;
      wdict->take( w );
      if ( item->deref() )
         delete item;
   }

   delete wdict;
   ctxt = 0;
}


/* removes the Context help associated with the widget.
   this happens automatically if the widget is destroyed. */
void ContextHelp::remove( QWidget * widget )
{
   setUp();
   ContextHelp::UrlItem * i = wdict->find( (void *)widget );
   if ( !i )
      return;

   wdict->take( (void *)widget );
   i->deref();
   if ( !i->count )
      delete i;
}


bool ContextHelp::eventFilter( QObject * obj, QEvent * ev )
{
   switch ( state ) {

   case Waiting: {
      if ( ev->type() == QEvent::MouseButtonPress && 
           obj->isWidgetType() ) {
         QWidget * w = (QWidget *) obj;
         if ( ( (QMouseEvent*)ev)->button() == RightButton )
            return false;   /* ignore RMB */
         ContextHelp::UrlItem * item = 0;
         QMouseEvent* me = (QMouseEvent*) ev;
         QPoint p = me->pos();
         while ( w && !item ) {
            if (w->isA("QMenuBar")) {
               /* If we're a qmenubar, allow event to pass on so menus work... */
               // TODO: find what menuitem we're sitting on, if any, and get that widget...
               return false;
            }
            item = wdict->find( w );
            if ( !item ) {
               p += w->pos();
               w = w->parentWidget( true );
            }
         }
         shutDown();
         if ( !item )
            return true;
         say( w, item->url );
         return true;
      } else if ( ev->type() == QEvent::MouseButtonRelease ) {
         if ( ( (QMouseEvent*)ev)->button() == RightButton )
            return false;   /* ignore RMB */
         return !obj->isWidgetType();
      } else if ( ev->type() == QEvent::MouseMove ) {
         return !obj->isWidgetType();
      } else if ( ev->type() == QEvent::KeyPress ) {
         QKeyEvent* kev = (QKeyEvent*)ev;
         if ( kev->key() == Qt::Key_Escape ) {
            shutDown();
            return true;
         } else if ( kev->key() == Key_Menu ||
                     ( kev->key() == Key_F10 &&
                       kev->state() == ShiftButton ) ) {
            /* don't react to these keys: they are used for context
               menus */
            return false;
         } else if ( kev->state() == kev->stateAfter() &&
                     kev->key() != Key_Meta ) { 
            /* not a modifier key */
            shutDown();
         }
      } else if ( ev->type() == QEvent::MouseButtonDblClick ) {
         return true;
      }
   } /* break; */

   case Inactive:
      break;
   }

   return false;
}


void ContextHelp::setUp()
{
   if ( !ctxt ) {
      ctxt = new ContextHelp();

      /* it is necessary to use a post routine, because the destructor
         deletes pixmaps and other stuff that needs a working X
         connection under X11. */
      qAddPostRoutine( qContextHelpCleanup );
   }
}


void ContextHelp::shutDown()
{
   if ( state == Waiting ) {
      QPtrDictIterator<ContextHelpButton> it( *(ctxt->buttons) );
      ContextHelpButton * b;
      while( ( b=it.current()) != 0 ) {
         ++it;
         b->setOn( false );
      }
      QApplication::restoreOverrideCursor();
      state = Inactive;
      qApp->removeEventFilter( this );
   }
}


void ContextHelp::say( QWidget* widget, const QString &text )
{
   if ( text.isEmpty() || !widget )
      return;

   if ( !hbook->isVisible() ) {

      /* find out where MainWindow is, and park up beside it */
      QWidget * mw = qApp->mainWidget();
      int scr = QApplication::desktop()->screenNumber( mw );
      QRect screen = QApplication::desktop()->screenGeometry( scr );

      int x;
      int hw = hbook->width();

      /* get the global co-ords of the top-left pixel of MainWin */
      QPoint pos = mw->mapToGlobal( QPoint( 0,0 ) );
      if ( hw < ( pos.x() - screen.x() ) )
         x = pos.x() - hw;
      else 
         x = pos.x() + mw->width();

      hbook->move( x, pos.y() );
      hbook->show();
   }

   hbook->raise();
   hbook->openUrl( text );
}


void ContextHelp::cleanupWidget() 
{
   const QObject* obj = sender();
   if ( obj->isWidgetType() ) {   /* sanity check */
      remove( (QWidget*)obj );
   }
}


void ContextHelp::newItem( QWidget* widget, const QString& url )
{
   UrlItem* item = wdict->find( (void *)widget );
   if ( item ) {
      remove( widget );
   }
   item = new UrlItem;
   wdict->insert( (void*)widget, item );
   QWidget* t = widget->topLevelWidget();
   if ( !tlw->find( (void*)t ) ) {
      tlw->insert( (void*)t, t );
      t->installEventFilter( this );
   }

   connect( widget, SIGNAL(destroyed()), 
            this,   SLOT(cleanupWidget()) );

   item->url = url;
}


/* adds url as context help for this widget.  
   the text is destroyed if the widget is later destroyed, so it need
   not be explicitly removed. */
void ContextHelp::add( QWidget* widget, const QString& url )
{
   vk_assert( widget != NULL );
   if ( !url.isEmpty() ) {
      setUp();
      ctxt->newItem( widget, url );
   }
}

