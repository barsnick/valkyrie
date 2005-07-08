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
#include <qwidgetstack.h>

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

  virtual void showWidgets() {}
  virtual void hideWidgets() {}

public slots:
  virtual void toggleToolbarLabels(bool) = 0;

protected:
  VkObject::ObjectId objId;
};




/* ToolViewStack */

typedef QPtrList<ToolView> ToolViewList;
typedef QPtrListIterator<ToolView> ToolViewListIter;

class ToolViewStack : public QWidgetStack
{
  Q_OBJECT
public:
  ToolViewStack ( QWidget * parent = 0, const char * name = 0 );
  ToolViewStack ( QWidget * parent, const char * name, WFlags f );
  ~ToolViewStack();

  int addView ( ToolView* tv, int id = -1 );
  void removeView( QWidget* w );
  ToolView* view ( int id ) const;

  const ToolViewList* viewList();
  ToolView* nextView( ToolView* lastView = 0 );

  ToolView* visibleView();

  void listViews();

public slots:
  void raiseView ( int id );
  void raiseView ( ToolView* tv );
};


#endif
