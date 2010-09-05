/****************************************************************************
** SpWidget implementation
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
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

#include <cmath>
#include "help/help_context.h"
#include "help/help_urls.h"
#include "options/widgets/opt_sp_widget.h"
#include "options/vk_option.h"
#include "utils/vk_utils.h"


/***************************************************************************/
/*!
    Constructs a SpWidget object
    has-a IntSpin (:QSpinBox)
*/
SpWidget::SpWidget( QWidget* parent, VkOption* vkopt,
                    bool mklabel, int num_sections )
   : OptionWidget( parent, vkopt, mklabel )
{
   this->setObjectName( "sp_widget" );

   m_intspin = new IntSpin( parent );
   m_widg    = m_intspin;
   
   m_numSections = num_sections;
   connect( m_intspin, SIGNAL( valueChanged( const QString& ) ),
            this,        SLOT( setCurrValue( const QString& ) ) );

   // not added if the url is empty
   ContextHelp::addHelp( m_widg, m_opt->urlAddress );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
SpWidget::~SpWidget()
{
   if ( m_intspin ) {
      delete m_intspin;
      m_intspin = 0;
   }
}



#define MAX_INT 0x7ffffff

void SpWidget::addSection( int min, int max, int defval,
                           int step, QString sep_char/*=" : "*/ )
{
   if ( step == 0 ) { // -> steps in pow2
      if ( min < 1 ) {
         cerr << "FIXME! Bad MIN value(<1) for SpinWidget" << endl;
         min = 1;
      }
      else {
         int i;
         
         for ( i = 1; i <= MAX_INT; i *= 2 ) {
            if ( i == min ) {
               break;
            }
            
            if ( i > min ) {
               cerr << "FIXME! Bad MIN value(not pow2) for SpinWidget" << endl;
               min = i / 2;
               break;
            }
         }
         
         if ( min > i ) {
            min = i;   // just in case
         }
      }
      
      if ( max < 1 ) {
         cerr << "FIXME! Bad MAX value(<1) for SpinWidget" << endl;
         max = 1;
      }
      else {
         int i;
         
         for ( i = 1; i <= MAX_INT; i *= 2 ) {
            if ( i == max ) {
               break;
            }
            
            if ( i > max ) {
               cerr << "FIXME! Bad MAX value(not pow2) for SpinWidget" << endl;
               max = i / 2;
               break;
            }
         }
         
         if ( max > i ) {
            max = i;   // just in case
         }
      }
   }
   
   m_intspin->addSection( min, max, defval, step, sep_char );
}


void SpWidget::update( const QString& txt )
{
   if ( m_numSections == 1 ) {
      //TODO: multisections
      //      m_intspin->setValue( m_initialValue.toInt(), 0 );
      m_intspin->setValue( txt.toInt() );
   }
   else {
      QStringList values = txt.split( ",", QString::SkipEmptyParts );

      for ( int i = 0; i < values.count(); i++ ) {
         //TODO: multisections
         //         m_intspin->setValue( values[i].toInt(), i );
         m_intspin->setValue( values[i].toInt() );
      }
   }
}


QHBoxLayout* SpWidget::hlayout()
{
   vk_assert( m_wLabel != 0 );
   
   //   m_hBox = new QHBoxLayout( 6 );
   m_hBox = new QHBoxLayout();
   m_hBox->addWidget( m_wLabel );
   m_hBox->addWidget( m_widg );
   m_hBox->setStretchFactor( m_wLabel, 10 );
   m_hBox->setStretchFactor( m_widg,    1 );
   
   return m_hBox;
}






IntSpin::IntSpin( QWidget* parent )
   : QSpinBox( parent )
{
   use_pwr2 = false;
}


void IntSpin::addSection( int min, int max, int defval, int step, QString ) //sep_char )
{
   if ( step == 0 ) {
      step = 1;
      use_pwr2 = true;
   }
   
   this->setRange( min, max );
   this->setSingleStep( step );
   this->setValue( defval );
}


void IntSpin::stepBy( int step )
{
   if ( !use_pwr2 ) {
      return QSpinBox::stepBy( step );
   }
   
   int lastVal = this->value();
   vk_assert( lastVal >= 1 );
   
   // nextVal is power2 of lastVal
   int nextVal = ( double )lastVal * pow( 2.0, ( double )step );
   
   // make the step (- or +) to nextVal
   QSpinBox::stepBy( nextVal - lastVal );
}


