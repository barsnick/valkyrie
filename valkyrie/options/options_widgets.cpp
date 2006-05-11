/* ---------------------------------------------------------------------- 
 * Implementation of class OptionWidget               options_widgets.cpp
 * Various widgets used on the 'pages' to control user input
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
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
#include "vk_option.h"       /* for listbox fileCheck() */



/* class OptionWidget -------------------------------------------------- */
OptionWidget::OptionWidget( QWidget* parent, const char* name,
                            Option* vkopt, bool mklabel )
   : QObject( parent, name ) 
{
   m_opt    = vkopt;
   m_widg   = 0;
   m_wLabel = 0;

   m_initialValue = vkConfig->rdEntry( m_opt->cfgKey(),
                                       m_opt->cfgGroup() );
   m_currentValue = m_initialValue;

   if ( mklabel ) {
      m_wLabel = new QLabel( m_opt->m_shortHelp, parent );
      m_wLabel->setMinimumSize( m_wLabel->sizeHint() );
      m_wLabel->setAlignment( AlignLeft|AlignVCenter|ExpandTabs );
   }
}

int OptionWidget::id()
{ return m_opt->m_key; }

QLabel * OptionWidget::label()
{ return m_wLabel; }

QWidget * OptionWidget::widget()
{ return m_widg; }

QString OptionWidget::currValue()
{ return m_currentValue; }

QString OptionWidget::initValue()
{ return m_initialValue; }

void OptionWidget::saveEdit()
{
   m_initialValue = m_currentValue;
   vkConfig->wrEntry( m_currentValue, m_opt->cfgKey(), m_opt->cfgGroup() );
}

void OptionWidget::cancelEdit()
{
   m_currentValue = m_initialValue;
   emit valueChanged( false, this );
   reset();
}

void OptionWidget::setEnabled( bool enable )
{
	if ( m_wLabel != 0 )
		m_wLabel->setEnabled( enable );
	if ( m_widg != 0 ) {
		m_widg->setEnabled( enable );
	}
}


/* default horizontal layout for widgets with labels.
   combos, spinboxes and lineedits have their own way of doings things */
QHBoxLayout * OptionWidget::hlayout()
{
   vk_assert( m_wLabel != 0 );

   m_hBox = new QHBoxLayout( 6, "hBox" );
   m_hBox->addWidget( m_wLabel );
   m_hBox->addWidget( m_widg );

   return m_hBox; 
}


QVBoxLayout* OptionWidget::vlayout()
{
   vk_assert( m_wLabel != 0 );
   m_vBox = new QVBoxLayout( 6, "vBox" );
   m_vBox->addWidget( m_wLabel );
   m_vBox->addWidget( m_widg );

   return m_vBox;
}




/* class CkWidget: QCheckBox ------------------------------------------- */
CkWidget::~CkWidget() 
{
   if ( m_cbox ) {
      delete m_cbox;
      m_cbox = 0;
   }
}

CkWidget::CkWidget( QWidget* parent, Option* vkopt, bool mklabel )
   : OptionWidget( parent, "ck_widget", vkopt, mklabel ) 
{ 
   m_cbox = new QCheckBox( m_opt->m_shortHelp, parent, "check_box" );
   m_widg = m_cbox;

   m_cbox->setChecked( vkConfig->strToBool( m_initialValue ) );
   connect( m_cbox, SIGNAL(toggled(bool)), 
            this,     SLOT(ckChanged(bool)) );

   /* not added if the url is empty */
   ContextHelp::add( m_widg, m_opt->url() );
}

void CkWidget::ckChanged( bool on )
{
   m_currentValue = (on) ? m_opt->m_possValues[0] : m_opt->m_possValues[1];
   bool edited = m_currentValue != m_initialValue;
   emit valueChanged( edited, this );
   /* for dis/enabling associated widgets */
   emit changed( on );
   emit clicked( m_opt->m_key );
}

void CkWidget::reset()
{ m_cbox->setChecked( vkConfig->strToBool( m_initialValue ) ); }

void CkWidget::resetDefault()
{
   bool on = ( m_opt->defValue() == "1"   || m_opt->defValue() == "on"  || 
               m_opt->defValue() == "yes" || m_opt->defValue() == "true" );
   setOn( on );
}

bool CkWidget::isOn()
{ return m_cbox->isChecked(); }

