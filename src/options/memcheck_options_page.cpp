/****************************************************************************
** MemcheckOptionsPage implementation
**  - subclass of VkOptionsPage to hold memcheck-specific options
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

#include "memcheck_options_page.h"

#include "help/help_context.h"
#include "help/help_urls.h"
#include "objects/memcheck_object.h"
//#include "utils/vk_config.h"
//#include "utils/vk_messages.h"
#include "utils/vk_utils.h"

#include "options/widgets/opt_base_widget.h"
//#include "options/widgets/opt_le_widget.h"
//#include "options/widgets/opt_lb_widget.h"
//#include "options/widgets/opt_ck_widget.h"


#include <QGroupBox>



MemcheckOptionsPage::MemcheckOptionsPage( VkObject* obj )
   : VkOptionsPage( obj )
{
}


void MemcheckOptionsPage::setupOptions()
{
   // group1: memcheck options
   QGroupBox* group1 = new QGroupBox( " Memcheck Options ", this );
   group1->setObjectName( QString::fromUtf8( "MemcheckOptionsPage_group1" ) );
   ContextHelp::addHelp( group1, urlMemcheck::optsMC );
   pageTopVLayout->addWidget( group1 );
   
   insertOptionWidget( MEMCHECK::LEAK_CHECK, group1, true );    // combobox
   insertOptionWidget( MEMCHECK::SHOW_REACH, group1, false );   // checkbox
   //   insertOptionWidget( MEMCHECK::UNDEF_VAL,  group1, false );   // checkbox
   insertOptionWidget( MEMCHECK::TRACK_ORI,  group1, false );   // checkbox
   insertOptionWidget( MEMCHECK::PARTIAL,    group1, false );   // checkbox
   insertOptionWidget( MEMCHECK::GCC_296,    group1, false );   // checkbox
   insertOptionWidget( MEMCHECK::LEAK_RES,   group1, true );    // combobox
   insertOptionWidget( MEMCHECK::FREELIST,   group1, true );    // ledit
   insertOptionWidget( MEMCHECK::ALIGNMENT,  group1, true );    // spinbox
   
   // grid layout for group1
   int i = 0;
   QGridLayout* grid1 = new QGridLayout( group1 );
   grid1->setRowMinimumHeight( i++, lineHeight / 2 ); // blank top row
   //   grid1->setColStretch( 1, 10 );         // push widgets to the left
   
   grid1->addLayout( m_itemList[MEMCHECK::LEAK_CHECK]->hlayout(), i++, 0 );
   grid1->addWidget( m_itemList[MEMCHECK::SHOW_REACH]->widget(),  i++, 0 );
   //grid1->addWidget( m_itemList[MEMCHECK::UNDEF_VAL]->widget(),   i++, 0 );
   
   grid1->addWidget( sep( group1 ), i++, 0, 1, 2 );
   grid1->addWidget( m_itemList[MEMCHECK::TRACK_ORI]->widget(),   i++, 0 );
   grid1->addWidget( m_itemList[MEMCHECK::PARTIAL]->widget(),     i++, 0 );
   grid1->addWidget( m_itemList[MEMCHECK::GCC_296]->widget(),     i++, 0 );
   
   grid1->addWidget( sep( group1 ), i++, 0, 1, 2 );
   grid1->addLayout( m_itemList[MEMCHECK::LEAK_RES]->hlayout(),    i++, 0 );
   grid1->addLayout( m_itemList[MEMCHECK::FREELIST]->hlayout(),    i++, 0 );
   grid1->addLayout( m_itemList[MEMCHECK::ALIGNMENT ]->hlayout(),  i++, 0 );
   
   pageTopVLayout->addStretch( 1 );
   
   
   // ============================================================
   // Disabled Widgets
   /* Valgrind presets and ignores some options when generating xml
      output. (see docs/internals/xml_output.txt) */
   m_itemList[MEMCHECK::LEAK_CHECK]->setEnabled( false );
   
   // sanity checks
   vk_assert( m_itemList.count() <= MEMCHECK::NUM_OPTS );
}
