/* ---------------------------------------------------------------------- 
 * Definition of class ToolObject                         tool_object.cpp
 *
 * Essential functionality is contained within a ToolObject.
 * Each ToolObject has an associated ToolView for displaying output.
 * 
 * See vk_object.h for information on how to add a new valgrind tool.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "tool_object.h"
#include "tool_view.h"
#include "vk_config.h"

#include <qtimer.h>



/* class ToolObject ---------------------------------------------------- */
ToolObject::~ToolObject() 
{ 
  killProc();
}

ToolObject::ToolObject( const QString& capt, 
                        const QString& txt, const QKeySequence& key )
  : VkObject( capt, txt, key, true )
{
  m_view = 0;
  proc = 0;

  is_Running = false;
  fileSaved  = true;
}

bool ToolObject::isRunning() 
{ return is_Running; }


/* kill proc if it is running */
void ToolObject::killProc()
{
  if ( proc != 0 ) {
    if ( proc->isRunning() ) {
      /* if this view is closed, don't leave the process running */
      proc->tryTerminate();
      QTimer::singleShot( 5000, proc, SLOT( kill() ) );
    }
    delete proc;
    proc = 0;
  }
}

void ToolObject::deleteView()
{
  emit message( "" );  /* clear the status bar */
  vk_assert( m_view != 0 );
  // CAB: which is correct: close or delete ?
  m_view->close( true );
  //  delete memcheckView;
  m_view = 0;
}

/* called from VkConfig::modFlags() when a toolview needs to know what
   flags to set || pass to a process. */
QStringList ToolObject::modifiedFlags()
{
  QStringList modFlags;

  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {

    QString defVal = opt->defaultValue;     /* opt holds the default */
    QString cfgVal = vkConfig->rdEntry( opt->longFlag, name() );

    if ( defVal != cfgVal )
      modFlags << "--" + opt->longFlag + "=" + cfgVal;

  }

  return modFlags;
}
