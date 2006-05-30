/* ---------------------------------------------------------------------- 
 * Definition of class MassifOptionsPage            massif_options_page.h
 * Subclass of OptionsPage to hold massif-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __MASSIF_OPTIONS_PAGE_H
#define __MASSIF_OPTIONS_PAGE_H

#include "options_page.h"


class MassifOptionsPage : public OptionsPage
{
   Q_OBJECT
public:
   MassifOptionsPage( QWidget* parent, VkObject* obj );
   void applyOption( int optId );
};


#endif
