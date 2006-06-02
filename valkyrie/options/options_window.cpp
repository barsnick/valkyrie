/* ---------------------------------------------------------------------- 
 * Implementation of class OptionsWindow               options_window.cpp
 * A container class for each tool's options / flags 'pane'.
 * Not modal, so user can keep it open and change flags as they work.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qframe.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qstatusbar.h>

#include "html_urls.h"
#include "context_help.h"
#include "options_window.h"
#include "vk_utils.h"
#include "vk_objects.h"
#include "vk_config.h"
#include "main_window.h"
#include "vk_messages.h"


/* class Categories ---------------------------------------------------- */
Categories::Categories( QWidget* parent )
   : QListBox( parent, "cat_listbox" ) 
{
   QFont fnt = font();
   fnt.setWeight( QFont::Bold );
   setFont( fnt );

   QFontMetrics fm = fontMetrics();
   m_categht = fm.height() * 2; 
}

int Categories::categHeight() 
{ return m_categht; }


/* class CategItem ----------------------------------------------------- */
CategItem::CategItem( QListBox* parent, OptionsPage* page,
                      const QString &txt, int id )
   : QListBoxItem(parent), m_catid(id), m_optpage( page )
{ setText( txt ); }

int CategItem::height( const QListBox * ) const
{ return ((Categories*)listBox())->categHeight(); }

int CategItem::catId() const 
{ return m_catid; }

void CategItem::setWidget( OptionsPage* page ) 
{ 
   vk_assert( m_optpage == NULL ); 
   m_optpage = page; 
}

OptionsPage * CategItem::page() const 
{ return m_optpage; }

void CategItem::paint( QPainter *painter )
{
   int itemHeight = height( listBox() );
   QFontMetrics fm = painter->fontMetrics();
   int xPos = fm.width("x");
   int yPos = ( ( itemHeight - fm.height() ) / 2 ) + fm.ascent();
   painter->drawText( xPos, yPos, text() );
}




/* class OptionsWindow ------------------------------------------------- */
OptionsWindow::~OptionsWindow()
{ m_optPages.clear(); }


OptionsWindow::OptionsWindow( QWidget* parent ) 
   : QMainWindow( parent, "options_win" )
{
   m_capt.sprintf("%s Options: ", vkConfig->vkName() );
   setCaption( m_capt );
   statusBar()->setSizeGripEnabled( false );

   ContextHelp::add( this, urlValkyrie::optsDlg );

   m_optPages.setAutoDelete( true );
   m_xpos = m_ypos = -1;

   /* status bar */
   QFrame *statusFrame = new QFrame( statusBar() );
   statusBar()->addWidget( statusFrame, 10, true );
   QHBoxLayout* buttLayout = new QHBoxLayout(statusFrame, 5, -1 );

   /* reset button: reset default options */
   QPushButton* pb = new QPushButton( "&Defaults", statusFrame );
   connect( pb, SIGNAL(clicked() ), this, SLOT(resetDefaults()));
   buttLayout->addWidget( pb );
   buttLayout->addStretch( 10 );

   int w = fontMetrics().width( "X&CancelX" );
   /* save button: apply and save to disk */
   m_saveButton = new QPushButton( "&Save", statusFrame );
   m_saveButton->setFixedWidth( w );
   buttLayout->addWidget( m_saveButton );
   connect( m_saveButton, SIGNAL(clicked() ), this, SLOT(save()) );
   m_saveButton->setEnabled(  vkConfig->isDirty() );

   /* reset button: undo everything since last apply */
   m_resetButton = new QPushButton( "&Reset", statusFrame );
   m_resetButton->setFixedWidth( w );
   buttLayout->addWidget( m_resetButton );
   connect( m_resetButton, SIGNAL(clicked() ), this, SLOT(reject()));
   m_resetButton->setEnabled( false );  /* nothing to reset yet */

   /* apply button: apply edits - no going back */
   m_applyButton = new QPushButton( "&Apply", statusFrame );
   m_applyButton->setFixedWidth( w );
   buttLayout->addWidget( m_applyButton );
   connect( m_applyButton, SIGNAL(clicked() ), this, SLOT(apply()));
   m_applyButton->setEnabled( false );  /* nothing to apply yet */

   /* okay button: apply and quit in one go */
   pb = new QPushButton( "&Ok", statusFrame );
   pb->setFixedWidth( w );
   pb->setDefault( true );                /* default button for dialog */
   buttLayout->addWidget( pb );
   connect( pb, SIGNAL(clicked() ), this, SLOT(accept()) );


   QSplitter* splitter = new QSplitter( this );
   setCentralWidget( splitter );

   /* category chooser */
   m_categories = new Categories( splitter );
   splitter->setResizeMode( m_categories, QSplitter::FollowSizeHint );
   connect( m_categories, SIGNAL( clicked( QListBoxItem *) ),
            this,           SLOT( categoryClicked( QListBoxItem *) ) );

   /* stack for the various widgets */
   m_wStack = new QWidgetStack( splitter );

   VkObjectList objList = ((MainWindow*)parent)->valkyrie()->vkObjList();
   for ( VkObject* obj = objList.first(); obj; obj = objList.next() ) {
      addCategory( obj );
   }
}


