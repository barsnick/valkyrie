/* ---------------------------------------------------------------------- 
 * Definition of class VkObject                            vk_objects.cpp
 *
 * Essential functionality is contained within a VkObject.
 * 
 * To add a new valgrind tool:
 * - create the subclass in its own files in the src/core/ directory.
 *   see the Example below w.r.t. addOpt(...)
 * - in Valgrind::initToolObjects() [valgrind_object.cpp],
 *   add 'm_toolObjList.append( new tool( objId++ ) )'
 *   this registers the tool with valkyrie.
 * - create a new options page for the Options dialog, and reimplement
 *   VgObject::createOptionsPage() to create this when needed.
 * - create the ToolView subclass in its own files, in the src/tool_view dir
 * That's all, folks.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#include "vk_objects.h"
#include "vk_config.h"
#include "vk_option.h"         // PERROR* and friends 
#include "vk_utils.h"          // vk_assert, VK_DEBUG, etc.
#include "vk_messages.h"       // vkInfo() and friends
#include "html_urls.h"

#include "valkyrie_object.h"   // for configEntries()
#include "valgrind_object.h"   // modFlags()

#include <qdir.h>
#include <qfileinfo.h>

/* Example:
addOpt( 
   LEAK_CHECK,                            int opt_key
   VkOPTION::ARG_BOOL,                    VkOPTION::ArgType    arg_type
   VkOPTION::WDG_CHECK,                   VkOPTION::WidgetType w_type
   "memcheck",                            QString cfg_group    // cfgGroup()
   '\0',                                  QChar   short_flag
   "leak-check",                          QString long_flag    // cfgKey()
   "<no|summary|full>",                   QString flag_desc    // cmd-line
   "no|summary|full",                     QString poss_vals
   "summary",                             QString default_val
   "Search for memory leaks at exit",     QString shelp        // gui
   "search for memory leaks at exit?",    QString lhelp        // cmd-line
   "manual.html#leak-check" );            QString url
*/


/* class VkObject ------------------------------------------------------ */
VkObject::~VkObject() 
{ 
   m_optList.setAutoDelete( true );
   m_optList.clear();
}


VkObject::VkObject( const QString& capt, const QString& txt,
                    const QKeySequence& key, int objId ) 
   : QObject( 0, capt )
{
   m_caption   = capt;
   m_accelText = txt;
   m_accel_Key = key;
   m_objId     = objId;
   //  vkPrintErr("VkObject::VkObject( %d: %s )", objId, capt.latin1() );
}


void VkObject::addOpt( 
                      int opt_key, VkOPTION::ArgType arg_type, VkOPTION::WidgetType w_type, 
                      QString cfg_group, QChar short_flag,         QString long_flag, 
                      QString flag_desc, QString poss_vals,        QString default_val, 
                      QString shelp,     QString lhelp,            const char* url )
{
   m_optList.append( new Option( opt_key,   arg_type,   w_type, 
                                 cfg_group, short_flag, long_flag, 
                                 flag_desc, poss_vals,  default_val, 
                                 shelp,     lhelp,      url ) );
}


Option * VkObject::findOption( int optkey )
{
   vk_assert( optkey >= 0 );
   Option* opt;
   for ( opt=m_optList.first(); opt; opt=m_optList.next() ) {
      if ( opt->m_key == optkey )
         break;
   }
   vk_assert( opt != NULL );

   return opt;
}


/* Gather all config entries that hold persistent data
   - basically all options with an associated option widget.
   Called from VkConfig::mkConfigFile() when we need to create the
   valkyrierc file for the very first time.
*/
QString VkObject::configEntries()
{
   QString cfgEntry = "\n[" + name() + "]\n";
   Option* opt;
   for ( opt = m_optList.first(); opt; opt = m_optList.next() ) {
      cfgEntry += opt->m_longFlag + "=" + opt->m_defaultValue + "\n";
   }
   return cfgEntry;
}


