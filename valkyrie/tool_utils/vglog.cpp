/* ---------------------------------------------------------------------
 * Implementation of VgLog                                     vglog.cpp
 * VgLog: QDomDocument based representation of a valgrind xml log.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vglog.h"

#include <qtextstream.h>
#include <assert.h>

/* different implementations for valkyrie/vk_logmerge */
extern void vklmPrint( int verb, const char* msg, ... )
   __attribute__((format (printf, 2, 3)));
extern void vklmPrintErr( const char* msg, ... )
   __attribute__((format (printf, 1, 2)));


/**********************************************************************/
/*
  setup static class maps
  - avoids setting up map in class cons, but keeps map where it belongs
*/
VgElement::ElemTypeMap setupElemTypeMap() {
   VgElement::ElemTypeMap etmap;
   etmap["valgrindoutput"]   = VgElement::ROOT;
   etmap["protocolversion"]  = VgElement::PROTOCOL;
   etmap["preamble"]         = VgElement::PREAMBLE;
   etmap["pid"]              = VgElement::PID;
   etmap["ppid"]             = VgElement::PPID;
   etmap["tool"]             = VgElement::TOOL;
   etmap["logfilequalifier"] = VgElement::LOGQUAL;
   etmap["var"]              = VgElement::VAR;
   etmap["value"]            = VgElement::VALUE;
   etmap["usercomment"]      = VgElement::COMMENT;
   etmap["args"]             = VgElement::ARGS;
   etmap["vargv"]            = VgElement::VARGV;
   etmap["argv"]             = VgElement::ARGV;
   etmap["exe"]              = VgElement::EXE;
   etmap["arg"]              = VgElement::ARG;
   etmap["status"]           = VgElement::STATUS;
   etmap["state"]            = VgElement::STATE;
   etmap["time"]             = VgElement::TIME;
   etmap["error"]            = VgElement::ERROR;
   etmap["unique"]           = VgElement::UNIQUE;
   etmap["tid"]              = VgElement::TID;
   etmap["kind"]             = VgElement::KIND;
   etmap["what"]             = VgElement::WHAT;
   etmap["stack"]            = VgElement::STACK;
   etmap["frame"]            = VgElement::FRAME;
   etmap["ip"]               = VgElement::IP;
   etmap["obj"]              = VgElement::OBJ;
   etmap["fn"]               = VgElement::FN;
   etmap["dir"]              = VgElement::SRCDIR;
   etmap["file"]             = VgElement::SRCFILE;
   etmap["line"]             = VgElement::LINE;
   etmap["auxwhat"]          = VgElement::AUXWHAT;
   etmap["errorcounts"]      = VgElement::ERRORCOUNTS;
   etmap["pair"]             = VgElement::PAIR;
   etmap["count"]            = VgElement::COUNT;
   etmap["suppcounts"]       = VgElement::SUPPCOUNTS;
   etmap["name"]             = VgElement::NAME;
   etmap["leakedbytes"]      = VgElement::LEAKEDBYTES;
   etmap["leakedblocks"]     = VgElement::LEAKEDBLOCKS;
   return etmap;
}
VgElement::ElemTypeMap VgElement::elemtypeMap = setupElemTypeMap();

VgError::LeakKindMap setupLeakKindMap() {
   VgError::LeakKindMap lkmap;
   lkmap["Leak_DefinitelyLost"] = VgError::UNREACHED;
   lkmap["Leak_IndirectlyLost"] = VgError::INDIRECT;
   lkmap["Leak_PossiblyLost"]   = VgError::INTERIOR;
   lkmap["Leak_StillReachable"] = VgError::PROPER;
   return lkmap;
}
VgError::LeakKindMap VgError::leakKindMap = setupLeakKindMap();




/**********************************************************************/
/*
  Search direct children for first/last element of 'tagname'
*/
VgElement VgElement::getFirstElem( QString tagname ) const
{
   //  vklmPrintErr("getFirstElem(): %s", tagname.latin1());

   QDomElement e;
   for ( e = firstChild().toElement(); !e.isNull();
         e = e.nextSibling().toElement() ) {
      //    vklmPrintErr(" - %s",e.tagName().latin1());
      if (e.tagName() == tagname)
         return (VgElement&)e;
   }
   return VgElement();
}

VgElement VgElement::getLastElem( QString tagname ) const
{
   QDomElement e;
   for ( e = lastChild().toElement(); !e.isNull();
         e = e.previousSibling().toElement() ) {
      if (e.tagName() == tagname)
         return (VgElement&)e;
   }
   return VgElement();
}


VgElement::ElemType VgElement::elemType()
{
   ElemTypeMap::Iterator it = elemtypeMap.find( tagName() );
   if ( it == elemtypeMap.end() )
      return VgElement::NUM_ELEMS;
   return it.data(); 
}