void CkWidget::setOn( bool on )
{ m_cbox->setChecked( on ); }



/* class RbWidget: QRadioButton ---------------------------------------- */
RbWidget::~RbWidget() 
{
   if ( m_radio ) {
      delete m_radio;
      m_radio = 0;
   }
}

RbWidget::RbWidget( QWidget* parent, Option* vkopt, bool mklabel )
   : OptionWidget( parent, "rb_widget", vkopt, mklabel ) 
{ 
   m_radio = new QRadioButton( m_opt->m_shortHelp, parent, "radio_button" );
   m_widg  = m_radio;

   m_radio->setChecked( vkConfig->strToBool( m_initialValue ) );
   connect( m_radio, SIGNAL(toggled(bool)), 
            this, SLOT(rbChanged(bool)) );

   /* not added if the url is empty */
   ContextHelp::add( m_widg, m_opt->url() );
}

void RbWidget::rbChanged( bool on )
{
   m_currentValue = (on) ? m_opt->m_possValues[0] : m_opt->m_possValues[1];
   bool edited = m_currentValue != m_initialValue;
   emit valueChanged( edited, this );
   /* for dis/enabling associated widgets */
   emit changed( on );
   emit clicked( m_opt->m_key );
}

void RbWidget::reset()
{ m_radio->setChecked( vkConfig->strToBool( m_initialValue ) ); }

void RbWidget::resetDefault()
{
   bool on = ( m_opt->defValue() == "1"   || m_opt->defValue() == "on"  || 
               m_opt->defValue() == "yes" || m_opt->defValue() == "true" );
   setOn( on );
}

bool RbWidget::isOn()
{ return m_radio->isOn(); }

void RbWidget::setOn( bool on )
{ m_radio->setChecked( on ); }



/* class LeWidget: QLineEdit ------------------------------------------- */
LeWidget::~LeWidget()
{
   if ( m_ledit ) {
      delete m_ledit;
      m_ledit = 0;
   }
   if ( m_pb ) {
      delete m_pb;
      m_pb = 0;
   }
}

LeWidget::LeWidget( QWidget *parent, Option * vkopt, bool mklabel )
   : OptionWidget( parent, "le_widget", vkopt, mklabel ) 
{
   m_pb    = 0;
   m_ledit = new QLineEdit( parent, "line_edit" ); 
   m_widg  = m_ledit;

   m_ledit->setText( m_initialValue );
   connect( m_ledit, SIGNAL( textChanged(const QString &) ),
            this,      SLOT( leChanged(const QString &) ) );
   connect( m_ledit, SIGNAL( returnPressed() ),
            this,    SIGNAL( returnPressed() ) );

   /* not added if the url is empty */
   ContextHelp::add( m_widg, m_opt->url() );
}

void LeWidget::setCurrValue( const QString& txt ) 
{ 
   m_ledit->setText( txt );  /* calls leChanged(txt) */
   if ( txt.length() > 0 )
      m_ledit->setCursorPosition( 0 );
}

void LeWidget::addCurrValue( const QString& txt )
{ 
   if ( m_currentValue.isEmpty() ) {
      setCurrValue( txt );
   } else {
      setCurrValue( m_ledit->text() + "," + txt ); 
   }
}

void LeWidget::leChanged( const QString& txt )
{
   m_currentValue = txt;
   bool edited  = m_currentValue != m_initialValue;
   emit valueChanged( edited, this );
}

void LeWidget::reset()
{ m_ledit->setText( m_initialValue ); }

void LeWidget::resetDefault()
{ setCurrValue( m_opt->defValue() ); }

QPushButton * LeWidget::button()
{ return m_pb; }

void LeWidget::setReadOnly( bool ro )
{ m_ledit->setReadOnly( ro ); }

void LeWidget::addButton( QWidget* parent, const QObject* receiver, 
                          const char* slot, QString txt/*=QString::null*/, 
                          bool /*icon=false*/ )
{
   QString label = !txt.isNull() ? txt : m_opt->m_shortHelp;
   m_pb = new QPushButton( label, parent );

   int pbht = m_ledit->height() - 8;
   m_pb->setMaximumHeight( pbht );
   connect( m_pb, SIGNAL(clicked()), receiver, slot );
}


/* layout for line edits where we want to have a pushbutton with
   m_opt->shortHelp as its text, instead of the standard QLabel */
