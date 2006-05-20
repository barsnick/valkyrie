/* ---------------------------------------------------------------------- 
 * Implementation of ValgrindOptionsPage        valgrind_options_page.cpp
 * Subclass of OptionsPage to hold valgrind-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qtabwidget.h>

#include "valgrind_options_page.h"
#include "valgrind_object.h"
#include "vk_config.h"
#include "vk_messages.h"
#include "vk_utils.h"
#include "context_help.h"
#include "html_urls.h"

/* from valgrind/coregrind/pub_core_options.h
   = maximum number of suppression files */
#define VG_CLO_MAX_SFILES 10


/* this page is different from the others in that it uses three tabs
   because there are just too many options to put on one page.
   (a) general core options, 
   (b) error-reporting options, and 
   (c) suppression-related options 
*/
ValgrindOptionsPage::ValgrindOptionsPage( QWidget* parent, VkObject* obj )
   : OptionsPage( parent, obj, "valgrind_options_page" )
{ 
   /* init the QIntDict list, resizing if necessary */
   unsigned int numItems = 29;
   m_itemList.resize( numItems );

   /* top layout: margin = 10; spacing = 25 */
   QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

   /* tab widget */
   QTabWidget* tabWidget = new QTabWidget( this );
   vbox->addWidget( tabWidget, m_space );


   /* tab 1: core options ------------------------------------------- */
   QWidget* tabCore = new QWidget( tabWidget, "tab_core");
   tabWidget->addTab( tabCore, " Core " );
   ContextHelp::add( tabCore, urlValkyrie::coreTab );
   /* tabCore - vbox */
   QVBoxLayout* core_vbox = new QVBoxLayout( tabCore, 10, 25, "core_vbox" );

   /* tabCore - group box 1 */
   QGroupBox* cgroup1 = new QGroupBox(" Common Options ", tabCore, "cgroup1");
   core_vbox->addWidget( cgroup1, m_space );
   /* tabCore - group box 1 - grid layout */
   int rows = 7;
   int cols = 2;
   QGridLayout* cgrid1 = new QGridLayout( cgroup1, rows, cols, m_margin, m_space );
#if (QT_VERSION-0 >= 0x030200)
   cgrid1->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   cgrid1->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif
   cgrid1->setColStretch( 1, 10 );         /* push widgets to the left */

   m_itemList.insert( Valgrind::TOOL,                        /* combobox */
                      optionWidget( Valgrind::TOOL,       cgroup1, true ) );
   m_itemList.insert( Valgrind::VERBOSITY,                   /* spinbox  */
                      optionWidget( Valgrind::VERBOSITY,  cgroup1, true ) );  
   m_itemList.insert( Valgrind::XML_OUTPUT,                  /* checkbox */
                      optionWidget( Valgrind::XML_OUTPUT, cgroup1, false ) );
   m_itemList.insert( Valgrind::TRACE_CH,                    /* checkbox */
                      optionWidget( Valgrind::TRACE_CH,   cgroup1, false ) );
   m_itemList.insert( Valgrind::TRACK_FDS,                   /* checkbox */
                      optionWidget( Valgrind::TRACK_FDS,  cgroup1, false ) );
   m_itemList.insert( Valgrind::TIME_STAMP,                  /* checkbox */
                      optionWidget( Valgrind::TIME_STAMP, cgroup1, false ) );

   cgrid1->addLayout( m_itemList[Valgrind::TOOL      ]->hlayout(), 1, 0 );
   cgrid1->addLayout( m_itemList[Valgrind::VERBOSITY ]->hlayout(), 2, 0 );
   cgrid1->addWidget( m_itemList[Valgrind::XML_OUTPUT]->widget(),  3, 0 );
   cgrid1->addWidget( m_itemList[Valgrind::TRACE_CH  ]->widget(),  4, 0 );
   cgrid1->addWidget( m_itemList[Valgrind::TRACK_FDS ]->widget(),  5, 0 );
   cgrid1->addWidget( m_itemList[Valgrind::TIME_STAMP]->widget(),  6, 0 );


   /* tabCore - group box 2 */
   QGroupBox* cgroup2 = new QGroupBox( " Less Common Options ",
                                       tabCore,"cgroup1" );
   core_vbox->addWidget( cgroup2, m_space );
   /* tabCore - group box 2 - grid layout */
   rows = 6;  cols = 2;
   QGridLayout* cgrid2 = new QGridLayout( cgroup2, rows, cols, m_margin, m_space );
#if (QT_VERSION-0 >= 0x030200)
   cgrid2->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   cgrid2->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif
   cgrid2->setColStretch( 1, 10 );         /* push widgets to the left */

   m_itemList.insert( Valgrind::RUN_LIBC,                    /* checkbox */
                      optionWidget( Valgrind::RUN_LIBC,   cgroup2, false ) );
   m_itemList.insert( Valgrind::EM_WARNS,                    /* checkbox */
                      optionWidget( Valgrind::EM_WARNS,   cgroup2, false ) );
  
   m_itemList.insert( Valgrind::SMC_CHECK,                   /* combobox */
                      optionWidget( Valgrind::SMC_CHECK,  cgroup2, true ) );
   m_itemList.insert( Valgrind::SIM_HINTS,                   /* combobox */
                      optionWidget( Valgrind::SIM_HINTS,  cgroup2, true ) );
   m_itemList.insert( Valgrind::KERN_VAR,                    /* combobox */
                      optionWidget( Valgrind::KERN_VAR,   cgroup2, true ) );

   cgrid2->addWidget( m_itemList[Valgrind::RUN_LIBC ]->widget(),  1, 0 );
   cgrid2->addWidget( m_itemList[Valgrind::EM_WARNS ]->widget(),  2, 0 );

   cgrid2->addLayout( m_itemList[Valgrind::SMC_CHECK]->hlayout(), 3, 0 );
   cgrid2->addLayout( m_itemList[Valgrind::SIM_HINTS]->hlayout(), 4, 0 );
   cgrid2->addLayout( m_itemList[Valgrind::KERN_VAR ]->hlayout(), 5, 0 );

   core_vbox->addStretch( m_space );


   /* tab 2: error-reporting ---------------------------------------- */
   QWidget* tabErep = new QWidget( tabWidget, "tab_erep");
   tabWidget->addTab( tabErep, " Error Reporting " );
   ContextHelp::add( tabCore, urlValkyrie::errorTab );
   /* tabErep - vbox */
   QVBoxLayout* erep_vbox = new QVBoxLayout( tabErep, 10, 25, "vbox" );

   /* tabErep - group box 1 */
   QGroupBox* egroup1 = new QGroupBox( " Options ", tabErep, "egroup1" );
   erep_vbox->addWidget( egroup1, m_space );
   /* tabErep - group box 1 - grid layout */
   rows = 14;  cols = 2;
   QGridLayout* egrid1 = new QGridLayout( egroup1, rows, cols, m_margin, m_space );
#if (QT_VERSION-0 >= 0x030200)
   egrid1->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   egrid1->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif

   m_itemList.insert( Valgrind::GEN_SUPP,                  /* combobox */
                      optionWidget( Valgrind::GEN_SUPP,    egroup1, true ) );
   m_itemList.insert( Valgrind::DEMANGLE,                  /* checkbox */
                      optionWidget( Valgrind::DEMANGLE,    egroup1, false ) );
   m_itemList.insert( Valgrind::ERROR_LIMIT,               /* checkbox */
                      optionWidget( Valgrind::ERROR_LIMIT, egroup1, false ) );
   m_itemList.insert( Valgrind::SHOW_BELOW,                /* checkbox */
                      optionWidget( Valgrind::SHOW_BELOW,  egroup1, false ) );
   m_itemList.insert( Valgrind::NUM_CALLERS,               /* intspin */
                      optionWidget( Valgrind::NUM_CALLERS, egroup1, true ) );
   m_itemList.insert( Valgrind::MAX_SFRAME,                /* spinbox  */
                      optionWidget( Valgrind::MAX_SFRAME,  egroup1, true ) );  

   m_itemList.insert( Valgrind::DB_ATTACH,                 /* checkbox */
                      optionWidget( Valgrind::DB_ATTACH,   egroup1, false ) );
   m_itemList.insert( Valgrind::DB_COMMAND,                /* ledit+button */
                      optionWidget( Valgrind::DB_COMMAND,  egroup1, false ) );
   LeWidget* dbLedit = ((LeWidget*)m_itemList[Valgrind::DB_COMMAND]);
   dbLedit->addButton( egroup1, this, SLOT(getDbBin()) );

   m_itemList.insert( Valgrind::INPUT_FD,                  /* spinbox  */
                      optionWidget( Valgrind::INPUT_FD,    egroup1, true ) );
   m_itemList.insert( Valgrind::LOG_FD,                    /* spinbox  */
                      optionWidget( Valgrind::LOG_FD,      egroup1, true ) );
   m_itemList.insert( Valgrind::LOG_PID,                   /* ledit    */
                      optionWidget(Valgrind::LOG_PID,      egroup1, true ) );
   m_itemList.insert( Valgrind::LOG_FILE,                  /* ledit    */
                      optionWidget(Valgrind::LOG_FILE,     egroup1, true ) );
   m_itemList.insert( Valgrind::LOG_SOCKET,                /* ledit    */
                      optionWidget(Valgrind::LOG_SOCKET,   egroup1, true ) );

   egrid1->addLayout( m_itemList[Valgrind::GEN_SUPP   ]->hlayout(),  1, 0 );
   egrid1->addWidget( m_itemList[Valgrind::DEMANGLE   ]->widget(),   2, 0 );
   egrid1->addWidget( m_itemList[Valgrind::ERROR_LIMIT]->widget(),   3, 0 );
   egrid1->addWidget( m_itemList[Valgrind::SHOW_BELOW ]->widget(),   4, 0 );
   egrid1->addLayout( m_itemList[Valgrind::NUM_CALLERS]->hlayout(),  5, 0 );
   egrid1->addLayout( m_itemList[Valgrind::MAX_SFRAME ]->hlayout(),  6, 0 );

   egrid1->addWidget( m_itemList[Valgrind::DB_ATTACH  ]->widget(),   7, 0 );
   egrid1->addLayout( m_itemList[Valgrind::DB_COMMAND ]->hlayout(),  8, 0 );

   egrid1->addMultiCellWidget( sep(egroup1,"sep1"), 9,9, 0,1 );
#if (QT_VERSION-0 >= 0x030200)
   egrid1->setRowSpacing( 9, m_topSpace );   /* add a bit more space here */
#else // QT_VERSION < 3.2
   egrid1->addRowSpacing( 9, m_topSpace );   /* add a bit more space here */
#endif

   //----------
   QHBoxLayout* hBox = new QHBoxLayout( 6, "hBox" );
   hBox->addLayout( m_itemList[Valgrind::INPUT_FD]->hlayout() );
   hBox->addLayout( m_itemList[Valgrind::LOG_FD]->hlayout() );
   egrid1->addLayout( hBox,                                       10, 0 );
   //------------
   egrid1->addLayout( m_itemList[Valgrind::LOG_PID    ]->hlayout(), 11, 0 );
   egrid1->addLayout( m_itemList[Valgrind::LOG_FILE   ]->hlayout(), 12, 0 );
   egrid1->addLayout( m_itemList[Valgrind::LOG_SOCKET ]->hlayout(), 13, 0 );

   erep_vbox->addStretch( m_space );


   /* tab 3: suppressions ------------------------------------------- */
   QWidget* tabSupps = new QWidget( tabWidget, "tab_supps");
   tabWidget->addTab( tabSupps, " Suppressions " );
   ContextHelp::add( tabCore, urlValkyrie::suppsTab );
   /* tabSupps - vbox */
   QVBoxLayout* supp_vbox = new QVBoxLayout( tabSupps, 10, 25, "supp_vbox" );


   m_itemList.insert( Valgrind::SUPPS_DIRS,                /* listbox */
                      optionWidget( Valgrind::SUPPS_DIRS, tabSupps, true ) );
   m_itemList.insert( Valgrind::SUPPS_AVAIL,                 /* listbox */
                      optionWidget( Valgrind::SUPPS_AVAIL, tabSupps, true ) );
   m_itemList.insert( Valgrind::SUPPS_SEL,                 /* listbox */
                      optionWidget( Valgrind::SUPPS_SEL, tabSupps, true ) );

   /*
     NOTE: suppression listbox widgets have knowledge about what they
           are and what they should do.  Ick, fix asap!
   */

   LbWidget* lbDirs  = (LbWidget*)m_itemList[Valgrind::SUPPS_DIRS];
   LbWidget* lbAvail = (LbWidget*)m_itemList[Valgrind::SUPPS_AVAIL];
   LbWidget* lbSel   = (LbWidget*)m_itemList[Valgrind::SUPPS_SEL];

   /* lbDirs updates lbAvail */
   connect( lbDirs,  SIGNAL(listChanged()), 
            this,      SLOT(suppDirsChanged()) );

   /* lbSel and lbAvail update each other */
   connect( lbAvail, SIGNAL(itemSelected(const QString&)), 
            this,      SLOT(updateSuppsSel(const QString&)) );
   connect( lbSel,   SIGNAL(listChanged()), 
            this,      SLOT(updateSuppsAvail()) );

   supp_vbox->addLayout( m_itemList[Valgrind::SUPPS_DIRS ]->vlayout(), 20 );
   supp_vbox->addLayout( m_itemList[Valgrind::SUPPS_AVAIL]->vlayout(), 30 );
   supp_vbox->addLayout( m_itemList[Valgrind::SUPPS_SEL  ]->vlayout(), 20 );

   supp_vbox->addStretch( m_space );


   /* finalise page ------------------------------------------------- */
   vk_assert( m_itemList.count() <= numItems );

   QIntDictIterator<OptionWidget> it( m_itemList );
   for ( ;  it.current(); ++it ) {
      connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
              this,         SLOT(updateEditList( bool, OptionWidget * )));
   }


   /* Disabled Widgets ---------------------------------------------- */
   /* These widgets are disabled because Valkyrie uses
      --log-file-exactly internally: logging options would interfere
      with this. */
	// error reporting tab
   m_itemList[Valgrind::LOG_FD     ]->setEnabled( false );
   m_itemList[Valgrind::LOG_PID    ]->setEnabled( false );
   m_itemList[Valgrind::LOG_FILE   ]->setEnabled( false );
   m_itemList[Valgrind::LOG_SOCKET ]->setEnabled( false );