/*
  Convert leaf element content to integer.
  Returns 0 if conversion fails
*/
unsigned long VgElement::toULong( bool* ok )
{
   if (!isLeaf()) {
      *ok = false;
      return 0;
   }
   QDomText textNode = firstChild().toText();
   if (textNode.isNull()) {
      *ok = false;
      return 0;
   }
   QString numStr = textNode.data();
   unsigned long num = numStr.toULong(ok);
   if (!ok) {
      *ok = false;
      return 0;
   }
   return num;
}


/*
  Sets content for a leaf element
*/
bool VgElement::setContent( QString content )
{
   if (!isLeaf()) return false;
   QDomText textNode = firstChild().toText();
   if (textNode.isNull())
      return false;
   textNode.setData( content );
   return true;
}


/*
  Gets content for a leaf element
*/
QString VgElement::getContent()
{
   if (!isLeaf()) return "";
   QDomText textNode = firstChild().toText();
   if (textNode.isNull())
      return "";
   return textNode.data();
}


/*
  add slave elem count to this count
*/
bool VgElement::updateCount( VgElement sElemNum )
{
   if (!isLeaf()) return false;
   bool ok;
   unsigned long mNum = toULong( &ok );
   if (!ok) return false;
   unsigned long sNum = sElemNum.toULong( &ok );
   if (!ok) return false;
   if (!setContent( QString::number( mNum + sNum ) ) )
      return false;
   return true;
}

bool VgElement::isLeaf()
{
   return ( (childNodes().count() == 1) &&
            (firstChild().childNodes().count() == 0) );
}


/**********************************************************************/
QString VgPreamble::toPlainTxt()
{
   QString output_str;
   for ( QDomNode n=firstChild(); !n.isNull(); n=n.nextSibling() ) {
      output_str += n.toElement().text() + "\n";    
   }
   return output_str;
}


/**********************************************************************/
/* "", "RUNNING", "FINISHED" */
VgStatus::StateType VgStatus::state()
{
   QDomElement n_state = firstChild().toElement();
   if (n_state.isNull())
      return INIT;
   if (n_state.text() == "RUNNING")
      return RUNNING;
   return FINISHED;
}


/**********************************************************************/
bool VgFrame::operator==( const VgFrame& frame2 ) const
{
   QDomNodeList frame_details1 = childNodes();
   assert( frame_details1.count() >= 1 );  /* only ip guaranteed */
   QDomElement iptr1 = frame_details1.item( 0 ).toElement();
   QDomElement objt1 = frame_details1.item( 1 ).toElement();
   QDomElement func1 = frame_details1.item( 2 ).toElement();
   QDomElement diry1 = frame_details1.item( 3 ).toElement();
   QDomElement file1 = frame_details1.item( 4 ).toElement();
   QDomElement line1 = frame_details1.item( 5 ).toElement();

   QDomNodeList frame_details2 = frame2.childNodes();
   assert( frame_details2.count() >= 1 );  /* only ip guaranteed */
   QDomElement iptr2 = frame_details2.item( 0 ).toElement();
   QDomElement objt2 = frame_details2.item( 1 ).toElement();
   QDomElement func2 = frame_details2.item( 2 ).toElement();
   QDomElement diry2 = frame_details2.item( 3 ).toElement();
   QDomElement file2 = frame_details2.item( 4 ).toElement();
   QDomElement line2 = frame_details2.item( 5 ).toElement();

   /* a frame may be the 'same' as another even if it is
      missing some data - do the best comparison we can */
  
   /* only field guaranteed to be present is 'ip'.
      comparing 'ip' is dodgy, so we try our hardest not to. */
  
   /* A: test fields: 'dir', 'file', 'line' */
   if ( !diry1.isNull() && !diry2.isNull() &&
        !file1.isNull() && !file2.isNull() &&
        !line1.isNull() && !line2.isNull() ) {
      vklmPrint( 3, "frame test A: dir, file, line" );
      if (diry1.text() != diry2.text() ||
          file1.text() != file2.text() ||
          line1.text() != line2.text())
         return false;
      return true;
   }
  
   /* B: test fields: 'file', 'line' */
   if ( !file1.isNull() && !file2.isNull() &&
        !line1.isNull() && !line2.isNull() ) {
      vklmPrint( 3, "frame test B: file, line" );
      if (file1.text() != file2.text() ||
          line1.text() != line2.text())
         return false;
      return true;
   }
  
   /* C: test fields: 'fn', 'ip' */
   if ( !func1.isNull() && !func2.isNull() ) {
      vklmPrint( 3, "frame test C: func, ip" );
      if (func1.text() != func2.text() ||
          iptr1.text() != iptr2.text())
         return false;
      return true;
   }
  
   /* D: test field: 'ip' */
   vklmPrint( 3, "frame test D: ip only" );
   if (iptr1.text() != iptr2.text())
      return false;
   return true;
}


