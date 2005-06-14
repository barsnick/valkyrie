/* ---------------------------------------------------------------------
 * Definition of HandBook                                    hand_book.h
 * Context-sensitive help browser
 * TODO: add search + index facility
 * --------------------------------------------------------------------- 
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

protected:
  void closeEvent( QCloseEvent *ce );

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

  enum { BACKWARD=0, FORWARD=1, HOME=2 };

  QTextBrowser* browser;
  QComboBox* pathCombo;

  QString selectedURL;
  QMap<int, QString> mapHistory;
  QMap<int, QString> mapBookmarks;

  QPopupMenu* historyMenu;

  QMenuBar* mainMenu;
  QPopupMenu* bookmkMenu;
};

#endif
