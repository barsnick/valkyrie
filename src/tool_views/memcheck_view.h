/* ---------------------------------------------------------------------
 * Definition of MemcheckView                            memcheck_view.h
 * Memcheck's personal window
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __MEMCHECK_VIEW_H
#define __MEMCHECK_VIEW_H


#include "tool_view.h"
#include "xml_parser.h"

#include <qlistview.h>
#include <qtoolbutton.h>
#include <qtextedit.h>

/* base class for SrcItem and OutputItem ------------------------------- */
class XmlOutputItem : public QListViewItem
{
public:
  XmlOutputItem( QListView* parent );
  XmlOutputItem( QListViewItem* parent );
  XmlOutputItem( QListViewItem* parent, QListViewItem* after );

  void initialise();
  void setText( QString str );
  virtual XmlOutput::ItemType itemType() = 0;
  /* some make-life-simpler stuff */
  XmlOutputItem* firstChild();
  XmlOutputItem* nextSibling();
  XmlOutputItem* parent();
public:
  bool isReadable, isWriteable;
  XmlOutput* xmlOutput;
};



/* class OutputItem ---------------------------------------------------- 
   small subclass of QListViewItem to add some extra functionality.
   these are the items displayed in the listview. */
class OutputItem : public XmlOutputItem
{
public:
  /* top-level status item */
  OutputItem( QListView* parent, XmlOutput* output );
  /* used for: preamble:line, info:argv + args, supp counts */
  OutputItem( OutputItem* parent, OutputItem* after, QString txt );
  /* everybody else */
  OutputItem( OutputItem* parent, OutputItem* after, XmlOutput* output );

  XmlOutput::ItemType itemType();
  void setOpen( bool open );
  void paintCell( QPainter* p, const QColorGroup& cg,
                  int col, int width, int align );

  /* some make-life-simpler stuff */
  OutputItem* firstChild();
  OutputItem* nextSibling();
  OutputItem* parent();
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
   QPixmap* pix;
};


/* class MemcheckView -------------------------------------------------- */
class Memcheck;
class MemcheckView : public ToolView
{
  Q_OBJECT
public:
  MemcheckView( QWidget* parent, Memcheck* mc );
  ~MemcheckView();

  /* called by memcheck: set state for buttons; set cursor state */
  void setState( bool run );
  /* clear and reset the listview for a new run */
  void clear();

  void loadClientOutput( const QString& client_output, int log_fd );

public slots:
  void toggleToolbarLabels( bool );

  /* updates the top status item in the listview */
  void updateStatus();
  void loadItem( XmlOutput* output );
  void updateErrors( ErrCounts* ecounts );

private:
  void mkToolBar();
  /* overriding to avoid casting everywhere */
  Memcheck* tool() { return (Memcheck*)m_tool; }

private slots:
  void openLogFile();       /* load and parse one log file */
  void openMergeFile();     /* open and check a list of logfiles-to-merge */
  void saveLogFile();       /* called by savelogButton */

  void showSuppEditor();

  void itemSelected();
  void openAllItems(bool);
  void openOneItem();
  void showSrcPath();
  void launchEditor(  QListViewItem*, const QPoint&, int );

private:
  QString logFilename;

  QListView* lView;
  QTextEdit* stdout_tedit;
  QTextEdit* stderr_tedit;

  QToolButton* savelogButton;
  QToolButton* openlogButton;
  QToolButton* suppedButton;

  QToolButton* openOneButton;
  QToolButton* openAllButton;
  QToolButton* srcPathButton;

  QToolBar* mcToolBar;
};


#endif
