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

#include "tool_object.h"


class ToolView : public QMainWindow
{
  Q_OBJECT
public:
  ToolView( QWidget* parent, ToolObject* tool );
  ~ToolView();

  /* used by MainWin::closeToolView() */
  int id() { return m_tool->id(); }

  /* called by the view's object */
  virtual void setState( bool run ) = 0;

  ToolObject* tool() { return m_tool; }

public slots:
  virtual void toggleToolbarLabels(bool) = 0;

protected:
  /* keep a ptr to parent tool so we can ask it to do stuff */
  ToolObject* m_tool;

  QWidget* central;
};




/* ToolViewStack */

typedef QPtrList<ToolView> ToolViewList;
typedef QPtrListIterator<ToolView> ToolViewListIter;

class ToolViewStack : public QWidgetStack
{
  Q_OBJECT
public:
  ToolViewStack( QWidget * parent = 0, const char * name = 0 );
  ToolViewStack( QWidget * parent, const char * name, WFlags f );
  ~ToolViewStack();

  int addView( ToolView* tv, int id = -1 );
  void removeView( QWidget* w );
  ToolView* view( int id ) const;

  const ToolViewList* viewList();
  ToolView* nextView( ToolView* lastView = 0 );

  ToolView* visible();   /* return currently-visible view */

  void listViews();

public slots:
  void raiseView( int id );
  void raiseView( ToolView* tv );
};


#endif
