/* --------------------------------------------------------------------- 
 * Definition of class Valkyrie                        valkyrie_object.h
 * Valkyrie-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VALKYRIE_OBJECT_H
#define __VALKYRIE_OBJECT_H


#include <qobject.h>
#include <qstringlist.h>

#include "vk_objects.h"
#include "valkyrie_options_page.h"
#include "valgrind_object.h"
#include "tool_object.h"


/* class Valkyrie ------------------------------------------------------
   Note: the very first option must be > 0, otherwise it conflicts
   with arg_flags in popt. */
class Valkyrie : public VkObject
{
   Q_OBJECT
public:
   Valkyrie();
   ~Valkyrie();

   bool runTool( int tId, VkRunState::State runState );
   void stopTool( int tId );
   bool queryToolDone( int tId );

   /* returns a '\n' separated list of current relevant flags */
   QString getDisplayFlags();
   /* update flags for current tool */
   void updateVgFlags( int tId );

   /* check argval for this option, updating if necessary.
      called by parseCmdArgs() and gui option pages */
   int checkOptArg( int optid, QString& argval );

   enum vkOpts {
      HELP,        OPT_VERSION,   VGHELP,
      TOOLTIP,     PALETTE,   ICONTXT,
      FNT_GEN_SYS, FNT_GEN_USR, FNT_TOOL_USR,
      SRC_EDITOR, SRC_LINES,
      VG_EXEC,      /* path to valgrind executable */
      /* FIRST_CMD_OPT */
      BINARY, BIN_FLAGS, VIEW_LOG, MERGE_EXEC, MERGE_LOGS,
      LAST_CMD_OPT  = MERGE_LOGS
   };

   OptionsPage* createOptionsPage( OptionsWindow* parent ) {
      return (OptionsPage*)new ValkyrieOptionsPage( parent, this );
   }

   QString configEntries();

   VkRunState::State startRunState();

   Valgrind* valgrind() { return m_valgrind; }
   /* for simplicity */
   VkObjectList vkObjList();
   VkObject*    vkObject( int objId );

public slots:
   void quit();

private:
   /* flags relating only to valkyrie */
   QStringList modifiedVgFlags();

   void initToolObjects();

private:
   Valgrind*         m_valgrind;         /* Vg[ Tools ] */

   VkRunState::State m_startRunState;    /* just used on startup */
   QStringList       m_flags;
};


#endif
