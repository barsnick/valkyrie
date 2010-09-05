/****************************************************************************
** HelgrindView definition
**  - helgrind's personal window
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
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

#ifndef __HELGRINDVIEW_H
#define __HELGRINDVIEW_H

#include "toolview/toolview.h"
#include "toolview/vglogview.h"

#include <QMenu>
#include <QTreeWidget>
#include <QToolButton>


// ============================================================
class HelgrindView : public ToolView
{
   Q_OBJECT
public:
   HelgrindView( QWidget* parent );
   ~HelgrindView();
   
   VgLogView* createVgLogView();

public slots:
   virtual void setState( bool run );

private:
   void setupLayout();
   void setupActions();
   void setupToolBar();

private slots:
   void openLogFile();
   void opencloseAllItems();
   void opencloseOneItem();
   void showSrcPath();
   void launchEditor( QTreeWidgetItem* item );
   void itemExpanded( QTreeWidgetItem* item );
   void itemCollapsed( QTreeWidgetItem* item );
   void updateItemActions();

private:
   QAction* act_OpenClose_all;
   QAction* act_OpenClose_item;
   QAction* act_ShowSrcPaths;
   QAction* act_OpenLog;
   QAction* act_SaveLog;

   QTreeWidget* treeView;
   VgLogView*   logview;
};

#endif // __HELGRINDVIEW_H
