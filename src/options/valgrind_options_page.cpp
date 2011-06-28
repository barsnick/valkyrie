/****************************************************************************
** ValgrindOptionsPage implementation
**  - subclass of VkOptionsPage to hold valgrind-specific options
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

#include <QDir>
#include <QFileDialog>
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
   
   tabWidget = new QTabWidget( this );
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
   
   // tabSupps - suppression files
   QGroupBox* suppfile_groupbox = new QGroupBox();
   QHBoxLayout* suppfile_hbox = new QHBoxLayout();
   suppfile_hbox->setObjectName( QString::fromUtf8( "suppfile_hbox" ) );
   suppfile_hbox->setMargin(0);
   // listbox(options)
   insertOptionWidget( VALGRIND::SUPPS_SEL, tabSupps, false );  // listbox
   LbWidget* lbSel = ( LbWidget* )m_itemList[VALGRIND::SUPPS_SEL];
   QListWidget* lwSuppFiles = (QListWidget*)lbSel->widget();
   connect( lwSuppFiles, SIGNAL(currentRowChanged(int)), this, SLOT(setSuppFileBtns()) );
   connect( lwSuppFiles, SIGNAL(currentRowChanged(int)), this, SLOT(suppLoad()) );
   // buttongroup
   QWidget* butts1_groupbox = new QWidget();
   QVBoxLayout* butts1_vbox = new QVBoxLayout();
   butts1_vbox->setObjectName( QString::fromUtf8( "butts1_vbox" ) );
   butts1_vbox->setMargin(0);
   
   QIcon icon_arrow_up;
   icon_arrow_up.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/arrow_up.png" ) ) );
   QIcon icon_arrow_down;
   icon_arrow_down.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/arrow_down.png" ) ) );
   
   btn_suppfile_up  = new QPushButton( icon_arrow_up, "", butts1_groupbox );
   btn_suppfile_dwn = new QPushButton( icon_arrow_down, "", butts1_groupbox );
   btn_suppfile_new = new QPushButton("New", butts1_groupbox );
   btn_suppfile_add = new QPushButton("Add", butts1_groupbox );
   btn_suppfile_rmv = new QPushButton("Remove", butts1_groupbox );
   setSuppFileBtns();
   connect( btn_suppfile_up,  SIGNAL(clicked()), this, SLOT( suppfileUp() ) );
   connect( btn_suppfile_dwn, SIGNAL(clicked()), this, SLOT( suppfileDown() ) );
   connect( btn_suppfile_new, SIGNAL(clicked()), this, SLOT( suppfileNew() ) );
   connect( btn_suppfile_add, SIGNAL(clicked()), this, SLOT( suppfileAdd() ) );
   connect( btn_suppfile_rmv, SIGNAL(clicked()), this, SLOT( suppfileRemove() ) );
   butts1_vbox->addWidget( btn_suppfile_up  );
   butts1_vbox->addWidget( btn_suppfile_dwn );
   butts1_vbox->addWidget( btn_suppfile_new );
   butts1_vbox->addWidget( btn_suppfile_add );
   butts1_vbox->addWidget( btn_suppfile_rmv );
   butts1_vbox->addStretch( 1 );
   butts1_groupbox->setLayout( butts1_vbox );
   // setup horizontal layout
   suppfile_hbox->addWidget( lbSel->widget() );
   suppfile_hbox->addWidget( butts1_groupbox );
   suppfile_groupbox->setLayout( suppfile_hbox );
   
   // tabSupps - suppressions
   QGroupBox* supps_groupbox = new QGroupBox();
   QHBoxLayout* supps_hbox = new QHBoxLayout();
   supps_hbox->setObjectName( QString::fromUtf8( "supps_hbox" ) );
   supps_hbox->setMargin(0);
   // listview
   lwSupps = new QListWidget( supps_groupbox );
   lwSupps->setObjectName( QString::fromUtf8( "list_widget_supps" ) );
   lwSupps->setSelectionMode( QAbstractItemView::SingleSelection );//QListWidget::Single );
   ContextHelp::addHelp( lwSupps, urlValkyrie::suppsTab );
   connect( lwSupps, SIGNAL(currentRowChanged(int)), this, SLOT(setSuppBtns()) );
   connect( lwSupps, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           this,       SLOT(suppEdit(QListWidgetItem*)) );
   
   // buttongroup
   QWidget* butts_groupbox = new QWidget();
   QVBoxLayout* butts_vbox = new QVBoxLayout();
   butts_vbox->setObjectName( QString::fromUtf8( "butts_vbox" ) );
   butts_vbox->setMargin(0);
   btn_supp_new = new QPushButton("New", butts_groupbox );
   btn_supp_edt = new QPushButton("Edit", butts_groupbox );
   btn_supp_del = new QPushButton("Delete", butts_groupbox );
   suppLoad();
   connect( btn_supp_new, SIGNAL(clicked()), this, SLOT( suppNew() ) );
   connect( btn_supp_edt, SIGNAL(clicked()), this, SLOT( suppEdit() ) );
   connect( btn_supp_del, SIGNAL(clicked()), this, SLOT( suppDelete() ) );
   butts_vbox->addWidget( btn_supp_new );
   butts_vbox->addWidget( btn_supp_edt );
   butts_vbox->addWidget( btn_supp_del );
   butts_vbox->addStretch( 1 );
   butts_groupbox->setLayout( butts_vbox );
   // setup horizontal layout   
   supps_hbox->addWidget( lwSupps );
   supps_hbox->addWidget( butts_groupbox );
   supps_groupbox->setLayout( supps_hbox );
   
   // tabSupps - Add everything to our top vbox layout
   QVBoxLayout* suppressions_vlayout = new QVBoxLayout();
   suppressions_vlayout->setObjectName( QString::fromUtf8( "suppfile_vlayout" ) );
   suppressions_vlayout->addWidget( suppfile_groupbox );
   suppressions_vlayout->addWidget( supps_groupbox );
   tabSupps->setLayout( suppressions_vlayout );
   
   
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
   m_itemList[VALGRIND::ERROR_LIMIT]->setEnabled( false );
   m_itemList[VALGRIND::DB_ATTACH  ]->setEnabled( false );
   m_itemList[VALGRIND::DB_COMMAND ]->setEnabled( false );
   dbLedit->button()->setEnabled( false );

   vk_assert( m_itemList.count() <= VALGRIND::NUM_OPTS );


   // ------------------------------------------------------------
   // tooltips
   // TODO: put these in the options.
   QString tip_supp = tr( "Tip: To enable generation of suppressions "
                         "from Valgrind output, set this to 'all'.<br/>"
                         "Then right-click an error item to generate a suppression.");
   m_itemList[VALGRIND::GEN_SUPP]->label()->setToolTip( tip_supp );
   m_itemList[VALGRIND::GEN_SUPP]->widget()->setToolTip( tip_supp );
}


void ValgrindOptionsPage::getDbBin()
{
   vkPrintErr( "TODO: ValgrindOptionsPage::getDbBin()\n" );
}





//---------------------------------------------------------------------
// Suppression routines
void ValgrindOptionsPage::setSuppFileBtns()
{
   QListWidget* lwSuppFiles = (QListWidget*)m_itemList[VALGRIND::SUPPS_SEL]->widget();
   
   if ( lwSuppFiles->currentItem() != 0 ) {
      btn_suppfile_rmv->setEnabled( true );
      btn_suppfile_up->setEnabled( lwSuppFiles->currentRow() > 0 );
      btn_suppfile_dwn->setEnabled( lwSuppFiles->currentRow() < lwSuppFiles->count()-1 );
   }
   else {
      btn_suppfile_rmv->setEnabled( false );
      btn_suppfile_up->setEnabled( false );
      btn_suppfile_dwn->setEnabled( false );
   }
}

void ValgrindOptionsPage::suppfileUp()
{
   QListWidget* lwSuppFiles = (QListWidget*)m_itemList[VALGRIND::SUPPS_SEL]->widget();
   int currRow = lwSuppFiles->currentRow();
   
   QListWidgetItem* item = lwSuppFiles->takeItem( currRow );
   lwSuppFiles->insertItem( currRow-1, item );
   lwSuppFiles->setCurrentItem( item );
}

void ValgrindOptionsPage::suppfileDown()
{
   QListWidget* lwSuppFiles = (QListWidget*)m_itemList[VALGRIND::SUPPS_SEL]->widget();
   int currRow = lwSuppFiles->currentRow();
   
   QListWidgetItem* item = lwSuppFiles->takeItem( currRow );
   lwSuppFiles->insertItem( currRow+1, item );
   lwSuppFiles->setCurrentItem( item );
}

void ValgrindOptionsPage::suppfileNew()
{
   LbWidget* lbSel = ( LbWidget* )m_itemList[VALGRIND::SUPPS_SEL];
   QListWidget* lwSuppFiles = (QListWidget*)lbSel->widget();

   QString supp_file =
      QFileDialog::getSaveFileName( this,
                                    tr( "Enter Supporessions Filename" ),
                                    "./", tr("Suppression Files (*.supp)") );

   if ( supp_file.isEmpty() ) { // user clicked Cancel
      // ignore.
   }
   else {
      // create clean suppfile, add filename to view and opt_list
      if ( supplist.initSuppsFile( supp_file ) ) {
         lwSuppFiles->addItem( supp_file );
         lwSuppFiles->setCurrentRow( lwSuppFiles->count()-1 );
      }
      else {
         vkPrintErr("Failed to write suppressions file '%s'", qPrintable(supp_file));
         // TODO: file write error: tell user
      }
   }
}


void ValgrindOptionsPage::suppfileAdd()
{
   LbWidget* lbSel = ( LbWidget* )m_itemList[VALGRIND::SUPPS_SEL];
   QListWidget* lwSuppFiles = (QListWidget*)lbSel->widget();

   QString supp_file =
      QFileDialog::getOpenFileName( this,
                                    tr( "Choose Suppression File" ),
                                    "./", tr("Suppression Files (*.supp)") );

   if ( ! supp_file.isEmpty() ) { // user clicked Cancel ?
      lwSuppFiles->addItem( supp_file );
      lwSuppFiles->setCurrentRow( lwSuppFiles->count()-1 );
   }
}

void ValgrindOptionsPage::suppfileRemove()
{
   LbWidget* lbSel = ( LbWidget* )m_itemList[VALGRIND::SUPPS_SEL];
   QListWidget* lwSuppFiles = (QListWidget*)lbSel->widget();
   
   QListWidgetItem* item = lwSuppFiles->currentItem();
   vk_assert( item );
   
   lwSuppFiles->takeItem( lwSuppFiles->row( item ) );
   
   setSuppFileBtns();
}



void ValgrindOptionsPage::setSuppBtns()
{
   QListWidget* lwFiles = (QListWidget*)m_itemList[VALGRIND::SUPPS_SEL]->widget();
   bool suppfileSelected = (lwFiles->currentItem() != 0);
   bool suppItemSelected = (lwSupps->currentItem() != 0);
   
   btn_supp_new->setEnabled( suppfileSelected );
   btn_supp_edt->setEnabled( suppfileSelected && suppItemSelected );
   btn_supp_del->setEnabled( suppfileSelected && suppItemSelected );
}

void ValgrindOptionsPage::suppLoad()
{
   QListWidget* lwFiles = (QListWidget*)m_itemList[VALGRIND::SUPPS_SEL]->widget();
   bool suppfileSelected = (lwFiles->currentItem() != 0);

   // first clear the last entries
   lwSupps->clear();
   supplist.clear();

   if ( suppfileSelected ) {    // show the suppression names from the file
      QString fname = lwFiles->currentItem()->text();
      if (!supplist.readSuppFile( fname )) {
         // TODO: error
         vkPrintErr("Failed to read/parse supp file '%s'", qPrintable(fname));
         return;
      }
      lwSupps->addItems( supplist.suppNames() );
      lwSupps->setCurrentRow( 0 );
   }
   
   setSuppBtns();
}

//
void ValgrindOptionsPage::setCurrentTab( int idx )
{
   tabWidget->setCurrentIndex( idx );
}


// add supp from string, pass to editor, save if ok
void ValgrindOptionsPage::suppNewFromStr( const QString& str )
{
   Suppression supp;
   if ( !supp.fromStringList( str.split("\n") ) ) {
      // TODO: err
      vkPrintErr("Failed to parse supp from input string '%s'", qPrintable(str));
      return;
   }
   
   // first check we have a file to write to
   LbWidget* lbSel = ( LbWidget* )m_itemList[VALGRIND::SUPPS_SEL];
   QListWidget* lwSuppFiles = (QListWidget*)lbSel->widget();
   if ( lwSuppFiles->count() == 0 ) {
      suppfileNew();
   }
   // still no supp file? user may have Cancelled...
   if ( lwSuppFiles->count() == 0 ) {
      return;
   }
   
   // Edit new supp: update model and suppfile first...
   if ( supplist.editSupp( -1, supp ) ) {
      // ... then update the view
      lwSupps->addItem( supplist.suppNames().last() );
      lwSupps->setCurrentRow( lwSupps->count()-1 );
   }
}
   

// create default supp, pass to editor, save if ok
void ValgrindOptionsPage::suppNew()
{
   // New supp: update model and suppfile first...
   if ( supplist.newSupp() ) {
      // then update the view
      lwSupps->addItem( supplist.suppNames().last() );
      lwSupps->setCurrentRow( lwSupps->count()-1 );
   }
}

// pass supp to editor, save to file
void ValgrindOptionsPage::suppEdit( QListWidgetItem* )
{
   // currentRow() already updated, so just use that
   suppEdit();
}

// pass supp to editor, save to file
void ValgrindOptionsPage::suppEdit()
{
   int suppIdx = lwSupps->currentRow();
   vk_assert( suppIdx != -1 );

   // Edit supp:
   if ( supplist.editSupp( suppIdx ) ) {
      // then update the view
      lwSupps->currentItem()->setText( supplist.suppNames().at(suppIdx) );
      lwSupps->setCurrentRow( suppIdx );
   }
}

// delete supp from file, view, list
void ValgrindOptionsPage::suppDelete()
{
   int suppIdx = lwSupps->currentRow();
   vk_assert( suppIdx != -1 );

   // Delete supp: update model and suppfile first...
   if ( supplist.deleteSupp( suppIdx ) ) {
      // then update the view
      lwSupps->takeItem( suppIdx );
   }
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

      