//   m_itemList[Valgrind::LOG_QUAL   ]->setEnabled( false );

   /* Disabled for now - can't deal with the multiple xml files this generates */
   /* Note: Also disabled in Valgrind::checkOptArg() */
   m_itemList[Valgrind::TRACE_CH   ]->setEnabled( false );

   /* Disabled for now - not yet implemented */
   m_itemList[Valgrind::DB_ATTACH  ]->setEnabled( false );
   m_itemList[Valgrind::DB_COMMAND ]->setEnabled( false );
   dbLedit->button()->setEnabled( false );
   m_itemList[Valgrind::INPUT_FD   ]->setEnabled( false );

}


/* Called from OptionsWindow::categoryClicked() */
void ValgrindOptionsPage::init()
{
   /* init available supps list */
   suppDirsChanged();
}


/* called when user clicks "Apply" / "Ok" / "Reset" buttons. */
bool ValgrindOptionsPage::applyOptions( int optId )
{ 
   vk_assert( optId <= Valgrind::LAST_CMD_OPT );

   /* check option */
   QString argval = m_itemList[optId]->currValue();
   int errval = m_vkObj->checkOptArg( optId, argval );
   if ( errval != PARSED_OK ) {
      vkError( this, "Invalid Entry", "%s:\n\"%s\"", 
               parseErrString(errval), argval.latin1() );
      m_itemList[optId]->cancelEdit();
      return false;
   }

   /* apply option */
   switch ( optId ) {
   default:
      break;
   }

   return true;
}


