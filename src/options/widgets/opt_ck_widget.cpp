/****************************************************************************
** CkWidget implementation
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
#include "options/widgets/opt_ck_widget.h"
#include "options/vk_option.h"
#include "utils/vk_config.h"
#include "utils/vk_utils.h"


/***************************************************************************/
/*!
    Constructs a CkWidget object
    has-a QCheckBox
*/
CkWidget::CkWidget( QWidget* parent, VkOption* vkopt, bool mklabel )
   : OptionWidget( parent, vkopt, mklabel )
{
   this->setObjectName( "ck_widget" );

   m_cbox = new QCheckBox( m_opt->shortHelp, parent );
   m_widg = m_cbox;
   
   bool cfgChecked = strToBool( m_initialValue );
   m_cbox->setChecked( cfgChecked );
   connect( m_cbox, SIGNAL( toggled( bool ) ),
            this,     SLOT( ckChanged( bool ) ) );
            
   // not added if the url is empty
   ContextHelp::addHelp( m_widg, m_opt->urlAddress );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
CkWidget::~CkWidget()
{
   if ( m_cbox ) {
      delete m_cbox;
      m_cbox = 0;
   }
}


void CkWidget::ckChanged( bool on )
{
   setCurrValue( m_opt->possValues[ (on ? 0 : 1) ] );

   // for dis/enabling associated widgets
   emit changed( on );
}

/*!
  txt value translated to boolean to check box.
  only updates if value valid 'boolean' string
*/
void CkWidget::update( const QString& txt )
{
   bool ok;
   bool checked = strToBool( txt, &ok );
   if ( ok ) {
      m_cbox->setChecked( checked );
      // toggled signal sent -> calls ckChanged()
   }
}
