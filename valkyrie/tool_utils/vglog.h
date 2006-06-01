/* ---------------------------------------------------------------------
 * Definition of VgLog                                           vglog.h
 * VgLog: QDomDocument based representation of a valgrind xml log.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_VGLOG_H
#define __VK_VGLOG_H

#include <qdom.h>
#include <qlistview.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qstring.h>

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>


/**********************************************************************/
class VgElement : public QDomElement
{
public:
   VgElement getFirstElem( QString tagname ) const;
   VgElement getLastElem( QString tagname ) const;

   bool isLeaf();
   /* leaf functions */
   unsigned long toULong( bool* ok );
   bool setContent( QString content );
   QString getContent();
   bool updateCount( VgElement sElemNum );

   enum ElemType {     /* mapping of tags to enum values */
      ROOT, PROTOCOL, PREAMBLE, PID, PPID, TOOL, 
      LOGQUAL, VAR, VALUE, COMMENT,
      ARGS, VARGV, ARGV, EXE, ARG, 
      STATUS, STATE, TIME,
      ERROR, UNIQUE, TID, KIND, WHAT, STACK, 
      FRAME, IP, OBJ, FN, SRCDIR, SRCFILE, LINE, AUXWHAT,
      ERRORCOUNTS, PAIR, COUNT,
      SUPPCOUNTS, NAME, LEAKEDBYTES, LEAKEDBLOCKS,
      NUM_ELEMS
   };
   typedef QMap<QString, VgElement::ElemType> ElemTypeMap;
   static ElemTypeMap elemtypeMap;

   ElemType elemType();
};

/**********************************************************************/
class VgPreamble : public VgElement
{
public:
   QString toPlainTxt();
};

/**********************************************************************/
class VgStatus : public VgElement
{
public:
   enum StateType { INIT=0, RUNNING, FINISHED };
   StateType state();
};

/**********************************************************************/
class VgFrame : public VgElement
{
public:
   QString describe_IP( bool withPath=false );
   bool operator==( const VgFrame& frame ) const;
   bool operator!=( const VgFrame& frame ) const;
};

/**********************************************************************/
class VgError : public VgElement
{
public:
   bool isLeak();
   QString toPlainTxt();
   bool operator==( const VgError& err ) const;
   bool operator!=( const VgError& err ) const;
   bool updateLeakErr( VgError sErr );
   QString unique();

   /*
     Error type         <kind>               // Description
     =========================================================
     CoreMemErr:        CoreMemError         // unaddressable/uninitialised // ?
     ValueErr:          UninitCondition      // cond jump dep on uninit value(s)
                        UninitValue          // use of uninit value
     ParamErr:          SyscallParam         // syscall param error
     UserErr:           ClientCheck          // x byte(s) found during client check request
     FreeErr:           InvalidFree          // invalid free/delete/delete[]
     FreeMismatchErr:   MismatchedFree       // mismatched free/delete/delete[]
     AddrErr:           InvalidRead          // invalid read
                        InvalidWrite         // invalid write
                        InvalidJump          // jump to invalid address
     OverlapErr:        Overlap              // source and destination overlap
     LeakErr:           Leak_DefinitelyLost  // loss unreached
                        Leak_IndirectlyLost  // loss indirectLeak
                        Leak_PossiblyLost    // loss interior
                        Leak_StillReachable  // loss proper
     IllegalMempoolErr: InvalidMemPool       // illegal memory pool address
   */

   enum LeakKind { UNREACHED=0, INDIRECT, INTERIOR, PROPER, NUM_LKS };
   typedef QMap<QString, VgError::LeakKind> LeakKindMap;
   static LeakKindMap leakKindMap;
   LeakKind leakKind();
   static QString leakDesc( LeakKind lk );
};

typedef QValueList<VgError> VgErrorList;

/**********************************************************************/
class VgErrCounts : public VgElement
{
public:
   VgElement getCount( VgError err );
};

/**********************************************************************/
class VgSuppCounts : public VgElement
{
public:
};



/**********************************************************************/
/*
  class VgLog: a representation of a valgrind xml log
  based on QDomDocument node tree:
  - exactly represents logfile structure, including repeated leaks etc.
  (merging doesn't need this, but listview does)
  - after init(), provides valid log:
  - first initialised with xml insn and document tag
  - subsequently accepts only complete top-level xml chunks
  - can add/sub/move around nodes within log easily
  - element tags dealt with transparently
  - not so easy to merge, but easy to read/print
  - children nodes are found via get<First|Last>Elem( tagname )
  - not as fast as holding refs everywhere, but speed looks ok.

  merging:
  before merging, cleans both this and slave tree:
  - removes duplicate leaks, leakcounts, empty errorcounts

  usage example:
  VgLogReader reader( &vgLog );
  bool ok = reader.parse( file_path );

  intention is for vglogview to inherit this,
  and populate its listview in reimp. of appendChunk()...
*/
class VgLog
{
public:
   VgLog();
   virtual ~VgLog();

   bool init( QDomProcessingInstruction xml_insn, QString doc_tag );
   virtual bool appendNode( QDomNode e );

   QString toString( int indent=2 );   /* xml output */
   QString toPlainTxt();               /* plain text output */

   /* TODO: handle merge of incomplete logs:
      - errors with no errorcount, ... */
   bool merge( VgLog& slave );

   VgStatus::StateType state();

private:
   QString plainTxtErrorSummary();
   QString plainTxtLeakSummary();

   bool mergeErrors( VgErrorList sErrors, VgErrCounts sErrCounts );
   bool mergeSuppCounts( VgSuppCounts sSuppCounts );
   bool mergeLeakErrors( VgErrorList sLeakErrors );

public:
   VgElement    docroot();     /* document element: <valgrindoutput/> */
   VgElement    protocol();
   VgPreamble   preamble();
   VgElement    pid();
   VgElement    ppid();
   VgElement    tool();
   VgElement    logqual();
   VgElement    comment();
   VgElement    args();
   VgStatus     status_beg();
   VgErrorList  errors();      /* errors (non-leak) before status_end */
   VgErrCounts  errorcounts(); /* last errcounts element */
   VgStatus     status_end();
   VgSuppCounts suppcounts();
   VgErrorList  leaks();       /* leak errors after status_end */

private:  
   QDomDocument log;
};


/* Notes re xml weaknesses
   -----------------------
   * v. awkward that both leak and normal errors have same <error> tag
   - leak errors have additional children, which adds to pain in
   retrieving any one child of an error
   - poss to rename leak error xml as <leak>, and then tighten up both
   <leak> and <error> specifications ?

   * leak errors need their what string taken apart and put in xml fields:
   - record x of N: need 'x', to detect new record group
   - remove all numbers, just have "memory is definitely lost", or something.

   * < <?xml version="1.0"?>
   ---
   > <?xml version = '1.0'?>

   * < <logfilequalifier> <var>VAR</var> <value>$VAR</value> </logfilequalifier>
   ---
   > <logfilequalifier>
   >   <var>VAR</var>
   >   <value>$VAR</value>
   > </logfilequalifier>

   * indent everything after <valgrindoutput> by 2 spaces more

*/


#endif // #ifndef __VK_VGLOG_H
