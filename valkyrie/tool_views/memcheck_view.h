/* ---------------------------------------------------------------------
 * Definition of MemcheckView                            memcheck_view.h
 * Memcheck's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __MEMCHECK_VIEW_H
#define __MEMCHECK_VIEW_H


#include "tool_view.h"

#include <qlistview.h>
#include <qtoolbutton.h>
#include <qtextedit.h>

#include "vglogview.h"


/* class MemcheckView -------------------------------------------------- */
class Memcheck;
class MemcheckView : public ToolView
{
   Q_OBJECT
public:
   MemcheckView( QWidget* parent, const char* name );
   ~MemcheckView();

   VgLog* vgLogPtr() { return logview; }

public slots:
   void toggleToolbarLabels( bool );
   /* called by memcheck: set state for buttons; set cursor state */
   void setState( bool run );

signals:
   void saveLogFile();    /* triggered by savelogButton */

private:
   void mkToolBar();

   private slots:
      void openLogFile();       /* load and parse one log file */
   void openMergeFile();     /* open and check a list of logfiles-to-merge */

   void showSuppEditor();

   void itemSelected();
   void openAllItems(bool);
   void openOneItem();
   void showSrcPath();
   void launchEditor(  QListViewItem* );

private:
   QListView* lView;
   VgLogView* logview;

   QToolButton* savelogButton;
   QToolButton* loadlogButton;
   QToolButton* mrglogButton;
   QToolButton* suppedButton;

   QToolButton* openOneButton;
   QToolButton* openAllButton;
   QToolButton* srcPathButton;

   QToolBar* mcToolBar;
};


#endif
