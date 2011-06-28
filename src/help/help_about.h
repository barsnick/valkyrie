/****************************************************************************
** HelpAbout definition
**  - small tabbed dialog showing misc. info re license etc.
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

#ifndef __VK_HELP_ABOUT_H
#define __VK_HELP_ABOUT_H

#include "help/help_handbook.h"   // VkTextBrowser

#include <QDialog>
#include <QTabWidget>

namespace HELPABOUT
{
/*!
   enum identification for available help page tabs
*/
enum TabId { ABOUT_VK = 0, LICENSE, SUPPORT, NUM_TABS };
}

// ============================================================
class HelpAbout : public QDialog
{
   Q_OBJECT
public:
   HelpAbout( QWidget* parent, HELPABOUT::TabId tabid );
   ~HelpAbout();
   
private slots:
   void showTab( int );
   
private:
   QString title;
   
   QTabWidget* tabParent;
   QTextBrowser* aboutVk;
   QTextBrowser* license;
   QTextBrowser* support;
};

#endif
