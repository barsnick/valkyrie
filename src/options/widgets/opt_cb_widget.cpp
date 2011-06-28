/****************************************************************************
** CbWidget implementation
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

#include "help/help_context.h"
#include "help/help_urls.h"
#include "options/widgets/opt_cb_widget.h"
#include "options/vk_option.h"
#include "utils/vk_utils.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QWidget>
#include <QString>


/***************************************************************************/
/*!
    Constructs a CbWidget object
    has-a QComboBox
*/
CbWidget::CbWidget( QWidget* parent, VkOption* vkopt, bool mklabel )
   : OptionWidget( parent, vkopt, mklabel )
{
   this->setObjectName( "cb_widget" );

   m_currIdx = 0;
   m_combo   = new QComboBox( parent );   // true
   m_widg    = m_combo;
   
   m_combo->setInsertPolicy( QComboBox::NoInsert );
   m_combo->setAutoCompletion( true );
   m_combo->addItems( m_opt->possValues );
   m_combo->setCurrentIndex( m_currIdx );
   
   for ( int i = 0; i < m_combo->count(); i++ ) {
      if ( m_initialValue == m_combo->itemText( i ) ) {
         m_currIdx = i;
         break;
      }
   }
   
   m_combo->setCurrentIndex( m_currIdx );
   connect( m_combo, SIGNAL( activated( const QString& ) ),
            this,      SLOT( update( const QString& ) ) );
            
   // not added if the url is empty
   ContextHelp::addHelp( m_widg, m_opt->urlAddress );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
CbWidget::~CbWidget()
{
   if ( m_combo ) {
      delete m_combo;
      m_combo = 0;
   }
}



void CbWidget::update( const QString& txt )
{
   bool found = false;

   for ( int i = 0; i < m_combo->count(); ++i ) {
      if ( txt == m_combo->itemText( i ) ) {
         found = true;
         m_combo->setCurrentIndex( i );
         break;
      }
   }
   
   if ( !found ) {
      // we didn't find the string the user typed in
      m_combo->setCurrentIndex( m_currIdx );
   }
   else {
      m_currIdx = m_combo->currentIndex();
      setCurrValue( m_combo->currentText() );
   }
}


QHBoxLayout* CbWidget::hlayout()
{
   vk_assert( m_wLabel != 0 );
   
   m_hBox = new QHBoxLayout();
   m_hBox->addWidget( m_wLabel );
   m_hBox->addWidget( m_widg );
   m_hBox->setStretchFactor( m_wLabel, 6 );
   m_hBox->setStretchFactor( m_widg,   2 );
   
   return m_hBox;
}
