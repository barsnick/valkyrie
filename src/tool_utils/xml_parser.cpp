/* ---------------------------------------------------------------------
 * implementation of XMLParser                            xml_parser.cpp
 * ---------------------------------------------------------------------
 * Subclass of QXmlDefaultHandler for parsing memcheck-specific xml output.
 *
 * Also contains various small classes encapsulating the different
 * types of xml output.
 */

#include "xml_parser.h"
#include "memcheck_view.h"
#include "vk_utils.h"

#include <qfileinfo.h>


/* class XmlOutput ----------------------------------------------------- */
XmlOutput::~XmlOutput() { }

XmlOutput::XmlOutput( ItemType itype ) 
{ 
  itemType = itype;
  display  = "NOT SET"; 
}

QString XmlOutput::displayString() 
{ return display; }


/* class Preamble ------------------------------------------------------ */
Preamble::~Preamble() { }

Preamble::Preamble() : XmlOutput( PREAMBLE ) 
{ display = "Preamble"; }


/* class TopStatus ----------------------------------------------------- */
TopStatus::TopStatus() : XmlOutput( STATUS ) 
{
  status     = "";
  object     = "";
  num_errs   = 0;
  num_leaks  = 0;
  num_blocks = 0;
}

void TopStatus::print() 
{
  display = QString("Valgrind: %1 '%2'   Errors: %3   Leaked bytes: %4 in %5 blocks")
    .arg( status )
    .arg( object )
    .arg( num_errs )
    .arg( num_leaks )
    .arg( num_blocks );
}


/* class Info ---------------------------------------------------------- */
Info::Info() : XmlOutput( INFO )
{
  pid = -1;
  ppid = -1;
  tool = "";
}

void Info::print() 
{
  tool[0] = tool[0].upper();
  display.sprintf("%s output for process id ==%d== (parent pid ==%d==)",
                  tool.ascii(), pid, ppid );
}


/* class Counts -------------------------------------------------------- */
Counts::~Counts() { }

Counts::Counts() : XmlOutput( COUNTS ) 
{
  key = "";
  value = -1;
  total_errors = 0; 
  countsMap.clear(); 
}

bool Counts::isEmpty() 
{ return total_errors == 0; }

void Counts::update() 
{ 
  countsMap[key] = value;
  total_errors += value;
}

int Counts::find( QString id ) 
{
  CountsMap::Iterator it = countsMap.find( id );
  return ( it == countsMap.end() ) ? -1 : it.data();
}

int Counts::count() 
{ return countsMap.count(); }

bool Counts::contains( QString id ) 
{  return countsMap.contains( id ); }

void Counts::print() 
{
  CountsMap::Iterator it;
  for ( it = countsMap.begin(); it != countsMap.end(); ++it ) {
    display.sprintf("%4d:  %s", it.data(), it.key().ascii() );
    supps << display;
  }
  display = "Suppressions";
}


/* class Frame --------------------------------------------------------- */
Frame::~Frame() { }

Frame::Frame( bool top_frame ) : XmlOutput( FRAME ) 
{ 
  at_by   = ( top_frame ) ? "at" : "by";
  lineno  = -1;
  display = ip = obj = fn = srcfile = srcdir = filepath = "";
  pathPrinted = readable = writeable = false;
}

void Frame::printPath()
{
  if ( pathPrinted )  /* been there, done that ... */
    return;

  if ( !filepath.isEmpty() ) {
    int spos = display.find( srcfile );
    vk_assert( spos != -1 );
    int epos = display.findRev( ':' );
    if ( epos == -1 ) /* no lineno found */
      epos = display.length();

    display = display.replace( spos, epos-spos, filepath );
    pathPrinted = true;
  }
}

