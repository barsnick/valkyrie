/* ---------------------------------------------------------------------
 * definition of MainWindow                                main_window.h
 * ---------------------------------------------------------------------
 */

#ifndef __VK_MAIN_WINDOW_H
#define __VK_MAIN_WINDOW_H

#include "workspace.h"
#include "help_about.h"
#include "hand_book.h"
#include "options_window.h"

#include <qbuttongroup.h>
#include <qlabel.h>
#include <qtoolbutton.h>



/* class MainWindow ---------------------------------------------------- */
class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();

public slots:
  /* sets run and stop buttons to correct state */
	void updateButtons( bool running );
	/* show a message in the status bar */
	void setStatus( QString );

protected:
  void resizeEvent( QResizeEvent* re );
  void moveEvent( QMoveEvent* me );
	void closeEvent( QCloseEvent* ce );

private slots:
  void run();
	void stop();

	void clearStatus();
  void showToolView( int id ); 
  void showFlagsWidget( bool show );
	void showOptionsWindow(int);
  void closeToolView();

	void dummy() { printf("dummy()\n"); }
	void dummy(int n) { printf("dummy( %d )\n", n); }
	void dummy( bool b ) { printf("dummy( %d )\n", b ); }

	void helpInfo( int id );

private:
  void setToggles( int );
  void mkMenuBar();
  void mkStatusBar();

private:
  Valkyrie* valkyrie;
  WorkSpace* wSpace;
	ToolView* activeView;

	HandBook* handBook;
  OptionsWindow* optionsWin;

	/* label to show non-default flags for current tool */
	QLabel* flagsLabel;
	QToolButton* flagsButton;

	/* messages label for status bar */
	QLabel* statusMsg;

  QPopupMenu* fileMenu;
	enum menuIds{ FILE_RUN, FILE_STOP, FILE_CLOSE };

  QPopupMenu* toolsMenu;
	QButtonGroup* viewButtGroup;

  QToolButton* runButton;
	QToolButton* stopButton;
	QToolButton* helpButton;
};


#endif