bool VgFrame::operator!=( const VgFrame& frame2 ) const
{
   return !operator==(frame2);
}


/* ref: coregrind/m_debuginfo/symtab.c :: VG_(describe_IP) */
/* CAB: why don't print dirname for non-xml ? */
QString VgFrame::describe_IP( bool withPath/*=false*/ )
{
   QDomNodeList frame_details = childNodes();
   assert( frame_details.count() >= 1 );  /* only ip guaranteed */
   QDomElement ip     = frame_details.item( 0 ).toElement();
   QDomElement obj    = frame_details.item( 1 ).toElement();
   QDomElement fn     = frame_details.item( 2 ).toElement();
   QDomElement dir    = frame_details.item( 3 ).toElement();
   QDomElement srcloc = frame_details.item( 4 ).toElement();
   QDomElement line   = frame_details.item( 5 ).toElement();

   bool  know_fnname  = !fn.isNull();
   bool  know_objname = !obj.isNull();
   bool  know_srcloc  = !srcloc.isNull() && !line.isNull();
   bool  know_dirinfo = !dir.isNull();

   QString str = ip.text() + ": ";
   if (know_fnname) {
      str += fn.text();
      if (!know_srcloc && know_objname)
         str += " (in " + obj.text() + ")";
   } else if (know_objname && !know_srcloc) {
      str += "(within " + obj.text() + ")";
   } else {
      str += "???";
   }
   if (know_srcloc) {
      QString path;
      if ( withPath && know_dirinfo )
         path = dir.text() + "/";
      path += srcloc.text();
      str += " (" + path + ":" + line.text() + ")";
   }
   return str;
}


/**********************************************************************/
bool VgError::isLeak()
{
   QString kind = getFirstElem( "kind" ).text();
   return kind.startsWith( "Leak_" );
}


QString VgError::toPlainTxt()
{
   if (firstChild().isNull())
      return "";

   QString output_str;
   QDomNode n = firstChild();
   for ( QDomNode n=firstChild(); !n.isNull(); n=n.nextSibling() ) {
      QDomElement e = n.toElement();
      if (e.tagName() == "what") {
         output_str += e.text() + "\n";
      }
      else if (e.tagName() == "stack") {
         QDomNode frame = e.firstChild();
         for ( ; !frame.isNull(); frame=frame.nextSibling() ) {
            QString ip_desc = ((VgFrame&)frame).describe_IP();
            QString pre = (frame == e.firstChild()) ? "   at " : "   by ";
            output_str += pre + ip_desc + "\n";
         }
      }
      else if (e.tagName() == "auxwhat") {
         output_str += "  " + e.text() + "\n";
      }
   }
   output_str += "\n";
   return output_str;
}


bool VgError::operator==( const VgError& err2 ) const
{
#define MAX_FRAMES_COMPARE 4

   QDomElement kind1 = getFirstElem( "kind" );
   QDomElement kind2 = err2.getFirstElem( "kind" );

   /* test #1: is the 'kind' the same */
   if (kind1.text() != kind2.text()) {
      vklmPrint( 3, "=> different error kind" );
      return false;
   }

   /* - one stack per error */
   QDomElement stack1 = getFirstElem( "stack" );
   QDomElement stack2 = err2.getFirstElem( "stack" );

   /* - one or more frames per stack */
   QDomNodeList framelist1 = stack1.childNodes();
   QDomNodeList framelist2 = stack2.childNodes();

   /* test #2: error stacks have same num frames,
      if either < MAX_FRAMES_COMPARE */
   if ( (framelist1.count() < MAX_FRAMES_COMPARE ||
         framelist2.count() < MAX_FRAMES_COMPARE) &&
        framelist1.count() != framelist2.count()) {
      vklmPrint( 3, "=> different number of frames" );
      return false;
   }

   /* test #3: all stack frames the same,
      to MAX_FRAMES_COMPARE */
   unsigned int i;
   for (i=0; i<framelist1.count() && i<MAX_FRAMES_COMPARE; i++) {
      QDomElement frame1 = framelist1.item(i).toElement();
      QDomElement frame2 = framelist2.item(i).toElement();
      if ( (VgFrame&)frame1 != (VgFrame&)frame2 ) {
         vklmPrint( 3, "=> stack comparison failed" );
         return false;
      }
   }
  
   return true;
}


bool VgError::operator!=( const VgError& frame2 ) const
{
   return !operator==(frame2);
}


