/* ---------------------------------------------------------------------- 
 * Definition of class ToolObject                         tool_object.cpp
 *
 * Essential functionality is contained within a ToolObject.
 * Each ToolObject has an associated ToolView for displaying output.
 * 
 * See vk_object.h for information on how to add a new valgrind tool.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "tool_object.h"
#include "tool_view.h"

#include <qtimer.h>



/* class ToolObject ---------------------------------------------------- */
ToolObject::~ToolObject() 
{ 
  killProc();
}

ToolObject::ToolObject( ObjectId id, const QString& capt, 
                        const QString& txt, const QKeySequence& key )
  : VkObject( id, capt, txt, key, true )
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
