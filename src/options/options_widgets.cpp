/* ---------------------------------------------------------------------- 
 * Implementation of class OptionWidget               options_widgets.cpp
 * Various widgets used on the 'pages' to control user input
 * ---------------------------------------------------------------------- 
 */

#include <qcursor.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>

#include "options_widgets.h"
#include "vk_option.h"
#include "vk_include.h"
#include "vk_utils.h"
#include "vk_config.h"
#include "vk_msgbox.h"



/* class OptionWidget -------------------------------------------------- */
OptionWidget::OptionWidget( QWidget* parent, const char* name,
                            Option* vkopt, bool mklabel )
  : QObject( parent, name ) 
{
  //VK_DEBUG("opt->long_flag: %s\n", vkopt->cfgKey().ascii() );

  widg   = 0;
  wLabel = 0;
  opt    = vkopt;

  initialValue = vkConfig->rdEntry( opt->cfgKey(), opt->cfgGroup() );
  currentValue = initialValue;

  if ( mklabel ) {
    wLabel = new QLabel( opt->shortHelp, parent );
    wLabel->setMinimumSize( wLabel->sizeHint() );
    wLabel->setAlignment( AlignLeft|AlignVCenter|ExpandTabs );
  }
}

int OptionWidget::id()
{ return opt->key; }

QLabel * OptionWidget::label()
{ return wLabel; }

QWidget * OptionWidget::widget()
{ return widg; }

QString OptionWidget::currValue()
{ return currentValue; }

QString OptionWidget::initValue()
{ return initialValue; }

void OptionWidget::saveEdit( bool perm )
{
  if ( perm ) {
    initialValue = currentValue;
  } 
  vkConfig->wrEntry( currentValue, opt->cfgKey(), opt->cfgGroup() );
}

void OptionWidget::cancelEdit()
{
  currentValue = initialValue;
  vkConfig->wrEntry( currentValue, opt->cfgKey(), opt->cfgGroup() );
  emit valueChanged( false, this );
  reset();
}


/* layout for comboboxes / spinboxes so we don't have to code
   specially for this */
QHBoxLayout * OptionWidget::layout()
{ 
  vk_assert( wLabel != 0 );

  hBox = new QHBoxLayout( 6, "hBox" );
  hBox->addWidget( wLabel );
  hBox->addWidget( widg );

  switch ( opt->widgType ) {
  case Option::COMBO:
    hBox->setStretchFactor( wLabel, 10 );
    hBox->setStretchFactor( widg,    0 );
    break;
  case Option::SPINBOX:
    hBox->setStretchFactor( wLabel, 10 );
    hBox->setStretchFactor( widg,    3 );
    break;
  default:
    break;
  }

  return hBox; 
}



/* class CkWidget: QCheckBox ------------------------------------------- */
CkWidget::~CkWidget() 
{
  if ( cbox ) {
    delete cbox;
    cbox = NULL;
  }
}

CkWidget::CkWidget( QWidget* parent, Option* vkopt, bool mklabel )
  : OptionWidget( parent, "ck_widget", vkopt, mklabel ) 
{ 
  cbox = new QCheckBox( opt->shortHelp, parent, "check_box" );
  widg = cbox;
  initialState = vkConfig->rdBool( opt->cfgKey(), opt->cfgGroup() );
  cbox->setChecked( initialState );
  connect( cbox, SIGNAL(toggled(bool)), 
           this, SLOT(ckChanged(bool)) );
}

void CkWidget::ckChanged( bool on )
{
  currentValue = (on) ? opt->possValues[0] : opt->possValues[1];
  bool edited = currentValue != vkConfig->rdEntry( opt->cfgKey(),
                                                   opt->cfgGroup() );
  emit valueChanged( edited, this );
  /* for dis/enabling associated widgets */
  emit changed( on );
  emit clicked( opt->key );
}

void CkWidget::reset()
{ cbox->setChecked( initialState ); }

void CkWidget::resetDefault()
{
#if 1
  bool on = ( opt->defValue() == "1"   || opt->defValue() == "on"  || 
              opt->defValue() == "yes" || opt->defValue() == "true" );
  setOn( on );
#else
  currentValue = opt->defValue();
  bool edited = currentValue != initialValue;
  bool on = ( opt->defValue() == "1"   || opt->defValue() == "on"  || 
              opt->defValue() == "yes" || opt->defValue() == "true" );
  setOn( on );
  emit valueChanged( edited, this );
  emit changed( on );
  emit clicked( opt->key );
#endif
}

bool CkWidget::isOn()
{ return cbox->isChecked(); }

void CkWidget::setOn( bool on )
{ cbox->setChecked( on ); }



/* class RbWidget: QRadioButton ---------------------------------------- */
RbWidget::~RbWidget() 
{
  if ( radio ) {
    delete radio;
    radio = NULL;
  }
}

