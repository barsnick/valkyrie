/* ---------------------------------------------------------------------
 * definition of XMLParser                                  xml_parser.h
 * ---------------------------------------------------------------------
 * Subclass of QXmlDefaultHandler for parsing memcheck-specific xml output.
 *
 * Also contains various small classes encapsulating the different
 * chunks of xml output.
 */

#ifndef __VK_XML_PARSER_H
#define __VK_XML_PARSER_H

#include <qmap.h>
#include <qptrstack.h>
#include <qxml.h>


/* class XmlOutput -----------------------------------------------------
   base class for all memcheck xml output */
class XmlOutput
{
public:
  enum ItemType { 
    PREAMBLE=0, STATUS=1, INFO=2, COUNTS=3, ERROR=4, STACK=5, FRAME=6,
    SRC=7, /* SRC is only used by SrcItem to return its itemType */
    AUX=8  /* AUX is only used by OutputItem to return its itemType */
  };

  XmlOutput( ItemType itype );
  ~XmlOutput();
  QString displayString();

public:
  ItemType itemType;
  QString display;
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
   top line in listview: shows current status of parsing:
   Valgrind: RUNNING './xml1'  Errors: 999  Leaked bytes: 999 in 9 blocks */
class TopStatus : public XmlOutput
{
public:
  TopStatus();
  void print();

public:
  int num_errs, num_leaks, num_blocks;
  QString tmp, status, object;
};


/* class Info ---------------------------------------------------------- 
   Memcheck output for process id ==31084== (parent pid ==2426==) */
class Info : public XmlOutput
{
public:
  Info();
  void print();

public:
  int pid, ppid;
  QString tool;
  QStringList infoList;
};


/* class Counts -------------------------------------------------------- */
class Counts : public XmlOutput
{
public:
  Counts();
  ~Counts();

  bool isEmpty();
  void update();
  int find( QString id );
  int count();
  bool contains( QString id );
  void print();

public:
  int value;
  int total_errors;
  QString key;
  typedef QMap<QString,int> CountsMap;
  CountsMap countsMap;
  QStringList supps;
};


/* class Frame --------------------------------------------------------- */
class Frame : public XmlOutput
{
public:
  Frame( bool top_frame );
  ~Frame();
  void print();
  void printPath();

public:
  int lineno;
  bool pathPrinted, readable, writeable;
  QString at_by, ip, obj, fn, srcfile, srcdir, filepath;
};


/* class Stack --------------------------------------------------------- */
class Stack : public XmlOutput
{
public:
  Stack( bool top );
  ~Stack();
  void mkFrame();

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
  void print();

public:
  int tid, num_times, leakedbytes, leakedblocks;
  QString unique, kind, what, acnym;
  QString auxwhat;
  Stack* stack1;
  Stack* stack2;      /* optional */
  Stack* currStack;   /* the stack we are currently working with */
};




/* class XMLParser ----------------------------------------------------- */
class MemcheckView;

class XMLParser : public QXmlDefaultHandler
{
public:
  XMLParser( MemcheckView* parent );
  ~XMLParser();

  void reset( bool reinit=true );
  bool startElement( const QString&, const QString&, const QString& stag, const QXmlAttributes& );
  bool endElement( const QString&, const QString&, const QString& etag );
  bool characters( const QString& content );

private:
  /* mapping of error::kind to 3-letter acronyms */
  typedef QMap<QString, QString> AcronymMap;
  AcronymMap acronymMap;
  QString acronym( QString kind );

  enum TagType {     /* mapping of tags to enum values */
    NONE=0, VGOUTPUT, PROTOCOL, PREAMBLE, PID, PPID, TOOL, ARGV, EXE, ARG, 
    STATUS, ERROR, UNIQUE, TID, KIND, WHAT, STACK, FRAME, IP, OBJ, FN, 
    SRCDIR, SRCFILE, LINE, AUXWHAT, ERRORCOUNTS, PAIR, COUNT, SUPPCOUNTS, 
    NAME, LEAKEDBYTES, LEAKEDBLOCKS };
  typedef QMap<QString, TagType> TagTypeMap;
  TagTypeMap tagtypeMap;

  TagType tagType( QString tag );

private:
  MemcheckView* memView;       /* ptr to parent              */
  TopStatus* topStatus;        /* for displaying status etc. */
  Info* info;                  /* info re tool / pid / ppid  */
  Preamble* preamble;          /* the preamble               */
  Counts* counts;              /* errorcounts || suppcounts  */
  Error* verror;
  QPtrList<Error> errorList;   /* list of errors             */

  /* for storing output */
  QString startTag, endTag, content;

  /* bools so we know where we are and what we're doing */
  bool inPreamble, inError, inStack, inFrame;
  bool inErrorCounts, inSuppCounts, inPair;
  bool statusPopped;

  /* this is completely pathetic: it's only here because J insists on
     the output order being one way, but I want to display it another
     way. So we have to push three (count 'em, 3) items onto the
     stack to get the ordering correct (bitch, moan, complain) */
  QPtrStack<XmlOutput> stack;
};



#endif
