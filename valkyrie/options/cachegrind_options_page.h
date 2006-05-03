/* ---------------------------------------------------------------------- 
 * Definition of class CachegrindOptionsPage    cachegrind_options_page.h
 * Subclass of OptionsPage to hold cachegrind-specific options | flags
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __CACHEGRIND_OPTIONS_PAGE_H
#define __CACHEGRIND_OPTIONS_PAGE_H

#include "options_page.h"


class CachegrindOptionsPage : public OptionsPage
{
   Q_OBJECT
public:
   CachegrindOptionsPage( QWidget* parent, VkObject* obj );
   bool applyOptions( int optId );

private slots:
   void getPidFile();
   void getIncludeDirs();
};


#endif
