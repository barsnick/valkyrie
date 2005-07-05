/* ---------------------------------------------------------------------- 
 * Definition of ValgrindOptionsPage              valgrind_options_page.h
 * Subclass of OptionsPage to hold valgrind-specific options | flags
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VALGRIND_OPTIONS_PAGE_H
#define __VALGRIND_OPTIONS_PAGE_H

#include "options_page.h"


class ValgrindOptionsPage : public OptionsPage
{
  Q_OBJECT
public:
  ValgrindOptionsPage( QWidget* parent, VkObject* obj );
  bool applyOptions( int id, bool undo=false );

private slots:
  void dummy();

};


#endif
