/****************************************************************************
** ContextHelp definition
**  - context-sensitive help button
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

#ifndef __VK_HELP_CONTEXT_H
#define __VK_HELP_CONTEXT_H

#include <QAction>
#include <QHash>
#include <QList>
#include <QString>
#include <QToolButton>
#include <QWidget>


// Forward decls
class HandBook;


// ============================================================
class ContextHelpAction: public QAction
{
   Q_OBJECT
public:
   ContextHelpAction( QWidget* parent, HandBook* book );
   ~ContextHelpAction();
   
public slots:
   void startListening( bool checked );
};




// ============================================================
class ContextHelp: public QObject
{
   Q_OBJECT
   friend class ContextHelpAction;
   
public:
   ContextHelp();
   ~ContextHelp();
   static void addHelp( QWidget*, const QString& );
   
private:
   static void setupSingleton();
   
   bool eventFilter( QObject*, QEvent* );
   void newItem( QWidget* widget, const QString& text );
   void showHelp( const QString& );
   void cancelHelpEvent();
   void remove( QWidget* );
   
   HandBook* hbook;   // ptr to the application-wide handbook
   
   QHash<QWidget*, QString> wdict;    // mapping widg->url
   QList<ContextHelpAction*> actions; // allows turning off all registered ctxt help actions
   bool listeningForEvent;
   
private slots:
   void cleanupWidget();
};


#endif // #ifndef __VK_HELP_CONTEXT_H
