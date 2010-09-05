/****************************************************************************
** HelgrindOptionsPage implementation
**  - subclass of VkOptionsPage to hold helgrind-specific options
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
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

#include "helgrind_options_page.h"

#include "objects/helgrind_object.h"
#include "options/widgets/opt_base_widget.h"
#include "utils/vk_utils.h"

#include <QGroupBox>



HelgrindOptionsPage::HelgrindOptionsPage( VkObject* obj )
   : VkOptionsPage( obj )
{
}


void HelgrindOptionsPage::setupOptions()
{
   // group1: Helgrind options
   QGroupBox* group1 = new QGroupBox( " Helgrind Options ", this );
   group1->setObjectName( QString::fromUtf8( "HelgrindOptionsPage_group1" ) );
   pageTopVLayout->addWidget( group1 );
   pageTopVLayout->addStretch( 1 );

   // no options.
   QLabel* lbl = new QLabel( "No Helgrind-specific options", group1 );
   QVBoxLayout* vbox = new QVBoxLayout( group1 );
   vbox->addSpacing( 20 );
   vbox->addWidget( lbl );

   // sanity checks
   vk_assert( m_itemList.count() <= HELGRIND::NUM_OPTS );
}