void Frame::print() 
{
  bool have_line = lineno != -1;
  bool have_file = !srcfile.isEmpty();
  bool have_path = !srcdir.isEmpty();
  bool have_fn   = !fn.isEmpty();
  bool have_obj  = !obj.isEmpty();

  /* check what perms the user has w.r.t. this file */
  if ( have_path && have_file ) {
    filepath = srcdir + "/" + srcfile;
    QFileInfo fi( filepath );
    if ( fi.exists() && fi.isFile() && !fi.isSymLink() ) {
      readable  = fi.isReadable();
      writeable = fi.isWritable();
      /* so we don't seek for lineno == -1 in file */
      lineno = ( lineno == -1 ) ? 0 : lineno;
    }
  }

  if ( have_fn && have_path && have_file && have_line ) {
    display.sprintf( "%s %s in %s:%d", 
                     at_by.ascii(), fn.ascii(), srcfile.ascii(), lineno );
  } else if ( have_fn && have_path && have_file ) {
    display.sprintf( "%s %s in %s", 
                     at_by.ascii(), fn.ascii(), srcfile.ascii() );
  } else if ( have_fn && have_file && have_line ) {
    display.sprintf( "%s %s in %s:%d", 
                     at_by.ascii(), fn.ascii(), srcfile.ascii(), lineno );
  } else if ( have_fn && have_file ) {
    display.sprintf( "%s %s in %s", 
                     at_by.ascii(), fn.ascii(), srcfile.ascii() );
  } else if ( have_fn && have_obj ) {
    display.sprintf( "%s %s in %s", 
                     at_by.ascii(), fn.ascii(), obj.ascii() );
  } else if ( have_fn ) {
    display.sprintf( "Address %s by %s", ip.ascii(), fn.ascii() );
  } else if ( have_obj ) {
    display.sprintf( "Address %s in %s", ip.ascii(), obj.ascii() );
  } else {
    display.sprintf( "Address %s", ip.ascii() );
  }

}



/* class Stack --------------------------------------------------------- */
Stack::~Stack() 
{ 
  //display = "stack";
  currFrame = 0;
  frameList.setAutoDelete( true );
  frameList.clear();
  frameList.setAutoDelete( false );
}

Stack::Stack( bool top ) : XmlOutput( STACK ) 
{ 
  is_first_stack = top;
  frameList.clear(); 
}

void Stack::mkFrame() 
{
  bool top_frame = frameList.isEmpty();
  currFrame = new Frame( top_frame );
  frameList.append( currFrame );
}


/* class Error --------------------------------------------------------- */
Error::~Error() 
{ 
  if ( stack1 )
    delete stack1;
  if ( stack2 )
    delete stack2;
}

Error::Error() : XmlOutput( ERROR ) 
{
  stack1 = 0;
  stack2 = 0;
  auxwhat = unique = kind = what = acnym = "";
  tid = leakedbytes = leakedblocks = -1;
  num_times = 1;
}

void Error::mkStack() 
{ 
  if ( stack1 == 0 ) {
    stack1 = new Stack( true );
    currStack = stack1;
  }  else { 
    stack2 = new Stack( false );
    currStack = stack2;
  }
}

void Error::print() 
{
  if ( acnym == "LDL" || acnym == "LSR" || 
       acnym == "LIL" || acnym == "LPL" ) {
    display.sprintf("%s: %s", acnym.ascii(), what.ascii() );
  } else {
    display.sprintf("%s [%d]: %s", acnym.ascii(), num_times, what.ascii() );
  }
}





/* class XMLParser ----------------------------------------------------- */
XMLParser::~XMLParser() 
{ reset( false ); }

