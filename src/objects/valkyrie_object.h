/****************************************************************************
** Valkyrie object class definition
**  - Valkyrie-specific: options / flags / functionality
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
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

#ifndef __VALKYRIE_OBJECT_H
#define __VALKYRIE_OBJECT_H

#include "objects/valgrind_object.h"
#include "objects/vk_objects.h"


// ============================================================
namespace VALKYRIE
{
/*!
   enum identification of all options for this object
   Note: command-line output order depends on this order
*/
enum vkOptId {

   // basic command-line options
   HELP,          // show cmdline help
   VGHELP,        // show extended cmdline help
   OPT_VERSION,   // show version

   // look 'n feel options
   TOOLTIP,       // show tooltips
   PALETTE,       // use valkyrie's palette
   ICONTXT,       // show toolbar text labels
   FNT_GEN_SYS,   // use system default or user-specific general-font
   FNT_GEN_USR,   // choose user-specific general-font
   FNT_TOOL_USR,  // choose font for tools

   // general options
   SRC_EDITOR,    // editor to use to edit source
   SRC_LINES,     // extra lines shown above/below target
   BROWSER,       // browser for external links
   PROJ_FILE,     // project file for valkyrie settings

   // valgrind related flags
   WORKING_DIR,   // where to run valgrind
   VG_EXEC,       // path to valgrind executable
   BINARY,        // user-binary to be valgrindised
   BIN_FLAGS,     // flags for user-binary
   VIEW_LOG,      // parse and view a valgrind logfile
   DFLT_LOGDIR,   // where to put our temporary logs

   NUM_OPTS
};

}


// ============================================================
class VkOptionsPage;

// ============================================================
class Valkyrie : public VkObject
{
   Q_OBJECT
public:
   Valkyrie();
   ~Valkyrie();
   
   bool runTool( VGTOOL::ToolID tId, VGTOOL::ToolProcessId procId );
   void stopTool( VGTOOL::ToolID tId );
   bool queryToolDone( VGTOOL::ToolID tId );
   
   unsigned int maxOptId() {
      return VALKYRIE::NUM_OPTS;
   }
   
   int checkOptArg( QString optGrp, int optid, QString& argval );
   int checkOptArg( int optid, QString& argval );
   
   void updateConfig( QString optGrp, int optid, QString& argval );
   void updateConfig( int optid, QString& argval );
   
   // Generate own options page
   VkOptionsPage* createVkOptionsPage();
   
   Valgrind* valgrind() {
      return m_valgrind;
   }

   VGTOOL::ToolProcessId getStartToolProcess() {
      return m_startToolProcess;
   }
   
   VkOption* findOption( QString& optKey );
//TODO: needed?
   //   VkOption* findOption( QString& optGrp, int optid );
   
   // list of all objects
   const VkObjectList vkObjList();
//TODO: needed?
   //   VkObject*    vkObject( int objId );
   
private:
   QStringList getVgFlags( VGTOOL::ToolID tId );
   QStringList getTargetFlags();
   void setupOptions();
   
private:
   Valgrind* m_valgrind;
   VGTOOL::ToolProcessId m_startToolProcess;
};

#endif  // __VALKYRIE_OBJECT_H
