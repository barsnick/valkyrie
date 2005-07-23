/* ---------------------------------------------------------------------- 
 * Implementation of class OptionWidget               options_widgets.cpp
 * Various widgets used on the 'pages' to control user input
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qcursor.h>
#include <qpopupmenu.h>
#include <qfiledialog.h>

#include "options_widgets.h"
#include "vk_option.h"
#include "vk_utils.h"
#include "vk_config.h"
#include "vk_messages.h"
#include "context_help.h"
#include "vk_popt_option.h"  /* for listbox fileCheck() */



/* class OptionWidget -------------------------------------------------- */
OptionWidget::OptionWidget( QWidget* parent, const char* name,
                            Option* vkopt, bool mklabel )
  : QObject( parent, name ) 
{
  opt    = vkopt;
  widg   = 0;
  wLabel = 0;

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


/* default horizontal layout for widgets with labels.
   combos, spinboxes and lineedits have their own way of doings things */
QHBoxLayout * OptionWidget::hlayout()
{
  vk_assert( wLabel != 0 );

  hBox = new QHBoxLayout( 6, "hBox" );
  hBox->addWidget( wLabel );
  hBox->addWidget( widg );

  return hBox; 
}


QVBoxLayout* OptionWidget::vlayout()
{
  vk_assert( wLabel != 0 );
  vBox = new QVBoxLayout( 6, "vBox" );
  vBox->addWidget( wLabel );
  vBox->addWidget( widg );

  return vBox;
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

  /* not added if the url is empty */
  ContextHelp::add( widg, opt->url() );
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
  bool on = ( opt->defValue() == "1"   || opt->defValue() == "on"  || 
              opt->defValue() == "yes" || opt->defValue() == "true" );
  setOn( on );
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

  /* not added if the url is empty */
  ContextHelp::add( widg, opt->url() );
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
  bool on = ( opt->defValue() == "1"   || opt->defValue() == "on"  || 
              opt->defValue() == "yes" || opt->defValue() == "true" );
  setOn( on );
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
  connect( ledit, SIGNAL( returnPressed() ),
           this,  SIGNAL( returnPressed() ) );

  /* not added if the url is empty */
  ContextHelp::add( widg, opt->url() );
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

void LeWidget::setReadOnly( bool ro )
{ ledit->setReadOnly( ro ); }

void LeWidget::addButton( QWidget* parent, const QObject* receiver, 
                          const char* slot, QString txt/*=QString::null*/, 
                          bool /*icon=false*/ )
{
#if 1
  QString label = !txt.isNull() ? txt : opt->shortHelp;
  pb = new QPushButton( label, parent );

  int pbht = ledit->height() - 8;
  pb->setMaximumHeight( pbht );
  connect( pb, SIGNAL(clicked()), receiver, slot );
#else
  int pbht = ledit->height() - 8;
  if ( !icon ) {
    pb = new QPushButton( opt->shortHelp, parent );
  } else {
    //RM: QPixmap openIcon( vkConfig->pixmap( "fileopen.xpm" ) );
    if ( !txt.isNull() )
      pb = new QPushButton( openIcon, txt, parent );
    else
      pb = new QPushButton( openIcon, opt->shortHelp, parent );
  }
  pb->setMaximumHeight( pbht );
  connect( pb, SIGNAL(clicked()), receiver, slot );
#endif
}


/* layout for line edits where we want to have a pushbutton with
   opt->shortHelp as its text, instead of the standard QLabel */
QHBoxLayout * LeWidget::hlayout()
{
  hBox = new QHBoxLayout( 6, "le_hBox" );
  if ( pb != NULL ) {
    hBox->addWidget( pb );
  } else {
    hBox->addWidget( wLabel );
  }
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

  /* not added if the url is empty */
  ContextHelp::add( widg, opt->url() );
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

  if ( !found ) {
    /* we didn't find the string the user typed in */
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

QHBoxLayout * CbWidget::hlayout()
{ 
  vk_assert( wLabel != 0 );

  hBox = new QHBoxLayout( 6, "hBox" );
  hBox->addWidget( wLabel );
  hBox->addWidget( widg );
  hBox->setStretchFactor( wLabel, 6 );
  hBox->setStretchFactor( widg,   2 );

  return hBox; 
}



/* class SpWidget: IntSpin --------------------------------------------- */
SpWidget::~SpWidget()
{
  if ( intspin ) {
    delete intspin;
    intspin = NULL;
  }
}

SpWidget::SpWidget( QWidget* parent, Option* vkopt, 
                    bool mklabel, int num_sections )
  : OptionWidget( parent, "sp_widget", vkopt, mklabel ) 
{
  intspin = new IntSpin( parent, "int_spin" );
  widg = intspin;

  numSections = num_sections;
  connect( intspin, SIGNAL(valueChanged(const QString&)), 
           this,    SLOT(spChanged(const QString&)) );

  /* not added if the url is empty */
  ContextHelp::add( widg, opt->url() );
}

void SpWidget::addSection( int min, int max, int defval,
                           int step, QString sep_char/*=" : "*/ )
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

QHBoxLayout * SpWidget::hlayout()
{ 
  vk_assert( wLabel != 0 );

  hBox = new QHBoxLayout( 6 );
  hBox->addWidget( wLabel );
  hBox->addWidget( widg );
  hBox->setStretchFactor( wLabel, 10 );
  hBox->setStretchFactor( widg,    1 );

  return hBox; 
}



/* class LbWidget: QListBox -------------------------------------------- 
   This widget was specifically written to handle suppression files
   stuff and nothing else. */

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
  lbox = new QListBox( parent, "list_box" );
  widg = lbox;
  lbox->setSelectionMode( QListBox::Single );

  sep  = vkConfig->sepChar();
  mode = ( opt->cfgKey() == "supps-all" ) ? AllSupps : SelSupps;

  load();
  connect( lbox, SIGNAL(contextMenuRequested(QListBoxItem*, 
                                             const QPoint &)),
           this, SLOT(popupMenu(QListBoxItem*, const QPoint &)));

  /* not added if the url is empty */
  ContextHelp::add( widg, opt->url() );
}


/* split values at the sep-char, and load all files into the listbox */
void LbWidget::load()
{
  QStringList sfiles = QStringList::split( sep, currentValue );
  for ( unsigned int i=0; i<sfiles.count(); i++ ) {
    lbox->insertItem( sfiles[i] );
  }
}


void LbWidget::reset()
{
  lbox->clear();
  load();
}

void LbWidget::resetDefault()
{
  lbox->clear();
  switch( mode ) {
    case AllSupps:
      currentValue = vkConfig->rdEntry( "supps-def", "valgrind" );
      break;
    case SelSupps:
      currentValue = opt->defValue();
      break;
    default:
      vk_assert_never_reached();
      break;
  }

  load();
}


/* compare the listbox item strings with initialValue strings */
void LbWidget::lbChanged( const QString& )
{
  currentValue = "";
  for ( unsigned int i=0; i<lbox->count(); i++ ) {
    currentValue += lbox->item(i)->text() + sep;
  }
  /* remove trailing ',' */
  currentValue.remove( currentValue.length()-1, 1 );

  bool edited = currentValue != initialValue;
  emit valueChanged( edited, this );
}

/* this slot should only be called when the lbox is in SelSupps mode */
void LbWidget::insertFile( const QString& fname ) 
{
  vk_assert( mode == SelSupps );
  /* check this file isn't already in the lbox */
  if ( 0 == lbox->findItem( fname, Qt::ExactMatch ) ) {
    lbox->insertItem( fname );
    lbChanged( QString::null );
  } else {
    vkInfo( lbox, "Duplicate File",
            "<p>The file '%s' is already in the list.</p>", fname.latin1() );
  }
}


/* different menus and stuff for the different modes */
void LbWidget::popupMenu( QListBoxItem* lb_item, const QPoint& )
{
  switch( mode ) {
    case AllSupps: popupAll( lb_item ); break;
    case SelSupps: popupSel( lb_item ); break;
    default: vk_assert_never_reached(); break;
  }
}

void LbWidget::popupSel( QListBoxItem* lb_item )
{
  if ( !lb_item )  /* lbox is empty, so nothing to do */
    return;

  QPopupMenu popMenu( lbox );
  int DESELECT = popMenu.insertItem( "Deselect File" );
  if ( !(lb_item->isSelected() && lb_item->isCurrent()) )
    popMenu.setItemEnabled( DESELECT, false );

  popMenu.setMouseTracking( true );
  int id = popMenu.exec( QCursor::pos() );

  if ( id == DESELECT ) {
    int indx = lbox->index( lb_item );
    lbox->removeItem( indx );
    lbChanged( QString::null );
  }

}


void LbWidget::popupAll( QListBoxItem* lb_item )
{
  enum { SELECT=10, REMOVE=11, ADD=12 };

  QPopupMenu popMenu( lbox );
  popMenu.insertItem( QPixmap(sel_supp_xpm), "Select File", SELECT );
  popMenu.insertSeparator();
  popMenu.insertItem( "Remove File", REMOVE );
  popMenu.insertItem( "Add File(s)", ADD );

  if ( !lb_item ) {   /* lbox is empty */
    popMenu.setItemEnabled( SELECT, false );
    popMenu.setItemEnabled( REMOVE, false );
  } else if ( !lb_item->isSelected() ) {
    popMenu.setItemEnabled( SELECT, false );
  }

  bool changed = false;
  popMenu.setMouseTracking( true );
  int id = popMenu.exec( QCursor::pos() );

  switch( id ) {

    case SELECT: {
      QString file_name = lb_item->text();
      emit fileSelected( lb_item->text() );
    } break;

    case REMOVE: {
      int indx = lbox->index( lb_item );
      lbox->removeItem( indx );
      changed = true;
    } break;

    case ADD: {
      QStringList supp_files = QFileDialog::getOpenFileNames( 
                 "Suppression Files (*.supp);;All Files (*)", 
                 "/home", lbox, "supp_dlg", "Select one or more files" );
      /* if user clicked cancel */
      if ( supp_files.isEmpty() )  return;

      for ( unsigned int i=0; i<supp_files.count(); i++ ) {

        /* check this file isn't already in the lbox */
        if ( 0 != lbox->findItem( supp_files[i], Qt::ExactMatch ) ) {
          vkInfo( lbox, "Duplicate File",
                  "<p>The file '%s' is already in the list.</p>", 
                  supp_files[i].latin1() );
          continue;
        }

        int errval = PARSED_OK;
        const char* argval = supp_files[i].latin1();
        supp_files[i] = fileCheck( &errval, argval, true, false );
        if ( errval == PARSED_OK ) {
          lbox->insertItem( supp_files[i] );
          changed = true;
        } else {
          vkError( lbox, "Invalid Entry",
                   "Invalid suppression file:\n \"%s\"", argval );
        }
      }
    } break;

    default:
      break;
  }

  if ( changed ) {
    lbChanged( QString::null );
  }

}


