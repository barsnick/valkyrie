/* ---------------------------------------------------------------------- 
 * Definition of OptionsWindow                           options_window.h
 * A small container class for each tool's options / flags 'pane'.
 * Not modal, so user can keep it open and change flags as they work.
 * ---------------------------------------------------------------------- 
 */

#ifndef __VK_OPTIONS_WINDOW_H
#define __VK_OPTIONS_WINDOW_H

#include <qdict.h>
#include <qlistbox.h>
#include <qmainwindow.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>

#include "options_page.h"


/* class Categories ---------------------------------------------------- */
class Categories : public QListBox
{
public:
	Categories( QWidget *parent );
	int categHeight();

private:
	int categht;
};



/* class CategItem ----------------------------------------------------- */
class CategItem : public QListBoxItem 
{
public:
  CategItem( QListBox * parent, OptionsPage * op,
						 const QString &text, int id );
  virtual int height( const QListBox * ) const;
  int catId() const;
  void setWidget( OptionsPage * op );
  OptionsPage * page() const;

protected:
  void paint( QPainter *p );

private:
  int catid;
  OptionsPage * optpage;
};



/* class OptionsWindow ------------------------------------------------- */
class OptionsWindow : public QMainWindow
{
  Q_OBJECT
public:
  OptionsWindow( QWidget* parent=0 );
  ~OptionsWindow();
	void showPage( int catid );

signals:
  void flagsChanged();

protected:
  void closeEvent( QCloseEvent * );
  void moveEvent( QMoveEvent * );

private slots:
  void accept();
  void reject();
  void apply();
  void modified();
	void resetDefaults();
  void categoryClicked( QListBoxItem * );

private:
	void adjustPosition();
  void setCategory( int catid );
  void addCategory( int cid, QString text );
  OptionsPage * mkOptionsPage( int catid );

private:
	int xpos, ypos;	  /* remember where user put the window */

  QString capt;
	QPushButton* applyButton;

  /* so we can iterate over the vkOptions widgets */
  QPtrList<OptionsPage> optPages;
  Categories* categories;
  QWidgetStack* wStack;
};


#endif
