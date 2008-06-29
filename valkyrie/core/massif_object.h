/* --------------------------------------------------------------------- 
 * Definition of class Massif                            massif_object.h
 * Massif-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
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
   Massif( int objId );
   ~Massif();

   /* returns the ToolView window (massifView) for this tool */
   ToolView* createView( QWidget* parent );
   /* called by MainWin::closeToolView() */
   bool queryDone();

   bool start( VkRunState::State rm, QStringList vgflags );
   void stop();

   /* check argval for this option, updating if necessary.
      called by parseCmdArgs() and gui option pages */
   int checkOptArg( int optid, QString& argval );

   enum msOpts {
      HEAP,
      HEAP_ADMIN,
      STACKS,
      DEPTH,
      ALLOC_FN,
      FORMAT,
      ALIGNMENT,
      NUM_OPTS
   };
   unsigned int maxOptId() { return NUM_OPTS; }

   OptionsPage* createOptionsPage( OptionsWindow* parent ) {
      return (OptionsPage*)new MassifOptionsPage( parent, this );
   }

private:
   /* overriding to avoid casting everywhere */
   MassifView* view() { return (MassifView*)m_view; }

   bool runValgrind( QStringList /*vgflags*/ ) { return true; }
};


#endif
