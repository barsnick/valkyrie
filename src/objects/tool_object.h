/****************************************************************************
** ToolObject class definition
**
** Essential functionality is contained within a ToolObject.
** Each ToolObject has an associated ToolView for displaying output.
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


/* TODO: don't have enum values for the objOpts; instead, init an int
   array in the constructor.  This means will have to implement addOpt
   somewhat differently, as won't have enums available */

#ifndef __TOOL_OBJECT_H
#define __TOOL_OBJECT_H

#include "objects/vk_objects.h"
#include "toolview/toolview.h"
#include "utils/vglogreader.h"
#include "utils/vk_logpoller.h"

#include <QList>
#include <QProcess>
#include <QStringList>



// ============================================================
// forward decls
class ToolView;
class ToolObject;

typedef QList<ToolObject*> ToolObjList;


// ============================================================
// class ToolObject: abstract base class
class ToolObject : public VkObject
{
   Q_OBJECT
   
public:
   ToolObject( const QString& toolname, VGTOOL::ToolID toolId );
   ~ToolObject();
   
   ToolView* createView( QWidget* parent );
   ToolView* view();
   
   bool start( VGTOOL::ToolProcessId procId,
               QStringList vgflags, QString logfile );
//   void stop();
   bool queryDone();
   bool isRunning();

   virtual VkOptionsPage* createVkOptionsPage() = 0;
   
   // returns a list of non-default flags to pass to valgrind
   virtual QStringList getVgFlags();

//public slots?
   void stop();

signals:
   void running( bool );
   void message( QString );
   
protected:
   void setProcessId( int procId );
   int getProcessId();

private:
   virtual ToolView* createToolView( QWidget* parent ) = 0;
   virtual void statusMsg( QString msg ) = 0;
   bool runValgrind( QStringList vgflags );
   bool parseLogFile();
   bool startProcess( QStringList flags );

   bool queryFileSave();
   bool saveParsedOutput( QString& fname );

private slots:
   void stopProcess();
   void killProcess();
   void processDone( int exitCode, QProcess::ExitStatus exitStatus );
   void readVgLog();
   void checkParserFinished();

public slots:
   bool fileSaveDialog( QString fname = QString() );

public:
   VGTOOL::ToolID getToolId() {
      return toolId;
   }
   
protected:
   ToolView*  toolView;  // the toolview window
   QString    saveFname;
   bool       fileSaved;

private:
   // tools need to add own processId's: classic enum extend problem :-(
   int processId;
   
   VGTOOL::ToolID toolId;  // which tool are we.

private:
   VgLogReader* vgreader;
   QProcess*    vgproc;
   VkLogPoller* logpoller;
};


#endif  // #ifndef __TOOL_OBJECT_H
