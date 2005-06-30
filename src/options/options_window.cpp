/* ---------------------------------------------------------------------- 
 * Implementation of OptionsWindow                     options_window.cpp
 * A small container class for each tool's options / flags 'pane'.
 * Not modal, so user can keep it open and change flags as they work.
 * ---------------------------------------------------------------------- 
 */

#include <qapplication.h>
#include <qframe.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qstatusbar.h>

#include "options_window.h"
#include "vk_utils.h"
#include "vk_objects.h"
#include "vk_config.h"
#include "main_window.h"

#include "valkyrie_options_page.h"
#include "valgrind_options_page.h"
#include "memcheck_options_page.h"
#include "cachegrind_options_page.h"
#include "massif_options_page.h"


/* class Categories ---------------------------------------------------- */
Categories::Categories( QWidget* parent )
  : QListBox( parent, "cat_listbox" ) 
{
  QFont fnt = font();
  fnt.setWeight( QFont::Bold );
  setFont( fnt );

  QFontMetrics fm = fontMetrics();
  categht = fm.height() * 2; 
}

int Categories::categHeight() 
{ return categht; }


/* class CategItem ----------------------------------------------------- */
CategItem::CategItem( QListBox* parent, OptionsPage* page,
                      const QString &txt, int id )
  : QListBoxItem(parent), catid(id), optpage( page )
{ setText( txt ); }

int CategItem::height( const QListBox * ) const
{ return ((Categories*)listBox())->categHeight(); }

int CategItem::catId() const 
{ return catid; }

void CategItem::setWidget( OptionsPage* page ) 
{ 
  vk_assert( optpage == NULL ); 
  optpage = page; 
}

OptionsPage * CategItem::page() const 
{ return optpage; }

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
{ optPages.clear(); }


OptionsWindow::OptionsWindow( QWidget* parent ) 
  : QMainWindow( parent, "options_win" )
{
  capt.sprintf("%s Options: ", vkConfig->vkName() );
  setCaption( capt );
  statusBar()->setSizeGripEnabled( false );

  optPages.setAutoDelete( true );
  xpos = ypos = -1;

  /* status bar */
  QFrame *statusFrame = new QFrame( statusBar() );
  statusBar()->addWidget( statusFrame, 10, true );
  QHBoxLayout* buttLayout = new QHBoxLayout(statusFrame, 5, -1 );

  /* reset: reset default options */
  QPushButton* pb = new QPushButton( "Reset Defaults", statusFrame );
  connect( pb, SIGNAL(clicked() ), this, SLOT(resetDefaults()));
  buttLayout->addWidget( pb );
  buttLayout->addStretch( 10 );

  int w = fontMetrics().width( "X&CancelX" );
  /* okay: apply and quit in one go */
  pb = new QPushButton( "&Ok", statusFrame );
  pb->setFixedWidth( w );
  pb->setDefault( true );
  buttLayout->addWidget( pb );
  connect( pb, SIGNAL(clicked() ), this, SLOT(accept()) );
  /* cancel: forget everything I just said, and quit */
  pb = new QPushButton( "&Cancel", statusFrame );
  pb->setFixedWidth( w );
  buttLayout->addWidget( pb );
  connect( pb, SIGNAL(clicked() ), this, SLOT(reject()));
  /* apply: do what I said, but let me change my mind */
  applyButton = new QPushButton( "&Apply", statusFrame );
  applyButton->setFixedWidth( w );
  buttLayout->addWidget( applyButton );
  connect( applyButton, SIGNAL(clicked() ), this, SLOT(apply()));
  applyButton->setEnabled( false );  /* nothing to apply yet */

  QSplitter* splitter = new QSplitter( this );
  setCentralWidget( splitter );

  /* category chooser */
  categories = new Categories( splitter );
  connect( categories, SIGNAL( clicked( QListBoxItem *) ),
           this,       SLOT( categoryClicked( QListBoxItem *) ) );

  /* stack for the various widgets */
  wStack = new QWidgetStack( splitter );

  /* we create the containers, but don't actually initialise them
     unless the user wants to view a page.*/
  VkObjectList objList = vkConfig->vkObjList();
  VkObject* obj;
  for ( obj = objList.first(); obj; obj = objList.next() ) {
    addCategory( obj->id(), obj->title() );
  }
}


void OptionsWindow::addCategory( int cid, QString text )
{
  OptionsPage* page = NULL;
  wStack->addWidget( page, cid );
  CategItem* item = new CategItem( categories, page, text, cid );

  if ( cid < 3 && 
       categories->height() < (3 * item->height(categories)) ) { 
    categories->setMinimumHeight( 3 * item->height( categories ) ); 
  }
}


