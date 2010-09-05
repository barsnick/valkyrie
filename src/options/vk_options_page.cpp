/****************************************************************************
** VkOptionsPage implementation
**  - Each vkObject has different options | flags | prefs, and
**    creates its own 'page', which is inherited from this base class. *
**  - The 'page' is contained within the top-level Options Window.
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
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

#include "options/vk_options_page.h"
#include "options/widgets/opt_base_widget.h"
#include "options/widgets/opt_cb_widget.h"
#include "options/widgets/opt_ck_widget.h"
#include "options/widgets/opt_lb_widget.h"
#include "options/widgets/opt_le_widget.h"
#include "options/widgets/opt_sp_widget.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"

#include <QList>


/***************************************************************************/
/*!
    Constructs an Categories
*/
VkOptionsPage::VkOptionsPage( VkObject* obj )
   : QWidget(), m_vkObj( obj )
{
   this->setObjectName( obj->objectName() + "OptPage" );
   
   m_mod      = false;
   lineHeight = fontMetrics().height();
   
   pageTopVLayout = new QVBoxLayout( this );
   pageTopVLayout->setObjectName( QString::fromUtf8( "pageTopVLayout" ) );
   pageTopVLayout->setContentsMargins( 0, 0, 0, 0 );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
VkOptionsPage::~VkOptionsPage()
{
   m_editList.clear();
   
   // cleanup all option widgets
   foreach( OptionWidget * optw, m_itemList ) {
      delete optw;
      optw = 0;
   }
   m_itemList.clear();
}


/*!
   Initialise page, using Template Method.
   Can't call virt methods from cons, so must provide this
   separate public method.
*/
void VkOptionsPage::init()
{
   // setup the widgets & layout for the object-specific pages
   setupOptions();

   // setup edit signals/slots
   It_OptWidgHash it = m_itemList.begin();

   while ( it != m_itemList.constEnd() ) {
      OptionWidget* widg = it.value();
      connect( widg, SIGNAL( valueChanged( bool, OptionWidget* ) ),
               this, SLOT( updateEditList( bool, OptionWidget* ) ) );
      ++it;
   }
}


/*!
    Provides a default seperator UI element
*/
QFrame* VkOptionsPage::sep( QWidget* parent )
{
   QFrame* line = new QFrame( parent );
   line->setFrameStyle( QFrame::HLine | QFrame::Sunken );
   line->setMinimumHeight( 16 );
   return line;
}


/*!
  updateEditList
  Only here is m_editList managed.
*/
void VkOptionsPage::updateEditList( bool edstate, OptionWidget* optw )
{
   // check this widget isn't already in the list
   int indx = m_editList.indexOf( optw );
   
   // widget has been edited and is not already in the list
   if ( edstate == true && indx == -1 ) {
      m_editList.append( optw );
   }
   else if ( edstate == false && indx != -1 ) {
      // widget was reset and is already in the list
      m_editList.removeAt( indx );
   }
   
   m_mod = m_editList.isEmpty() ? false : true;
   emit modified();
}


/*!
  Reset action activated
*/
void VkOptionsPage::rejectEdits()
{
   if ( m_editList.count() != 0 ) {
      // first copy the list: the list is managed by this->updateEditList().
      QList<OptionWidget*> tmpList = m_editList;
      
      // now remove all the items from m_editList:
      // signal is emitted for by each optw, calls this->updateEditList()
      foreach ( OptionWidget* opt, tmpList ) {
         opt->cancelEdit();
      }
   }
   
   vk_assert( m_mod == false );
   vk_assert( m_editList.isEmpty() == true );
}


/*!
  applyEdits()
  user chose Ok/Apply after editing some items
*/
bool VkOptionsPage::applyEdits()
{
   if ( m_editList.isEmpty() && m_mod == false ) {
      return true;
   }
      
   // verify all edited entries before committing any.
   // TODO: shouldn't need to do this anymore, as we now verify upon
   // edit completion. But keeping it in for now.
   foreach( OptionWidget* optw, m_editList ) {
      if ( !checkOption( optw ) ) {
         return false;
      }
   }
   
   // all edited entries ok: commit them all
   QList<OptionWidget*> tmpList;
   foreach( OptionWidget* optw, m_editList ) {
      // optw->option emits valueChanged() on save:
      optw->saveEdit();
      tmpList.append( optw );
   }
   
   // now remove all the saved items from m_editList
   foreach( OptionWidget* optw, tmpList ) {
      updateEditList( false, optw );
   }
   
   vk_assert( m_mod == false );
   vk_assert( m_editList.isEmpty() == true );
   return true;
}


/*!
  checkOption()
  Check valid option via VkObject
*/
bool VkOptionsPage::checkOption( OptionWidget* opt )
{
   vk_assert( opt );
   vk_assert( opt->id() < (int)m_vkObj->maxOptId() );

   QString argval = opt->currValue();
   int errval = m_vkObj->checkOptArg( opt->id(), argval );
   
   if ( errval != PARSED_OK ) {
      vkError( this, "Invalid Entry", "%s:<br><i>%s</i>",
               parseErrString( errval ),
               qPrintable( opt->printCurrValue() ) );

      opt->cancelEdit();
      return false;
   }
   
   // argval may have been altered by checkOptArg(), e.g. a file path
   if ( argval != opt->currValue() ) {
      opt->setValue( argval );
   }
   
   return true;
}



OptionWidget* VkOptionsPage::insertOptionWidget( int optid,
                                                 QWidget* parent,
                                                 bool mklabel )
{
   OptionWidget* optWidget = 0;
   VkOption* opt = m_vkObj->getOption( optid );
   
   switch ( opt->widgType ) {
   case VkOPT::WDG_NONE:
      vk_assert_never_reached();
      break;
   case VkOPT::WDG_CHECK:
      optWidget = ( OptionWidget* )new CkWidget( parent, opt, mklabel );
      break;
   case VkOPT::WDG_RADIO:
      // TODO
      cerr << "TODO: WDG_RADIO" << endl;
      //      optWidget = (OptionWidget*)new RbWidget( parent, opt, mklabel );
      break;
   case VkOPT::WDG_LEDIT:
      optWidget = ( OptionWidget* )new LeWidget( parent, opt, mklabel );
      break;
   case VkOPT::WDG_COMBO:
      optWidget = ( OptionWidget* )new CbWidget( parent, opt, mklabel );
      break;
   case VkOPT::WDG_LISTBOX:
      optWidget = ( OptionWidget* )new LbWidget( parent, opt, mklabel );
      break;
   case VkOPT::WDG_SPINBOX: {
      SpWidget* spinw = new SpWidget( parent, opt, mklabel, 1 );
      
      bool ok;
      int step = ( opt->argType == VkOPT::ARG_PWR2 ) ? 0 : 1;
      QString val = vkConfig->value( opt->configKey() ).toString();
      int ival = val.toInt( &ok );
      
      if ( !ok ) {
         cerr << "Error in VkOptionsPage::insertOptionWidget( " << optid << " ): "
              << "VkOPT::WDG_SPINBOX: bad int conversion from: '"
              << qPrintable( val ) << "'" << endl;
         // soldier on...
         ival = 0;
      }
      
      spinw->addSection( opt->possValues[0].toInt(),  // min
                         opt->possValues[1].toInt(),  // max
                         ival,                        // default
                         step );                      // step (if 0: pwr2)
                         
      optWidget = ( OptionWidget* )spinw;
   }
   break;
   default:
      // should never be here!
      optWidget = 0;
      break;
   }

   // check validity of option values immediately upon finishing edit
   connect( optWidget, SIGNAL( editDone( OptionWidget* ) ),
            this,        SLOT( checkOption( OptionWidget* ) ) );

   // ------------------------------------------------------------
   // Insert into the itemlist
   m_itemList.insert( optid, optWidget );
   
   //   vk_assert( optWidget != 0 );
   return optWidget;
}
