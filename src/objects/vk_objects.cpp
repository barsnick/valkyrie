/****************************************************************************
** VkObject implementation
**  - abstract base class for all application 'objects'
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

#include "objects/valkyrie_object.h"
#include "objects/vk_objects.h"
#include "options/vk_option.h"
#include "utils/vk_config.h"
#include "utils/vk_utils.h"



/***************************************************************************/
/*!
    Constructs a VkObject
*/
VkObject::VkObject( QString objName )
{
   this->setObjectName( objName );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
VkObject::~VkObject()
{
}


/*!
    Return object->option based on its \a optid
*/
VkOption* VkObject::getOption( int optid )
{
   return options.getOption( optid );
}


/*!
    Return entire optionhash for this object
*/
OptionHash& VkObject::getOptions()
{
   return options.getOptionHash();
}


/*!
  Update config - general case
*/
void VkObject::updateConfig( int optid, QString& argval )
{
   vk_assert( optid >= 0 && optid < VALKYRIE::NUM_OPTS );
   
   VkOption* opt = getOption( optid );
   vk_assert( opt != NULL );
   
   opt->updateConfig( argval );
}



/*!
  Setup factory defaults for this object
*/
void VkObject::resetOptsToFactoryDefault()
{
   foreach( VkOption * opt, options.getOptionHash() ) {
      // Only update config for options that hold persistant data
      if ( opt->isaConfigOpt() ) {
         opt->updateConfig( opt->dfltValue );
      }
   }
}