RbWidget::RbWidget( QWidget* parent, Option* vkopt, bool mklabel )
  : OptionWidget( parent, "rb_widget", vkopt, mklabel ) 
{ 
  radio = new QRadioButton( opt->shortHelp, parent, "radio_button" );
  widg = radio;
  initialState = vkConfig->rdBool( opt->cfgKey(), opt->cfgGroup() );
  radio->setChecked( initialState );
  connect( radio, SIGNAL(toggled(bool)), 
           this, SLOT(rbChanged(bool)) );
}

void RbWidget::rbChanged( bool on )
{
  currentValue = (on) ? opt->possValues[0] : opt->possValues[1];
  bool edited = currentValue != vkConfig->rdEntry( opt->cfgKey(),
                                                   opt->cfgGroup() );
  emit valueChanged( edited, this );
  /* for dis/enabling associated widgets */
  emit changed( on );
  emit clicked( opt->key );
}

void RbWidget::reset()
{ radio->setChecked( initialState ); }

void RbWidget::resetDefault()
{
#if 1
  bool on = ( opt->defValue() == "1"   || opt->defValue() == "on"  || 
              opt->defValue() == "yes" || opt->defValue() == "true" );
  setOn( on );
#else
  currentValue = opt->defValue();
  bool edited  = currentValue != initialValue;
  bool on = ( opt->defValue() == "1"   || opt->defValue() == "on"  || 
              opt->defValue() == "yes" || opt->defValue() == "true" );
  setOn( on );
  emit valueChanged( edited, this );
  emit changed( on );
  emit clicked( opt->key );
#endif
}

bool RbWidget::isOn()
{ return radio->isOn(); }

void RbWidget::setOn( bool on )
{ radio->setChecked( on ); }



/* class LeWidget: QLineEdit ------------------------------------------- */
LeWidget::~LeWidget()
{
  if ( ledit ) {
    delete ledit;
    ledit = NULL;
  }
  if ( pb ) {
    delete pb;
    pb = NULL;
  }
}

LeWidget::LeWidget( QWidget *parent, Option * vkopt, bool mklabel )
  : OptionWidget( parent, "le_widget", vkopt, mklabel ) 
{
  pb     = NULL;
  ledit  = new QLineEdit( parent, "line_edit" ); 
  widg = ledit;
  ledit->setText( initialValue );
  connect( ledit, SIGNAL( textChanged(const QString &) ),
           this,  SLOT( leChanged(const QString &) ) );
}

void LeWidget::setCurrValue( const QString& txt ) 
{ 
  ledit->setText( txt );  /* calls leChanged(txt) */
  if ( txt.length() > 0 )
    ledit->setCursorPosition( 0 );
}

void LeWidget::addCurrValue( const QString& txt )
{ 
  if ( currentValue.isEmpty() ) {
    setCurrValue( txt );
  } else {
    setCurrValue( ledit->text() + "," + txt ); 
  }
}

void LeWidget::leChanged(const QString& txt)
{
  currentValue = txt;
  bool edited  = currentValue != vkConfig->rdEntry( opt->cfgKey(),
                                                    opt->cfgGroup() );
  emit valueChanged( edited, this );
}

void LeWidget::reset()
{ ledit->setText( initialValue ); }

void LeWidget::resetDefault()
{ setCurrValue( opt->defValue() ); }

QPushButton * LeWidget::button()
{ return pb; }

void LeWidget::addButton( QWidget *parent, const QObject * receiver, 
                          const char * slot, 
                          QString txt/*=QString::null*/,
                          bool icon/*=true*/ )
{
  int pbht = ledit->height() - 8;
  if ( !icon ) {
    pb = new QPushButton( opt->shortHelp, parent );
  } else {
    QPixmap openIcon( vkConfig->pixmap( "fileopen.xpm" ) );
    if ( !txt.isNull() )
      pb = new QPushButton( openIcon, txt, parent );
    else
      pb = new QPushButton( openIcon, opt->shortHelp, parent );
  }
  pb->setMaximumHeight( pbht );
  connect( pb, SIGNAL(clicked()), receiver, slot );
}


/* layout for line edits where we want to have a pushbutton with
   opt->shortHelp as its text, instead of the standard QLabel */
QHBoxLayout * LeWidget::layout()
{
  hBox = new QHBoxLayout( 6, "le_hBox" );
  if ( pb != NULL )
    hBox->addWidget( pb );
  else
    hBox->addWidget( wLabel );
  hBox->addWidget( widg );

  return hBox; 
}


/* class CbWidget: QComboBox ------------------------------------------- */
CbWidget::~CbWidget()
{
  if ( combo ) {
    delete combo;
    combo = NULL;
  }
}

