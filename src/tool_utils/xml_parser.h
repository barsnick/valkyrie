/* ---------------------------------------------------------------------
 * Definition of XMLParser                                  xml_parser.h
 * Subclass of QXmlDefaultHandler for parsing memcheck-specific xml output.
 * Also contains various small classes encapsulating the different
 * chunks of xml output.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_XML_PARSER_H
#define __VK_XML_PARSER_H

#include <qmap.h>
#include <qobject.h>
#include <qptrstack.h>
#include <qxml.h>
#include <qvaluelist.h>     // class Pair (ErrCounts + SuppCounts)


/* class XmlOutput -----------------------------------------------------
   base class for all memcheck xml output */
class XmlOutput
{
public:
  enum ItemType { 
    PREAMBLE=0, STATUS, INFO, SUPP_COUNTS, ERR_COUNTS, ERROR, 
    LEAK_ERROR, STACK, FRAME,
    SRC,   /* SRC is only used by SrcItem to return its itemType */
    AUX    /* AUX is only used by OutputItem to return its itemType */
  };

  XmlOutput( ItemType itype );
  virtual ~XmlOutput() { }

  QString displayString();
  virtual void printDisplay() { }

public:
  ItemType itemType;
  QString  display;
};

/* class Preamble ------------------------------------------------------ */
class Preamble : public XmlOutput
{
public:
  Preamble();
  ~Preamble();

public:
  QStringList lines;
};


/* class TopStatus -----------------------------------------------------
   top line in listview: shows current status of parsing, eg:
   Valgrind: RUNNING './xml1'  Errors: 999  Leaked bytes: 999 in 9 blocks */
class TopStatus : public XmlOutput
{
public:
  TopStatus();
  void printDisplay();

public:
  int num_errs, num_leaks, num_blocks;
  QString state, time, object;
};


/* class Info ---------------------------------------------------------- 
   Eg: Memcheck output for process id ==31084== (parent pid ==2426==) */
class Info : public XmlOutput
{
public:
  Info();
  void printDisplay();

public:
  int protocolVersion;
  int pid, ppid;
  QString tool;
  QString exe;
  QString startStatus, endStatus;
  QString startTime, endTime;
  QStringList vgInfoList;  /* valgrind args */
  QStringList exInfoList;  /* executable args */
};



/* class Pair: used by ErrCounts and SuppCounts ------------------------ */
class Pair
{
public:
  Pair() : number(-1), data("") { }
  Pair( int num, QString str ) : number(num), data(str) { }
  int number;
  QString data;
};


/* class ErrCounts ----------------------------------------------------- */
class ErrCounts : public XmlOutput
{
public:
  ErrCounts();
  ~ErrCounts();

  bool isEmpty();
  void appendPair( QString str );
  void appendPair( int val, QString str );
  void appendList( ErrCounts* ec );
  void print2File( QTextStream& stream );
  /* returns the 'count' value associated with the 'uniq' string */
  int findUnique( QString uniq );
  /* updates the 'count' value associated with the 'uniq' string */
  bool updateCount( int count, QString uniq );
  /* if 'count' and 'uniq' match, remove the pair from the list */
  void remove( int count, QString uniq );

public:
  int num;
  int totalErrors;

  typedef QValueList<Pair> PairList;
  PairList pairList;
};


/* class SuppCounts ---------------------------------------------------- */
class SuppCounts : public XmlOutput
{
public:
  SuppCounts();
  ~SuppCounts();

  void appendPair( QString str );
  void appendPair( int val, QString str );
  void updateList( SuppCounts* sc );
  void printDisplay();
  void print2File( QTextStream& stream );
  /* returns the 'count' value associated with the 'name' string */
  int findName( QString name );
  /* updates the 'count' value associated with the 'name' string */
  bool updateCount( int count, QString name );

public:
  int num;
  QStringList supps;   /* for printing display output */

  typedef QValueList<Pair> PairList;
  PairList pairList;
};


/* class Frame --------------------------------------------------------- */
class Frame : public XmlOutput
{
public:
  Frame( bool top_frame );
  ~Frame();
  void printDisplay();
  void printPath();
  void print2File( QTextStream& stream );

