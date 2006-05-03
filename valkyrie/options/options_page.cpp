/* ---------------------------------------------------------------------- 
 * Implementation of class OptionsPage                   options_page.cpp
 * 
 * Each vkObject has different options | flags | prefs, and 
 * creates its own 'page', which is inherited from this base class.  
 * The 'page' is contained within the top-level Options Window.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qpopupmenu.h>
#include <qcursor.h>
#include <qfiledialog.h>

#include "options_page.h"
#include "valgrind_object.h"   /* Valgrind::enums */
#include "valkyrie_object.h"   /* Valkyrie::enums */
//#include "memcheck_object.h"
//#include "massif_object.h"
#include "vk_utils.h"
#include "vk_config.h"
#include "context_help.h"
#include "vk_messages.h"
#include "vk_option.h"


/* class OptionsPage --------------------------------------------------- */
OptionsPage::~OptionsPage()
{
   m_editList.clear();
   m_itemList.setAutoDelete( true );
   m_itemList.clear();
}


OptionsPage::OptionsPage( QWidget* parent, VkObject* obj, const char* name )
   : QWidget( parent, name )
{
   m_vkObj    = obj;
   m_mod      = false;
   m_topSpace = fontMetrics().height();
   m_space    = 5;
   m_margin   = 10;
}


int OptionsPage::optId()
{
   vk_assert(m_vkObj != 0);
   return m_vkObj->objId();
}


QFrame* OptionsPage::sep( QWidget* parent, const char* name ) 
{
   QFrame* line = new QFrame( parent, name );
   line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
   return line;
}


void OptionsPage::resetDefaults()
{
   int ok = vkQuery( this, 2, "Reset Defaults", 
                     "<p>This will reset <b>all</b> options for the "
                     "option page %s to the installation defaults.<br/>"
                     "(Other option pages will be unaffected).</p>"
                     "<p>Continue ?</p>", m_vkObj->title().latin1() );
   if ( ok == MsgBox::vkYes ) { 
      QIntDictIterator<OptionWidget> it( m_itemList );
      for ( ;  it.current(); ++it ) {
         it.current()->resetDefault();
      }
   }
}


void OptionsPage::updateEditList( bool edstate, OptionWidget* optw )
{
   /* check this widget isn't already in the list */
   int indx = m_editList.findRef( optw );

   /* widget has been edited and is not already in the list */
   if ( edstate == true && indx == -1 ) {
      m_editList.append( optw );
   }
   else if ( edstate == false && indx != -1 ) {
      /* widget was reset and is already in the list */
      m_editList.remove( indx );    
   }

   m_mod = m_editList.isEmpty() ? false : true;
   emit modified();
}


/* reset button clicked */
bool OptionsPage::rejectEdits()
{
   if ( m_editList.count() != 0 ) {
      OptionWidget* optw;
      QPtrList<OptionWidget> tmpList;

      for ( optw=m_editList.first(); optw; optw=m_editList.next() ) {
         tmpList.append( optw );
      }
      /* now remove all the items from m_editList: signal is emitted */
      for ( optw=tmpList.first(); optw; optw=tmpList.next() ) {
         optw->cancelEdit();
      }
      tmpList.clear();
   }

   vk_assert( m_mod == false );
   vk_assert( m_editList.isEmpty() == true );

   return ( !m_mod && m_editList.isEmpty() );
}


/* called from OptionsWindow::apply(), accept() */
bool OptionsPage::applyEdits()
{
   if ( m_editList.isEmpty() && m_mod == false ) 
      return true;

   /* user clicked OK / apply after editing some items */
   OptionWidget* optw;
   QPtrList<OptionWidget> tmpList;

   /* verify entries before committing them */
   for ( optw=m_editList.first(); optw; optw=m_editList.next() ) {
      if ( !applyOptions( optw->id() ) )
         return false;
   }
   
   /* if we got to here, then we passed all checks */
   for ( optw=m_editList.first(); optw; optw=m_editList.next() ) {
      /* no signal emitted: optw is still in m_editList */
      optw->saveEdit( true );
      tmpList.append( optw );
   }
   /* now remove all the saved items from m_editList */
   for ( optw=tmpList.first(); optw; optw=tmpList.next() )
      updateEditList( false, optw );
   tmpList.clear();

   vk_assert( m_mod == false );
   vk_assert( m_editList.isEmpty() == true );
   return true;
}


OptionWidget* OptionsPage::optionWidget( int optid, QWidget* parent, 
                                         bool mklabel )
{
   OptionWidget* optWidget = 0;
   Option* opt = m_vkObj->findOption( optid );

   switch ( opt->m_widgType ) {

   case VkOPTION::WDG_NONE:
      vk_assert_never_reached();
      break;
   case VkOPTION::WDG_CHECK:
      optWidget = (OptionWidget*)new CkWidget( parent, opt, mklabel );
      break;
   case VkOPTION::WDG_RADIO:
      optWidget = (OptionWidget*)new RbWidget( parent, opt, mklabel );
      break;
   case VkOPTION::WDG_LEDIT:
      optWidget = (OptionWidget*)new LeWidget( parent, opt, mklabel );
      break;
   case VkOPTION::WDG_COMBO:
      optWidget = (OptionWidget*)new CbWidget( parent, opt, mklabel );
      break;
   case VkOPTION::WDG_LISTBOX:
      optWidget = (OptionWidget*)new LbWidget( parent, opt, mklabel );
      break;
   case VkOPTION::WDG_SPINBOX: {
      int step = (opt->m_argType == VkOPTION::ARG_PWR2) ? 0 : 1;

      SpWidget* spinw = new SpWidget( parent, opt, mklabel, 1 );
      QString ival = vkConfig->rdEntry( opt->cfgKey(), opt->cfgGroup() );
      spinw->addSection( opt->m_possValues[0].toInt(),  /* min */
                         opt->m_possValues[1].toInt(),  /* max */
                         ival.toInt(),                /* def */
                         step );        /* step (if 0: pwr2) */
      optWidget = (OptionWidget*)spinw;
   } break;

   }

   vk_assert( optWidget != 0 );
   return optWidget;
}
