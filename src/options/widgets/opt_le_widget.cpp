/****************************************************************************
** LeWidget implementation
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

#include "options/widgets/opt_le_widget.h"
#include "options/vk_option.h"
#include "help/help_context.h"


/***************************************************************************/
/*!
    Constructs a LeWidget object
    has-a QLineEdit
*/
LeWidget::LeWidget( QWidget* parent, VkOption* vkopt, bool mklabel )
   : OptionWidget( parent, vkopt, mklabel )
{
   this->setObjectName( "le_widget" );

   m_pb    = 0;
   m_ledit = new QLineEdit( parent );
   m_widg  = m_ledit;
   
   m_ledit->setText( m_initialValue );
   connect( m_ledit, SIGNAL( textChanged( const QString& ) ),
            this,      SLOT( setCurrValue( const QString& ) ) );
   connect( m_ledit, SIGNAL( editingFinished() ),
            this,      SLOT( editingFinished() ) );

   // not added if the url is empty
   ContextHelp::addHelp( m_widg, m_opt->urlAddress );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
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


void LeWidget::editingFinished()
{
   emit editDone( this );
}

/*!
  overloaded from base class
  just emit valueChanged, _not_ editDone()
   - we do that on signal m_ledit->editingFinished()
*/
void LeWidget::setCurrValue( const QString& value )
{
   m_currentValue = value;
   emit valueChanged( (m_currentValue != m_initialValue), this );
}


void LeWidget::update( const QString& txt )
{
   m_ledit->setText( txt );
   // m_ledit signal -> calls setCurrValue()

   if ( txt.length() > 0 ) {
      m_ledit->setCursorPosition( 0 );
   }
}


void LeWidget::setDisabled( bool disable )
{
   m_ledit->setDisabled( disable );

   if ( m_pb != 0 ) {
      m_pb->setDisabled( disable );
   }
}

QPushButton* LeWidget::button()
{
   return m_pb;
}

void LeWidget::setReadOnly( bool ro )
{
   m_ledit->setReadOnly( ro );
}

void LeWidget::addButton( QWidget* parent, const QObject* receiver,
                          const char* slot, QString txt/*=QString::null*/,
                          bool /*icon=false*/ )
{
   QString label = !txt.isNull() ? txt : m_opt->shortHelp;
   m_pb = new QPushButton( label, parent );
   
   int pbht = m_ledit->height() - 8;
   m_pb->setMaximumHeight( pbht );
   connect( m_pb, SIGNAL( clicked() ), receiver, slot );
}


/* layout for line edits where we want to have a pushbutton with
   m_opt->shortHelp as its text, instead of the standard QLabel */
QHBoxLayout* LeWidget::hlayout()
{
   m_hBox = new QHBoxLayout();
   
   if ( m_pb != 0 ) {
      m_hBox->addWidget( m_pb );
   }
   else {
      m_hBox->addWidget( m_wLabel );
   }
   
   m_hBox->addWidget( m_widg );
   
   return m_hBox;
}
