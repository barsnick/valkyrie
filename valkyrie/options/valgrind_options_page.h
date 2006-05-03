/* ---------------------------------------------------------------------- 
 * Definition of ValgrindOptionsPage              valgrind_options_page.h
 * Subclass of OptionsPage to hold valgrind-specific options | flags
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VALGRIND_OPTIONS_PAGE_H
#define __VALGRIND_OPTIONS_PAGE_H

#include "options_page.h"

#include <qstringlist.h>

class ValgrindOptionsPage : public OptionsPage
{
   Q_OBJECT
public:
   ValgrindOptionsPage( QWidget* parent, VkObject* obj );
   bool applyOptions( int optId );

   void init();

private slots:
   void suppDirsChanged();
   void updateSuppsAvail();
   void getDbBin();
   void dummy();

private:
   /* hold on to these, so don't have to rescan dirs all the time */
   QStringList m_allAvailSuppFiles;
};


#endif
