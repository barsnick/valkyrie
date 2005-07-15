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


#include "tool_object.h"
#include "massif_view.h"
#include "massif_options_page.h"


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

  bool start( Valkyrie::RunMode rm );
  bool stop( Valkyrie::RunMode rm );
  bool run( QStringList /*flags*/ ) { return true; }

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum msOpts {
    HEAP,     HEAP_ADMIN, STACKS,    DEPTH,
    ALLOC_FN, FORMAT,     ALIGNMENT,
    LAST_CMD_OPT  = ALIGNMENT
  };

  bool optionUsesPwr2( int optId ) {
    if (optId == ALIGNMENT) return true;
    return false;
  }

  OptionsPage* createOptionsPage( OptionsWindow* parent ) {
    return (OptionsPage*)new MassifOptionsPage( parent, this );
  }

private:
  /* overriding to avoid casting everywhere */
  MassifView* view() { return (MassifView*)m_view; }

  void emitRunning( bool );
};


#endif
