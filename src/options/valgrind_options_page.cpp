/****************************************************************************
** ValgrindOptionsPage implementation
**  - subclass of VkOptionsPage to hold valgrind-specific options
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

#include <QDir>
#include <QListWidget>
#include <QTabWidget>

#include "help/help_context.h"
#include "help/help_urls.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"

#include "objects/valgrind_object.h"
#include "options/widgets/opt_base_widget.h"
#include "options/widgets/opt_le_widget.h"
#include "options/widgets/opt_lb_widget.h"
#include "options/valgrind_options_page.h"



/* from valgrind/coregrind/pub_core_options.h
   = maximum number of suppression files */
#define VG_CLO_MAX_SFILES 10


/***************************************************************************/
/*!
  Valgrind Options Page
  This page is different from the others in that it uses three tabs
  because there are just too many options to put on one page.
  (a) general core options,
  (b) error-reporting options, and
  (c) suppression-related options
*/
ValgrindOptionsPage::ValgrindOptionsPage( VkObject* obj )
   : VkOptionsPage( obj )
{
}


void ValgrindOptionsPage::setupOptions()
{
   group1 = new QGroupBox( " Valgrind Options ", this );
   group1->setObjectName( QString::fromUtf8( "ValgrindOptionsPage_group1" ) );
   pageTopVLayout->addWidget( group1 );
   
   QTabWidget* tabWidget = new QTabWidget( this );
   tabWidget->setObjectName( QString::fromUtf8( "ValgrindOptionsPage_tabWidget" ) );
   pageTopVLayout->addWidget( tabWidget );
   
   // ============================================================
   // tabCore - tab 1: core options
   QWidget* tabCore = new QWidget( tabWidget );
   tabCore->setObjectName( QString::fromUtf8( "tab_core" ) );
   tabWidget->addTab( tabCore, " Core " );
   ContextHelp::addHelp( tabCore, urlValkyrie::coreTab );
   
   // tabCore - vbox
   QVBoxLayout* core_vbox = new QVBoxLayout( tabCore );
   core_vbox->setObjectName( QString::fromUtf8( "core_vbox" ) );
   
   // ------------------------------------------------------------
   // tabCore - group box 1
   QGroupBox* cgroup1 = new QGroupBox( " Common Options ", tabCore );
   cgroup1->setObjectName( QString::fromUtf8( "cgroup1" ) );
   core_vbox->addWidget( cgroup1 ); // , m_space
   
   // tabCore - group box 1 - options
   insertOptionWidget( VALGRIND::TOOL,       cgroup1, true );   // combobox
   insertOptionWidget( VALGRIND::VERBOSITY,  cgroup1, true );   // spinbox
   insertOptionWidget( VALGRIND::XML_OUTPUT, cgroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::TRACE_CH,   cgroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::SILENT_CH,  cgroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::TRACK_FDS,  cgroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::TIME_STAMP, cgroup1, false );  // checkbox
   
   // tabCore - group box 1 - layout
   int i = 0;
   QGridLayout* cgrid1 = new QGridLayout( cgroup1 );
   cgrid1->setRowMinimumHeight( i++, lineHeight / 2 ); // blank top row
   cgrid1->addLayout( m_itemList[VALGRIND::TOOL      ]->hlayout(), i++, 0 );
   cgrid1->addLayout( m_itemList[VALGRIND::VERBOSITY ]->hlayout(), i++, 0 );
   cgrid1->addWidget( m_itemList[VALGRIND::XML_OUTPUT]->widget(),  i++, 0 );
   cgrid1->addWidget( m_itemList[VALGRIND::TRACE_CH  ]->widget(),  i++, 0 );
   cgrid1->addWidget( m_itemList[VALGRIND::SILENT_CH ]->widget(),  i++, 0 );
   cgrid1->addWidget( m_itemList[VALGRIND::TRACK_FDS ]->widget(),  i++, 0 );
   cgrid1->addWidget( m_itemList[VALGRIND::TIME_STAMP]->widget(),  i++, 0 );
   
   
   // ------------------------------------------------------------
   // tabCore - group box 2
   QGroupBox* cgroup2 = new QGroupBox( " Less Common Options ", tabCore );
   cgroup2->setObjectName( QString::fromUtf8( "cgroup2" ) );
   core_vbox->addWidget( cgroup2 ); // , m_space
   
   // tabCore - group box 2 - options
   insertOptionWidget( VALGRIND::RUN_LIBC,  cgroup2, false );  // checkbox
   insertOptionWidget( VALGRIND::EM_WARNS,  cgroup2, false );  // checkbox
   insertOptionWidget( VALGRIND::SMC_CHECK, cgroup2, true );   // combobox
   insertOptionWidget( VALGRIND::SIM_HINTS, cgroup2, true );   // combobox
   insertOptionWidget( VALGRIND::KERN_VAR,  cgroup2, true );   // combobox
   
   // tabCore - group box 2 - layout
   i = 0;
   QGridLayout* cgrid2 = new QGridLayout( cgroup2 );
   cgrid1->setRowMinimumHeight( i++, lineHeight / 2 ); // blank top row
   cgrid2->addWidget( m_itemList[VALGRIND::RUN_LIBC ]->widget(),  i++, 0 );
   cgrid2->addWidget( m_itemList[VALGRIND::EM_WARNS ]->widget(),  i++, 0 );
   cgrid2->addLayout( m_itemList[VALGRIND::SMC_CHECK]->hlayout(), i++, 0 );
   cgrid2->addLayout( m_itemList[VALGRIND::SIM_HINTS]->hlayout(), i++, 0 );
   cgrid2->addLayout( m_itemList[VALGRIND::KERN_VAR ]->hlayout(), i++, 0 );
   
   // v. stretch at bottom
   core_vbox->addStretch( 1 );
   
   
   // ============================================================
   // tabErep - tab 2: error-reporting
   QWidget* tabErep = new QWidget( tabWidget );
   tabErep->setObjectName( QString::fromUtf8( "tab_erep" ) );
   tabWidget->addTab( tabErep, " Error Reporting " );
   ContextHelp::addHelp( tabErep, urlValkyrie::errorTab );
   
   // tabErep - vbox
   QVBoxLayout* erep_vbox = new QVBoxLayout( tabErep );
   erep_vbox->setObjectName( QString::fromUtf8( "erep_vbox" ) );
   
   // ------------------------------------------------------------
   // tabErep - group box 1
   QGroupBox* egroup1 = new QGroupBox( " Options ", tabErep );
   egroup1->setObjectName( QString::fromUtf8( "egroup1" ) );
   erep_vbox->addWidget( egroup1 ); // , m_space
   
   // tabErep - group box 1 - options
   insertOptionWidget( VALGRIND::GEN_SUPP,    egroup1, true );   // combobox
   insertOptionWidget( VALGRIND::DEMANGLE,    egroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::ERROR_LIMIT, egroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::SHOW_BELOW,  egroup1, false );  // checkbox
   insertOptionWidget( VALGRIND::NUM_CALLERS, egroup1, true );   // intspin
   insertOptionWidget( VALGRIND::MAX_SFRAME,  egroup1, true );   // spinbox
   
   insertOptionWidget( VALGRIND::DB_ATTACH,  egroup1, false );   // checkbox
   insertOptionWidget( VALGRIND::DB_COMMAND, egroup1, false );   // ledit+button
   LeWidget* dbLedit = (( LeWidget* )m_itemList[VALGRIND::DB_COMMAND] );
   dbLedit->addButton( egroup1, this, SLOT( getDbBin() ) );
   
   insertOptionWidget( VALGRIND::INPUT_FD,   egroup1, true );    // spinbox
   insertOptionWidget( VALGRIND::LOG_FD,     egroup1, true );    // spinbox
   insertOptionWidget( VALGRIND::LOG_FILE,   egroup1, true );    // ledit
   insertOptionWidget( VALGRIND::LOG_SOCKET, egroup1, true );    // ledit
   
   // tabErep - group box 1 - layout
   i = 0;
   QGridLayout* egrid1 = new QGridLayout( egroup1 );
   egrid1->setRowMinimumHeight( i++, lineHeight / 2 ); // blank top row
   egrid1->addLayout( m_itemList[VALGRIND::GEN_SUPP   ]->hlayout(),  i++, 0 );
   egrid1->addWidget( m_itemList[VALGRIND::DEMANGLE   ]->widget(),   i++, 0 );
   egrid1->addWidget( m_itemList[VALGRIND::ERROR_LIMIT]->widget(),   i++, 0 );
   egrid1->addWidget( m_itemList[VALGRIND::SHOW_BELOW ]->widget(),   i++, 0 );
   egrid1->addLayout( m_itemList[VALGRIND::NUM_CALLERS]->hlayout(),  i++, 0 );
   egrid1->addLayout( m_itemList[VALGRIND::MAX_SFRAME ]->hlayout(),  i++, 0 );
   egrid1->addWidget( m_itemList[VALGRIND::DB_ATTACH  ]->widget(),   i++, 0 );
   egrid1->addLayout( m_itemList[VALGRIND::DB_COMMAND ]->hlayout(),  i++, 0 );
   
   egrid1->addWidget( sep( egroup1 ), i++, 0, 1, 2 );
   
   QHBoxLayout* hBox = new QHBoxLayout();
   hBox->addLayout( m_itemList[VALGRIND::INPUT_FD]->hlayout() );
   hBox->addLayout( m_itemList[VALGRIND::LOG_FD]->hlayout() );
   egrid1->addLayout( hBox,                                         i++, 0 );
   egrid1->addLayout( m_itemList[VALGRIND::LOG_FILE   ]->hlayout(), i++, 0 );
   egrid1->addLayout( m_itemList[VALGRIND::LOG_SOCKET ]->hlayout(), i++, 0 );
   
   // v. stretch at bottom
   erep_vbox->addStretch( 1 );
   
   
   
   // ============================================================
   // tabSupps - tab 3: suppressions
   QWidget* tabSupps = new QWidget( tabWidget );
   tabSupps->setObjectName( QString::fromUtf8( "tab_supps" ) );
   tabWidget->addTab( tabSupps, " Suppressions " );
   ContextHelp::addHelp( tabSupps, urlValkyrie::suppsTab );
   
   // tabErep - vbox
   QVBoxLayout* supp_vbox = new QVBoxLayout( tabSupps );
   supp_vbox->setObjectName( QString::fromUtf8( "supp_vbox" ) );
   
   // tabErep - options
   insertOptionWidget( VALGRIND::SUPPS_SEL, tabSupps, true );  // listbox
   
   LbWidget* lbSel = ( LbWidget* )m_itemList[VALGRIND::SUPPS_SEL];
   supp_vbox->addLayout( lbSel->vlayout() );
   
   
   
   // ============================================================
   // Disabled Widgets
   /* These widgets are disabled because Valkyrie uses
      --log-file-exactly internally: logging options would interfere
      with this. */
   // error reporting tab
   m_itemList[VALGRIND::LOG_FD     ]->setEnabled( false );
   m_itemList[VALGRIND::LOG_FILE   ]->setEnabled( false );
   m_itemList[VALGRIND::LOG_SOCKET ]->setEnabled( false );
   
   /* Disabled because Valkyrie always requires xml output from Valgrind */
   m_itemList[VALGRIND::XML_OUTPUT ]->setEnabled( false );
   
   /* Disabled for now: Only supporting memcheck so far. */
   m_itemList[VALGRIND::TOOL       ]->setEnabled( false );
   
   /* Disabled for now - can't deal with the multiple xml files this generates */
   /* Note: Also disabled in Valgrind::checkOptArg() */
   m_itemList[VALGRIND::TRACE_CH   ]->setEnabled( false );
   
   /* Disabled - must be left on to generate clean XML */
   /* Note: Also disabled in Valgrind::checkOptArg() */
   m_itemList[VALGRIND::SILENT_CH  ]->setEnabled( false );
   
   /* Disabled for now - not yet implemented */
   m_itemList[VALGRIND::INPUT_FD   ]->setEnabled( false );
   
   /* Disabled for now - Valgrind presets these options for XML output
      - See valgrind/docs/internals/xml_output.txt */
   m_itemList[VALGRIND::VERBOSITY  ]->setEnabled( false );
   m_itemList[VALGRIND::TRACK_FDS  ]->setEnabled( false );
   m_itemList[VALGRIND::EM_WARNS   ]->setEnabled( false );
   m_itemList[VALGRIND::GEN_SUPP   ]->setEnabled( false );
   m_itemList[VALGRIND::ERROR_LIMIT]->setEnabled( false );
   m_itemList[VALGRIND::DB_ATTACH  ]->setEnabled( false );
   m_itemList[VALGRIND::DB_COMMAND ]->setEnabled( false );
   dbLedit->button()->setEnabled( false );
   
   vk_assert( m_itemList.count() <= VALGRIND::NUM_OPTS );
}


void ValgrindOptionsPage::getDbBin()
{
   vkPrintErr( "TODO: ValgrindOptionsPage::getDbBin()\n" );
}


#if 0 //TODO
{
   if ((( QListWidget* )lbSel->widget() )->count() < VG_CLO_MAX_SFILES ) {
      lbSel->insertItem( suppr );
   }
   else {
      // valgrind doesn't accept any more suppression files
      vkError( this, "Error",
               "Valgrind won't accept more than %d suppression files",
               VG_CLO_MAX_SFILES );
   }
}
#endif
