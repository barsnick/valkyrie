/* ---------------------------------------------------------------------
 * Definition of ToolView                                    tool_view.h
 * Base class for all tool views
 * ---------------------------------------------------------------------
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_TOOL_VIEW_H
#define __VK_TOOL_VIEW_H


#include <qwidget.h>

#include "vk_objects.h"


class ToolView : public QWidget
{
  Q_OBJECT
public:
  ToolView( QWidget* parent, QString title, VkObject::ObjectId id );
  ~ToolView();

  /* used by MainWin::closeToolView() */
  VkObject::ObjectId id() { return objId; }

  /* called by the view's object */
  virtual void setState( bool run ) = 0;

  void showToolBar() { if (toolBar) toolBar->show(); }
  void hideToolBar() { if (toolBar) toolBar->hide(); }

public slots:
  virtual void toggleToolbarLabels(bool) = 0;

protected:
  VkObject::ObjectId objId;
  QToolBar* toolBar;
};


#endif
