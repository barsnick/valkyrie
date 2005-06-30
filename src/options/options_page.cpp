/* ---------------------------------------------------------------------- 
 * Implementation of OptionsPage                         options_page.cpp
 * 
 * Each vkObject has different options | flags | prefs, and creates
 * its own 'page', which is inherited from this base class.  The
 * 'page' is contained within the top-level Options Window.
 * ---------------------------------------------------------------------- 
 */

#include <qpopupmenu.h>
#include <qcursor.h>
#include <qfiledialog.h>

#include "options_page.h"
#include "vk_objects.h"
#include "vk_utils.h"
#include "vk_config.h"
#include "context_help.h"
#include "vk_msgbox.h"


/* class OptionsPage --------------------------------------------------- */
OptionsPage::~OptionsPage()
{
  undoList.clear();
  editList.clear();
  itemList.setAutoDelete( true );
  itemList.clear();
}


OptionsPage::OptionsPage( QWidget* parent, VkObject* obj, const char* name )
  : QWidget( parent, name )
{
  vkObj = obj;
  mod   = false;
  topSpace = fontMetrics().height();
}


QFrame* OptionsPage::sep( QWidget* parent, const char* name ) 
{
  QFrame* line = new QFrame( parent, name );
  line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
  return line;
}


void OptionsPage::updateEditList( bool edstate, OptionWidget* optw )
{
  /* check this widget isn't already in the list */
  int indx = editList.findRef( optw );

  /* widget has been edited and is not already in the list */
  if ( edstate == true && indx == -1 ) {
    editList.append( optw );
  }
  else if ( edstate == false && indx != -1 ) {
    /* widget was reset and is already in the list */
    editList.remove( indx );    
  }

  mod = editList.isEmpty() ? false : true;
  emit modified();
}


bool OptionsPage::acceptEdits() 
{
  /* user clicked OK after editing some items */
  if ( editList.count() != 0 ) {
    OptionWidget* optw;
    QPtrList<OptionWidget> tmpList;

    /* user might have clicked OK button first (rather than Apply and
       then Ok), so we need to verify various entries before
       committing them */
    for ( optw=editList.first(); optw; optw=editList.next() ) {
      if ( !applyOptions( optw->id() ) )
        return false;
    }

    /* if we got to here, then we passed all checks */
    for ( optw=editList.first(); optw; optw=editList.next() ) {
      /* no signal emitted: optw is still in editList */
      optw->saveEdit( true );
      tmpList.append( optw );
    }
    /* now remove all the saved items from editList */
    for ( optw=tmpList.first(); optw; optw=tmpList.next() )
      updateEditList( false, optw );
    tmpList.clear();
    undoList.clear();
  }

  /* user clicked Ok after clicking Apply: opt widgets are not REALLY
     saved, but ARE removed from editList. ptrs are in undoList in
     case Cancel was called. */
  if ( undoList.count() != 0 ) {
    OptionWidget* optw;
    QPtrList<OptionWidget> tmpList;

    for ( optw=undoList.first(); optw; optw=undoList.next() ) {
      optw->saveEdit( true);
    }
    undoList.clear();
  }

  vk_assert( mod == false );
  vk_assert( editList.isEmpty() == true );
  return ( !mod && editList.isEmpty() );
}


void OptionsPage::resetDefaults()
{
	int ok = vkQuery( this, 2, "Reset Defaults", 
										"<p>This will reset <b>all</b> options for %s "
										"to the installation defaults.</p>"
										"<p>Continue ?</p>", vkObj->title().latin1() );
  if ( ok == MsgBox::vkYes ) { 
		QIntDictIterator<OptionWidget> it( itemList );
		for ( ;  it.current(); ++it ) {
			it.current()->resetDefault();
		}
	}
}


/* Cancel button clicked */
bool OptionsPage::rejectEdits()
{
  /* user clicked Cancel after clicking Apply */
  if ( undoList.count() != 0 ) {
    OptionWidget* optw;
    QPtrList<OptionWidget> tmpList;

    for ( optw=undoList.first(); optw; optw=undoList.next() ) {
      /* signal is emitted to remove obj from editList */
      optw->cancelEdit();
      applyOptions( optw->id(), true );
    }
    undoList.clear();
  }

  /* user clicked Cancel after editing some items */
  if ( editList.count() != 0 ) {
    OptionWidget* optw;
    QPtrList<OptionWidget> tmpList;

    for ( optw=editList.first(); optw; optw=editList.next() ) {
      tmpList.append( optw );
    }
    /* now remove all the items from editList: signal is emitted */
    for ( optw=tmpList.first(); optw; optw=tmpList.next() ) {
      optw->cancelEdit();
    }
    tmpList.clear();
  }

  vk_assert( mod == false );
  vk_assert( editList.isEmpty() == true );
  vk_assert( undoList.isEmpty() == true );

  return ( !mod && editList.isEmpty() && undoList.isEmpty() );
}


bool OptionsPage::applyEdits()
{
  if ( editList.count() == 0 ) 
    return true;

  OptionWidget* optw;
  for ( optw=editList.first(); optw; optw=editList.next() ) {
    optw->saveEdit( false );
    undoList.append( optw );
    applyOptions( optw->id() );
  }
  /* now remove all the saved opt widgets from editList, but keep a
     ptr to them in undoList */
  for ( optw=undoList.first(); optw; optw=undoList.next() )
    updateEditList( false, optw );

  vk_assert( editList.isEmpty() == true );
  return ( editList.isEmpty() );
}


OptionWidget* OptionsPage::optionWidget( int optid, QWidget* parent, 
                                         bool mklabel )
{
  OptionWidget* optWidget = 0;
  Option* opt = vkObj->findOption( optid );

  switch ( opt->widgType ) {

    case Option::NONE:
      vk_assert_never_reached();
      break;
    case Option::CHECK:
      optWidget = (OptionWidget*)new CkWidget( parent, opt, mklabel );
      break;
    case Option::RADIO:
      optWidget = (OptionWidget*)new RbWidget( parent, opt, mklabel );
      break;
    case Option::LEDIT:
      optWidget = (OptionWidget*)new LeWidget( parent, opt, mklabel );
      break;
    case Option::COMBO:
      optWidget = (OptionWidget*)new CbWidget( parent, opt, mklabel );
      break;
    case Option::LISTBOX:
      optWidget = (OptionWidget*)new LbWidget( parent, opt, mklabel );
      break;
    case Option::SPINBOX: {
			/* 0 == use_powers_of_two, 1 == do not */
			int use_powers = ( opt->key == Memcheck::ALIGNMENT || 
												 opt->key == Massif::ALIGNMENT ) ? 0 : 1;
      SpWidget* spinw = new SpWidget( parent, opt, mklabel, 1 );
      QString ival = vkConfig->rdEntry( opt->cfgKey(), opt->cfgGroup() );
      spinw->addSection( opt->possValues[0].toInt(),  /* min */
                         opt->possValues[1].toInt(),  /* max */
                         ival.toInt(),                /* def */
                         use_powers );
      optWidget = (OptionWidget*)spinw;
		} break;

  }

  vk_assert( optWidget != 0 );
  return optWidget;
}