void OptionsWindow::setCategory( int catid )
{
  if ( catid != -1 ) {
    categories->setCurrentItem( catid );
    categoryClicked( categories->item(catid) );
  }
}


/* we make the widgets on demand here */
void OptionsWindow::categoryClicked( QListBoxItem *item )
{
  if ( item ) {
    CategItem* cit = (CategItem*)item;
    //capt.sprintf("%s Options: %s", vkName(), item->text().latin1() );
    setCaption( capt + item->text() );

    /* first time this item has been selected */
    if ( !cit->page() ) {
      OptionsPage* page = mkOptionsPage( cit->catId() );
      if ( !page ) {
        VK_DEBUG("cit->text = %s", cit->text().ascii() );
        return;
      } else {
        cit->setWidget( page );
        wStack->addWidget( page, cit->catId() );
      } 
    }

    wStack->raiseWidget( cit->page() );
  }

}


OptionsPage * OptionsWindow::mkOptionsPage( int catid )
{
  VkObject* obj = vkConfig->vkObject( catid, false );
  OptionsPage* page = 0;

  switch ( obj->id() ) {

    case VkObject::VALKYRIE: {
      page = (OptionsPage*)new ValkyrieOptionsPage( this, obj );
    } break;
    case VkObject::VALGRIND: {
      page = (OptionsPage*)new ValgrindOptionsPage( this, obj );
    } break;
    case VkObject::MEMCHECK: {
      page = (OptionsPage*)new MemcheckOptionsPage( this, obj );
    } break;
    case VkObject::CACHEGRIND: {
      page = (OptionsPage*)new CachegrindOptionsPage( this, obj );
    } break;
    case VkObject::MASSIF: {
      page = (OptionsPage*)new MassifOptionsPage( this, obj );
    } break;
    default: {
      vk_assert_never_reached();
    } break;

  }
  vk_assert( page != 0 );

  optPages.append( page );
  connect( page, SIGNAL(modified()), this, SLOT(modified()) );

  return page;
}


void OptionsWindow::showPage( int catid )
{
  if ( isMinimized() ) {
    setCategory( catid );
    showNormal();
    raise();
    return;
  }  

  /* been there, done that ... */
  if ( xpos != -1 && ypos != -1 ) {
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
  /* sanity check for decoration frames. With embedding, we might get
     extraordinary values */
  if ( extraw == 0 || extrah == 0 || extraw >= 10 || extrah >= 40 ) {
    extrah = 40;
    extraw = 10;
  }

  QPoint p( 0, 0 );
  /* Use mapToGlobal rather than geometry() in case w might be
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


void OptionsWindow::accept()
{
  OptionsPage* page;
  for ( page = optPages.first(); page; page = optPages.next() ) {
    if ( !page->acceptEdits() ) {
      VK_DEBUG("Failed to save edits");
      return;  /* don't close window */
    }
  }

	/* let the toolviews know that the flags (may) have changed */
  emit flagsChanged();
	/* then tell MainWin to (possibly) reload the flagsWidget */
	MainWindow* vkWin = (MainWindow*)qApp->mainWidget();
	vkWin->updateFlagsWidget();

  close();
}


void OptionsWindow::reject()
{
  OptionsPage* page;
  for ( page = optPages.first(); page; page = optPages.next() ) {
    if ( !page->rejectEdits() ) {
      VK_DEBUG("Failed to reject edits");
    }
  }

  close(); 
}


void OptionsWindow::apply()
{
  OptionsPage* page;
  for ( page = optPages.first(); page; page = optPages.next() ) {
    if ( !page->applyEdits() ) {
      VK_DEBUG("Failed to apply edits");
    }
  }
}


void OptionsWindow::resetDefaults()
{
  /* get the current page */
  int catid = categories->currentItem();
  if ( catid != -1 ) {
    CategItem* cit = (CategItem*)categories->item(catid);
    OptionsPage* optpage = cit->page();
    optpage->resetDefaults();
  }
}


void OptionsWindow::moveEvent( QMoveEvent* me )
{ 
  xpos = me->pos().x();
  ypos = me->pos().y();
}


void OptionsWindow::closeEvent( QCloseEvent * )
{ hide(); }


void OptionsWindow::modified()
{
  bool edited = false;
  OptionsPage* page;
  for ( page = optPages.first(); page; page = optPages.next() ) {
    if ( page->isModified() ) {
      edited = true;
      break;
    }
  }

  applyButton->setEnabled( edited );
}