XMLParser::XMLParser( MemcheckView * parent )
{
  memView = parent;
  info      = 0;
  verror    = 0;
  counts    = 0;
  preamble  = 0;
  topStatus = 0;

  /* init our pretend-namespace */
  tagtypeMap["valgrindoutput"]  = VGOUTPUT;
  tagtypeMap["protocolversion"] = PROTOCOL;
  tagtypeMap["preamble"]        = PREAMBLE;
  tagtypeMap["pid"]             = PID;
  tagtypeMap["ppid"]            = PPID;
  tagtypeMap["tool"]            = TOOL;
  tagtypeMap["argv"]            = ARGV;
  tagtypeMap["exe"]             = EXE;
  tagtypeMap["arg"]             = ARG;
  tagtypeMap["status"]          = STATUS;
  tagtypeMap["error"]           = ERROR;
  tagtypeMap["unique"]          = UNIQUE;
  tagtypeMap["tid"]             = TID;
  tagtypeMap["kind"]            = KIND;
  tagtypeMap["what"]            = WHAT;
  tagtypeMap["stack"]           = STACK;
  tagtypeMap["frame"]           = FRAME;
  tagtypeMap["ip"]              = IP;
  tagtypeMap["obj"]             = OBJ;
  tagtypeMap["fn"]              = FN;
  tagtypeMap["dir"]             = SRCDIR;
  tagtypeMap["file"]            = SRCFILE;
  tagtypeMap["line"]            = LINE;
  tagtypeMap["auxwhat"]         = AUXWHAT;
  tagtypeMap["errorcounts"]     = ERRORCOUNTS;
  tagtypeMap["pair"]            = PAIR;
  tagtypeMap["count"]           = COUNT;
  tagtypeMap["suppcounts"]      = SUPPCOUNTS;
  tagtypeMap["name"]            = NAME;
  tagtypeMap["leakedbytes"]     = LEAKEDBYTES;
  tagtypeMap["leakedblocks"]    = LEAKEDBLOCKS;

  /* init the 3-letter acronyms for kinds of errors */
  acronymMap["InvalidFree"]         = "IVF";
  acronymMap["MismatchedFree"]      = "MMF";
  acronymMap["InvalidRead"]         = "IVR";
  acronymMap["InvalidWrite"]        = "IVW";
  acronymMap["InvalidJump"]         = "IVJ";
  acronymMap["Overlap"]             = "OVL";
  acronymMap["InvalidMemPool"]      = "IMP";
  acronymMap["UninitCondition"]     = "UNC";
  acronymMap["UninitValue"]         = "UNV";
  acronymMap["SyscallParam"]        = "SCP";
  acronymMap["ClientCheck"]         = "CCK";
  acronymMap["Leak_DefinitelyLost"] = "LDL";
  acronymMap["Leak_IndirectlyLost"] = "LIL";
  acronymMap["Leak_PossiblyLost"]   = "LPL";
  acronymMap["Leak_StillReachable"] = "LSR";
}


void XMLParser::reset( bool reinit/*=true*/ )
{
  if ( info )      { delete info;      info      = 0; }
  if ( verror )    { delete verror;    verror     = 0; }
  if ( counts )    { delete counts;    counts    = 0; }
  if ( preamble )  { delete preamble;  preamble  = 0; }
  if ( topStatus ) { delete topStatus; topStatus = 0; }

  inError = false;
  inStack = false;
  inFrame = false;
  statusPopped  = false;
  inPreamble    = false;
  inErrorCounts = false;
  inSuppCounts  = false;
  inPair        = false;

  errorList.clear();

  if ( reinit ) {
    info      = new Info();
    topStatus = new TopStatus();
  }
}


/* Returns the tag type corresponding to the given tag */
QString XMLParser::acronym( QString kind )
{
  AcronymMap::Iterator it = acronymMap.find( kind );
  if ( it == acronymMap.end() )
    return "???";
  else
    return it.data(); 
}


/* Returns the tag type corresponding to the given tag */
XMLParser::TagType XMLParser::tagType( QString tag )
{
  TagTypeMap::Iterator it = tagtypeMap.find( tag );
  if ( it == tagtypeMap.end() )
    return NONE;
  else
    return it.data(); 
}


bool XMLParser::characters( const QString& str )
{
  QString chars = str.simplifyWhiteSpace();
  if ( !chars.isEmpty() ) {
    content = chars;
  }
  return true;
}


bool XMLParser::startElement( const QString&, const QString&, 
                              const QString& stag, const QXmlAttributes& )
{
  startTag = stag;

  TagType ttype = tagType( startTag );
  switch ( ttype ) {
    case PREAMBLE: 
      inPreamble = true;
      preamble = new Preamble();
      break;
    case ERROR:
      inError = true;
      verror = new Error();
      errorList.append( verror );
      break;
    case STACK:
      if ( inError ) {
        inStack = true;
        verror->mkStack();
      } break;
    case FRAME:
      if ( inError && inStack ) {
        inFrame = true;
        verror->currStack->mkFrame();
      } break;
    case ERRORCOUNTS:
      inErrorCounts = true;
      counts = new Counts();
      break;
    case PAIR:
      if ( inErrorCounts || inSuppCounts ) {
        inPair = true;
      } break;
    case SUPPCOUNTS:
      inSuppCounts = true;
      counts = new Counts();
      break;
    default:
      break;
  }    

  return true;
}


