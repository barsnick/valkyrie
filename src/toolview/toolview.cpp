/****************************************************************************
** ToolView implementation
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

#include <QMenuBar>
#include <QToolBar>

#include "toolview/toolview.h"
#include "mainwindow.h"
#include "utils/vk_utils.h"


/***************************************************************************/
/*!
  \class ToolView
  \brief This provides an abstract base class for all toolviews

  This class is based on a QWidget, and is used as a basis for
  all toolviews. The toolviews are organised within a ToolViewStack.
  The toolviews provide a tool-specific menu and toolbar, and
  dynamically add/remove these from the MainWindow.

  * TODO: more on link to tool_objects, when those are integrated...

  \sa ToolViewStack, MainWindow
*/


/*!
    Constructs a ToolView with the given \a parent, \a toolId.

    The toolId is used to track which tool corresponds to which interface.
*/
ToolView::ToolView( QWidget* parent, VGTOOL::ToolID id )
   : QWidget( parent ), toolId( id )
{
   // Create and add toolToolBar to MainWindow
   //  - Note: this reparents it to MainWindow, which is fine.
   toolToolBar = new QToolBar( this );
   (( MainWindow* )parent )->addToolBar( Qt::TopToolBarArea, toolToolBar );
   
   // Create toolMenu, and add its QAction to MainWindow
   //  - Note: remains our child: only the 'action' is added to MainWindow
   toolMenu = new QMenu( this );
   (( MainWindow* )parent )->insertToolMenuAction( toolMenu->menuAction() );
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
ToolView::~ToolView()
{
   // Cleanup the menus/toolbars for this ToolView
   
   //TODO: this still right? parent is MainWindow these days, no?
   
   // We need to get access to the mainwindow:
   //  - this->widgetStack->ToolViewStack->MainWindow
   QWidget* prnt = this->parentWidget()->parentWidget()->parentWidget();
   MainWindow* mw = ( MainWindow* )prnt;
   
   if ( mw ) {
      if ( toolMenu ) {
         // The toolMenu remains ours, so no need to reparent,
         // but do unregister the action nicely:
         mw->removeToolMenuAction( toolMenu->menuAction() );
      }
      
      if ( toolToolBar ) {
         // Remove toolbar and reparent to this.
         mw->removeToolBar( toolToolBar );
         toolToolBar->setParent( this );
      }
   }
}



/*!
  Sets font of central widget
*/
void ToolView::setToolFont( QFont font )
{
   font.setStyleHint( QFont::TypeWriter );
   this->setFont( font );
}


/*!
    Show toolBar/Menu in MainWindow
*/
void ToolView::showToolMenus()
{
   if ( toolToolBar ) {
      toolToolBar->setVisible( true );
   }
   
   if ( toolMenu ) {
      toolMenu->menuAction()->setVisible( true );
   }
}


/*!
    Hide toolBar/Menu in MainWindow
*/
void ToolView::hideToolMenus()
{
   if ( toolToolBar ) {
      toolToolBar->setVisible( false );
   }
   
   if ( toolMenu ) {
      toolMenu->menuAction()->setVisible( false );
   }
}




/***************************************************************************/
/*!
  \class ToolViewStack
  \brief This provides a container for the toolviews

  This class is based on a QFrame, and provides an ToolView* based
  interface to a QWidgetStack.

  \sa ToolView, MainWindow
*/

/*!
    Constructs an empty ToolViewStack with the given \a parent.

    \a addview()
*/
ToolViewStack::ToolViewStack( QWidget* parent )
   : QFrame( parent )
{
   setObjectName( QString::fromUtf8( "ToolViewStack" ) );
   
   widgetStack = new QStackedWidget( this );
   widgetStack->setObjectName( QString::fromUtf8( "widgetStack" ) );
   
   QHBoxLayout* layout = new QHBoxLayout;
   layout->setMargin( 0 );
   layout->setSpacing( 0 );
   layout->addWidget( widgetStack );
   setLayout( layout );
}


/*!
    Destroys this widget, and frees any allocated resources.

    The widgetStack will be auto deleted as we're the parent,
    and the ToolViews are all children of widgetStack, so
    all's taken care of.
*/
ToolViewStack::~ToolViewStack()
{
}


/*!
    Adds \a toolview to the stack
*/
void ToolViewStack::addView( ToolView* toolview )
{
   widgetStack->addWidget( toolview );
}


/*!
    Removes \a toolview from the stack, deletes it, and shows the next toolview.
*/
void ToolViewStack::removeView( ToolView* toolview )
{
   // hide the actions, and kill the toolview
   toolview->hideToolMenus();
   widgetStack->removeWidget( toolview );
   delete toolview;
   
   // if there's a view left on the stack: show it
   ToolView* toolview_next = ( ToolView* )widgetStack->widget( widgetStack->count() - 1 );
   
   if ( toolview_next ) {
      toolview_next->showToolMenus();
      widgetStack->setCurrentWidget( toolview_next );
   }
}


/*!
    Finds a toolview on the stack, based on \a toolId, and returns it.
*/
ToolView* ToolViewStack::findView( VGTOOL::ToolID toolId ) const
{
   for ( int i = 0; i < widgetStack->count(); ++i ) {
      ToolView* tv = ( ToolView* )widgetStack->widget( i );
      
      if ( tv->getToolId() == toolId ) {
         return tv;
      }
   }
   
   return 0;
}


/*!
    Returns the currently-visible toolview, or 0 if there's none visible.
*/
ToolView* ToolViewStack::currentView()
{
   return ( ToolView* )widgetStack->currentWidget();
}


/*!
    Returns the toolId of the currently-visible toolview,
    or VGTOOL::ID_NULL if there's none visible.
*/
VGTOOL::ToolID ToolViewStack::currentToolId()
{
   if ( widgetStack->currentIndex() >= 0 ) {
      return currentView()->getToolId();
   }
   
   return VGTOOL::ID_NULL;
}


/*!
    Raises the toolview given by the \a toolview to the top (i.e. visible).
    It thereby hides all other toolviews
*/
void ToolViewStack::raiseView( ToolView* toolview )
{
   // cleanup last view, if there was one
   ToolView* tv_last = currentView();
   
   if ( tv_last && tv_last != toolview ) {
      tv_last->hideToolMenus();
   }
   
   // prepare next view
   toolview->showToolMenus();
   widgetStack->setCurrentWidget( toolview );
}


/*!
    Print some diagnostics - handy for debugging
*/
void ToolViewStack::print( QString str ) const
{
#ifdef DEBUG_ON
   cerr << endl << str.toLatin1().data() << endl;
   cerr << "current idx: " << widgetStack->currentIndex() << endl;
   cerr << "num views: " << widgetStack->count() << endl;
   
   for ( int i = 0; i < widgetStack->count(); ++i ) {
      ToolView* tv = ( ToolView* )widgetStack->widget( i );
      cerr << "idx: " << i << " => tvid: " << tv->getToolId() << endl;
   }
#endif
}
