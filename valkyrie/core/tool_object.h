/* ---------------------------------------------------------------------- 
 * Definition of class ToolObject                           tool_object.h
 * 
 * Essential functionality is contained within a ToolObject.
 * Each ToolObject has an associated ToolView for displaying output.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

/* TODO: don't have enum values for the objOpts; instead, init an int
   array in the constructor.  This means will have to implement addOpt
   somewhat differently, as won't have enums available */

#ifndef __TOOL_OBJECT_H
#define __TOOL_OBJECT_H

#include <qptrlist.h>

#include "vk_objects.h"

class ToolObject;
typedef QPtrList<ToolObject> ToolObjList;

/* Own namespace for ease of use */
namespace VkRunState {
   /* STOPPED:    no process running
    * VALGRIND:   running valgrind
    * TOOL*:      running a tool-defined process
    */
   enum State { STOPPED=-1, VALGRIND=0, TOOL1, TOOL2, TOOL3 };
}


/* class ToolObject ---------------------------------------------------- */
class ToolView;

class ToolObject : public VkObject 
{
   Q_OBJECT
public:

   ToolObject( const QString& capt, const QString& txt,
               const QKeySequence& key, int objId );
   ~ToolObject();

   /* creates and init's this tool's ToolView window */
   virtual ToolView* createView( QWidget* parent ) = 0;
   /* called by MainWin::closeToolView() */
   virtual bool queryDone() = 0;
   virtual void deleteView();
   ToolView* view();

   /* start a process: may fail */
   virtual bool start( VkRunState::State rs, QStringList vgflags ) = 0;
   /* stop a process: doesn't exit until success */
   virtual void stop() = 0;

   bool isRunning();

   virtual OptionsPage* createOptionsPage( OptionsWindow* parent ) = 0;

   /* returns a list of non-default flags to pass to valgrind */
   virtual QStringList modifiedVgFlags();

signals:
   void running( bool );
   void message( QString );
   void fatal();

protected:
   /* set runstate and emit signal running(bool) */
   void setRunState( VkRunState::State );
   /* get runstate */
   VkRunState::State runState() { return m_runState; }

   virtual bool runValgrind( QStringList vgflags ) = 0;

protected:
   ToolView*  m_view;  /* the toolview window */
   bool       m_fileSaved;

private:
   /* Keep track of current running state */
   VkRunState::State m_runState;
};


#endif  // #ifndef __TOOL_OBJECT_H
