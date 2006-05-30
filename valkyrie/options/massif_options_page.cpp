/* ---------------------------------------------------------------------- 
 * Implementation of MassifOptionsPage            massif_options_page.cpp
 * Subclass of OptionsPage to hold massif-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "massif_options_page.h"
#include "massif_object.h"
#include "vk_messages.h"
#include "vk_utils.h"


MassifOptionsPage::MassifOptionsPage( QWidget* parent, VkObject* obj )
   : OptionsPage( parent, obj, "massif_options_page" )
{ 
   /* init the QIntDict list, resizing if necessary */
   unsigned int numItems = 7;
   m_itemList.resize( numItems );

   /* top layout: margin = 10; spacing = 25 */
   QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

   /* group1: massif options */
   QGroupBox* group1 = new QGroupBox( " Massif Options ", this, "group1");
   vbox->addWidget( group1, m_space );

   m_itemList.insert( Massif::HEAP,                        /* checkbox */
                      optionWidget( Massif::HEAP,       group1, false ) );
   m_itemList.insert( Massif::HEAP_ADMIN,                  /* spinbox  */
                      optionWidget( Massif::HEAP_ADMIN, group1, true ) );
   m_itemList.insert( Massif::STACKS,                      /* checkbox */
                      optionWidget( Massif::STACKS,     group1, false ) );
   m_itemList.insert( Massif::DEPTH,                       /* spinbox  */
                      optionWidget( Massif::DEPTH,      group1, true ) );
   m_itemList.insert( Massif::ALLOC_FN,                    /* lineedit */
                      optionWidget( Massif::ALLOC_FN,   group1, true ) );
   m_itemList.insert( Massif::FORMAT,                      /* combobox */
                      optionWidget( Massif::FORMAT,     group1, true ) );
   m_itemList.insert( Massif::ALIGNMENT,                   /* spinbox  */
                      optionWidget( Massif::ALIGNMENT,  group1, true ) );
  
   /* grid layout for group1 */
   int rows = 7;
   int cols = 2;
   QGridLayout* grid1 = new QGridLayout( group1, rows, cols, m_margin, m_space );
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif
   grid1->setColStretch( 1, 10 );         /* push widgets to the left */

   grid1->addWidget( m_itemList[Massif::HEAP      ]->widget(),   1, 0 );
   grid1->addLayout( m_itemList[Massif::HEAP_ADMIN]->hlayout(),  2, 0 );

   grid1->addMultiCellWidget( sep(group1,"sep1"), 3,3, 0,1 );
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( 3, m_topSpace );   /* add a bit more space here */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( 3, m_topSpace );   /* add a bit more space here */
#endif

   grid1->addWidget( m_itemList[Massif::STACKS    ]->widget(),   4, 0 );
   grid1->addLayout( m_itemList[Massif::DEPTH     ]->hlayout(),  5, 0 );

   grid1->addMultiCellWidget( sep(group1,"sep2"), 6,6, 0,1 );
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( 6, m_topSpace );   /* add a bit more space here */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( 6, m_topSpace );   /* add a bit more space here */
#endif

   grid1->addLayout( m_itemList[Massif::ALLOC_FN  ]->vlayout(),  7, 0 );

   grid1->addMultiCellWidget( sep(group1,"sep3"), 8,8, 0,1 );
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( 8, m_topSpace );   /* add a bit more space here */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( 8, m_topSpace );   /* add a bit more space here */
#endif

   grid1->addLayout( m_itemList[Massif::FORMAT    ]->hlayout(),  9, 0 );
   grid1->addLayout( m_itemList[Massif::ALIGNMENT ]->hlayout(), 10, 0 );

   vbox->addStretch( m_space );
   vk_assert( m_itemList.count() <= numItems );

   QIntDictIterator<OptionWidget> it( m_itemList );
   for ( ;  it.current(); ++it ) {
      connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
              this,         SLOT(updateEditList( bool, OptionWidget * )));
   }

}


/* called when user clicks "Apply" / "Ok" / "Reset" buttons. */
void MassifOptionsPage::applyOption( int optId )
{ 
   vk_assert( optId >= 0 && optId < Massif::NUM_OPTS );

//   QString argval = m_itemList[optId]->currValue();

   /* apply option */
   switch ( optId ) {
   default:
      break;
   }
}


