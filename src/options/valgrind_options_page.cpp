/* ---------------------------------------------------------------------- 
 * Implementation of ValgrindOptionsPage        valgrind_options_page.cpp
 * subclass of OptionsPage to hold valgrind-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#include <qtabwidget.h>

#include "valgrind_options_page.h"
#include "vk_objects.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "vk_msgbox.h"


/* This page is different from the others in that it uses tabs because
   there are just too many options to put on one page.  The tabs
   contain (a) general options, and (b) suppression-related options */

ValgrindOptionsPage::ValgrindOptionsPage( QWidget* parent, VkObject* obj )
  : OptionsPage( parent, obj, "valgrind_options_page" )
{ 
  /* init the QIntDict list, resizing if necessary */
  unsigned int numItems = 29;
  itemList.resize( numItems );

  int space  = 5;  /* no. of pixels between cells */
  int margin = 11; /* no. of pixels to edge of widget */
  /* top layout: margin = 10; spacing = 25 */
  QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

  /* tab widget */
  QTabWidget* tabWidget = new QTabWidget( this );
  vbox->addWidget( tabWidget, space );


  /* tab 1: core options ----------------------------------------------- */
  QWidget* tabCore = new QWidget( tabWidget, "tab_core");
  tabWidget->addTab( tabCore, " Core " );
  /* tabCore - vbox */
  QVBoxLayout* core_vbox = new QVBoxLayout( tabCore, 10, 25, "core_vbox" );

  /* tabCore - group box 1 */
  QGroupBox* cgroup1 = new QGroupBox(" Common Options ", tabCore, "cgroup1");
  core_vbox->addWidget( cgroup1, space );
  /* tabCore - group box 1 - grid layout */
  int rows = 7;
  int cols = 2;
  QGridLayout* cgrid1 = new QGridLayout( cgroup1, rows, cols, margin, space );
  cgrid1->setRowSpacing( 0, topSpace );   /* blank top row */
  cgrid1->setColStretch( 1, 10 );         /* push widgets to the left */

  itemList.insert( Valgrind::TOOL,                        /* combobox */
                   optionWidget( Valgrind::TOOL,       cgroup1, true ) );
  itemList.insert( Valgrind::VERBOSITY,                   /* spinbox  */
                   optionWidget( Valgrind::VERBOSITY,  cgroup1, true ) );  
  itemList.insert( Valgrind::XML_OUTPUT,                  /* checkbox */
                   optionWidget( Valgrind::XML_OUTPUT, cgroup1, false ) );
  itemList.insert( Valgrind::TRACE_CH,                    /* checkbox */
                   optionWidget( Valgrind::TRACE_CH,   cgroup1, false ) );
  itemList.insert( Valgrind::TRACK_FDS,                   /* checkbox */
                   optionWidget( Valgrind::TRACK_FDS,  cgroup1, false ) );
  itemList.insert( Valgrind::TIME_STAMP,                  /* checkbox */
                   optionWidget( Valgrind::TIME_STAMP, cgroup1, false ) );

  cgrid1->addLayout( itemList[Valgrind::TOOL      ]->hlayout(), 1, 0 );
  cgrid1->addLayout( itemList[Valgrind::VERBOSITY ]->hlayout(), 2, 0 );
  cgrid1->addWidget( itemList[Valgrind::XML_OUTPUT]->widget(),  3, 0 );
  cgrid1->addWidget( itemList[Valgrind::TRACE_CH  ]->widget(),  4, 0 );
  cgrid1->addWidget( itemList[Valgrind::TRACK_FDS ]->widget(),  5, 0 );
  cgrid1->addWidget( itemList[Valgrind::TIME_STAMP]->widget(),  6, 0 );


  /* tabCore - group box 2 */
  QGroupBox* cgroup2 = new QGroupBox( " Less Common Options ",
                                      tabCore,"cgroup1" );
  core_vbox->addWidget( cgroup2, space );
  /* tabCore - group box 2 - grid layout */
  rows = 6;  cols = 2;
  QGridLayout* cgrid2 = new QGridLayout( cgroup2, rows, cols, margin, space );
  cgrid2->setRowSpacing( 0, topSpace );   /* blank top row */
  cgrid2->setColStretch( 1, 10 );         /* push widgets to the left */

  itemList.insert( Valgrind::RUN_LIBC,                    /* checkbox */
                   optionWidget( Valgrind::RUN_LIBC,   cgroup2, false ) );
  itemList.insert( Valgrind::PTR_CHECK,                   /* checkbox */
                   optionWidget( Valgrind::PTR_CHECK,  cgroup2, false ) );
  itemList.insert( Valgrind::ELAN_HACKS,                  /* checkbox */
                   optionWidget( Valgrind::ELAN_HACKS, cgroup2, false ) );
  itemList.insert( Valgrind::EM_WARNS,                    /* checkbox */
                   optionWidget( Valgrind::EM_WARNS,   cgroup2, false ) );
  itemList.insert( Valgrind::WEIRD,                       /* combobox */
                   optionWidget( Valgrind::WEIRD,      cgroup2, true ) );

  cgrid2->addWidget( itemList[Valgrind::RUN_LIBC  ]->widget(),  1, 0 );
  cgrid2->addWidget( itemList[Valgrind::PTR_CHECK ]->widget(),  2, 0 );
  cgrid2->addWidget( itemList[Valgrind::ELAN_HACKS]->widget(),  3, 0 );
  cgrid2->addWidget( itemList[Valgrind::EM_WARNS  ]->widget(),  4, 0 );
  cgrid2->addLayout( itemList[Valgrind::WEIRD     ]->hlayout(), 5, 0 );

  core_vbox->addStretch( space );


  /* tab 2: error-reporting -------------------------------------------- */
  QWidget* tabErep = new QWidget( tabWidget, "tab_erep");
  tabWidget->addTab( tabErep, " Error Reporting " );
  /* tabErep - vbox */
  QVBoxLayout* erep_vbox = new QVBoxLayout( tabErep, 10, 25, "vbox" );

  /* tabErep - group box 1 */
  QGroupBox* egroup1 = new QGroupBox( " Options ", tabErep, "egroup1" );
  erep_vbox->addWidget( egroup1, space );
  /* tabErep - group box 1 - grid layout */
  rows = 14;  cols = 2;
  QGridLayout* egrid1 = new QGridLayout( egroup1, rows, cols, margin, space );
  egrid1->setRowSpacing( 0, topSpace );   /* blank top row */
  //egrid1->setColStretch( 1, 10 );         /* push widgets to the left */

  itemList.insert( Valgrind::GEN_SUPP,                  /* checkbox */
                   optionWidget( Valgrind::GEN_SUPP,    egroup1, false ) );
  itemList.insert( Valgrind::DEMANGLE,                  /* checkbox */
                   optionWidget( Valgrind::DEMANGLE,    egroup1, false ) );
  itemList.insert( Valgrind::ERROR_LIMIT,               /* checkbox */
                   optionWidget( Valgrind::ERROR_LIMIT, egroup1, false ) );
  itemList.insert( Valgrind::SHOW_BELOW,                /* checkbox */
                   optionWidget( Valgrind::SHOW_BELOW,  egroup1, false ) );
  itemList.insert( Valgrind::NUM_CALLERS,               /* intspin */
                   optionWidget( Valgrind::NUM_CALLERS, egroup1, true ) );
  itemList.insert( Valgrind::MAX_SFRAME,                /* spinbox  */
                   optionWidget( Valgrind::MAX_SFRAME,  egroup1, true ) );  

  itemList.insert( Valgrind::DB_ATTACH,                 /* checkbox */
                   optionWidget( Valgrind::DB_ATTACH,   egroup1, false ) );
  itemList.insert( Valgrind::DB_COMMAND,                /* ledit+button */
                   optionWidget( Valgrind::DB_COMMAND,  egroup1, false ) );
  LeWidget* dbLedit = ((LeWidget*)itemList[Valgrind::DB_COMMAND]);
  dbLedit->addButton( egroup1, this, SLOT(dummy()) );

  itemList.insert( Valgrind::INPUT_FD,                  /* spinbox  */
                   optionWidget( Valgrind::INPUT_FD,    egroup1, true ) );
  itemList.insert( Valgrind::LOG_FD,                    /* spinbox  */
                   optionWidget( Valgrind::LOG_FD,      egroup1, true ) );

  itemList.insert( Valgrind::LOG_PID,                   /* ledit    */
                   optionWidget(Valgrind::LOG_PID,      egroup1, true ) );
  itemList.insert( Valgrind::LOG_FILE,                  /* ledit    */
                   optionWidget(Valgrind::LOG_FILE,     egroup1, true ) );
  itemList.insert( Valgrind::LOG_SOCKET,                /* ledit    */
                   optionWidget(Valgrind::LOG_SOCKET,   egroup1, true ) );

  egrid1->addWidget( itemList[Valgrind::GEN_SUPP   ]->widget(),   1, 0 );
  egrid1->addWidget( itemList[Valgrind::DEMANGLE   ]->widget(),   2, 0 );
  egrid1->addWidget( itemList[Valgrind::ERROR_LIMIT]->widget(),   3, 0 );
  egrid1->addWidget( itemList[Valgrind::SHOW_BELOW ]->widget(),   4, 0 );
  egrid1->addLayout( itemList[Valgrind::NUM_CALLERS]->hlayout(),  5, 0 );
  egrid1->addLayout( itemList[Valgrind::MAX_SFRAME ]->hlayout(),  6, 0 );

  egrid1->addWidget( itemList[Valgrind::DB_ATTACH  ]->widget(),   7, 0 );
  egrid1->addLayout( itemList[Valgrind::DB_COMMAND ]->hlayout(),  8, 0 );

  egrid1->addMultiCellWidget( sep(egroup1,"sep1"), 9,9, 0,1 );
  egrid1->setRowSpacing( 9, topSpace );   /* add a bit more space here */
	//----------
  QHBoxLayout* hBox = new QHBoxLayout( 6, "hBox" );
	hBox->addLayout( itemList[Valgrind::INPUT_FD]->hlayout() );
	hBox->addLayout( itemList[Valgrind::LOG_FD]->hlayout() );
  //egrid1->addLayout( itemList[Valgrind::INPUT_FD   ]->hlayout(),  9, 0 );
  //egrid1->addLayout( itemList[Valgrind::LOG_FD     ]->hlayout(), 10, 0 );
  egrid1->addLayout( hBox,                                       10, 0 );
	//------------
  egrid1->addLayout( itemList[Valgrind::LOG_PID    ]->hlayout(), 11, 0 );
  egrid1->addLayout( itemList[Valgrind::LOG_FILE   ]->hlayout(), 12, 0 );
  egrid1->addLayout( itemList[Valgrind::LOG_SOCKET ]->hlayout(), 13, 0 );

  erep_vbox->addStretch( space );


  /* tab 3: suppressions ----------------------------------------------- */
  QWidget* tabSupps = new QWidget( tabWidget, "tab_supps");
  tabWidget->addTab( tabSupps, " Suppressions " );
  /* tabSupps - vbox */
  QVBoxLayout* supp_vbox = new QVBoxLayout( tabSupps, 10, 25, "supp_vbox" );


  itemList.insert( Valgrind::SUPPS_SEL,                 /* listbox */
                   optionWidget( Valgrind::SUPPS_SEL, tabSupps, true ) );

  itemList.insert( Valgrind::SUPPS_ALL,                 /* listbox */
                   optionWidget( Valgrind::SUPPS_ALL, tabSupps, true ) );

	LbWidget* lbSel = ((LbWidget*)itemList[Valgrind::SUPPS_SEL]);
	LbWidget* lbAll = ((LbWidget*)itemList[Valgrind::SUPPS_ALL]);
  connect( lbAll, SIGNAL(fileSelected(const QString&)), 
					 lbSel, SLOT(insertFile(const QString&)) );

  supp_vbox->addLayout( itemList[Valgrind::SUPPS_SEL]->vlayout(), 10 );
  supp_vbox->addSpacing( 30 );
  supp_vbox->addLayout( itemList[Valgrind::SUPPS_ALL]->vlayout(), 20 );


  supp_vbox->addStretch( space );


  vk_assert( itemList.count() <= numItems );

  QIntDictIterator<OptionWidget> it( itemList );
  for ( ;  it.current(); ++it ) {
    connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
            this,         SLOT(updateEditList( bool, OptionWidget * )));
  }
}


/* Called when user clicks "Apply" or "Ok" button.  
   Also called when Cancel button is clicked, to reset toggled values */
bool ValgrindOptionsPage::applyOptions( int /*id*/, bool/*=false*/ )
{ 
  return true;
}


void ValgrindOptionsPage::dummy()
{ printf("slot dummy()\n"); }

