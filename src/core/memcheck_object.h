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

  void stop() { }

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum mcOpts { 
    FIRST_CMD_OPT = 43, 
    PARTIAL       = FIRST_CMD_OPT,
    FREELIST      = 44,
    LEAK_CHECK    = 45,
    LEAK_RES      = 46,
    SHOW_REACH    = 47,
    GCC_296       = 48,
    ALIGNMENT     = 49,
    STRLEN        = 50,
    LAST_CMD_OPT  = STRLEN
  };

private slots:
  void parseOutput();
  void saveParsedOutput();

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

  int log_fd;                   /* stdout=1 | stderr=2 | ... */
  QFile logFile;
  QString save_fname;
  QTextStream logStream;
};


#endif
