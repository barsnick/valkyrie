/* ---------------------------------------------------------------------- 
 * Implementation of MassifOptionsPage            massif_options_page.cpp
 * subclass of OptionsPage to hold massif-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#include "massif_options_page.h"
#include "vk_objects.h"
#include "vk_utils.h"


MassifOptionsPage::MassifOptionsPage( QWidget* parent, VkObject* obj )
  : OptionsPage( parent, obj, "massif_options_page" )
{ 
  /* init the QIntDict list, resizing if necessary */
  unsigned int numItems = 7;
  itemList.resize( numItems );

	int space  = 5;  /* no. of pixels between cells */
	int margin = 11; /* no. of pixels to edge of widget */
  /* top layout: margin = 10; spacing = 25 */
  QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

  /* group1: massif options */
  QGroupBox* group1 = new QGroupBox( "Massif Options", this, "group1");
  vbox->addWidget( group1, space );

  itemList.insert( Massif::HEAP,        /* checkbox */
									 optionWidget( Massif::HEAP,       group1, false ) );
  itemList.insert( Massif::HEAP_ADMIN,  /* spinbox */
									 optionWidget( Massif::HEAP_ADMIN, group1, true ) );
  itemList.insert( Massif::STACKS,      /* checkbox */
									 optionWidget( Massif::STACKS,     group1, false ) );
  itemList.insert( Massif::DEPTH,       /* spinbox */
									 optionWidget( Massif::DEPTH,      group1, true ) );
  itemList.insert( Massif::ALLOC_FN,    /* lineedit */
									 optionWidget( Massif::ALLOC_FN,   group1, true ) );
  itemList.insert( Massif::FORMAT,      /* combobox*/
									 optionWidget( Massif::FORMAT,     group1, true ) );
  itemList.insert( Massif::ALIGNMENT,   /* spinbox */
									 optionWidget( Massif::ALIGNMENT,  group1, true ) );

	/* grid layout for group1 */
	int rows = 7;
	int cols = 1;
  QGridLayout* grid1 = new QGridLayout( group1, rows, cols, margin, space );
  grid1->setRowSpacing( 0, topspc );   /* blank top row */
  grid1->setColStretch( 1, space );    /* push widgets to the left */
  grid1->addWidget( itemList[Massif::HEAP      ]->widget(),  1, 0 );
  grid1->addLayout( itemList[Massif::HEAP_ADMIN]->layout(),  2, 0 );
  grid1->addWidget( itemList[Massif::STACKS    ]->widget(),  3, 0 );
  grid1->addLayout( itemList[Massif::DEPTH     ]->layout(),  4, 0 );
  grid1->addLayout( itemList[Massif::ALLOC_FN  ]->layout(),  5, 0 );
  grid1->addLayout( itemList[Massif::FORMAT    ]->layout(),  6, 0 );
  grid1->addLayout( itemList[Massif::ALIGNMENT ]->layout(),  7, 0 );

  vbox->addStretch( space );
  vk_assert( itemList.count() <= numItems );

  QIntDictIterator<OptionWidget> it( itemList );
  for ( ;  it.current(); ++it ) {
    connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
            this,         SLOT(updateEditList( bool, OptionWidget * )));
  }

}


/* Called when user clicks "Apply" or "Ok" button.  
	 Also called when Cancel button is clicked, to reset toggled values */
bool MassifOptionsPage::applyOptions( int, bool )
{ return true;  /* nothing to do */ }



/*
Massif::Massif()
  : VkObject( MASSIF, "Massif", "Ma&ssif", Qt::SHIFT+Qt::Key_S ) 
{
  addOpt( HEAP,        Option::ARG_BOOL,   Option::CHECK,
          "massif",    '\0',               "heap",
          "<yes|no>",  "yes|no",           "yes",
          "Profile heap blocks",
          "profile heap blocks",           urlMassifHeap );
  addOpt(  HEAP_ADMIN, Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "heap-admin", 
          "<number>",  "4|15",             "8",
          "Average admin bytes per heap block:",
          "average admin bytes per heap block", urlMassifHeapAdmin );
  addOpt( STACKS,      Option::ARG_BOOL,   Option::CHECK,
          "massif",    '\0',               "stacks",
          "<yes|no>",  "yes|no",           "yes",
          "Profile stack(s)",
          "profile stack(s)",              urlMassifStacks );
  addOpt(  DEPTH,      Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "depth", 
          "<number>",  "1|20",             "3",
          "Average admin bytes per heap block:",
          "average admin bytes per heap block", urlMassifDepth );
  addOpt( ALLOC_FN,    Option::ARG_STRING, Option::LEDIT, 
          "massif",    '\0',               "alloc-fn", 
          "<name>",    "",                 "empty",
          "Specify <fn> as an alloc function:",
          "specify <fn> as an alloc function", urlMassifAllocFn );
  addOpt( FORMAT,      Option::ARG_STRING,   Option::COMBO,  
          "massif",    '\0',                 "format",
          "<text|html|xml>", "text|html|xml", "text",
          "Format of textual output",
          "format of textual output",      urlMassifFormat );
  addOpt(  ALIGNMENT,  Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "alignment", 
          "<number>",  "8|1048576",        "8",
          "Minimum alignment of allocations:",
          "set minimum alignment of allocations", urlCoreAlignment );
}
*/
