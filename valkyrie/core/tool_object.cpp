/* ---------------------------------------------------------------------
 * Definition of class ToolObject                        tool_object.cpp
 *
 * Essential functionality is contained within a ToolObject.
 * Each ToolObject has an associated ToolView for displaying output.
 * 
 * See vk_objects.cpp for information on how to add a new valgrind tool.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#include "tool_object.h"
#include "tool_view.h"
#include "vk_config.h"         // vkConfig
#include "vk_utils.h"          // vk_assert, VK_DEBUG, etc.

#include <qtimer.h>



/* class ToolObject ---------------------------------------------------- */
ToolObject::~ToolObject() 
{}

ToolObject::ToolObject( const QString& capt, const QString& txt,
                        const QKeySequence& key, int objId )
   : VkObject( capt, txt, key, objId )
{
   m_view      = 0;
   m_fileSaved = true;
   m_runState  = VkRunState::STOPPED;
}

bool ToolObject::isRunning() 
{
   return (m_runState != VkRunState::STOPPED);
}


void ToolObject::setRunState( VkRunState::State rs )
{
   m_runState = rs;
   emit running( isRunning() );
}


void ToolObject::deleteView()
{
   emit message( "" );  /* clear the status bar */
   vk_assert( m_view != 0 );

   m_view->close( true/*alsoDelete*/ );
   m_view = 0;
}

ToolView* ToolObject::view()
{
   return m_view;
}


/* called from Valkyrie::updateVgFlags() whenever flags have been changed */
QStringList ToolObject::modifiedVgFlags()
{
   QStringList modFlags;

   for ( Option* opt = m_optList.first(); opt; opt = m_optList.next() ) {

      QString defVal = opt->m_defaultValue;     /* opt holds the default */
      QString cfgVal = vkConfig->rdEntry( opt->m_longFlag, name() );

      if ( defVal != cfgVal )
         modFlags << "--" + opt->m_longFlag + "=" + cfgVal;
   }
   return modFlags;
}
