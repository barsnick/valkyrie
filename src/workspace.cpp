/* ---------------------------------------------------------------------
 * Implementation of WorkSpace                             workspace.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "workspace.h"

#include <qwindowsstyle.h>

/* - http://doc.trolltech.com/qq/qq09-q-and-a.html
   - 
*/

/* subclass of QStyle so that QWorkspace doesn't show the window
   controls when windows are maximised (sigh) */
class VkStyle : public QWindowsStyle
{
public:
  VkStyle() { }
  ~VkStyle() { }
  int styleHint( StyleHint sh, const QWidget* w, 
                 const QStyleOption & opt = QStyleOption::Default,
                 QStyleHintReturn* ret = 0 ) const;
};

int VkStyle::styleHint( StyleHint hint, const QWidget* w,
                        const QStyleOption& opt,
                        QStyleHintReturn *ret ) const
{
  if ( hint == QStyle::SH_Workspace_FillSpaceOnMaximize ) {
    return 1;
  } else {
    return QWindowsStyle::styleHint( hint, w, opt, ret );
  }
}




/* class WorkSpace ----------------------------------------------------- */
WorkSpace::~WorkSpace() { }

WorkSpace::WorkSpace( QWidget* parent, const char* name )
  : QWorkspace( parent, name )
{
  VkStyle * vs = new VkStyle();
  setStyle( vs );
  /* can't just connect signals, because different arg types */
  connect( this, SIGNAL(windowActivated(QWidget*)),
           this, SLOT(slotWindowActivated(QWidget*)) );
}


/* signal windowActivated --> signal formActivated */
void WorkSpace::slotWindowActivated( QWidget *w )
{ emit viewActivated( (ToolView*)w ); }


int WorkSpace::numViews()
{ return QWorkspace::windowList().count(); }


/* currently active toolview */
ToolView* WorkSpace::activeView() const
{
  QWidget* w = activeWindow();
  if ( w && w->inherits( "ToolView" ) )
    return (ToolView*)w;
  else
    return NULL;
}

/* list of live toolviews */
QPtrList<ToolView> WorkSpace::viewList() const
{
  QPtrList<ToolView> view_list;
  QWidgetList wlist = QWorkspace::windowList();

  for ( QWidget* w = wlist.first(); w; w = wlist.next() ) {
    if ( w->inherits( "ToolView" ) )
      view_list.append( (ToolView*)w );
  }

  return view_list;
}

/* find a toolview based on its object's ObjectId. 
   if none exists, returns 0 */
ToolView* WorkSpace::findView( int view_id ) const
{
  QPtrList<ToolView> views = viewList();

  ToolView* tview = 0;
  for ( ToolView* tv = views.first(); tv; tv = views.next()) {
    if ( tv->id() == view_id ) {
      tview = tv;
      break;
    }
  }
  return tview;
}

