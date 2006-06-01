/* ---------------------------------------------------------------------
 * Definition of CachegrindView                        cachegrind_view.h
 * Cachegrind's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __CACHEGRIND_VIEW_H
#define __CACHEGRIND_VIEW_H


#include "tool_view.h"

/* class CachegrindView ------------------------------------------------ */
class Cachegrind;
class CachegrindView : public ToolView
{
   Q_OBJECT
public:
   CachegrindView( QWidget* parent, const char* name );
   ~CachegrindView();

   /* called by cachegrind: set state for buttons; set cursor state */
   void setState( bool run );

public slots:
   void toggleToolbarLabels( bool );

private:
   void mkToolBar();

private:
   QToolBar* cgToolBar;
};


#endif
