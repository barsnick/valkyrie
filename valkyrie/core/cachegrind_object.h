/* --------------------------------------------------------------------- 
 * Definition of class Cachegrind                    cachegrind_object.h
 * Cachegrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __CACHEGRIND_OBJECT_H
#define __CACHEGRIND_OBJECT_H


#include "tool_object.h"
#include "cachegrind_view.h"
#include "cachegrind_options_page.h"


/* class Cachegrind ---------------------------------------------------- */
class Cachegrind : public ToolObject
{
public:
   Cachegrind( int objId );
   ~Cachegrind();

   /* returns the ToolView window (cachegrindView) for this tool */
   ToolView* createView( QWidget* parent );
   /* called by MainWin::closeToolView() */
   bool queryDone();

   bool start( VkRunState::State rm, QStringList vgflags );
   void stop();

   /* check argval for this option, updating if necessary.
      called by parseCmdArgs() and gui option pages */
   int checkOptArg( int optid, QString& argval );

   enum cgOpts {
      I1_CACHE,
      D1_CACHE,
      L2_CACHE,
      PID_FILE,
      SHOW,
      SORT,
      THRESH,
      AUTO,
      CONTEXT,
      INCLUDE,
      NUM_OPTS
   };
   unsigned int maxOptId() { return NUM_OPTS; }

   OptionsPage* createOptionsPage( OptionsWindow* parent ) {
      return (OptionsPage*)new CachegrindOptionsPage( parent, this );
   }

private:
   /* overriding to avoid casting everywhere */
   CachegrindView* view() { return (CachegrindView*)m_view; }
   bool runValgrind( QStringList /*vgflags*/ ) { return true; }
};


#endif
