/* --------------------------------------------------------------------- 
 * Definition of class Massif                            massif_object.h
 * Massif-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __MASSIF_OBJECT_H
#define __MASSIF_OBJECT_H


#include "vk_objects.h"
#include "massif_view.h"


/* class Massif -------------------------------------------------------- */
class Massif : public ToolObject
{
public:
  Massif();
  ~Massif();

  /* returns the ToolView window (massifView) for this tool */
  ToolView* createView( QWidget* parent );
  /* called by MainWin::closeToolView() */
  bool isDone();

  void stop() { }
  bool run( QStringList /*flags*/ ) { return true; }

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum msOpts {
    FIRST_CMD_OPT = 61,
    HEAP          = FIRST_CMD_OPT, 
    HEAP_ADMIN    = 62,
    STACKS        = 63,
    DEPTH         = 64,
    ALLOC_FN      = 65,
    FORMAT        = 66,
    ALIGNMENT     = 67,
    LAST_CMD_OPT  = ALIGNMENT
  };

private:
  /* overriding to avoid casting everywhere */
  MassifView* view() { return (MassifView*)m_view; }

  void emitRunning( bool );
};


#endif