bool XMLParser::endElement( const QString&, const QString&, 
                            const QString& etag )
{
  endTag = etag;

  TagType ttype = tagType( endTag );
  switch ( ttype ) {

    case VGOUTPUT:   /* ignore */
      break;
    case PROTOCOL:
      if ( content != "1" ) 
        printf("Fatal Error: wrong protocol version\n");
      break;
    case PREAMBLE:
      inPreamble = false;
      stack.push( preamble );
      break;
    case PID:
      info->pid  = content.toInt();  
      break;
    case PPID:
      info->ppid = content.toInt();
      break;
    case TOOL:
      info->tool = content;
      info->print();
      break;
    case ARG:
      info->infoList << content;
      break;
    case ARGV:
      stack.push( info );
      break;

    case EXE: {
      /* get the name of the executable */
      QString tmp;
      int pos = content.findRev( '/' );
      if ( pos == -1 ) {
        tmp = content;
      } else {
        tmp = content.right( content.length() - pos-1 );
      }
      topStatus->object = tmp;
      info->infoList << content; 
    } break;

    case STATUS:
      if ( statusPopped ) {
        topStatus->status = content;
        memView->updateStatus();
      } else {
        topStatus->status = content;
        topStatus->print();
        memView->loadItem( topStatus );
        statusPopped = true;
        /* pop the others now as well */
        memView->loadItem( stack.pop() );   /* Info */
        memView->loadItem( stack.pop() );   /* Preamble */
      } break;

    case ERROR:
      inError = false;
      memView->loadItem( verror );
      break;

    case UNIQUE:
      if ( inError ) {
        verror->unique = content;
      } else if ( inErrorCounts && inPair ) {
        counts->key = content;
        counts->update();
      } break;

    case TID:
      if ( inError ) {
        verror->tid = content.toInt();
      }
      break;
    case KIND:
      if ( inError ) {
        verror->kind = content;
        verror->acnym = acronym( content );
      }
      break;
    case WHAT:
      if ( inError ) {
        verror->what = content;
        verror->print();
      }
      break;
    case AUXWHAT:
      if ( inError ) {
        verror->auxwhat = content;
      }
      break;
    case STACK:
      inStack = false;
      break;
    case FRAME:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->print();
        inFrame = false;
      } break;
    case IP:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->ip = content;
      } break;
    case OBJ:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->obj = content;
      } break;
    case FN:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->fn = content;
      } break;
    case SRCDIR:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->srcdir = content;
      } break;
    case SRCFILE:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->srcfile = content;
      } break;
    case LINE:
      if ( inPreamble ) {
        preamble->lines << content; 
      } else if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->lineno = content.toInt();
      }  break;

    case ERRORCOUNTS:
      inErrorCounts = false;
      if ( !counts->isEmpty() ) {
        topStatus->num_errs = counts->total_errors;
        memView->updateStatus();
        memView->updateErrors( counts );
      }
      delete counts;
      counts = 0;
      break;
    case PAIR:
      if ( inErrorCounts || inSuppCounts ) {
        inPair = false;
      } break;
    case COUNT:
      if ( inErrorCounts && inPair ) {
        counts->value = content.toInt();
      } else if ( inSuppCounts && inPair ) {
        counts->value = content.toInt();
      } break;
    case NAME:
      if ( inSuppCounts && inPair ) {
        counts->key = content;
        counts->update();
      } break;
    case SUPPCOUNTS:
      inSuppCounts = false;
      counts->print();
      memView->loadItem( counts );
      break;
    case LEAKEDBYTES:
      verror->leakedbytes = content.toInt();
      topStatus->num_leaks += content.toInt();
      memView->updateStatus();
      break;
    case LEAKEDBLOCKS:
      verror->leakedblocks = content.toInt();
      topStatus->num_blocks += content.toInt();
      memView->updateStatus();
      break;
    case NONE:
      printf("no such tag '%s'\n", endTag.ascii());
      vk_assert_never_reached();
      break;
  }

  return true;
}
