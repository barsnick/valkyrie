/* --------------------------------------------------------------------- 
 * Definition of class Memcheck                        memcheck_object.h
 * Memcheck-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __MEMCHECK_OBJECT_H
#define __MEMCHECK_OBJECT_H


#include "tool_object.h"
#include "memcheck_view.h"
#include "xml_parser.h"
#include "memcheck_options_page.h"


/* class Memcheck ------------------------------------------------------ */
class Memcheck : public ToolObject
{
  Q_OBJECT
public:
  Memcheck();
  ~Memcheck();

  bool parseLogFile( bool checked=true );
  bool mergeLogFiles();
  bool run( QStringList flags );

  /* returns the ToolView window (memcheckView) for this tool */
  ToolView* createView( QWidget* parent );
  /* called by MainWin::closeToolView() */
  bool isDone();

  bool start( Valkyrie::RunMode rm );
  bool stop( Valkyrie::RunMode rm );

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum mcOpts { 
    PARTIAL,    FREELIST, LEAK_CHECK, LEAK_RES,
    SHOW_REACH, GCC_296,  ALIGNMENT,  STRLEN,
    LAST_CMD_OPT  = STRLEN
  };

  bool optionUsesPwr2( int optId ) {
    if (optId == ALIGNMENT) return true;
    return false;
  }

  OptionsPage* createOptionsPage( OptionsWindow* parent ) {
    return (OptionsPage*)new MemcheckOptionsPage( parent, this );
  }

  void saveParsedOutput( QString& fname );

public slots:
  void loadClientOutput( const QString&, int log_fd=-1 );

private slots:
  void parseOutput();
  void processDone();

private:
  friend class MemcheckView;

  /* overriding to avoid casting everywhere */
  MemcheckView* view() { return (MemcheckView*)m_view; }

  void emitRunning( bool );
  bool parseLog( QString logfile );
  QString validateFile( QString log_file, bool* ok );
  void statusMsg(  QString hdr, QString msg );

  void setupProc( bool init );
  void setupParser( bool init );
  bool setupFileStream( bool init );

private:
  XMLParser* xmlParser;
  QXmlSimpleReader reader;
  QXmlInputSource source;

  QFile logFile;
  QString saveFname;
  QTextStream logStream;
};


#endif
