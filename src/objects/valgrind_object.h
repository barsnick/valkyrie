/****************************************************************************
** Valgrind object class definition
**  - Valgrind-specific: options / flags / functionality
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __VALGRIND_OBJECT_H
#define __VALGRIND_OBJECT_H

#include "objects/vk_objects.h"
#include "objects/tool_object.h"


// ============================================================
namespace VALGRIND
{
/*!
   enum identification of all options for this object
*/
enum vgOptId {
   TOOL,           // --tool
   // common options relevant to all tools
   VERBOSITY,      // --verbosity
   TRACE_CH,       // --trace-children
   SILENT_CH,      // --child-silent-after-fork
   TRACK_FDS,      // --track-fds
   TIME_STAMP,     // --time-stamp
   LOG_FD,         // --log-fd
   LOG_FILE,       // --log-file
   LOG_SOCKET,     // --log-socket
   
   // uncommon options relevant to all tools
   RUN_LIBC,       // --run-libc-freeres
   SIM_HINTS,      // --sim-hints
   KERN_VAR,       // --kernel-variant
   EM_WARNS,       // --show-emwarns
   SMC_CHECK,      // --smc-check
   
   // options relevant to error-reporting tools
   XML_OUTPUT,     // --xml
   XML_COMMENT,    // --xml-user-comment
   DEMANGLE,       // --demangle
   NUM_CALLERS,    // --num-callers
   ERROR_LIMIT,    // --error-limit
   SHOW_BELOW,     // --show-below-main
   
   // suppressions hackery
   SUPPS_SEL,      // selected suppression files
   
   // misc
   GEN_SUPP,       // --gen-suppressions
   DB_ATTACH,      // --db-attach
   DB_COMMAND,     // --db-command
   INPUT_FD,       // --input-fd
   MAX_SFRAME,     // --max-stackframe
   NUM_OPTS
};
}


// ============================================================
class Valgrind : public VkObject
{
public:
   Valgrind();
   ~Valgrind();
   
   QStringList getVgFlags( ToolObject* tool_obj );
   
   unsigned int maxOptId() {
      return VALGRIND::NUM_OPTS;
   }
   
   int checkOptArg( int optid, QString& argval );
   
   VkOptionsPage* createVkOptionsPage();
   
public:
   // ToolObject access
   ToolObjList getToolObjList();
   ToolObject* getToolObj( VGTOOL::ToolID tid );
//TODO: needed?
   //   int         getToolObjId( const QString& name );
   //   ToolObject* getToolObj( const QString& name );
   
private:
   // create & init vg tools, ready for cmdline parsing
   void initToolObjects();
   ToolObjList toolObjList;  // Valgrind Tools
   
private:
   void setupOptions();
};

#endif