QHBoxLayout * LeWidget::hlayout()
{
   m_hBox = new QHBoxLayout( 6, "le_hBox" );
   if ( m_pb != 0 ) {
      m_hBox->addWidget( m_pb );
   } else {
      m_hBox->addWidget( m_wLabel );
   }
   m_hBox->addWidget( m_widg );

   return m_hBox; 
}


/* class CbWidget: QComboBox ------------------------------------------- */
CbWidget::~CbWidget()
{
   if ( m_combo ) {
      delete m_combo;
      m_combo = 0;
   }
}

CbWidget::CbWidget( QWidget *parent, Option * vkopt, bool mklabel )
   : OptionWidget( parent, "cb_widget", vkopt, mklabel ) 
{
   m_currIdx = 0;
   m_combo   = new QComboBox( true, parent, "combo_box" );
   m_widg    = m_combo;

   m_combo->setInsertionPolicy( QComboBox::NoInsertion );
   m_combo->setAutoCompletion( true );
   m_combo->insertStringList( m_opt->m_possValues );
   m_combo->setCurrentItem( m_currIdx );

   for ( int i=0; i<m_combo->count(); i++ ) {
      if ( m_initialValue == m_combo->text(i) ) {
         m_currIdx = i;
         break;
      }
   }

   m_combo->setCurrentItem( m_currIdx );
   connect( m_combo, SIGNAL( activated(const QString &) ),
            this,      SLOT( cbChanged(const QString &) ) );

   /* not added if the url is empty */
   ContextHelp::add( m_widg, m_opt->url() );
}

void CbWidget::cbChanged( const QString& txt )
{
   bool found = false;
   for ( int i=0; i<m_combo->count(); ++i ) {
      if ( txt == m_combo->text(i) ) {
         found = true;
         m_combo->setCurrentItem( i );
         break;
      }
   }

   if ( !found ) {
      /* we didn't find the string the user typed in */
      m_combo->setCurrentItem( m_currIdx );
   } else {
      m_currIdx = m_combo->currentItem();
      m_currentValue = m_combo->currentText();
      bool edited = m_currentValue != m_initialValue;
      emit valueChanged( edited, this );
   }
}

void CbWidget::reset()
{ cbChanged( m_initialValue ); }

void CbWidget::resetDefault()
{ cbChanged( m_opt->defValue() ); }

QHBoxLayout * CbWidget::hlayout()
{ 
   vk_assert( m_wLabel != 0 );

   m_hBox = new QHBoxLayout( 6, "hBox" );
   m_hBox->addWidget( m_wLabel );
   m_hBox->addWidget( m_widg );
   m_hBox->setStretchFactor( m_wLabel, 6 );
   m_hBox->setStretchFactor( m_widg,   2 );

   return m_hBox; 
}



/* class SpWidget: IntSpin --------------------------------------------- */
SpWidget::~SpWidget()
{
   if ( m_intspin ) {
      delete m_intspin;
      m_intspin = 0;
   }
}

SpWidget::SpWidget( QWidget* parent, Option* vkopt, 
                    bool mklabel, int num_sections )
   : OptionWidget( parent, "sp_widget", vkopt, mklabel ) 
{
   m_intspin = new IntSpin( parent, "int_spin" );
   m_widg    = m_intspin;

   m_numSections = num_sections;
   connect( m_intspin, SIGNAL(valueChanged(const QString&)), 
            this,    SLOT(spChanged(const QString&)) );

   /* not added if the url is empty */
   ContextHelp::add( m_widg, m_opt->url() );
}

void SpWidget::addSection( int min, int max, int defval,
                           int step, QString sep_char/*=" : "*/ )
{ m_intspin->addSection( min, max, defval, step, sep_char ); }

void SpWidget::spChanged( const QString &val )
{
   m_currentValue = val;
   bool edited  = m_currentValue != m_initialValue;
   emit valueChanged( edited, this );
}

void SpWidget::reset()
{
   if ( m_numSections == 1 ) {
      m_intspin->setValue( m_initialValue.toInt(), 0 );
   }  else {
      QStringList values = QStringList::split( ",", m_initialValue );
      for ( unsigned int i=0; i<values.count(); i++ ) {
         m_intspin->setValue( values[i].toInt(), i );
      }
   }
}