void OptionsWindow::addCategory( VkObject* obj )
{
   /* to look up object later to call obj->createOptionsPage() */
   int catid = obj->objId();
   OptionsPage* page = NULL;
   m_wStack->addWidget( page, catid );
   new CategItem( m_categories, page, obj->title(), catid );
}


void OptionsWindow::setCategory( int catid )
{
   if ( catid != -1 ) {
      m_categories->setCurrentItem( catid );
      categoryClicked( m_categories->item(catid) );
   }
}


/* we make the widgets on demand here */
void OptionsWindow::categoryClicked( QListBoxItem *item )
{
   if ( item ) {
      CategItem* cit = (CategItem*)item;

      /* check no uncommited edits in last page */
      OptionsPage* last_page = (OptionsPage*)m_wStack->visibleWidget();

      if (last_page != 0 &&                  /* moving from previous */
          last_page != cit->page()) {        /* prev not same as next */
         /* first pull back to the right item selection */
         m_categories->setSelected( last_page->optId(), true );

         if (m_applyButton->isEnabled()) {
            /* choose to apply/reset edits */
            CategItem* last_cit = (CategItem*)m_categories->item( last_page->optId());
            int ok = vkQuery( this, "Apply/Reset Edits",
                              "&Apply;&Reset;&Cancel",
                              "<p>There are non-committed edits in option page %s.<br/>"
                              "Would you like to Apply or Reset these edits?</p>",
                              last_cit->text().latin1() );
            switch ( ok ) {
            case MsgBox::vkYes:    apply(); break;
            case MsgBox::vkNo:     reject(); break;
            case MsgBox::vkCancel: return;  /* jump back to last page */
            default:
               vk_assert_never_reached();
            }
         }
      }

      setCaption( m_capt + item->text() );

      /* first time this item has been selected */
      if ( cit->page() == 0 ) {
         OptionsPage* page = mkOptionsPage( cit->catId() );
         if ( page == 0 ) {
            VK_DEBUG("cit->text = %s", cit->text().latin1() );
            return;
         }
         cit->setWidget( page );
         m_wStack->addWidget( page, cit->catId() );
      }

      /* make sure the item seletion is sync'd */
      m_categories->setSelected( cit->catId(), true );
      cit->page()->init();
      m_wStack->raiseWidget( cit->page() );
   }

}


OptionsPage* OptionsWindow::mkOptionsPage( int catid )
{
   VkObject* obj = ((MainWindow*)parent())->valkyrie()->vkObject( catid );
   OptionsPage* page = obj->createOptionsPage( this );
   vk_assert( page != 0 );

   m_optPages.append( page );
   connect( page, SIGNAL(modified()), this, SLOT(modified()) );

   /* handle e.g. user pressing return in an ledit */
   connect( page, SIGNAL(apply()), this, SLOT(apply()) );

   return page;
}


void OptionsWindow::showPage( int catid )
{
   if ( isMinimized() ) {
      setCategory( catid );
      showNormal();
      return;
   }  

   /* been there, done that ... */
   if ( m_xpos != -1 && m_ypos != -1 ) {
      setCategory( catid );
      show();
      return;
   }

   /* first time we've been shown */
   if ( !isVisible() ) {
      adjustSize();
      adjustPosition();
      setCategory( catid );
      show();
   }
}


