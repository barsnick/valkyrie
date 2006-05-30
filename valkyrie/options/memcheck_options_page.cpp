/* ---------------------------------------------------------------------- 
 * Implementation of MemcheckOptionsPage        memcheck_options_page.cpp
 * Subclass of OptionsPage to hold memcheck-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "memcheck_options_page.h"
#include "memcheck_object.h"
#include "vk_messages.h"
#include "vk_utils.h"



MemcheckOptionsPage::MemcheckOptionsPage( QWidget* parent, VkObject* obj )
   : OptionsPage( parent, obj, "memcheck_options_page" )
{ 
   /* init the QIntDict list, resizing if necessary */
   unsigned int numItems = 11;
   m_itemList.resize( numItems );

   /* top layout: margin = 10; spacing = 25 */
   QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

   /* group1: memcheck options */
   QGroupBox* group1 = new QGroupBox( " Memcheck Options ", this, "group1");
   vbox->addWidget( group1, m_space );

   m_itemList.insert( Memcheck::LEAK_CHECK,                 /* combobox */
                      optionWidget( Memcheck::LEAK_CHECK, group1, true ) );
   m_itemList.insert( Memcheck::SHOW_REACH,                 /* checkbox */
                      optionWidget( Memcheck::SHOW_REACH, group1, false ) );
   m_itemList.insert( Memcheck::UNDEF_VAL,                  /* checkbox */
                      optionWidget( Memcheck::UNDEF_VAL,  group1, false ) );
  
   m_itemList.insert( Memcheck::PARTIAL,                    /* checkbox */
                      optionWidget( Memcheck::PARTIAL,    group1, false ) );
   m_itemList.insert( Memcheck::GCC_296,                    /* checkbox */
                      optionWidget( Memcheck::GCC_296,    group1, false ) );
  
   m_itemList.insert( Memcheck::LEAK_RES,                   /* combobox */
                      optionWidget( Memcheck::LEAK_RES,   group1, true ) );
   m_itemList.insert( Memcheck::FREELIST,                   /* ledit    */
                      optionWidget( Memcheck::FREELIST,   group1, true ) );
   m_itemList.insert( Memcheck::ALIGNMENT,                  /* spinbox  */
                      optionWidget( Memcheck::ALIGNMENT,  group1, true ) );

   /* grid layout for group1 */
   int rows = 11;
   int cols = 2;
   QGridLayout* grid1 = new QGridLayout( group1, rows, cols, m_margin, m_space );
   int row=0;
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( row++, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( row++, m_topSpace );   /* blank top row */
#endif
   grid1->setColStretch( 1, 10 );         /* push widgets to the left */

   grid1->addLayout( m_itemList[Memcheck::LEAK_CHECK]->hlayout(), row++, 0 );
   grid1->addWidget( m_itemList[Memcheck::SHOW_REACH]->widget(),  row++, 0 );
   grid1->addWidget( m_itemList[Memcheck::UNDEF_VAL]->widget(),   row++, 0 );

   grid1->addMultiCellWidget( sep(group1,"sep1"), row,row, 0,1 );
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( row++, m_topSpace );   /* add a bit more space here */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( row++, m_topSpace );   /* add a bit more space here */
#endif

   grid1->addWidget( m_itemList[Memcheck::PARTIAL]->widget(),     row++, 0 );
   grid1->addWidget( m_itemList[Memcheck::GCC_296]->widget(),     row++, 0 );

   grid1->addMultiCellWidget( sep(group1,"sep2"), row,row, 0,1 );
   row++;
#if (QT_VERSION-0 >= 0x030200)
   grid1->setRowSpacing( row, m_topSpace );   /* add a bit more space here */
#else // QT_VERSION < 3.2
   grid1->addRowSpacing( row, m_topSpace );   /* add a bit more space here */
#endif

   grid1->addLayout( m_itemList[Memcheck::LEAK_RES]->hlayout(),    row++, 0 );
   grid1->addLayout( m_itemList[Memcheck::FREELIST]->hlayout(),    row++, 0 );
   grid1->addLayout( m_itemList[Memcheck::ALIGNMENT ]->hlayout(),  row++, 0 );

   vk_assert(row == rows);

   vbox->addStretch( m_space );

   /* finalise page ------------------------------------------------- */
   vk_assert( m_itemList.count() <= numItems );

   QIntDictIterator<OptionWidget> it( m_itemList );
   for ( ;  it.current(); ++it ) {
      connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
              this,         SLOT(updateEditList( bool, OptionWidget * )));
   }


   /* Disabled Widgets -------------------------------------------------
      Valgrind presets and ignores some options when generating xml
      output. (see docs/internals/xml_output.txt) */
   m_itemList[Memcheck::LEAK_CHECK]->setEnabled( false );
}


/* called when user clicks "Apply" / "Ok" / "Reset" buttons. */
void MemcheckOptionsPage::applyOption( int optId )
{ 
   vk_assert( optId >= 0 && optId < Memcheck::NUM_OPTS );

//   QString argval = m_itemList[optId]->currValue();

   /* apply option */
   switch ( optId ) {
   default:
      break;
   }
}