  void setObj( QString str );
  void setFun( QString str );
  void setDir( QString str );
  void setFile( QString str );
  void setLine( int no );

public:
  int lineno;
  bool pathPrinted, readable, writeable;
  /* Note: the only tag guaranteed to never be empty is 'ip' */
  QString at_by, ip, obj, fn, srcfile, srcdir, filepath;
  bool haveLine, haveFile, haveDir, haveFunc, haveObj;
};


/* class Stack --------------------------------------------------------- */
class Stack : public XmlOutput
{
public:
  Stack( bool top );
  ~Stack();
  void mkFrame();
  void print2File( QTextStream& stream );

public:
  Frame* currFrame;
  bool is_first_stack;
  QPtrList<Frame> frameList;
};


/* class Error --------------------------------------------------------- */
class Error : public XmlOutput
{
public:
  Error();
  ~Error();
  void mkStack();
  void printDisplay();
  void print2File( QTextStream& stream );

  void setAux( QString str );
  /* if leakedbytes or leakedblocks is set, 
     then this is a leak error as opposed to a 'normal' error */
  void setLeakedBytes( int bytes );
  void setLeakedBlocks( int blocks );

public:
  int tid, num_times;
  int leakedBytes, leakedBlocks;     /* leak errors only */
  QString unique, kind, what, acnym;
  bool haveAux;
  QString auxwhat;
  Stack* currStack;           /* the stack we are currently working with */
  QPtrList<Stack> stackList;  /* list of stacks */
};




/* class XMLParser ----------------------------------------------------- */
class XMLParser : public QObject, public QXmlDefaultHandler
{
  Q_OBJECT
public:
  XMLParser( QObject* parent=0, bool parse_ents=false );
  ~XMLParser();

  void reset( bool reinit=true );
  bool startElement( const QString&, const QString&, const QString& stag, 
                     const QXmlAttributes& );
  bool endElement( const QString&, const QString&, const QString& etag );
  bool characters( const QString& content );

signals:
  void loadItem( XmlOutput * );
  void updateErrors( ErrCounts* );
  void updateStatus();
  void loadClientOutput( const QString& );

private:
  /* if the output is being displayed in a listview, then don't bother
     to parse 'content' for '<', '>' and '&'.  but if we are going to
     print to file, or if we are comparing 'content' with a view to
     merging errors, then do bother (sigh) */
  bool escEntities;

  /* mapping of error::kind to 3-letter acronyms */
  typedef QMap<QString, QString> AcronymMap;
  AcronymMap acronymMap;
  QString acronym( QString kind );

  enum TagType {     /* mapping of tags to enum values */
    NONE=0, VGOUTPUT, PROTOCOL, PREAMBLE, PID, PPID, TOOL, 
    ARGS, VARGV, ARGV, EXE, ARG, 
    STATUS, STATE, TIME,
    ERROR, UNIQUE, TID, KIND, WHAT, STACK, FRAME, IP, OBJ, FN, 
    SRCDIR, SRCFILE, LINE, AUXWHAT, ERRORCOUNTS, PAIR, COUNT, SUPPCOUNTS, 
    NAME, LEAKEDBYTES, LEAKEDBLOCKS 
  };
  typedef QMap<QString, TagType> TagTypeMap;
  TagTypeMap tagtypeMap;

  TagType tagType( QString tag );

private:
  TopStatus* topStatus;        /* for displaying status etc. */
  Preamble* preamble;          /* the preamble */
  Info* info;                  /* info re tool / pid / ppid  */
  Error* verror;
  ErrCounts*  errCounts;       /* errorcounts  */
  SuppCounts* suppCounts;      /* suppcounts   */

  QString content;             /* for storing output */

  /* bools so we know where we are and what we're doing */
  bool inPreamble, inVargV, inError, inStack, inFrame;
  bool inErrorCounts, inSuppCounts, inPair;
  bool statusPopped;

  /* this is completely pathetic: it's only here because J insists on
     the output order being one way, but I want to display it another
     way.  so we have to push three (count 'em, 3) items onto the
     stack to get the ordering correct (bitch, moan, complain) */
  QPtrStack<XmlOutput> stack;
};



#endif
