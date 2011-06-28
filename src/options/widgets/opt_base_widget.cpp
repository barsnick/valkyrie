/****************************************************************************
** OptionWidget implementation
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "options/widgets/opt_base_widget.h"
#include "options/vk_option.h"    // for listbox fileCheck()
#include "utils/vk_config.h"
#include "utils/vk_utils.h"



/* class OptionWidget -------------------------------------------------- */
OptionWidget::OptionWidget( QWidget* parent, VkOption* vkopt, bool mklabel )
   : QObject( parent )
{
   this->setObjectName( "option_widget" );

   m_opt    = vkopt;
   m_widg   = 0;
   m_wLabel = 0;
   
   m_initialValue = vkCfgProj->value( m_opt->configKey() ).toString();
   m_currentValue = m_initialValue;
   
   if ( mklabel ) {
      m_wLabel = new QLabel( m_opt->shortHelp, parent );
      m_wLabel->setMinimumSize( m_wLabel->sizeHint() );
      m_wLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   }
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
OptionWidget::~OptionWidget()
{ }


void OptionWidget::setCurrValue( const QString& txt )
{
   if ( m_currentValue != txt ) {
      m_currentValue = txt;
      bool edited = (m_currentValue != m_initialValue);
      emit valueChanged( edited, this );

      // default behaviour is that any change is a complete change
      // - can then e.g. test new value.
      if ( edited ) {
         emit editDone( this );
      }
   }
}

int OptionWidget::id()
{
   return m_opt->optid;
}

QLabel* OptionWidget::label()
{
   return m_wLabel;
}

QWidget* OptionWidget::widget()
{
   return m_widg;
}

QString OptionWidget::currValue()
{
   return m_currentValue;
}

QString OptionWidget::initValue()
{
   return m_initialValue;
}



void OptionWidget::setValue( const QString& txt )
{
   update( txt );
}

void OptionWidget::reset()
{
   update( m_initialValue );
}

void OptionWidget::resetDefault()
{
   update( m_opt->dfltValue.toString() );
}


/*!
  virtual function to print current value prettily
   - some widgets (e.g. listbox) need to overload this.
*/
QString OptionWidget::printCurrValue()
{
   return currValue();
}

void OptionWidget::saveEdit()
{
   m_initialValue = m_currentValue;
   m_opt->updateConfig( m_currentValue );
}

void OptionWidget::cancelEdit()
{
   reset();
}

void OptionWidget::setEnabled( bool enable )
{
   if ( m_wLabel != 0 ) {
      m_wLabel->setEnabled( enable );
   }
   
   if ( m_widg != 0 ) {
      m_widg->setEnabled( enable );
   }
}


/* default horizontal layout for widgets with labels.
   combos, spinboxes and lineedits have their own way of doings things */
QHBoxLayout* OptionWidget::hlayout()
{
   vk_assert( m_wLabel != 0 );
   
   m_hBox = new QHBoxLayout();
   m_hBox->addWidget( m_wLabel );
   m_hBox->addWidget( m_widg );
   
   return m_hBox;
}


QVBoxLayout* OptionWidget::vlayout()
{
   vk_assert( m_wLabel != 0 );
   
   m_vBox = new QVBoxLayout();
   m_vBox->addWidget( m_wLabel );
   m_vBox->addWidget( m_widg );
   
   return m_vBox;
}
