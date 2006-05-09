/* ---------------------------------------------------------------------- 
 * Implementation of CachegrindOptionsPage    cachegrind_options_page.cpp
 * Subclass of OptionsPage to hold cachegrind-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "cachegrind_options_page.h"
#include "cachegrind_object.h"
#include "vk_utils.h"
#include "vk_messages.h"
#include "vk_popt_option.h"    // PERROR* and friends 


CachegrindOptionsPage::CachegrindOptionsPage( QWidget* parent, VkObject* obj )
   : OptionsPage( parent, obj, "cachegrind_options_page" )
{ 
   /* init the QIntDict list, resizing if necessary */
   unsigned int numItems = 11;
   m_itemList.resize( numItems );

   /* top layout: margin = 10; spacing = 25 */
   QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

   /* group1: cache configuration options */
   QGroupBox* group1 = new QGroupBox(" Cache Configuration ", this, "group1");
   vbox->addWidget( group1, m_space );

   /* we make the three spin-widgets 'by hand' here */
   Option* opt;
   SpWidget* spinw;

   /* specify I1 cache configuration */
   opt = m_vkObj->findOption( Cachegrind::I1_CACHE );
   spinw = new SpWidget( group1, opt, true, 3 );
   spinw->addSection( 0, 1048576, 0, 0 );  /* min, max, def, step/pwr2 */
   spinw->addSection( 0, 8,       0, 1 );
   spinw->addSection( 0, 8192,    0, 0 );
   m_itemList.insert( Cachegrind::I1_CACHE, spinw );

   /* specify D1 cache configuration */
   opt = m_vkObj->findOption( Cachegrind::D1_CACHE );
   spinw = new SpWidget( group1, opt, true, 3 );
   spinw->addSection( 0, 1048576, 0, 0 );  /* min, max, def, use_powers */
   spinw->addSection( 0, 8,       0, 1 );
   spinw->addSection( 0, 8192,    0, 0 );
   m_itemList.insert( Cachegrind::D1_CACHE, spinw );

   /* specify L2 cache configuration */
   opt = m_vkObj->findOption( Cachegrind::L2_CACHE );
   spinw = new SpWidget( group1, opt, true, 3 );
   spinw->addSection( 0, 1048576, 0, 0 );  /* min, max, def, use_powers */
   spinw->addSection( 0, 8,       0, 1 );
   spinw->addSection( 0, 8192,    0, 0 );
   m_itemList.insert( Cachegrind::L2_CACHE, spinw );

   /* grid layout for group1 */
   int rows = 4;
   int cols = 3;
   QGridLayout* grid1 = new QGridLayout( group1, rows, cols, m_margin, m_space );
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif
   grid1->setColStretch( 2, 10 );         /* push widgets to the left */

   grid1->addWidget( m_itemList[Cachegrind::I1_CACHE]->label(),  1, 0 );
   grid1->addWidget( m_itemList[Cachegrind::I1_CACHE]->widget(), 1, 1 );
   grid1->addWidget( m_itemList[Cachegrind::D1_CACHE]->label(),  2, 0 );
   grid1->addWidget( m_itemList[Cachegrind::D1_CACHE]->widget(), 2, 1 );
   grid1->addWidget( m_itemList[Cachegrind::L2_CACHE]->label(),  3, 0 );
   grid1->addWidget( m_itemList[Cachegrind::L2_CACHE]->widget(), 3, 1 );


   /* group2: annotation options */
   QGroupBox* group2 = new QGroupBox(" Annotation Options ", this, "group2");
   vbox->addWidget( group2, m_space );

   m_itemList.insert( Cachegrind::AUTO,                    /* checkbox */
                      optionWidget( Cachegrind::AUTO,     group2, false ) );
   m_itemList.insert( Cachegrind::SHOW,                    /* line edit */
                      optionWidget( Cachegrind::SHOW,     group2, true ) );
   m_itemList.insert( Cachegrind::SORT,                    /* line edit */
                      optionWidget( Cachegrind::SORT,     group2, true ) );
   m_itemList.insert( Cachegrind::THRESH,                  /* spinbox */
                      optionWidget( Cachegrind::THRESH,   group2, true ) );
   m_itemList.insert( Cachegrind::CONTEXT,                 /* spinbox */
                      optionWidget( Cachegrind::CONTEXT,  group2, true ) );
   m_itemList.insert( Cachegrind::PID_FILE,                /* line edit */
                      optionWidget( Cachegrind::PID_FILE, group2, false ));
   LeWidget* pidLedit = ((LeWidget*)m_itemList[Cachegrind::PID_FILE]);
   pidLedit->addButton( group2, this, SLOT(getPidFile()) );

   m_itemList.insert( Cachegrind::INCLUDE,                 /* line edit */
                      optionWidget( Cachegrind::INCLUDE, group2, false ));
   LeWidget* incLedit = ((LeWidget*)m_itemList[Cachegrind::INCLUDE]);
   incLedit->addButton( group2, this, SLOT(getIncludeDirs()) );

   /* grid layout for group2 */
   rows = 4;
   cols = 2;
   QGridLayout* grid2 = new QGridLayout( group2, rows, cols, m_margin, m_space );
