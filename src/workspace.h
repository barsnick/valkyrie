/* ---------------------------------------------------------------------
 * Definition of class WorkSpace                             workspace.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_WORKSPACE_H
#define __VK_WORKSPACE_H

#include <qworkspace.h>
#include <qptrlist.h>

#include "tool_view.h"


/* simple subclass of QWorkspace to remove constant casting in
   MainWindow.  Better way to do this? */
class WorkSpace : public QWorkspace
{
  Q_OBJECT
public:
  WorkSpace( QWidget* parent=0, const char* name=0 );
  ~WorkSpace();

  /* currently active window */
  ToolView* activeView() const;

  /* find toolview based on view_id (else return 0) */
  ToolView* findView( int view_id ) const;

  /* return number of toolview windows currently open */
  int numViews();

  /* List of alive toolviews */
  QPtrList<ToolView> viewList() const;

signals:
  /* emmitted whenever a toolview becomes active */
  void viewActivated( ToolView* tv );

private slots:
  void slotWindowActivated( QWidget* w );
};


#endif
