/* --------------------------------------------------------------------- 
 * Definition of class Cachegrind                    cachegrind_object.h
 * Cachegrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __CACHEGRIND_OBJECT_H
#define __CACHEGRIND_OBJECT_H


#include "tool_object.h"
#include "cachegrind_view.h"


/* class Cachegrind ---------------------------------------------------- */
class Cachegrind : public ToolObject
{
public:
  Cachegrind();
  ~Cachegrind();

  /* returns the ToolView window (cachegrindView) for this tool */
  ToolView* createView( QWidget* parent );
  /* called by MainWin::closeToolView() */
  bool isDone();

  void stop() { }
  bool run( QStringList /*flags*/ ) { return true; }

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum cgOpts {
    FIRST_CMD_OPT = 51,
    I1_CACHE      = FIRST_CMD_OPT, 
    D1_CACHE      = 52,
    L2_CACHE      = 53,
    PID_FILE      = 54,
    SHOW          = 55,
    SORT          = 56,
    THRESH        = 57,
    AUTO          = 58,
    CONTEXT       = 59,
    INCLUDE       = 60,
    LAST_CMD_OPT = INCLUDE 
  };

private:
  /* overriding to avoid casting everywhere */
  CachegrindView* view() { return (CachegrindView*)m_view; }

  void emitRunning( bool );
};


#endif