#if (QT_VERSION-0 >= 0x030200)
   grid2->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   grid2->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif
   grid2->setColStretch( 2, 10 );         /* push widgets to the left */

   grid2->addWidget(m_itemList[Cachegrind::AUTO   ]->widget(),  1, 0 );
   grid2->addLayout(m_itemList[Cachegrind::SHOW   ]->hlayout(), 2, 0 );
   grid2->addLayout(m_itemList[Cachegrind::SORT   ]->hlayout(), 3, 0 );
   grid2->addLayout(m_itemList[Cachegrind::THRESH ]->hlayout(), 4, 0 );
   grid2->addLayout(m_itemList[Cachegrind::CONTEXT]->hlayout(), 5, 0 );

#if (QT_VERSION-0 >= 0x030200)
   grid2->setRowSpacing( 6, m_topSpace );
#else // QT_VERSION < 3.2
   grid2->addRowSpacing( 6, m_topSpace );
#endif
   grid2->addMultiCellLayout( pidLedit->hlayout(), 7,7, 0,2 );
   grid2->addMultiCellLayout( incLedit->hlayout(), 8,8, 0,2 );


   vbox->addStretch( m_space );
   vk_assert( m_itemList.count() <= numItems );

   QIntDictIterator<OptionWidget> it( m_itemList );
   for ( ;  it.current(); ++it ) {
      connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
              this,         SLOT(updateEditList( bool, OptionWidget * )));
   }

}


/* called when user clicks "Apply" / "Ok" / "Reset" buttons. */
bool CachegrindOptionsPage::applyOptions( int optId )
{ 
   vk_assert( optId <= Cachegrind::LAST_CMD_OPT );

   /* check option */
   QString argval = m_itemList[optId]->currValue();
   int errval = m_vkObj->checkOptArg( optId, argval );
   if ( errval != PARSED_OK ) {
      vkError( this, "Invalid Entry", 
               "%s:\n\"%s\"", parseErrString(errval), argval.latin1() );
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


void CachegrindOptionsPage::getPidFile()
{ 
   QString pidfile = QFileDialog::getOpenFileName( QString::null, 
                                                   "Pid Files(*.pid);;All Files (*)", 
                                                   this, "fdlg", "Select Pid File to View" );

   if ( !pidfile.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Cachegrind::PID_FILE])->setCurrValue(pidfile);
      applyOptions( Cachegrind::PID_FILE );
   }

}


void CachegrindOptionsPage::getIncludeDirs()
{ 
   QString incdir = QFileDialog::getExistingDirectory( QString::null,
                                                       this, "get_inc_dir",
                                                       "Choose directory to include", true );
   if ( !incdir.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Cachegrind::INCLUDE])->addCurrValue(incdir);
      applyOptions( Cachegrind::INCLUDE );
   }

}