CbWidget::CbWidget( QWidget *parent, Option * vkopt, bool mklabel )
  : OptionWidget( parent, "cb_widget", vkopt, mklabel ) 
{
  currIdx = 0;
  combo = new QComboBox( true, parent, "combo_box" );
  widg = combo;
  combo->setInsertionPolicy( QComboBox::NoInsertion );
  combo->setAutoCompletion( true );
  combo->insertStringList( opt->possValues );
  combo->setCurrentItem( currIdx );

  for ( int i=0; i<combo->count(); i++ ) {
    if ( initialValue == combo->text(i) ) {
      currIdx = i;
      break;
    }
  }

  combo->setCurrentItem( currIdx );
  connect( combo, SIGNAL( activated(const QString &) ),
           this,  SLOT( cbChanged(const QString &) ) );
}

void CbWidget::cbChanged( const QString& txt )
{
  bool found = false;
  for ( int i=0; i<combo->count(); ++i ) {
    if ( txt == combo->text(i) ) {
      found = true;
      combo->setCurrentItem( i );
      break;
    }
  }

  if ( !found ) {  // didn't find the string the user typed in
    combo->setCurrentItem( currIdx );
  } else {
    currIdx = combo->currentItem();
    currentValue = combo->currentText();
    bool edited = currentValue != vkConfig->rdEntry( opt->cfgKey(),
                                                     opt->cfgGroup() );
    emit valueChanged( edited, this );
  }
}

void CbWidget::reset()
{ cbChanged( initialValue ); }

void CbWidget::resetDefault()
{ cbChanged( opt->defValue() ); }




/* class SpWidget: IntSpin --------------------------------------------- */
SpWidget::~SpWidget()
{
  if ( intspin ) {
    delete intspin;
    intspin = NULL;
  }
}

SpWidget::SpWidget( QWidget* parent, Option* vkopt, 
                    bool mklabel, int num_sections/*=1*/ )
  : OptionWidget( parent, "sp_widget", vkopt, mklabel ) 
{
  numSections = num_sections;
  intspin = new IntSpin( parent, "int_spin" );
  widg = intspin;
  connect( intspin, SIGNAL(valueChanged(const QString&)), 
           this,    SLOT(spChanged(const QString&)) );
}

void SpWidget::addSection( int min, int max, int defval,
                           int step, QString sep_char )
{ intspin->addSection( min, max, defval, step, sep_char ); }

void SpWidget::spChanged( const QString &val )
{
  currentValue = val;
  bool edited  = currentValue != vkConfig->rdEntry( opt->cfgKey(),
                                                    opt->cfgGroup() );
  emit valueChanged( edited, this );
}

void SpWidget::reset()
{
  if ( numSections == 1 ) {
    intspin->setValue( initialValue.toInt(), 0 );
  }  else {
    QStringList values = QStringList::split( ",", initialValue );
    for ( unsigned int i=0; i<values.count(); i++ ) {
      intspin->setValue( values[i].toInt(), i );
    }
  }
}

void SpWidget::resetDefault()
{ 
  spChanged( opt->defValue() ); 
  if ( numSections == 1 ) {
    intspin->setValue( opt->defValue().toInt(), 0 );
  } else {
    QStringList values = QStringList::split( ",", opt->defValue() );
    for ( unsigned int i=0; i<values.count(); i++ ) {
      intspin->setValue( values[i].toInt(), i );
    }
  }
}




/* class LbWidget: QListBox -------------------------------------------- 
   FIXME: don't overwrite the current supp files in valkyrierc, just
   add the new ones onto the end of the list. */

static const char* sel_supp_xpm[] = {
"11 11 8 1",
"   c None",
".  c #024266",
"+  c #5A9AB8",
"@  c #1B5F8E",
"#  c #79B7CD",
"$  c #5A97B5",
"%  c #AEDDE9",
"&  c #8ECADC",
"     .     ",
"  . .+. .  ",
" .+.@#@.+. ",
"  .#$%$#.  ",
" .@$&%&$@. ",
".+#%%%%%#+.",
" .@$&%&$@. ",
"  .#$%$#.  ",
" .+.@#@.+. ",
"  . .+. .  ",
"     .     "};

static const char* unsel_supp_xpm[] = {
"11 11 1 1",
"   c None",
"           ",
"           ",
"           ",
"           ",
"           ",
"           ",
"           ",
"           ",
"           ",
"           ",
"           "};

LbWidget::~LbWidget()
{
  if ( lbox ) {
    delete lbox;
    lbox = NULL;
  }
}

LbWidget::LbWidget( QWidget *parent, Option * vkopt, bool mklabel )
  : OptionWidget( parent, "lb_widget", vkopt, mklabel ) 
{
  maxSelections = 10;

  lbox = new QListBox( parent, "list_box" );
  widg = lbox;
  lbox->setHScrollBarMode( QScrollView::AlwaysOff );
  lbox->setSelectionMode ( QListBox::Single );

  pmSel   = QPixmap( sel_supp_xpm );
  pmUnsel = QPixmap( unsel_supp_xpm );
  load();
  connect( lbox, SIGNAL(contextMenuRequested(QListBoxItem*, 
                                             const QPoint &)),
           this, SLOT(popupMenu(QListBoxItem*, const QPoint &)));
}

