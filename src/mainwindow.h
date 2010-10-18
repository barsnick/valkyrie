/****************************************************************************
** MainWindow definition
**  - the top-level application window
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

#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <QActionGroup>
#include <QEvent>
#include <QFont>
#include <QLabel>
#include <QMainWindow>
#include <QPalette>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "help/help_handbook.h"
#include "objects/valkyrie_object.h"
#include "options/vk_options_dialog.h"
#include "toolview/toolview.h"


// ============================================================
class MainWindow : public QMainWindow
{
   Q_OBJECT
   
public:
   MainWindow( Valkyrie* vk );
   ~MainWindow();
   
   void insertToolMenuAction( QAction* action );
   void removeToolMenuAction( QAction* action );
   Valkyrie* getValkyrie() {
      return valkyrie;
   }
   
public slots:
   void setStatus( QString msg );
   void showToolView( VGTOOL::ToolID toolId );
   void runTool( VGTOOL::ToolProcessId procId );
   void openOptions();

private:
   void setupLayout();
   void setupActions();
   void setupMenus();
   void setupToolBars();
   void setupStatusBar();
   void setToggles( VGTOOL::ToolID toolId );
   void updateEventFilters( QObject* obj );
   bool eventFilter( QObject* obj, QEvent* e );
   void setCurrentProject( const QString &projName );
   void updateActionsRecentProjs();

private slots:
   void toolGroupTriggered( QAction* );
   void createNewProject();
   void openProject();
   void openRecentProject();
   void saveAsProject();
   void closeToolView();
   void runValgrind();
   void stopTool();
   void openHandBook();
   void openAboutVk();
   void openAboutLicense();
   void openAboutSupport();
   void updateVgButtons( bool running );
   
   // functions for dealing with config updates
   void showLabels();
   void showToolTips();
   void setGenFont();
   void setToolFont();
   void setPalette();
   
   // functions for dealing with toolview updates
   void setLogFile( QString logFilename );
   
   
private:
   enum { MaxRecentProjs = 5 };
   QAction* actFile_NewProj;
   QAction* actFile_OpenProj;
   QAction* actFile_RecentProjs[ MaxRecentProjs ];
   QAction* actFile_SaveAs;
   QAction* actFile_Exit;
   QAction* actFile_Close;
   QAction* actEdit_Options;
   QAction* actEdit_Search;
   QAction* actProcess_Run;
   QAction* actProcess_Stop;
   QAction* actHelp_Handbook;
   QAction* actHelp_About_Valkyrie;
   QAction* actHelp_About_Qt;
   QAction* actHelp_License;
   QAction* actHelp_Support;
   QActionGroup* toolActionGroup;

   QMenuBar* menuBar;
   QMenu* menuFile;
   QMenu* menuRecentProjs;
   QMenu* menuEdit;
   QMenu* menuTools;
   QMenu* menuProcess;
   QMenu* menuHelp;
   
   QToolBar* mainToolBar;
   QStatusBar* mainStatusBar;
   
private:
   Valkyrie*        valkyrie;
   ToolViewStack*   toolViewStack;
   QLabel*          statusLabel;
   HandBook*        handBook;
   VkOptionsDialog* optionsDialog;
   
   bool     fShowToolTips;
   QFont    lastAppFont;
   QPalette lastPalette;
};

#endif // __MAINWINDOW_H
