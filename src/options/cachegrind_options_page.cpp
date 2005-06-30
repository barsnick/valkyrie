/* ---------------------------------------------------------------------- 
 * Implementation of CachegrindOptionsPage    cachegrind_options_page.cpp
 * subclass of OptionsPage to hold cachegrind-specific options | flags.
 * ---------------------------------------------------------------------- 
 */

#include "cachegrind_options_page.h"
#include "vk_objects.h"
#include "vk_utils.h"
#include "vk_msgbox.h"


CachegrindOptionsPage::CachegrindOptionsPage( QWidget* parent, VkObject* obj )
  : OptionsPage( parent, obj, "cachegrind_options_page" )
{ 
  /* init the QIntDict list, resizing if necessary */
  unsigned int numItems = 11;
  itemList.resize( numItems );

  int space  = 5;  /* no. of pixels between cells */
  int margin = 11; /* no. of pixels to edge of widget */
  /* top layout: margin = 10; spacing = 25 */
  QVBoxLayout* vbox = new QVBoxLayout( this, 10, 25, "vbox" );

  /* group1: cache configuration options */
  QGroupBox* group1 = new QGroupBox(" Cache Configuration ", this, "group1");
  vbox->addWidget( group1, space );

  /* we make the three spin-widgets 'by hand' here */
  Option* opt;
  SpWidget* spinw;

  /* Specify I1 cache configuration */
  opt = vkObj->findOption( Cachegrind::I1_CACHE );
  spinw = new SpWidget( group1, opt, true, 3 );
  spinw->addSection( 0, 1048576, 0, 0 );  /* min, max, def, use_powers */
  spinw->addSection( 0, 8,       0, 1 );
  spinw->addSection( 0, 8192,    0, 0 );
  itemList.insert( Cachegrind::I1_CACHE, spinw );

  /* Specify D1 cache configuration */
  opt = vkObj->findOption( Cachegrind::D1_CACHE );
  spinw = new SpWidget( group1, opt, true, 3 );
  spinw->addSection( 0, 1048576, 0, 0 );  /* min, max, def, use_powers */
  spinw->addSection( 0, 8,       0, 1 );
  spinw->addSection( 0, 8192,    0, 0 );
  itemList.insert( Cachegrind::D1_CACHE, spinw );

  /* Specify L2 cache configuration */
  opt = vkObj->findOption( Cachegrind::L2_CACHE );
  spinw = new SpWidget( group1, opt, true, 3 );
  spinw->addSection( 0, 1048576, 0, 0 );  /* min, max, def, use_powers */
  spinw->addSection( 0, 8,       0, 1 );
  spinw->addSection( 0, 8192,    0, 0 );
  itemList.insert( Cachegrind::L2_CACHE, spinw );

  /* grid layout for group1 */
  int rows = 4;
  int cols = 3;
  QGridLayout* grid1 = new QGridLayout( group1, rows, cols, margin, space );
  grid1->setRowSpacing( 0, topSpace );   /* blank top row */
  grid1->setColStretch( 2, 10 );         /* push widgets to the left */

  grid1->addWidget( itemList[Cachegrind::I1_CACHE]->label(),  1, 0 );
  grid1->addWidget( itemList[Cachegrind::I1_CACHE]->widget(), 1, 1 );
  grid1->addWidget( itemList[Cachegrind::D1_CACHE]->label(),  2, 0 );
  grid1->addWidget( itemList[Cachegrind::D1_CACHE]->widget(), 2, 1 );
  grid1->addWidget( itemList[Cachegrind::L2_CACHE]->label(),  3, 0 );
  grid1->addWidget( itemList[Cachegrind::L2_CACHE]->widget(), 3, 1 );


  /* group2: annotation options */
  QGroupBox* group2 = new QGroupBox(" Annotation Options ", this, "group2");
  vbox->addWidget( group2, space );

  itemList.insert( Cachegrind::AUTO,                    /* checkbox */
                   optionWidget( Cachegrind::AUTO,     group2, false ) );
  itemList.insert( Cachegrind::SHOW,                    /* line edit */
                   optionWidget( Cachegrind::SHOW,     group2, true ) );
  itemList.insert( Cachegrind::SORT,                    /* line edit */
                   optionWidget( Cachegrind::SORT,     group2, true ) );
  itemList.insert( Cachegrind::THRESH,                  /* spinbox */
                   optionWidget( Cachegrind::THRESH,   group2, true ) );
  itemList.insert( Cachegrind::CONTEXT,                 /* spinbox */
                   optionWidget( Cachegrind::CONTEXT,  group2, true ) );
  itemList.insert( Cachegrind::PID_FILE,                /* line edit */
                   optionWidget( Cachegrind::PID_FILE, group2, false ));
  LeWidget* pidLedit = ((LeWidget*)itemList[Cachegrind::PID_FILE]);
  pidLedit->addButton( group2, this, SLOT(getPidFile()) );

  itemList.insert( Cachegrind::INCLUDE,                 /* line edit */
                   optionWidget( Cachegrind::INCLUDE, group2, false ));
  LeWidget* incLedit = ((LeWidget*)itemList[Cachegrind::INCLUDE]);
  incLedit->addButton( group2, this, SLOT(getIncludeDirs()) );

  /* grid layout for group2 */
  rows = 4;
  cols = 2;
  QGridLayout* grid2 = new QGridLayout( group2, rows, cols, margin, space );
  grid2->setRowSpacing( 0, topSpace );   /* blank top row */
  grid2->setColStretch( 2, 10 );         /* push widgets to the left */

  grid2->addWidget(itemList[Cachegrind::AUTO   ]->widget(),  1, 0 );
  grid2->addLayout(itemList[Cachegrind::SHOW   ]->hlayout(), 2, 0 );
  grid2->addLayout(itemList[Cachegrind::SORT   ]->hlayout(), 3, 0 );
  grid2->addLayout(itemList[Cachegrind::THRESH ]->hlayout(), 4, 0 );
  grid2->addLayout(itemList[Cachegrind::CONTEXT]->hlayout(), 5, 0 );

  grid2->setRowSpacing( 6, topSpace );
  grid2->addMultiCellLayout( pidLedit->hlayout(), 7,7, 0,2 );
  grid2->addMultiCellLayout( incLedit->hlayout(), 8,8, 0,2 );


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
bool CachegrindOptionsPage::applyOptions( int id, bool undo/*=false*/ )
{ 
  bool retval = true;

  if ( undo ) return retval;   /* nothing to do */

  switch ( id ) {

    case Cachegrind::SHOW:
    case Cachegrind::SORT:
      break;

    case Cachegrind::I1_CACHE:
    case Cachegrind::D1_CACHE:
    case Cachegrind::L2_CACHE:
    case Cachegrind::PID_FILE:
    case Cachegrind::INCLUDE: {
      const char* argval = itemList[id]->currValue().latin1();
      int errval = vkObj->checkOptArg( id, argval, true );
      if ( errval != PARSED_OK ) {
        vkError( this, "Invalid Entry", 
                 "%s:\n\"%s\"", parseErrString(errval), argval );
        itemList[id]->cancelEdit();
        retval = false;
      }
    } break;

  }

  return retval;
}


void CachegrindOptionsPage::getPidFile()
{ 
  QString pidfile = QFileDialog::getOpenFileName( QString::null, 
                    "Pid Files(*.pid);;All Files (*)", 
                    this, "fdlg", "Select Pid File to View" );

  if ( !pidfile.isEmpty() ) { /* user might have clicked Cancel */
    ((LeWidget*)itemList[Cachegrind::PID_FILE])->setCurrValue(pidfile);
    applyOptions( Cachegrind::PID_FILE );
  }

}


void CachegrindOptionsPage::getIncludeDirs()
{ 
  QString incdir = QFileDialog::getExistingDirectory( QString::null,
                           this, "get_inc_dir",
                           "Choose directory to include", true );
  if ( !incdir.isEmpty() ) { /* user might have clicked Cancel */
    ((LeWidget*)itemList[Cachegrind::INCLUDE])->addCurrValue(incdir);
    applyOptions( Cachegrind::INCLUDE );
  }

}
