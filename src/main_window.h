/* ---------------------------------------------------------------------
 * Definition of MainWindow                                main_window.h
 * Application's top-level window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_MAIN_WINDOW_H
#define __VK_MAIN_WINDOW_H


#include "workspace.h"
#include "help_about.h"
#include "hand_book.h"
#include "options_window.h"

#include "vk_objects.h"
#include "valkyrie_object.h"

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qtoolbutton.h>



/* class MainWindow ---------------------------------------------------- */
class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow( Valkyrie* valk );
  ~MainWindow();

public slots:
  /* sets run and stop buttons to correct state */
  void updateButtons( bool running );
  /* show a message in the status bar */
  void setStatus( QString );
  /* dis/enable tooltips */
  void toggleToolTips();
  /* show toolbutton text labels (or not) */
  void toggleToolbarLabels();
  /* show toolview for tool set in [valgrind:tool] */
  void showToolView( int tvid ); 
  /* connected to optionsWin signal flagsChanged() */
  void updateFlagsWidget();

signals:
  void toolbarLabelsToggled(bool);

protected:
  void resizeEvent( QResizeEvent* re );
  void moveEvent( QMoveEvent* me );
  void closeEvent( QCloseEvent* ce );

private slots:
  void run();
  void stop();

  void showFlagsWidget( bool show );
  void showOptionsWindow(int);
  void showAboutInfo( int id );
  void closeToolView();

  //void dummy() { printf("dummy()\n"); }
  //void dummy(int n) { printf("dummy( %d )\n", n); }
  //void dummy( bool b ) { printf("dummy( %d )\n", b ); }

private:
  void setToggles( int );
  void mkMenuBar();
  void mkStatusBar();

private:
  WorkSpace* wSpace;

  Valkyrie* valkyrie;
  ToolView* activeView;
  ToolObject* activeTool;

  HandBook* handBook;
  OptionsWindow* optionsWin;

  /* label to show non-default flags for current tool */
  QLabel* flagsLabel;
  QToolButton* flagsButton;

  /* messages label for status bar */
  QLabel* statusMsg;

  bool showToolTips;
  bool showToolbarLabels;

  QPopupMenu* fileMenu;
  enum menuIds{ FILE_RUN, FILE_STOP, FILE_CLOSE };

  QPopupMenu* toolsMenu;
  QButtonGroup* viewButtGroup;

  QToolButton* runButton;
  QToolButton* stopButton;
  QToolButton* helpButton;
};


#endif