/* Split initialValue at the sep char ',', then strip the [+|-] prefix
   from each string, and allocate the appropriate icon.  If > 10 files
   are selected, unselect those.  */
void LbWidget::load()
{
  numSelected = 0;
  QStringList files = QStringList::split( ",", initialValue );
  QString fname;
  QString marker;
  QPixmap pm;
  SuppFile *suppFile;
  
  for ( unsigned int i=0; i<files.count(); i++ ) {

    if ( files[i].left(3) == "[+]" )
      numSelected++;

    vk_assert( numSelected < maxSelections );
    marker = ( numSelected >= maxSelections ) 
             ? "[-]" : files[i].left(3);

    fname  = files[i];
    fname.remove( 0, 3 );

    pm = ( marker == "[+]" ) ? pmSel : pmUnsel;
    suppFile = new SuppFile( pm, fname, marker );
    lbox->insertItem( suppFile );
  }

  lbox->setCurrentItem( 0 );
}

void LbWidget::reset()
{
  lbox->clear();
  load();
}

void LbWidget::resetDefault()
{
  printf("LbWidget::resetDefault()\n");
}

/* compare the listbox item strings with initialValue strings */
void LbWidget::lbChanged( const QString& )
{
  SuppFile *suppFile;

  /* update currentValue */
  currentValue = "";
  for ( unsigned int i=0; i<lbox->count(); i++ ) {
    suppFile = (SuppFile*)lbox->item(i);
    currentValue += suppFile->selMark + suppFile->fileName() + ",";
  }
  /* remove trailing ',' */
  currentValue.remove( currentValue.length()-1, 1 );

  bool edited = currentValue != initialValue;
  emit valueChanged( edited, this );
}

void LbWidget::popupMenu( QListBoxItem *item, const QPoint& )
{
  QPopupMenu popMenu( lbox );
  int SELECT   = popMenu.insertItem( "Select File" );
  int UNSELECT = popMenu.insertItem( "Unselect File" );
  popMenu.insertSeparator();
  int REMOVE   = popMenu.insertItem( "Remove File" );
  int ADD      = popMenu.insertItem( "Add a File" );

  SuppFile *suppFile;

  if ( !item ) {   /* lbox is empty */
    popMenu.setItemEnabled( SELECT, false );
    popMenu.setItemEnabled( UNSELECT, false );
    popMenu.setItemEnabled( REMOVE, false );
  } else {
    suppFile = (SuppFile*)item;
    if ( suppFile->selected() || 
         (numSelected == maxSelections) )
      popMenu.setItemEnabled( SELECT, false );
    else if ( !suppFile->selected() )
      popMenu.setItemEnabled( UNSELECT, false );
  }

  popMenu.setMouseTracking( true );
  int id = popMenu.exec( QCursor::pos() );

  if ( id == -1 )
    return;

  int indx = lbox->index( item );
  QString fname;

  if ( id == SELECT ) {
    fname = suppFile->fileName();
    suppFile = new SuppFile( pmSel, fname, "[+]" );
    lbox->removeItem( indx );
    lbox->insertItem( suppFile, indx );
    numSelected++;
  } else if ( id == UNSELECT ) {
    fname = suppFile->fileName();
    suppFile = new SuppFile( pmUnsel, fname, "[-]" );
    lbox->removeItem( indx );
    lbox->insertItem( suppFile, indx );
    numSelected--;
  } else if ( id == REMOVE ) {
    if ( suppFile->selected() )
      numSelected--;
    lbox->takeItem( item );
    indx = -1;
  } else if ( id == ADD ) {
    QString supp_fn 
      = QFileDialog::getOpenFileName( QString::null, 
                     "Supp-Files (*.supp);;All Files (*)", lbox );
    if ( supp_fn.isEmpty() ) /* user clicked cancel */
      return;
    const char *argval = supp_fn.latin1();
    int errval = 0;  /* PARSED_OK */
    supp_fn = opt->fileCheck( &errval, argval, true, false );
    if ( errval != 0 ) {
      vkError( lbox, "Invalid Entry",
              "Invalid suppressions file selected:\n"
              "\"%s\"", argval );
      return;
    } else {
      suppFile = new SuppFile( pmUnsel, supp_fn, "[-]" );
      lbox->insertItem( suppFile );
      indx = lbox->index( suppFile );
    }
  }

  if ( indx != -1 )
    lbox->setCurrentItem( indx );
  /* check if listbox contents have been changed */
  lbChanged( QString::null );
}

