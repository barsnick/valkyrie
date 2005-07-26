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
#include "vk_utils.h"


MassifOptionsPage::MassifOptionsPage( QWidget* parent, VkObject* obj )
  : OptionsPage( parent, obj, "massif_options_page" )
{ 
  /* init the QIntDict list, resizing if necessary */
  unsigned int numItems = 7;
  itemList.resize( numItems );

  /* top layout: margin = 10; spacing = 25 */
  QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

  /* group1: massif options */
  QGroupBox* group1 = new QGroupBox( " Massif Options ", this, "group1");
  vbox->addWidget( group1, space );

  itemList.insert( Massif::HEAP,                        /* checkbox */
                   optionWidget( Massif::HEAP,       group1, false ) );
  itemList.insert( Massif::HEAP_ADMIN,                  /* spinbox  */
                   optionWidget( Massif::HEAP_ADMIN, group1, true ) );
  itemList.insert( Massif::STACKS,                      /* checkbox */
                   optionWidget( Massif::STACKS,     group1, false ) );
  itemList.insert( Massif::DEPTH,                       /* spinbox  */
                   optionWidget( Massif::DEPTH,      group1, true ) );
  itemList.insert( Massif::ALLOC_FN,                    /* lineedit */
                   optionWidget( Massif::ALLOC_FN,   group1, true ) );
  itemList.insert( Massif::FORMAT,                      /* combobox */
                   optionWidget( Massif::FORMAT,     group1, true ) );
  itemList.insert( Massif::ALIGNMENT,                   /* spinbox  */
                   optionWidget( Massif::ALIGNMENT,  group1, true ) );

  /* grid layout for group1 */
  int rows = 7;
  int cols = 2;
  QGridLayout* grid1 = new QGridLayout( group1, rows, cols, margin, space );
  grid1->setRowSpacing( 0, topSpace );   /* blank top row */
  grid1->setColStretch( 1, 10 );         /* push widgets to the left */

  grid1->addWidget( itemList[Massif::HEAP      ]->widget(),   1, 0 );
  grid1->addLayout( itemList[Massif::HEAP_ADMIN]->hlayout(),  2, 0 );

  grid1->addMultiCellWidget( sep(group1,"sep1"), 3,3, 0,1 );
  grid1->setRowSpacing( 3, topSpace );   /* add a bit more space here */

  grid1->addWidget( itemList[Massif::STACKS    ]->widget(),   4, 0 );
  grid1->addLayout( itemList[Massif::DEPTH     ]->hlayout(),  5, 0 );

  grid1->addMultiCellWidget( sep(group1,"sep2"), 6,6, 0,1 );
  grid1->setRowSpacing( 6, topSpace );   /* add a bit more space here */

  grid1->addLayout( itemList[Massif::ALLOC_FN  ]->vlayout(),  7, 0 );

  grid1->addMultiCellWidget( sep(group1,"sep3"), 8,8, 0,1 );
  grid1->setRowSpacing( 8, topSpace );   /* add a bit more space here */

  grid1->addLayout( itemList[Massif::FORMAT    ]->hlayout(),  9, 0 );
  grid1->addLayout( itemList[Massif::ALIGNMENT ]->hlayout(), 10, 0 );

  vbox->addStretch( space );
  vk_assert( itemList.count() <= numItems );

  QIntDictIterator<OptionWidget> it( itemList );
  for ( ;  it.current(); ++it ) {
    connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
            this,         SLOT(updateEditList( bool, OptionWidget * )));
  }

}


/* Called when user clicks "Apply" or "Ok" button.  
   Also called when Cancel button is clicked, to reset toggled values.
   Note: _never_ have a default in the switch case here, in order to
   catch 'forgotten' args. Any cases not handled should be listed. */
bool MassifOptionsPage::applyOptions( int, bool )
{ 
  VK_DEBUG( "nothing implemented in here yet" );
  return true;
}