bool VgError::updateLeakErr( VgError sErr )
{
   if ( !isLeak() ) {
      return false;
   }

   VgElement mElemBytes = getFirstElem( "leakedbytes" );
   VgElement sElemBytes = sErr.getFirstElem( "leakedbytes" );
   if ( ! mElemBytes.updateCount( sElemBytes ) )
      return false;

   VgElement mElemBlocks = getFirstElem( "leakedblocks" );
   VgElement sElemBlocks = sErr.getFirstElem( "leakedblocks" );
   if ( ! mElemBlocks.updateCount( sElemBlocks ) )
      return false;

   /* update leak::what
      - don't try to parse the current 'what' strings,
      as not intended for machine consumption.
      - just re-create a what string of our own.
      (not distinguishing direct/indirect errors)
   */
   bool ok;
   unsigned long mBytes  = mElemBytes.toULong( &ok );
   if (!ok) return false;
   unsigned long mBlocks = mElemBlocks.toULong( &ok );
   if (!ok) return false;

   QString mWhatStr = QString("%1 bytes in %2 blocks are %3")
      .arg( mBytes )
      .arg( mBlocks )
      .arg( leakDesc( leakKind() ) );
  
   VgElement mWhat = getFirstElem( "what" );
   mWhat.setContent( mWhatStr );
   return true;
}


QString VgError::unique()
{ return firstChild().toElement().text(); }


/* retrieve count for given error */
VgElement VgErrCounts::getCount( VgError err )
{
   VgElement elem;
   QString err_unique = err.unique();

   QDomNodeList pairs = childNodes();
   for (unsigned int i=0; i<pairs.count(); i++) {
      QDomNode pair = pairs.item(i);
      QDomNodeList pair_details = pair.childNodes();
      QString unique = pair_details.item(1).toElement().text();
      if (err_unique == unique ) {
         QDomElement el = pair_details.item(0).toElement();
         elem = (VgElement&)el;
      }
   }
   return elem;
}


VgError::LeakKind VgError::leakKind()
{
   QString kind = getFirstElem("kind").text();
   LeakKindMap::Iterator it = leakKindMap.find( kind );
   if ( it == leakKindMap.end() ) {
      vklmPrintErr("VgError::leakKind(): bad kind; %s", kind.latin1());
      return VgError::NUM_LKS;
   }
   return it.data(); 
}


QString VgError::leakDesc( LeakKind lk )
{
   switch( lk ) {
   case UNREACHED: return "definitely lost";
   case INDIRECT:  return "indirectly lost";
   case INTERIOR:  return "possibly lost";
   case PROPER:    return "still reachable";
   default:        return "";
   }
}





/**********************************************************************/
/*
  VgLog: a representation of a valgrind xml log based on QDomDocument.
*/
VgLog::VgLog()
{ }

VgLog::~VgLog()
{ }

bool VgLog::init( QDomProcessingInstruction xml_insn, QString doc_tag )
{
   if ( xml_insn.isNull() || doc_tag.isEmpty() )
      return false;

   QString init_str;
   QTextStream strm( init_str, IO_WriteOnly );
   xml_insn.save( strm, 2 );
   strm << "<" << doc_tag << "/>";

   log.setContent( init_str );
   return true;
}


/*
  append top-level xml element to log
*/
bool VgLog::appendNode( QDomNode node )
{
   if (log.isNull() || docroot().isNull()) {
      vklmPrintErr("VgLog::appendNode(): log not initialised");
      return false;
   }

   QDomElement e = node.toElement();
   if (e.isNull()) {
      vklmPrintErr("VgLog::appendNode(): node not an element");
      return false;
   }

   VgElement elem = (VgElement&)e;

   /* check elem is a top-level xml chunk */

   VgElement::ElemType type = elem.elemType();
   if ( type == VgElement::NUM_ELEMS ) {
      vklmPrintErr("VgLog::appendNode(): unrecognised tagname: %s", elem.tagName().latin1());
      return false;
   }

   switch ( type ) {
   case VgElement::PROTOCOL:
      if ( elem.text() != "1" && elem.text() != "2" ) {
         vklmPrintErr("VgLog::appendNode(): bad xml protocol version");
         return false;
      }
      break;
   case VgElement::PREAMBLE:
   case VgElement::PID:
   case VgElement::PPID:
   case VgElement::TOOL:
   case VgElement::LOGQUAL:
   case VgElement::COMMENT:
   case VgElement::ARGS:
   case VgElement::STATUS:
   case VgElement::ERROR:
   case VgElement::ERRORCOUNTS:
   case VgElement::SUPPCOUNTS:
      break;
   default:
      vklmPrintErr("VgLog::appendNode(): not a top-level tagname: %s", elem.tagName().latin1());
      return false;
      break;
   }

   docroot().appendChild( node );   /* reparents node */
   return true;
}


