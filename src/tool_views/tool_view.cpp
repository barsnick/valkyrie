/* ---------------------------------------------------------------------
 * Implementation of class ToolView                        tool_view.cpp
 * Base class for all tool views
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "tool_view.h"

#include <qlayout.h>


ToolView::~ToolView() { }

ToolView::ToolView( QWidget* parent, ToolObject* tool )
  : QMainWindow( parent, tool->name(), WDestructiveClose )
{
  m_tool = tool;

  QString name = tool->name();
  name[0] = name[0].upper();
  setCaption( name );

  central = new QWidget( this );
  setCentralWidget( central );
}





/* Stack of ToolViews */

ToolViewStack::~ToolViewStack() { }

ToolViewStack::ToolViewStack( QWidget* parent/*=0*/, const char * name/*=0*/ )
  : QWidgetStack( parent, name ) { }

ToolViewStack::ToolViewStack( QWidget* parent, const char * name, WFlags f )
  : QWidgetStack( parent, name, f ) { }

int ToolViewStack::addView( ToolView* tv, int id )
{ return addWidget(tv, id); }

void ToolViewStack::removeView( QWidget* w )
{ removeWidget( w ); }

ToolView* ToolViewStack::view( int id ) const
{ return (ToolView*)widget(id); }


/* returns a list of ToolViews */
const ToolViewList* ToolViewStack::viewList()
{ return (ToolViewList*)queryList( "ToolView", 0, false, false); } 

/* iterate over the views to find one != lastView
   returns 0 if none found */
ToolView* ToolViewStack::ToolViewStack::nextView( ToolView* lastView/*=0*/ )
{
  const ToolViewList* views = viewList();
  ToolViewListIter it( *views );
  ToolView* view;
  for (; ((view = it.current()) != 0); ++it ) {
    if (view != lastView)
      return view;
  }
  return 0;
}

ToolView* ToolViewStack::visible()
{ return (ToolView*)QWidgetStack::visibleWidget(); }

void ToolViewStack::listViews()
{
  printf("=============\n");
  const ToolViewList* views = viewList();
  ToolViewListIter it( *views ); // iterate over the views
  ToolView* view;
  for (; ((view = it.current()) != 0); ++it ) {
    printf("ToolView: id(%d), name(%s)\n", id(view), view->name());
  }
}


/* Bring new ToolView to front
    - Hide any previous ToolView widgets
    - Raise new ToolView
    - Show any current ToolView widgets */
void ToolViewStack::raiseView( int id )
{ raiseWidget(id); }

/* Ditto */
void ToolViewStack::raiseView( ToolView* tv )
{ raiseWidget(tv); }
