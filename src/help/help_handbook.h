/****************************************************************************
** HandBook definition
**  - Context-sensitive help browser
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

#ifndef __VK_HELP_HANDBOOK_H
#define __VK_HELP_HANDBOOK_H

#include <QComboBox>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTextBrowser>



// ============================================================
class HandBook : public QMainWindow
{
   Q_OBJECT
public:
   HandBook( QWidget* parent = 0 );
   ~HandBook();
   
public slots:
   void openUrl( const QString& url );
   void showYourself();
   
protected:
   void closeEvent( QCloseEvent* ce );
   
private slots:
   void sourceChanged( const QUrl& url );
   void openFile();
   void historyChosen( QAction* act );
   void bookmarkChosen( QAction* act );
   void bookmarkHighlighted( QAction* act );
   void addBookmark();
   
private:
   void mkMenuToolBars();
   void save();
   void readHistory();
   void readBookmarks();
   
private:
   QString       caption;
   QTextBrowser* browser;
   QComboBox*    pathCombo;
   QMenuBar*     menuBar;
   QStatusBar*   helpStatusBar;
   QMenu*        bookmarkMenu;
   QMenu*        historyMenu;
   
   int max_history;
   int max_bookmarks;
};


#endif  // #ifndef __VK_HELP_HANDBOOK_H