VgElement VgLog::docroot()
{
   QDomElement e = log.documentElement();
   return (VgElement&)e;
}

VgElement VgLog::protocol()
{ return docroot().getFirstElem("protocolversion"); }

VgPreamble VgLog::preamble()
{
   VgElement e = docroot().getFirstElem("preamble");
   return (VgPreamble&)e;
}

VgElement VgLog::pid()
{ return docroot().getFirstElem("pid"); }

VgElement VgLog::ppid()
{ return docroot().getFirstElem("ppid"); }

VgElement VgLog::tool()
{ return docroot().getFirstElem("tool"); }

VgElement VgLog::logqual()
{ return docroot().getFirstElem("logfilequalifier"); }

VgElement VgLog::comment()
{ return docroot().getFirstElem("usercomment"); }

VgElement VgLog::args()
{ return docroot().getFirstElem("args"); }

VgStatus VgLog::status_beg()
{
   VgElement e = docroot().getFirstElem("status");
   VgStatus s = (VgStatus&)e;
   if (s.state() == VgStatus::RUNNING)
      return s;
   return VgStatus();
}

VgStatus VgLog::status_end()
{
   VgElement e = docroot().getLastElem("status");
   VgStatus s = (VgStatus&)e;
   if (s.state() == VgStatus::FINISHED)
      return s;
   return VgStatus();
}

VgErrorList VgLog::errors()
{
   /* all non-leak errors before status == finished */
   VgErrorList errlist;
   QDomElement e = docroot().firstChild().toElement();
   for ( ; !e.isNull(); e=e.nextSibling().toElement() ) {
      /* quit after status == FINISHED */
      if (e.tagName() == "status") {
         if (((VgStatus&)e).state() == VgStatus::FINISHED)
            break;
      }
      else if (e.tagName() == "error") {
         VgError err = (VgError&)e;
         if (!err.isLeak()) {
            errlist.append( err );
         }
      }
   }
   return errlist;
}

VgErrorList VgLog::leaks()
{
   VgErrorList errlist;

   if ( state() == VgStatus::INIT ) {
      return errlist;
   }

   if ( 0 /*state() == VgStatus::RUNNING */ ) {
      /* last record group of leak errors */
    
   }
   else { // VgStatus::FINISHED
      /* all leaks after status_end */
      bool start = false;
      QDomElement e = docroot().firstChild().toElement();
      for ( ; !e.isNull(); e=e.nextSibling().toElement() ) {
         /* start after status == FINISHED */
         if (e.tagName() == "status") {
            if (((VgStatus&)e).state() == VgStatus::FINISHED)
               start = true;
         }
         else if (start && e.tagName() == "error") {
            VgError err = (VgError&)e;
            if (err.isLeak()) {
               errlist.append( err );
            }
         }
      }
   }
   return errlist;
}

VgErrCounts VgLog::errorcounts()
{
   VgElement e = docroot().getLastElem("errorcounts");
   return (VgErrCounts&)e;
}

VgSuppCounts VgLog::suppcounts()
{
   VgElement e = docroot().getFirstElem("suppcounts");
   return (VgSuppCounts&)e;
}


VgStatus::StateType VgLog::state() {
   if (!status_end().isNull())
      return status_end().state();
   if (!status_beg().isNull())
      return status_beg().state();
   return VgStatus::INIT;
}



/* ref: memcheck/mac_leakcheck.c :: MAC_(do_detect_memory_leaks) */
QString VgLog::plainTxtErrorSummary()
{
   int i_errs=0, i_err_contexts=0, i_supps=0, i_supp_contexts=0;
   QDomNodeList pairs;
   QDomNode n;

   /* count errors */
   pairs = errorcounts().childNodes();
   i_err_contexts = pairs.count();
   n = pairs.item(0);
   for ( ; !n.isNull(); n=n.nextSibling() ) {
      QString count = n.firstChild().toElement().text();
      i_errs += count.toULong();
   }

   /* count suppressions */
   pairs = suppcounts().childNodes();
   i_supp_contexts = pairs.count();
   n = pairs.item(0);
   for ( ; !n.isNull(); n=n.nextSibling() ) {
      QString count = n.firstChild().toElement().text();
      i_supps += count.toULong();
   }

   QString output_str = "ERROR SUMMARY: " +
      QString::number(i_errs) + " errors from " +
      QString::number(i_err_contexts) + " contexts " +
      "(suppressed: " + QString::number(i_supps) + " from " +
      QString::number(i_supp_contexts) + ")\n\n\n";
   return output_str;
}


