/* ---------------------------------------------------------------------- 
 * Definition of MemcheckOptionsPage              memcheck_options_page.h
 * Subclass of OptionsPage to hold memcheck-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#ifndef __MEMCHECK_OPTIONS_PAGE_H
#define __MEMCHECK_OPTIONS_PAGE_H

#include "options_page.h"


class MemcheckOptionsPage : public OptionsPage
{
   Q_OBJECT
public:
   MemcheckOptionsPage( QWidget* parent, VkObject* obj );
   void applyOption( int optId );

private slots:

};


#endif