void SpWidget::resetDefault()
{ 
   spChanged( m_opt->defValue() ); 
   if ( m_numSections == 1 ) {
      m_intspin->setValue( m_opt->defValue().toInt(), 0 );
   } else {
      QStringList values = QStringList::split( ",", m_opt->defValue() );
      for ( unsigned int i=0; i<values.count(); i++ ) {
         m_intspin->setValue( values[i].toInt(), i );
      }
   }
}

QHBoxLayout * SpWidget::hlayout()
{ 
   vk_assert( m_wLabel != 0 );

   m_hBox = new QHBoxLayout( 6 );
   m_hBox->addWidget( m_wLabel );
   m_hBox->addWidget( m_widg );
   m_hBox->setStretchFactor( m_wLabel, 10 );
   m_hBox->setStretchFactor( m_widg,    1 );

   return m_hBox; 
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
   if ( m_lbox ) {
      delete m_lbox;
      m_lbox = 0;
   }
}

LbWidget::LbWidget( QWidget *parent, Option * vkopt, bool mklabel )
   : OptionWidget( parent, "lb_widget", vkopt, mklabel ) 
{
   m_lbox = new QListBox( parent, "list_box" );
   m_widg = m_lbox;
   m_lbox->setSelectionMode( QListBox::Single );

   m_sep  = vkConfig->sepChar();

   /* horrible hack alert! */
   vk_assert( m_opt->cfgGroup() == "valgrind");
   if (m_opt->cfgKey() == "supps-dirs")
      m_mode = LbWidget::LB_SUPPDIRS;
   else if (m_opt->cfgKey() == "supps-avail")
      m_mode = LbWidget::LB_SUPPAVAIL;
   else if (m_opt->cfgKey() == "suppressions")
      m_mode = LbWidget::LB_SUPPSEL;
   else
      vk_assert_never_reached();

   lbLoad();
   connect( m_lbox, SIGNAL(contextMenuRequested(QListBoxItem*, const QPoint &)),
            this,     SLOT(popupMenu(QListBoxItem*, const QPoint &)));
   connect( m_lbox, SIGNAL(doubleClicked(QListBoxItem*)),
            this,     SLOT(selectItem(QListBoxItem*)));

   /* not added if the url is empty */
   ContextHelp::add( m_widg, m_opt->url() );
}


/* load items from m_currentValue to listbox */
void LbWidget::lbLoad()
{
   m_lbox->clear();
   QStringList sfiles = QStringList::split( m_sep, m_currentValue );
   for ( unsigned int i=0; i<sfiles.count(); i++ ) {
      m_lbox->insertItem( sfiles[i] );
   }
}


/* reset lbox to m_currentValue
   called from OptionWidget::cancelEdit(), after m_currentValue reset.
 */
void LbWidget::reset()
{
   lbLoad();
   emit listChanged();
}

/* called from OptionsPage::resetDefaults()
   reset to installation defaults
 */
void LbWidget::resetDefault()
{
   m_currentValue = m_opt->defValue();
   lbLoad();
   lbChanged();
}


void LbWidget::setCurrValue( const QString& txt ) 
{ 
   m_currentValue = txt;
   lbLoad();
   lbChanged();
}

/* return all contents concat'd with m_sep  */
QString LbWidget::lbText()
{
   QStringList items;
   for ( unsigned int i=0; i<m_lbox->count(); i++ )
      items += m_lbox->text( i );
   return (items.count() == 0) ? "" : items.join( m_sep );
}


/* emit signals to indicate widget content has changed */
void LbWidget::lbChanged()
{
   switch( m_mode ) {
   case LbWidget::LB_SUPPAVAIL:
      /* Never test this widget's editedness - holds a dynamic list */
      break;

   case LbWidget::LB_SUPPDIRS:
   case LbWidget::LB_SUPPSEL: {
      bool edited = m_currentValue != m_initialValue;
      emit valueChanged( edited, this );
      emit listChanged();
   } break;

   default:
      vk_assert_never_reached();
      break;
   }
}


/* insert new list item.
   only valid mode is LB_SUPPSEL
*/
void LbWidget::insertItem( const QString& entry )
{
   switch( m_mode ) {
   case LbWidget::LB_SUPPSEL:
      m_lbox->insertItem( entry );
      m_currentValue = lbText();
      lbChanged();
      break;

   case LbWidget::LB_SUPPDIRS:
   case LbWidget::LB_SUPPAVAIL:  
   default: vk_assert_never_reached(); break;
   }
}