void OptionsWindow::adjustPosition()
{
   /* need to make sure these events are already sent to be sure our
      information below is correct */
   QApplication::sendPostedEvents( this, QEvent::LayoutHint );
   QApplication::sendPostedEvents( this, QEvent::Resize );

   QWidget *w = topLevelWidget();
   int scrn   = QApplication::desktop()->screenNumber( w );
   QRect desk = QApplication::desktop()->availableGeometry( scrn );
  
   int extraw = w->geometry().x() - w->x();
   int extrah = w->geometry().y() - w->y();
   /* sanity check for decoration frames.  with embedding, we might get
      extraordinary values */
   if ( extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40 ) {
      extrah = 40;
      extraw = 10;
   }

   QPoint p( 0, 0 );
   /* use mapToGlobal rather than geometry() in case w might be
      embedded in another application */
   QPoint pp = w->mapToGlobal( QPoint(0,0) );
   p = QPoint( pp.x() + w->width()/2,
               pp.y() + w->height()/ 2 );
   /* p = origin of this */
   p = QPoint( p.x()-width()/2  - extraw,
               p.y()-height()/2 - extrah );

   if ( p.x() + extraw + width() > desk.x() + desk.width() ) {
      p.setX( desk.x() + desk.width() - width() - extraw );
   }
   if ( p.x() < desk.x() ) {
      p.setX( desk.x() );
   }
   if ( p.y() + extrah + height() > desk.y() + desk.height() ) {
      p.setY( desk.y() + desk.height() - height() - extrah );
   }
   if ( p.y() < desk.y() ) {
      p.setY( desk.y() );
   }
  
   move( p.x(), p.y() );
}


/* save edits to disk
   - applies current page edits
   - saves all changes to disk.

   Need a 'save' because we're not auto-saving 'applied' edits.
   'Apply' just means check edits and update Valkyrie.
   'Save' means save changes to disk for next startup.
   This is because Vk accepts cmdline values... if started with option
   values xyz, don't necessarily want those saved to disk for next run.
   This needs some clear thinking...
*/
void OptionsWindow::save()
{
   if (!apply())
      return;
   
   /* save opts to disk... */
   if ( vkConfig->isDirty() )
      vkConfig->sync( ((MainWindow*)parent())->valkyrie() );

   m_saveButton->setEnabled(  vkConfig->isDirty() );
}


/* reject edits
   - only current page can be in an edited state.
   - don't emit flagsChanged - only 'changed' once they're 'applied'.
*/
void OptionsWindow::reject()
{
   OptionsPage* page = (OptionsPage*)m_wStack->visibleWidget();
   vk_assert( page );
   if ( !page->rejectEdits() ) {
      VK_DEBUG("Failed to reject edits");
   }
}


/* apply edits
   - only current page can be in an edited state.
   - emit flagsChanged to tell Valkyrie option values have been changed.
*/
bool OptionsWindow::apply()
{
   OptionsPage* page = (OptionsPage*)m_wStack->visibleWidget();
   vk_assert( page );
   bool applied = page->applyEdits();
   if (!applied) {
      VK_DEBUG("Failed to apply edits");
      return false;
   }

   /* let the toolviews know that the flags (may) have changed */
   emit flagsChanged();
   return true;
}


/* apply edits and quit if no problems */
void OptionsWindow::accept()
{
   if ( apply() )
      close();
}


/* reset to installation defaults
   - only reset current page
*/
void OptionsWindow::resetDefaults()
{
   OptionsPage* page = (OptionsPage*)m_wStack->visibleWidget();
   vk_assert( page );
   page->resetDefaults();
}


void OptionsWindow::moveEvent( QMoveEvent* me )
{ 
   m_xpos = me->pos().x();
   m_ypos = me->pos().y();
}


void OptionsWindow::closeEvent( QCloseEvent * )
{ hide(); }


/* slot called by page->modified() signal
   - only current page can have been modified.
*/
void OptionsWindow::modified()
{
   bool edited = false;
   OptionsPage* page = (OptionsPage*)m_wStack->visibleWidget();
   vk_assert( page );
   edited = page->isModified();

   m_applyButton->setEnabled( edited );
   m_resetButton->setEnabled( edited );
   m_saveButton->setEnabled(  edited || vkConfig->isDirty() );
}