/* ref: memcheck/mac_leakcheck.c :: MAC_(do_detect_memory_leaks) */
/* print leak error summary */
/* CAB: suppressed leak errs not given in vg xml output */
QString VgLog::plainTxtLeakSummary()
{
   unsigned long lk_bytes[VgError::NUM_LKS];
   unsigned long lk_blocks[VgError::NUM_LKS];
   for (unsigned int i=0; i<VgError::NUM_LKS; i++) {
      lk_bytes[i] = lk_blocks[i] = 0;
   }

   VgErrorList errs = leaks();
   /* summ the leaks for each leak kind */
   VgErrorList::iterator it;
   for ( it = errs.begin(); it != errs.end(); ++it ) {
      VgError err = *it;
      QDomNodeList err_details = err.childNodes();
      assert( err_details.count() >= 6 );
      QString bytes  = err_details.item( 4 ).toElement().text();
      QString blocks = err_details.item( 5 ).toElement().text();

      VgError::LeakKind kind = err.leakKind();    
      assert( kind < VgError::NUM_LKS );
      lk_bytes [ kind ] += bytes.toULong();
      lk_blocks[ kind ] += blocks.toULong();
   }

   QString output_str = "LEAK SUMMARY\n";
   int field_width = VgError::leakDesc(VgError::UNREACHED).length();
   for (int i=0; i<VgError::NUM_LKS; i++) {
      if (i == VgError::INDIRECT && lk_blocks[i] == 0)
         continue;
      output_str += QString("   %1 %2 bytes in %3 blocks\n")
         .arg( VgError::leakDesc( (VgError::LeakKind)i ), field_width )
         .arg( lk_bytes[i] )
         .arg( lk_blocks[i] );
   }
   return output_str;
}


/* print entire log to plain text string,
   a-la valgrind non-xml output */
QString VgLog::toPlainTxt()
{
   QString output_str;
   output_str = preamble().toPlainTxt() + "\n";

   VgErrorList errs = errors();
   VgErrorList::iterator it;
   for ( it = errs.begin(); it != errs.end(); ++it ) {
      /* find count from matching pair in errorcount */
      QString count = errorcounts().getCount( *it ).getContent();
      if (count == "") count = "?";
      output_str += "Occurred " + count + " times:\n";
      output_str += (*it).toPlainTxt();
   }
   output_str += plainTxtErrorSummary();

   errs = leaks();
   for ( it = errs.begin(); it != errs.end(); ++it ) {
      output_str += (*it).toPlainTxt();
   }
   output_str += plainTxtLeakSummary();
   return output_str;
}


/*
  xml string
  - with \n between each top-level chunk
  - reimplementation of QDomDocument::toString()
*/
QString VgLog::toString( int )
{
   if (log.isNull())
      return "";

   QString str;
   QTextStream s( str, IO_WriteOnly );

   s << log.firstChild() << "\n";
   QString doc_elem = docroot().nodeName();
   s  << "<" << doc_elem << ">\n\n";
   QDomNode n = docroot().firstChild();
   for ( ; !n.isNull(); n=n.nextSibling() ) {
      n.save( s, 2 );
      s << "\n";
   }
   s  << "</" << doc_elem << ">\n";
   return str;
}


