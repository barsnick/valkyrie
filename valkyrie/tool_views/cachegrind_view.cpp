/* ---------------------------------------------------------------------
 * Implementation of CachegrindView                  cachegrind_view.cpp
 * Cachegrind's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#include "cachegrind_view.h"

#include <qcursor.h>
#include <qlayout.h>
#include <qlabel.h>


/* class CachegrindView ------------------------------------------------ */
CachegrindView::~CachegrindView() 
{ }


CachegrindView::CachegrindView( QWidget* parent, const char* name )
   : ToolView( parent, name )
{
   mkToolBar();

   QVBoxLayout* vLayout = new QVBoxLayout( central );

   /* create the listview */
   QLabel* lbl = new QLabel( "Cachegrind", central, "cachegrind label" );
   lbl->setAlignment( AlignCenter );
   vLayout->addWidget( lbl );
}


/* called by cachegrind: set state for buttons; set cursor state */
void CachegrindView::setState( bool run )
{ 
   if ( run ) {       /* startup */
      setCursor( QCursor(Qt::WaitCursor) );
   } else {           /* finished */
      unsetCursor();
   }
}


/* slot: connected to MainWindow::toggleToolbarLabels(). 
   called when user toggles 'show-butt-text' in Options page */
void CachegrindView::toggleToolbarLabels( bool /*state*/ )
{  }


void CachegrindView::mkToolBar( )
{
}