/* double clicked item, or via right-click menu
   for a single-entry menu, do the only thing possible
   else do nothing
*/
void LbWidget::selectItem( QListBoxItem* lb_item)
{
   if ( !lb_item )
      return;

   switch( m_mode ) {
   case LbWidget::LB_SUPPDIRS:
      break;

   case LbWidget::LB_SUPPSEL:
      emit itemSelected( lb_item->text() );
      m_lbox->removeItem( m_lbox->index( lb_item ) );
      m_currentValue = lbText();
      lbChanged();
      break;

   case LbWidget::LB_SUPPAVAIL:
      emit itemSelected( lb_item->text() );
      /* Not removing item - list recalculated in valgrind opts page */
      break;

   default: vk_assert_never_reached(); break;
   }
}


/* different menus and stuff for the different modes */
void LbWidget::popupMenu( QListBoxItem* lb_item, const QPoint& )
{
   switch( m_mode ) {
   case LbWidget::LB_SUPPDIRS:  popupSuppDirs( lb_item );  break;
   case LbWidget::LB_SUPPAVAIL: popupSuppAvail( lb_item ); break;
   case LbWidget::LB_SUPPSEL:   popupSuppSel( lb_item );   break;
   default: vk_assert_never_reached(); break;
   }
}


void LbWidget::popupSuppSel( QListBoxItem* lb_item )
{
   vk_assert( m_mode == LbWidget::LB_SUPPSEL );

   if ( !lb_item )  /* m_lbox is empty, so nothing to do */
      return;

   QPopupMenu popMenu( m_lbox );
   int DESELECT = popMenu.insertItem( "Deselect File" );
   if ( !(lb_item->isSelected() || lb_item->isCurrent()) )
      popMenu.setItemEnabled( DESELECT, false );

   popMenu.setMouseTracking( true );
   int id = popMenu.exec( QCursor::pos() );

   if ( id == DESELECT )
      selectItem( lb_item );
}


void LbWidget::popupSuppAvail( QListBoxItem* lb_item )
{
   vk_assert( m_mode == LbWidget::LB_SUPPAVAIL );

   QPopupMenu popMenu( m_lbox );
   int SELECT = popMenu.insertItem( QPixmap(sel_supp_xpm), "Select File" );
   if ( !lb_item || !(lb_item->isSelected() || lb_item->isCurrent()) )
      popMenu.setItemEnabled( SELECT, false );


   popMenu.setMouseTracking( true );
   int id = popMenu.exec( QCursor::pos() );

   if ( id == SELECT )
      selectItem( lb_item );
}


void LbWidget::popupSuppDirs( QListBoxItem* lb_item )
{
   vk_assert( m_mode == LbWidget::LB_SUPPDIRS );

   enum { DIR_RM, DIR_ADD };

   QPopupMenu popMenu( m_lbox );
   popMenu.insertItem( "Remove Dir", DIR_RM );
   popMenu.insertItem( "Add Dir", DIR_ADD );

   if ( !lb_item/* empty */ )
      popMenu.setItemEnabled( DIR_RM, false );

   bool changed = false;
   popMenu.setMouseTracking( true );
   int id = popMenu.exec( QCursor::pos() );

   switch( id ) {
   case DIR_RM:
      m_lbox->removeItem(  m_lbox->index( lb_item ) );
      changed = true;
      break;

   case DIR_ADD: {
      QString startdir = m_currentValue.isEmpty()
         ? QDir::currentDirPath()
         : QStringList::split( m_sep, m_currentValue ).first();

      QString supp_dir = QFileDialog::getExistingDirectory(
                             startdir,
                             m_lbox, "get_suppression_dir",
                             "Choose Suppressions Directory", 
                             false/*=show files too*/ );

      if ( supp_dir.isEmpty() ) /* user clicked Cancel ? */
         break;

      /* check not a duplicate entry */
      if ( m_lbox->findItem( supp_dir, Qt::ExactMatch ) != 0 ) {
         vkInfo( m_lbox, "Duplicate Entry",
                 "<p>The entry '%s' is already in the list.</p>",
                 supp_dir.latin1() );
      } else {
         m_lbox->insertItem( supp_dir );
         changed = true;
      }
   } break;

   default:
      break;
   }

   if ( changed ) {
      m_currentValue = lbText();
      lbChanged();
   }
}
