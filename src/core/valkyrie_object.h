/* --------------------------------------------------------------------- 
 * Definition of class Valkyrie                        valkyrie_object.h
 * Valkyrie-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VALKYRIE_OBJECT_H
#define __VALKYRIE_OBJECT_H


#include <qobject.h>
#include <qstringlist.h>

#include "vk_objects.h"

class Valgrind;
class Memcheck;
class Cachegrind;
class Massif;
class ToolObject;

typedef QPtrList<ToolObject> ToolList;

/* class Valkyrie ------------------------------------------------------
   Note: the very first option must be > 0, otherwise it conflicts
   with arg_flags in popt. */
class Valkyrie : public VkObject
{
  Q_OBJECT
public:
  Valkyrie();
  ~Valkyrie();

  void init();        /* set usingGui + get ptrs to all tools */
  bool runTool( ToolObject* activeTool=0 );
  void stopTool( ToolObject* activeTool=0 );

  /* used in gui mode. returns a list containing only tool objects. */
  ToolList toolList();

  /* modeNotSet:      no cmd-line options given
   * modeParseLog:    read <logfile> from disk
   * modeMergeLogs:   read <file-list> from disk and merge the contents
   * modeParseOutput: read output direct from valgrind 
   */
  enum RunMode { modeNotSet=0, modeParseLog, modeMergeLogs, modeParseOutput };
  void setRunMode( Valkyrie::RunMode rm );

  /* returns a '\n' separated list of current relevant flags */
  QString currentFlags( ToolObject* tool_obj );
  /* flags relating only to valkyrie */
  QStringList modifiedFlags();

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum vkOpts {
    FIRST_CMD_OPT = 11,
    HELP_OPT      = 1,
    TOOLTIP       = 2,
    PALETTE       = 3,
    ICONTXT       = 4,
    FONT_SYSTEM   = 5,
    FONT_USER     = 6,
    SRC_EDITOR    = 7,
    SRC_LINES     = 8,
    /* path to valgrind executable (/usr/bin/valgrind) */
    VG_EXEC       = 9,
    /* path to supp. files dir [def = /usr/lib/valgrind/] */
    VG_SUPPS_DIR  = 10,
    BINARY        = FIRST_CMD_OPT,
    BIN_FLAGS     = 12,
    VIEW_LOG      = 13,
    MERGE_LOGS    = 14,
    USE_GUI       = 15,
    LAST_CMD_OPT  = USE_GUI
  };

public slots:
  void quit();

private:
  RunMode runMode;
  QStringList flags;

  Valgrind* valgrind;
  Memcheck* memcheck;
  Cachegrind* cachegrind;
  Massif* massif;

  ToolList toolObjectList;
};


#endif
