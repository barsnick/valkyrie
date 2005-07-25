/* ---------------------------------------------------------------------
 * Definition of HandBook                                    hand_book.h
 * Context-sensitive help browser
 * TODO: add search + index facility
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_HAND_BOOK_H
#define __VK_HAND_BOOK_H


#include <qcombobox.h>
#include <qmainwindow.h>
#include <qtextbrowser.h>


/* class HandBook ------------------------------------------------------ */
class HandBook : public QMainWindow
{
  Q_OBJECT
public:
  HandBook( QWidget* parent=0, const char* name="handbook" );
  ~HandBook();

  void save();
  void openUrl( const QString& url );

public slots:
  void showYourself();

protected:
  void closeEvent( QCloseEvent* ce );

private slots:
  void setBackwardAvailable( bool );
  void setForwardAvailable( bool );
  void textChanged();
  void openUrl();
  void print();
  void pathSelected( const QString & );
  void historyChosen( int );
  void bookmarkChosen( int );
  void addBookmark();

private:
  void mkMenuToolBars();

private:
  QString caption;

  QTextBrowser* browser;
  QComboBox* pathCombo;

  QString selectedURL;
  QMap<int, QString> mapHistory;
  QMap<int, QString> mapBookmarks;

  enum { BACKWARD=0, FORWARD=1, HOME=2 };
  QMenuBar* mainMenu;
  QPopupMenu* bookmarkMenu;
  QPopupMenu* historyMenu;
};

#endif
