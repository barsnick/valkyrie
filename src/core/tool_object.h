/* ---------------------------------------------------------------------- 
 * Definition of class ToolObject                           tool_object.h
 * 
 * Essential functionality is contained within a ToolObject.
 * Each ToolObject has an associated ToolView for displaying output.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

/* TODO: don't have enum values for the objOpts; instead, init an int
   array in the constructor.  This means will have to implement addOpt
   somewhat differently, as won't have enums available */

#ifndef __TOOL_OBJECT_H
#define __TOOL_OBJECT_H

#include <qprocess.h>
#include <qptrlist.h>

#include "vk_objects.h"
#include "valkyrie_object.h"

class ToolObject;
typedef QPtrList<ToolObject> ToolList;

/* class ToolObject ---------------------------------------------------- */
class ToolView;

class ToolObject : public VkObject 
{
 Q_OBJECT
public:

  ToolObject( ObjectId id, const QString& capt, const QString& txt,
              const QKeySequence& key );
  ~ToolObject();

  /* creates and returns the ToolView window for a tool object */
  virtual ToolView* createView( QWidget* parent ) = 0;
  /* called by MainWin::closeToolView() */
  virtual bool isDone() = 0;
  virtual void deleteView();

  ToolView* view() { return m_view; }

  virtual bool start( Valkyrie::RunMode rm ) = 0;
  virtual bool stop( Valkyrie::RunMode rm ) = 0;
  virtual bool run( QStringList flags ) = 0;

  bool isRunning();

  virtual OptionsPage* createOptionsPage( OptionsWindow* parent ) = 0;

signals:
  void setRunMode( Valkyrie::RunMode );
  void running( bool );
  void message( QString );
  /* connected to valkyrie's quit() slot when in non-gui mode */
  void finished();
  void fatal();

protected:
  void killProc();
  virtual void emitRunning( bool ) = 0;

protected:
  ToolView* m_view;  /* the toolview window */

  bool is_Running;
  bool fileSaved;

  QProcess* proc;
};


#endif  // #ifndef __TOOL_OBJECT_H
