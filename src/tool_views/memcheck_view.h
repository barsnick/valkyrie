/* ---------------------------------------------------------------------
 * definition of MemcheckView                            memcheck_view.h
 * ---------------------------------------------------------------------
 */

#ifndef __VK_MEMCHECK_VIEW_H
#define __VK_MEMCHECK_VIEW_H

#include "tool_view.h"
#include "xml_parser.h"

#include <qlistview.h>
#include <qtoolbutton.h>

/* base class for SrcItem and OutputItem ------------------------------- */
class XmlOutputItem : public QListViewItem
{
public:
  XmlOutputItem( QListView * parent );
  XmlOutputItem( QListViewItem* parent );
  XmlOutputItem( QListViewItem* parent, QListViewItem* after );

  void initialise();
  void setText( QString str );
  virtual XmlOutput::ItemType itemType() = 0;
  /* some make-life-simpler stuff */
  XmlOutputItem * firstChild();
  XmlOutputItem * nextSibling();
  XmlOutputItem * parent();
public:
  bool isReadable, isWriteable;
  XmlOutput * xmlOutput;
};



/* class OutputItem ---------------------------------------------------- 
   small subclass of QListViewItem to add some extra functionality.
   these are the items displayed in the listview. */
class OutputItem : public XmlOutputItem
{
public:
  /* top-level status item */
  OutputItem( QListView* parent, XmlOutput * output );
  /* used for: preamble:line, info:argv + args, supp counts */
  OutputItem( OutputItem* parent, OutputItem* after, QString txt );
  /* everybody else */
  OutputItem( OutputItem* parent, OutputItem* after, XmlOutput * output );

  XmlOutput::ItemType itemType();
  void setOpen( bool open );
  void paintCell( QPainter* p, const QColorGroup& cg,
                  int col, int width, int align );

  /* some make-life-simpler stuff */
  OutputItem * firstChild();
  OutputItem * nextSibling();
  OutputItem * parent();
};


/* class SrcItem ------------------------------------------------------- 
   teeny subclass so we can paint a pale gray background colour.
   shows a 'read' pixmap if the user has read perms, or an a 'write'
   pixmap if the user has write perms for the source file.  if the
   pixmap is clicked, launches an editor with the source file loaded. */
class SrcItem : public XmlOutputItem
{
public:
  SrcItem( OutputItem* parent, QString txt );
  ~SrcItem();

  void setPixmap( QString pix_file );
  const QPixmap* pixmap( int i ) const;
  void paintCell( QPainter* p, const QColorGroup& cg,
                  int col, int width, int align );
  XmlOutput::ItemType itemType();
  void setReadWrite( bool read, bool write );

private:
   QPixmap * pix;
};


/* class MemcheckView -------------------------------------------------- */
class MemcheckView : public ToolView
{
  Q_OBJECT
public:
  MemcheckView( QWidget* parent, VkObject* obj );
  ~MemcheckView();

  bool run();
  void stop();
  void clear();

  /* updates the top status item in the listview */
  void updateStatus();
  void loadItem( XmlOutput * output );
  void updateErrors( Counts * counts );

public slots:
  void parseXmlOutput();
  void processExited();
  void toggleToolbarLabels(bool);

private:
  enum ParseFormat{ INVALID=-1, NOT_SET=0, TEXT, XML };
  ParseFormat inputFormat;
  ParseFormat validateLogFile();
  ParseFormat validateLogFile( const QString& );
  ParseFormat validateOutput();

	void setRunning( bool b );
  void parseXmlLog();
  void saveLogfile( QString fname );
  void mkMenuBar();

private slots:
	void openLogFile();       /* load and parse one log file */
  void openLogFiles();      /* load and parse multiple log files */
  void getSaveFilename();   /* called by savelogButton */
  void showSuppEditor();
  void itemSelected();
  void openAllItems(bool);
  void openOneItem();
  void showSrcPath();
  void launchEditor(  QListViewItem*, const QPoint&, int );

private:
  QString inputData;
	int lineNumber;
	QString logFilename;
	QFile logFile;
	QTextStream logStream;

  QListView * lView;
  XMLParser * xmlParser;
  QXmlSimpleReader reader;
  QXmlInputSource source;

  Valkyrie* valkyrie;

  QToolButton* savelogButton;
  QToolButton* openlogButton;
	QToolButton* suppedButton;

  QToolButton* openOneButton;
  QToolButton* openAllButton;
  QToolButton* srcPathButton;
};


#endif