void ValgrindOptionsPage::getDbBin()
{
   vkPrintErr("TODO: ValgrindOptionsPage::getDbBin()\n");
}


void ValgrindOptionsPage::dummy()
{ VK_DEBUG("slot dummy()\n"); }


/* Scan dirs set in option "valgrind::supps-dirs" for available
   suppression files.
   Update other widgets with result.
*/
void ValgrindOptionsPage::suppDirsChanged()
{
   m_allAvailSuppFiles = QStringList();
   LbWidget* lbDirs  = (LbWidget*)m_itemList[Valgrind::SUPPS_DIRS];
   QChar m_sep = vkConfig->sepChar();

   /* Get list of dirs from "valgrind::supps-dirs" */
   QStringList suppDirs = QStringList::split( m_sep, lbDirs->currValue() );
   for ( unsigned int i=0; i<suppDirs.count(); i++ ) {

      /* for each suppDir, find all supp files */
      QDir suppDir( suppDirs[i] );
      QString path = suppDir.absPath() + '/';
      QStringList entries = suppDir.entryList( "*.supp", QDir::Files );
      for (unsigned int i=0; i<entries.count(); i++) {
         m_allAvailSuppFiles += (path + entries[i]);
      }
   }

   updateSuppsAvail();
}


/* Given available suppfiles from dirscan,
   remove those already selected in option "valgrind::supps-sel"
   Set suppsAvail list with result.
*/
void ValgrindOptionsPage::updateSuppsAvail()
{
   QChar       m_sep      = vkConfig->sepChar();
   LbWidget*   lbAvail    = (LbWidget*)m_itemList[Valgrind::SUPPS_AVAIL];
   LbWidget*   lbSel      = (LbWidget*)m_itemList[Valgrind::SUPPS_SEL  ];
   QStringList suppsAvail = m_allAvailSuppFiles;
   QStringList currSupps  = QStringList::split( m_sep, lbSel->currValue() );

   for ( unsigned int i=0; i<currSupps.count(); i++ ) {
      QStringList::iterator it = suppsAvail.find( currSupps[i] );
      if ( it != suppsAvail.end() ) {
         suppsAvail.remove( it );
      }
   }

   lbAvail->setCurrValue( suppsAvail.join( m_sep ) );
}


/* Called by selecting an item in suppsAvail listbox.
   Adds item to selected supps listbox (up to limit)
*/
void ValgrindOptionsPage::updateSuppsSel(const QString& suppr)
{
   LbWidget* lbSel = (LbWidget*)m_itemList[Valgrind::SUPPS_SEL];

   if (((QListBox*)lbSel->widget())->numRows() < VG_CLO_MAX_SFILES) {
      lbSel->insertItem(suppr);
   } else {
      /* valgrind doesn't accept any more suppression files */
      vkError( this, "Error",
               "Valgrind won't accept more than %d suppression files", 
               VG_CLO_MAX_SFILES );      
   }
}

