/* ---------------------------------------------------------------------
 * Custom file dialog                                   vk_file_dialog.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_FILE_DIALOG_H
#define __VK_FILE_DIALOG_H

#include <qdialog.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qsplitter.h>
#include <qwidgetstack.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qtoolbutton.h>


class FileListBox;
class FileDialogFileListView;

class FileDialog : public QDialog
{
  Q_OBJECT
public:
  FileDialog( QWidget* parent=0, const char* name=0 );
  ~FileDialog();




private:
  bool geometryDirty;

  QLineEdit* nameEdit;   /* name/filter editor */
  QSplitter* splitter;
	QWidgetStack* stack;
  FileDialogFileListView* files;
  FileListBox* moreFiles;
  QComboBox *paths;
  QComboBox *types;
  QPushButton* okB;
  QPushButton* cancelB;
  QLabel * pathL;
  QLabel * fileL;
  QLabel * typeL;
	QToolButton *goBack, *cdToParent ;
  QButtonGroup* modeButtons;

};


/* class FileListBox --------------------------------------------------- */
class FileListBox : public QListBox
{
  friend class FileDialog;
  Q_OBJECT
private:
  FileListBox( QWidget *parent, FileDialog* d );

  void clear();
  void show();
  //void startRename( bool check = TRUE );
  void viewportMousePressEvent( QMouseEvent *e );
  void viewportMouseReleaseEvent( QMouseEvent *e );
  void viewportMouseDoubleClickEvent( QMouseEvent *e );
  void viewportMouseMoveEvent( QMouseEvent *e );
  //void viewportDragEnterEvent( QDragEnterEvent *e );
  //void viewportDragMoveEvent( QDragMoveEvent *e );
  //void viewportDragLeaveEvent( QDragLeaveEvent *e );
  //void viewportDropEvent( QDropEvent *e );
  //bool acceptDrop( const QPoint &pnt, QWidget *source );
  //void setCurrentDropItem( const QPoint &pnt );
  void keyPressEvent( QKeyEvent *e );

private slots:
	//void rename();
	//void cancelRename();
  void doubleClickTimeout();
  //void changeDirDuringDrag();
  //void dragObjDestroyed();
  //void contentsMoved( int, int );

private:
  //QRenameEdit *lined;
  FileDialog* filedialog;
  //bool renaming;
  //QTimer* renameTimer;
  //QListBoxItem *renameItem, *dragItem;
  QPoint pressPos/*, oldDragPos*/;
  bool mousePressed;
  int urls;
  //QString startDragDir;
  //QListBoxItem *currDropItem;
  QTimer* changeDirTimer;
  bool firstMousePressEvent;
  //QUrlOperator startDragUrl;
};


/* class FileDialogFileListView ---------------------------------------- */
class FileDialogFileListView : public QListView
{
  Q_OBJECT
public:
  FileDialogFileListView( QWidget *parent, FileDialog* d );

  void clear();
  //void startRename( bool check = TRUE );
  void setSorting( int column, bool increasing = TRUE );

  //QRenameEdit *lined;
  //bool renaming;
  //QListViewItem *renameItem;

private:
  void viewportMousePressEvent( QMouseEvent *e );
  void viewportMouseDoubleClickEvent( QMouseEvent *e );
  void keyPressEvent( QKeyEvent *e );
  void viewportMouseReleaseEvent( QMouseEvent *e );
  void viewportMouseMoveEvent( QMouseEvent *e );
  //void viewportDragEnterEvent( QDragEnterEvent *e );
  //void viewportDragMoveEvent( QDragMoveEvent *e );
  //void viewportDragLeaveEvent( QDragLeaveEvent *e );
  //void viewportDropEvent( QDropEvent *e );
  //bool acceptDrop( const QPoint &pnt, QWidget *source );
  //void setCurrentDropItem( const QPoint &pnt );

private slots:
  //void rename();
  //void cancelRename();
  void changeSortColumn2( int column );
  void doubleClickTimeout();
  //void changeDirDuringDrag();
  //void dragObjDestroyed();
  //void contentsMoved( int, int );

private:
  FileDialog* filedialog;
  //QTimer* renameTimer;
  QPoint pressPos, oldDragPos;
  bool mousePressed;
  int urls;
  //QString startDragDir;
  //QListViewItem *currDropItem, *dragItem;
  QTimer* changeDirTimer;
  bool firstMousePressEvent;
  bool ascending;
  int sortcolumn;
  //QUrlOperator startDragUrl;
};



#endif
