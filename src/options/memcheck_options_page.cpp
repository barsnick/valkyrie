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
  itemList.resize( numItems );

  /* top layout: margin = 10; spacing = 25 */
  QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

  /* group1: memcheck options */
  QGroupBox* group1 = new QGroupBox( " Memcheck Options ", this, "group1");
  vbox->addWidget( group1, space );

  itemList.insert( Memcheck::LEAK_CHECK,                 /* combobox */
                   optionWidget( Memcheck::LEAK_CHECK, group1, true ) );
  itemList.insert( Memcheck::SHOW_REACH,                 /* checkbox */
                   optionWidget( Memcheck::SHOW_REACH, group1, false ) );

  itemList.insert( Memcheck::PARTIAL,                    /* checkbox */
                   optionWidget( Memcheck::PARTIAL,    group1, false ) );
  itemList.insert( Memcheck::GCC_296,                    /* checkbox */
                   optionWidget( Memcheck::GCC_296,    group1, false ) );
  itemList.insert( Memcheck::STRLEN,                     /* checkbox */
                   optionWidget( Memcheck::STRLEN,     group1, false ) );

  itemList.insert( Memcheck::LEAK_RES,                   /* combobox */
                   optionWidget( Memcheck::LEAK_RES,   group1, true ) );
  itemList.insert( Memcheck::FREELIST,                   /* ledit    */
                   optionWidget( Memcheck::FREELIST,   group1, true ) );
  itemList.insert( Memcheck::ALIGNMENT,                  /* spinbox  */
                   optionWidget( Memcheck::ALIGNMENT,  group1, true ) );

  /* grid layout for group1 */
  int rows = 9;
  int cols = 2;
  QGridLayout* grid1 = new QGridLayout( group1, rows, cols, margin, space );
  grid1->setRowSpacing( 0, topSpace );   /* blank top row */
  grid1->setColStretch( 1, 10 );         /* push widgets to the left */

  grid1->addLayout( itemList[Memcheck::LEAK_CHECK]->hlayout(), 1, 0 );
  grid1->addWidget( itemList[Memcheck::SHOW_REACH]->widget(),  2, 0 );

  grid1->addMultiCellWidget( sep(group1,"sep1"), 3,3, 0,1 );
  grid1->setRowSpacing( 3, topSpace );   /* add a bit more space here */

  grid1->addWidget( itemList[Memcheck::PARTIAL]->widget(),     4, 0 );
  grid1->addWidget( itemList[Memcheck::GCC_296]->widget(),     5, 0 );
  grid1->addWidget( itemList[Memcheck::STRLEN]->widget(),      6, 0 );

  grid1->addMultiCellWidget( sep(group1,"sep2"), 7,7, 0,1 );
  grid1->setRowSpacing( 7, topSpace );   /* add a bit more space here */

  grid1->addLayout( itemList[Memcheck::LEAK_RES]->hlayout(),    8, 0 );
  grid1->addLayout( itemList[Memcheck::FREELIST]->hlayout(),    9, 0 );
  grid1->addLayout( itemList[Memcheck::ALIGNMENT ]->hlayout(), 10, 0 );

	/* Note: this widget is disabled because valgrind disables them when
		 generating xml output (see docs/internals/xml_output.txt) */
  itemList[Memcheck::LEAK_CHECK]->setEnabled( false );



  vbox->addStretch( space );
  vk_assert( itemList.count() <= numItems );

  QIntDictIterator<OptionWidget> it( itemList );
  for ( ;  it.current(); ++it ) {
    connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
            this,         SLOT(updateEditList( bool, OptionWidget * )));
  }

}


/* called when user clicks "Apply" or "Ok" button.  
   also called when Cancel button is clicked, to reset toggled values */
bool MemcheckOptionsPage::applyOptions( int optId, bool/*=false*/ )
{ 
  bool retval = true;

  switch ( optId ) {

    case Memcheck::FREELIST: {
      const char* argval = itemList[optId]->currValue().latin1();
      int errval = vkObj->checkOptArg( optId, argval, true );
      if ( errval != PARSED_OK ) {
        vkInfo( this, "Invalid Entry", "%s:\n\"%s\"", 
                parseErrString(errval), argval );
        itemList[optId]->cancelEdit();
        retval = false;
      }
    } break;

    default:
      break;
  }

  return retval;
}



