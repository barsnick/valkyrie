/* ---------------------------------------------------------------------
 * Definition of HelpAbout                                  help_about.h
 * Small tabbed dialog showing misc. info re license etc.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#ifndef __VK_HELP_ABOUT_H
#define __VK_HELP_ABOUT_H

#include <qdialog.h>
#include <qtabwidget.h>

#include "hand_book.h"   /* VkTextBrowser */


/* class HelpAbout ----------------------------------------------------- */
class HelpAbout : public QDialog
{
   Q_OBJECT
public:
   enum TabId { ABOUT_VK=0, ABOUT_QT, LICENSE, SUPPORT, NUM_TABS };

   HelpAbout( QWidget* parent, TabId tabid );
   ~HelpAbout();

private slots:
   void showTab( QWidget* );

private:
   QString title;

   QTabWidget* tabParent;
   VkTextBrowser* aboutVk;
   VkTextBrowser* aboutQt;
   VkTextBrowser* license;
   VkTextBrowser* support;
};

#endif