bool VgLog::mergeErrors( VgErrorList sErrors,
                         VgErrCounts sErrCounts )
{
   VgErrCounts mErrCounts = errorcounts();

   vklmPrint( 2, "--- update matches (n=%d) --- ", sErrors.count());

   /* --- find matches: update master err, delete slave err ---  */

   /* --- for each error in master ---  */
   VgErrorList errs = errors();
   VgErrorList::Iterator mIt;
   for ( mIt = errs.begin(); mIt != errs.end(); ++mIt ) {
      VgError mErr = (VgError&)(*mIt);

      vklmPrint( 2, "master err: '%s'", mErr.unique().latin1());
      vklmPrint( 3, " " );

      /* get master errorcount for this error */
      VgElement mCount = mErrCounts.getCount( mErr );
      if (mCount.isNull()) {
         vklmPrint( 0, "error: no matching master errorcount\n" );
         return false;
      }

      /* --- for each error in slave --- */
      VgErrorList::Iterator sIt;
      for ( sIt = sErrors.begin(); sIt != sErrors.end(); ++sIt ) {
         VgError sErr = *sIt;

         vklmPrint( 2, "slave err: '%s'", sErr.unique().latin1() );

         /* get slave errorcount for this error */
         VgElement sCount = sErrCounts.getCount( sErr );
         if (sCount.isNull()) {
            vklmPrint( 0, "error: no matching slave errorcount" );
            return false;
         }

         if ( mErr == sErr ) {
            vklmPrint( 2, "=> matched" );

            /* --- master count += slave count --- */
            if ( ! mCount.updateCount( sCount ) ) {
               vklmPrint( 0, "error: failed master errorcount update" );
               return false;
            }

            /* --- remove error from slave list --- */
            sIt = sErrors.remove( sIt );
            sIt--;

            /* NOTE: not simply break'ing, as we can't guarantee there isn't
               another slave error that the same master error would match.
               This implies our error-matching heuristic isn't the same as
               valgrind's, i.e. match(slaveErr1, slaveErr2) may be true.

               CAB: Q: Why is it better to continue here?
               If we miss a potential second match to current master error,
               it'll be appended anyway.
               Neither way is entirely accurate, but break'ing would be
               faster, especially if can expect errors in similar order.
            */
         }
         vklmPrint( 3, " " );
      }
      vklmPrint( 2, " ");
   }

   vklmPrint( 2, "--- append non-matches (n=%d) --- ", sErrors.count() );

   /* if no errcounts, and sErrors > 0, create empty errcounts */
   if ( mErrCounts.isNull() && sErrors.count() > 0 ) {
      vklmPrint( 3, "creating new errcounts node" );

      /* create <errorcounts/> */
      QDomElement ec = log.createElement( "errorcounts" );
      /* and insert before status_finished */
      docroot().insertBefore( ec, status_end() );
   }

   /* --- append remaining slave errors to master --- */
   VgErrorList::Iterator sIt;
   for ( sIt = sErrors.begin(); sIt != sErrors.end(); ++sIt ) {
      VgError sErr = *sIt;
      vklmPrint( 2, "appending slave err: '%s'", sErr.unique().latin1() );
  
      /* get slave errorcount::pair for this error */
      VgElement sCount = sErrCounts.getCount( sErr );
      QDomNode sPair = sCount.parentNode();
      if (sPair.isNull()) {
         vklmPrint( 0, "error: no matching slave errorcount" );
         return false;
      }
    
      /* --- append slave error to master, before last errcounts --- */
      docroot().insertBefore( sErr, mErrCounts );

      /* --- append slave errorcount to master --- */
      mErrCounts.appendChild( sPair );
   }
   return true;
}


bool VgLog::mergeLeakErrors( VgErrorList sLeakErrors )
{
   vklmPrint( 2, "--- update matches (n=%d) ---", sLeakErrors.count() );

   /* --- for each leak in master ---  */
   VgErrorList errs = leaks();
   VgErrorList::Iterator mIt;
   for ( mIt = errs.begin(); mIt != errs.end(); ++mIt ) {
      VgError mErr = (VgError&)(*mIt);

      vklmPrint( 2, "master leak: '%s'", mErr.unique().latin1() );
      vklmPrint( 3, " " );

      /* --- for each leak in slave ---  */
      VgErrorList::Iterator sIt;
      for ( sIt = sLeakErrors.begin(); sIt != sLeakErrors.end(); ++sIt ) {
         VgError sErr = (VgError&)(*sIt);

         vklmPrint( 2, "slave leak: '%s'", sErr.unique().latin1() );

         if ( mErr == sErr ) {
            vklmPrint( 2, "=> matched" );

            /* --- update master leakedBytes, leakedBlocks, what str --- */
            if ( ! mErr.updateLeakErr( sErr ) ) {
               vklmPrint( 0, "error: failed to update master leak error" );
               return false;
            }

            /* --- remove error from slave list --- */
            sIt = sLeakErrors.remove( sIt );
            sIt--;
         }
         vklmPrint( 3, " " );
      }
      vklmPrint( 2, " ");
   }

   vklmPrint( 2, "--- append non-matches (n=%d) ---", sLeakErrors.count() );

   /* --- append remaining slave leaks to master --- */
   VgErrorList::Iterator sIt;
   for ( sIt = sLeakErrors.begin(); sIt != sLeakErrors.end(); ++sIt ) {
      vklmPrint( 2, "appending slave leak: '%s'",
                 (*sIt).unique().latin1() );
      docroot().appendChild( *sIt );
   }
   return true;
}


