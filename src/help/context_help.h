/* ---------------------------------------------------------------------
 * Definition of ContextHelp                              context_help.h
 * Context-sensitive help button
 * --------------------------------------------------------------------- 
 */

#ifndef __VK_CONTEXT_HELP_H
#define __VK_CONTEXT_HELP_H

#include <qptrdict.h>
#include <qtoolbutton.h>


/* class ContextHelpButton --------------------------------------------- */
class HandBook;
class ContextHelpButton: public QToolButton
{
	Q_OBJECT
public:
	ContextHelpButton( QWidget* parent, HandBook* book );
	~ContextHelpButton();
public slots:
  void mouseReleased();
};


/* class ContextHelp --------------------------------------------------- */
class ContextHelp: public QObject
{
	Q_OBJECT
public:
	ContextHelp();
	~ContextHelp();
  static void add( QWidget *, const QString &);
	static void setUp();

	enum State { Inactive, Waiting };
  /* just so we can return a pointer to a QString :( */
  struct UrlItem : public QShared {
		UrlItem() : QShared() { }
		~UrlItem();
		QString url;
	};

	bool eventFilter( QObject *, QEvent * );
	void newItem( QWidget * widget, const QString & text );
	//void say( QWidget *, const QString&, const QPoint&  );
	void say( QWidget *, const QString& );
	void shutDown();
  void remove( QWidget * );

	HandBook* hbook;   /* ptr to the application-wide handbook */
	QPtrDict<QWidget> * tlw;
	QPtrDict<UrlItem> * wdict;
	QPtrDict<ContextHelpButton> * buttons;
	State state;

private slots:
  void cleanupWidget();
};

#endif
