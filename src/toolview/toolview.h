/****************************************************************************
** ToolView definition
**  - the abstract base class for all tool views
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

#ifndef __VK_TOOLVIEW_H
#define __VK_TOOLVIEW_H

#include "toolview/vglogview.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QList>


// ============================================================
// forward declarations
class ToolViewStack;


// ============================================================
// type definitions
namespace VGTOOL
{

// Valgrind Tools
enum ToolID {
   ID_NULL = -1,
   ID_MEMCHECK = 0,
   ID_HELGRIND,
   ID_MAX
};

// Tool Processes: Tools may extend this enum with their own process id's.
enum ToolProcessId {
   PROC_NONE = -1,    // no process running
   PROC_VALGRIND = 0, // run valgrind for given tool
   PROC_PARSE_LOG = 1,

   // If Tools need their own processes, extend this enum as follows:
   // Add PROC_TOOL_FIRST here: tool defined proc-id's start here, and run to < PROC_MAX
   // Within tool: enum toolProcess { PROC_TOOL1 = VGTOOL::PROC_TOOL_FIRST, ... }

   PROC_MAX = 2       // to a reasonable max number of processes per tool
};
}


// ============================================================
class ToolView : public QWidget
{
   Q_OBJECT
   friend class ToolViewStack; // needs access to protected members
   
public:
   ToolView( QWidget* parent, VGTOOL::ToolID toolId );
   ~ToolView();
   
   virtual VgLogView* createVgLogView() = 0;

   void setToolFont( QFont font );

signals:
   void saveLogFile();

protected:
   virtual void setupLayout() = 0;
   virtual void setupActions() = 0;
   virtual void setupToolBar() = 0;
   
protected:
   void showToolMenus();
   void hideToolMenus();
   VGTOOL::ToolID getToolId() {
      return toolId;
   }
   
protected slots:
   void openLogFile();
   
public slots:
   // called by the view's object
   virtual void setState( bool run ) = 0;
   
signals:
   // start appropriate process for given runState
   void run( VGTOOL::ToolProcessId procId );
   void logFileChosen( QString logFilename );
   
protected:
   VGTOOL::ToolID toolId;
   QToolBar*      toolToolBar;
   QMenu*         toolMenu;
};



// ============================================================
class ToolViewStack : public QFrame
{
   Q_OBJECT
public:
   ToolViewStack( QWidget* parent );
   ~ToolViewStack();
   
   void addView( ToolView* toolview );
   void removeView( ToolView* toolview );
   
   ToolView* findView( VGTOOL::ToolID toolId ) const;
   
   ToolView*      currentView();     // return currently-visible view
   VGTOOL::ToolID currentToolId();   // return toolId of currently-visible view
   
public slots:
   void raiseView( ToolView* tv );
   
private:
   void print( QString str ) const; // for debugging only.

private:
   QStackedWidget* widgetStack;
};


#endif // __VK_TOOLVIEW_H