bool VgLog::mergeSuppCounts( VgSuppCounts sSuppCnts )
{
   /* deep copy so we can delete from it safely */
   QDomNode     sSuppCnts_copy = sSuppCnts.cloneNode();
   VgSuppCounts sSuppCounts    = (VgSuppCounts&)sSuppCnts_copy;

   VgSuppCounts mSuppCounts = suppcounts();

   vklmPrint( 2, "--- update matches (n=%d) ---",
              sSuppCounts.childNodes().count() );

   /* --- for each suppcount::pair in master --- */
   QDomElement m = mSuppCounts.firstChild().toElement();
   for( ; !m.isNull(); m = m.nextSibling().toElement() ) {
      VgElement mPair  = (VgElement&)m;
      QString   mStr   = mPair.getFirstElem( "name" ).text();
      VgElement mCount = mPair.getFirstElem( "count" );

      vklmPrint( 2, "master suppcount pair: '%s'", mStr.latin1() );

      QDomElement s = sSuppCounts.firstChild().toElement();
      for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
         VgElement sPair = (VgElement&)s;
         QString   sStr  = mPair.getFirstElem( "name" ).text();

         vklmPrint( 2, "slave  suppcount pair: '%s'", sStr.latin1() );

         if ( mStr == sStr ) { /* matching pair */
            vklmPrint( 2, "=> matched");

            VgElement sCount = sPair.getFirstElem( "count" );
            /* --- master pair::count += slave pair::count --- */
            if ( ! mCount.updateCount( sCount ) ) {
               vklmPrint( 0, "error: failed master suppcount update" );
               return false;
            }

            /* --- remove err from node --- */
            sSuppCounts.removeChild( sPair );
            /* can only be one match, so just go to next mPair */
            break;
         }
         vklmPrint( 2, " " );
      }
      vklmPrint( 2, " " );
   }

   vklmPrint( 2, "--- append non-matches (n=%d) ---",
              sSuppCounts.childNodes().count() );

   /* --- append remaining slave suppcount::pairs to master --- */
   QDomElement s = sSuppCounts.firstChild().toElement();
   for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
      mSuppCounts.appendChild( s );
   }
   return true;
}


bool VgLog::merge( VgLog& slave )
{
   /* clean master of leaks before status_end */
   QDomElement e = docroot().firstChild().toElement();
   for ( ; !e.isNull(); e=e.nextSibling().toElement() ) {
      /* quit after status == FINISHED */
      if (e.tagName() == "status") {
         if (((VgStatus&)e).state() == VgStatus::FINISHED)
            break;
      }
      else if (e.tagName() == "error") {
         VgError err = (VgError&)e;
         if (err.isLeak()) {
            e=e.previousSibling().toElement();
            docroot().removeChild( err );
         }
      }
   }
   /* clean slave of leaks before status_end */
   e = slave.docroot().firstChild().toElement();
   for ( ; !e.isNull(); e=e.nextSibling().toElement() ) {
      /* quit after status == FINISHED */
      if (e.tagName() == "status") {
         if (((VgStatus&)e).state() == VgStatus::FINISHED)
            break;
      }
      else if (e.tagName() == "error") {
         VgError err = (VgError&)e;
         if (err.isLeak()) {
            e=e.previousSibling().toElement();
            docroot().removeChild( err );
         }
      }
   }

   /* check the same tool was used */
   if (slave.tool().text() != tool().text()) {
      vklmPrintErr( "Different tool used for this logfile" );
      return false;
   }

   /* check the same binary was used */
   QDomElement sExe = slave.args().getFirstElem( "argv" ).getFirstElem( "exe" );
   QDomElement mExe = args().getFirstElem( "argv" ).getFirstElem( "exe" );
   if (sExe.text() != mExe.text()) {
      vklmPrintErr( "Different executable used for this logfile" );
      return false;
   }

   /* merge errors */
   vklmPrint( 2, " " );
   vklmPrint( 2, "=== MERGE ERRORS ===" );
   vklmPrint( 2, " " );
   VgErrorList sErrors = slave.errors();
   if ( sErrors.count() != 0) {
      if ( ! mergeErrors( slave.errors(), slave.errorcounts() ) )
         return false;
   } else {
      vklmPrint( 2, "no errors to merge" );
   }

   /* merge suppcounts */
   vklmPrint( 2, " " );
   vklmPrint( 2, "=== MERGE SUPPCOUNTS ===" );
   vklmPrint( 2, " " );
   VgSuppCounts sSuppCounts = slave.suppcounts();
   if ( sSuppCounts.childNodes().count() != 0) {
      if ( ! mergeSuppCounts( sSuppCounts ) )
         return false;
   } else {
      vklmPrint( 2, "no suppcounts to merge" );
   }

   /* merge leaks */
   vklmPrint( 2, " " );
   vklmPrint( 2, "=== MERGE LEAKS ===" );
   vklmPrint( 2, " " );
   VgErrorList sLeakErrors = slave.leaks();
   if ( sLeakErrors.count() != 0) {
      if ( ! mergeLeakErrors( sLeakErrors ) )
         return false;
   } else {
      vklmPrint( 2, "no leak errors to merge" );
   }
   vklmPrint( 2, " " );

   return true;
}

