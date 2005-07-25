/* ---------------------------------------------------------------------
 * Definition of XMLParser                                xml_parser.cpp
 * Subclass of QXmlDefaultHandler for parsing memcheck-specific xml output.
 * Also contains various small classes encapsulating the different
 * chunks of xml output.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "xml_parser.h"
#include "vk_utils.h"
#include "vk_popt_option.h"

#include <qfileinfo.h>


/* class XmlOutput ----------------------------------------------------- */
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
  state      = "";
  stime      = "";
  etime      = "";
  object     = "";
  num_errs   = 0;
  num_leaks  = 0;
  num_blocks = 0;
}

/* format output for displaying in listview */
void TopStatus::printDisplay() 
{
  QTextStream ts( &display, IO_WriteOnly );
  ts << "Valgrind: " << state << " '" << object << "'  ";

  int syear, smonth, sday, shours, smins, ssecs, smsecs;
  sscanf( stime.ascii(), "%d-%d-%d %d:%d:%d.%4d", 
          &syear, &smonth, &sday, &shours, &smins, &ssecs, &smsecs );
  QDate sta_date( syear, smonth, sday );
  QTime sta_time( shours, smins, ssecs, smsecs );

  /* start date */
  ts << sta_date.toString( "MMM/d/yy  " );
  /* start time */
  ts << sta_time.toString( "hh:mm:ss.zzz" );

  if ( !etime.isEmpty() ) {     /* finished */
    ts << " - ";

    int eyear, emonth, eday, ehours, emins, esecs, emsecs;
    sscanf( etime.ascii(), "%d-%d-%d %d:%d:%d.%4d", 
            &eyear, &emonth, &eday, &ehours, &emins, &esecs, &emsecs );
    QDate end_date( eyear, emonth, eday );
    QTime end_time( ehours, emins, esecs, emsecs );

    /* end date, but only if > start_date */
    if ( end_date > sta_date )
      ts << end_date.toString( "MMM/d/yy  " );
    /* end time */
    ts << end_time.toString( "hh:mm:ss.zzz" );
    /* elapsed time */
    ts << " (";
    if ( end_date > sta_date ) {
      QDate elap_date( end_date.year()  - sta_date.year(),
                       end_date.month() - sta_date.month(),
                       end_date.day()   - sta_date.day() );
      ts << elap_date.toString( "MMM/d/yy  " );
    }

    /* elapsed date-time:  86,400,000 msecs in 1 day */
    int elapsed_date_msecs = sta_date.daysTo( end_date ) * 86400000;
    /* elapsed time-time */
    int elapsed_time_msecs = sta_time.msecsTo( end_time );
    /* add everything together */
    int elapsed_msecs    = elapsed_date_msecs + elapsed_time_msecs;

    QTime time_a;    /* 00:00:00.000 */
    QTime elapsed_time = time_a.addMSecs( elapsed_msecs );
    ts << elapsed_time.toString( "hh:mm:ss.zzz" ) << ")";
  }

  /* errors, leaks */
  ts << "\nErrors: " << num_errs << "    "
     << "Leaked Bytes: " << num_leaks << " in " << num_blocks << " blocks";
}


/* class Info ---------------------------------------------------------- */
Info::Info() : XmlOutput( INFO )
{
  pid  = -1;
  ppid = -1;
  tool = "";
  startStatus = "";
  endStatus   = "";
  startTime   = "";
  endTime     = "";
  protocolVersion = -1;
}

/* format output for displaying in listview */
void Info::printDisplay() 
{
  tool[0] = tool[0].upper();
  display.sprintf("%s output for process id ==%d== (parent pid ==%d==)",
                  tool.latin1(), pid, ppid );
}


/* class ErrCounts ----------------------------------------------------- */
ErrCounts::~ErrCounts() 
{ pairList.clear(); }

ErrCounts::ErrCounts() : XmlOutput( ERR_COUNTS ) 
{ 
  num = -1;
  totalErrors = 0;
}

bool ErrCounts::isEmpty() 
{ return totalErrors == 0; }

void ErrCounts::appendPair( QString str )
{ 
  pairList.append( Pair( num, str ) ); 
  totalErrors += num;
}

void ErrCounts::appendPair( int val, QString str )
{ 
  num = val;
  appendPair( str );
}

/* Iterate over ec->pairList, appending each pair to this pairList */
void ErrCounts::appendList( ErrCounts* ec )
{
  PairList::iterator it;
  for ( it = ec->pairList.begin(); it != ec->pairList.end(); ++it ) {
    pairList.append( Pair( (*it).number, (*it).data ) ); 
  }
}

/* returns the 'count' value associated with the 'uniq' string */
int ErrCounts::findUnique( QString uniq )
{
  int count = -1;
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    if ( uniq == (*it).data ) {
      count = (*it).number;
      break;
    }
  }

  return count;
}

/* If 'count' and 'uniq' match, remove the pair from the list */
void ErrCounts::remove( int count, QString uniq )
{
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    if ( count == (*it).number && uniq == (*it).data ) {
       pairList.remove( it );
      break;
    }
  }
}

/* Searches the pairList for a match on 'uniq'. 
   If found, updates the count value */
bool ErrCounts::updateCount( int count, QString uniq )
{
  bool ok = false;
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    if ( uniq == (*it).data ) {
      int& val = (*it).number;
      val += count;
      ok = true;
    }
  }

  return ok;
}

/* format output for printing to file */
void ErrCounts::print2File( QTextStream& stream )
{
  stream << "<errorcounts>\n";
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    stream << "  <pair> <count>" << (*it).number << "</count> <unique>" 
           << (*it).data << "</unique> </pair>\n";
  }
  stream << "</errorcounts>\n\n";
}


/* class SuppCounts ---------------------------------------------------- */
SuppCounts::~SuppCounts() { }

SuppCounts::SuppCounts() : XmlOutput( SUPP_COUNTS ) 
{ num = -1; }

void SuppCounts::appendPair( QString str )
{ pairList.append( Pair( num, str ) ); }

void SuppCounts::appendPair( int val, QString str )
{ 
  num = val;
  appendPair( str );
}

int SuppCounts::findName( QString name )
{
  int count = -1;
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    if ( name == (*it).data ) {
      count = (*it).number;
      break;
    }
  }

  return count;
}

/* searches the pairList for a match on 'name'. 
   if found, updates the count value */
bool SuppCounts::updateCount( int count, QString name )
{
  bool ok = false;
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    if ( name == (*it).data ) {
      int& val = (*it).number;
      val += count;
      ok = true;
    }
  }

  return ok;
}

/* iterate over 'sc->pairList'; if we find a match on 'name',
   increment our 'count' value; otherwise, add the not-found pair onto
   our pairList. */
void SuppCounts::updateList( SuppCounts* sc )
{
  int count;
  QString name;
  PairList::iterator it;
  for ( it = sc->pairList.begin(); it != sc->pairList.end(); ++it ) {
    count = (*it).number;
    name  = (*it).data;
    if ( findName( name ) != -1 ) {
      updateCount( count, name );
    } else {
      appendPair( count, name );
    }
  }
}

/* format output for displaying in listview */
void SuppCounts::printDisplay() 
{
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    display.sprintf("%4d:  %s", (*it).number, (*it).data.latin1() );
    supps << display;
  }
  display = "Suppressions";
}

/* format output for printing to file */
void SuppCounts::print2File( QTextStream& stream )
{
  stream << "<suppcounts>\n";
  PairList::iterator it;
  for ( it = pairList.begin(); it != pairList.end(); ++it ) {
    stream << "  <pair> <count>" << (*it).number << "</count> <name>"
           << (*it).data << "</name> </pair>\n";
  }
  stream << "</suppcounts>\n\n";
}



/* class Frame --------------------------------------------------------- */
Frame::~Frame() { }

Frame::Frame( bool top_frame ) : XmlOutput( FRAME ) 
{ 
  at_by   = ( top_frame ) ? "at" : "by";
  lineno  = -1;
  display = ip = obj = fn = srcfile = srcdir = filepath = "";
  pathPrinted = readable = writeable = false;
  haveLine = haveFile = haveDir = haveFunc = haveObj = false;
}

void Frame::setObj( QString str )
{ obj = str; haveObj  = true; }

void Frame::setFun( QString str )
{ fn = str; haveFunc = true; }

void Frame::setDir( QString str )
{ srcdir = str; haveDir  = true; }

void Frame::setFile( QString str )
{ srcfile = str; haveFile = true; }

void Frame::setLine( int no )
{ lineno = no;  haveLine = true; }

void Frame::printPath()
{
  if ( pathPrinted )     /* been there, done that ... */
    return;

  if ( !filepath.isEmpty() ) {
    int spos = display.find( srcfile );
    vk_assert( spos != -1 );
    int epos = display.findRev( ':' );
    if ( epos == -1 )    /* no lineno found */
      epos = display.length();

    display = display.replace( spos, epos-spos, filepath );
    pathPrinted = true;
  }
}

/* format output for printing to file */
void Frame::print2File( QTextStream& stream )
{
                  stream << "    <frame>\n"
                         << "      <ip>"   << ip      << "</ip>\n";
  if ( haveObj  ) stream << "      <obj>"  << obj     << "</obj>\n";
  if ( haveFunc ) stream << "      <fn>"   << fn      << "</fn>\n";
  if ( haveDir  ) stream << "      <dir>"  << srcdir  << "</dir>\n";
  if ( haveFile ) stream << "      <file>" << srcfile << "</file>\n";
  if ( haveLine ) stream << "      <line>" << lineno  << "</line>\n";
                  stream << "    </frame>\n";
}

/* format output for displaying in listview */
void Frame::printDisplay() 
{
  /* check what perms the user has w.r.t. this file */
  if ( haveDir && haveFile ) {
    filepath = srcdir + "/" + srcfile;
    QFileInfo fi( filepath );
    if ( fi.exists() && fi.isFile() && !fi.isSymLink() ) {
      readable  = fi.isReadable();
      writeable = fi.isWritable();
      /* so we don't seek for lineno == -1 in file */
      lineno = ( lineno == -1 ) ? 0 : lineno;
    }
  }

  if ( haveFunc && haveDir && haveFile && haveLine ) {
    display.sprintf( "%s %s in %s:%d", 
                     at_by.latin1(), fn.latin1(), srcfile.latin1(), lineno );
  } else if ( haveFunc && haveDir && haveFile ) {
    display.sprintf( "%s %s in %s", 
                     at_by.latin1(), fn.latin1(), srcfile.latin1() );
  } else if ( haveFunc && haveFile && haveLine ) {
    display.sprintf( "%s %s in %s:%d", 
                     at_by.latin1(), fn.latin1(), srcfile.latin1(), lineno );
  } else if ( haveFunc && haveFile ) {
    display.sprintf( "%s %s in %s", 
                     at_by.latin1(), fn.latin1(), srcfile.latin1() );
  } else if ( haveFunc && haveObj ) {
    display.sprintf( "%s %s in %s", 
                     at_by.latin1(), fn.latin1(), obj.latin1() );
  } else if ( haveFunc ) {
    display.sprintf( "Address %s by %s", ip.latin1(), fn.latin1() );
  } else if ( haveObj ) {
    display.sprintf( "Address %s in %s", ip.latin1(), obj.latin1() );
  } else {
    display.sprintf( "Address %s", ip.latin1() );
  }

}



/* class Stack --------------------------------------------------------- */
Stack::~Stack() 
{ 
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

/* format output for printing to file */
void Stack::print2File( QTextStream& stream )
{
  stream << "  <stack>\n";
  for (Frame* frame = frameList.first(); frame; frame = frameList.next() )
    frame->print2File( stream );
  stream << "  </stack>\n";
}


/* class Error --------------------------------------------------------- */
Error::~Error() 
{ stackList.clear(); }

Error::Error() : XmlOutput( ERROR ) 
{
  auxwhat = unique = kind = what = acnym = "";
  tid = leakedBytes = leakedBlocks = -1;
  num_times = 1;
  haveAux = false;
}

void Error::mkStack() 
{ 
  bool first = ( stackList.isEmpty() ) ? true : false;
  currStack = new Stack( first );
  stackList.append( currStack );
}

void Error::setAux( QString str ) 
{ auxwhat = str; haveAux = true; }

/* if leakedbytes or leakedblocks is set, then 
   this is a leak error as opposed to a 'normal' error */
void Error::setLeakedBytes( int bytes )
{ leakedBytes  = bytes;  itemType = LEAK_ERROR; }

void Error::setLeakedBlocks( int blocks )
{ leakedBlocks = blocks; itemType = LEAK_ERROR; }

/* format output for displaying in listview */
void Error::printDisplay() 
{
  if ( acnym == "LDL" || acnym == "LSR" || 
       acnym == "LIL" || acnym == "LPL" ) {
    display.sprintf("%s: %s", acnym.latin1(), what.latin1() );
  } else {
    display.sprintf("%s [%d]: %s", acnym.latin1(), num_times, what.latin1() );
  }
}

/* format output for printing to file */
void Error::print2File( QTextStream& stream )
{
  stream << "<error>\n"
         << "  <unique>" << unique << "</unique>\n"
         << "  <tid>"    << tid    << "</tid>\n"
         << "  <kind>"   << kind   << "</kind>\n"
         << "  <what>"   << what   << "</what>\n";

  if ( itemType == LEAK_ERROR ) {
    stream << "  <leakedbytes>"  << leakedBytes  << "</leakedbytes>\n"
           << "  <leakedblocks>" << leakedBlocks << "</leakedblocks>\n";
  }

  for (Stack* stack = stackList.first(); stack; stack = stackList.next() ) {
    stack->print2File( stream );
    /* 'auxwhat' comes between stacks, and There Can Be Only One */
    if ( haveAux ) {
      stream << "  <auxwhat>" << auxwhat << "</auxwhat>\n";
      haveAux = false;
    }
  }

  stream << "</error>\n\n";
}



/* class XMLParser ----------------------------------------------------- */
/* Beware leaks: the parser *never* deletes any XmlOutput items; 
   this is the sole responsibility of the caller */

XMLParser::~XMLParser() 
{ 
  tagtypeMap.clear();
  acronymMap.clear();
}


XMLParser::XMLParser( QObject* parent, bool esc_ents/*=false*/  ) 
  : QObject( parent, "xml_parser" )
{
  info        = 0;
  verror      = 0;
  suppCounts  = 0;
  errCounts   = 0;
  preamble    = 0;
  topStatus   = 0;
  escEntities = esc_ents;

  /* init our pretend-namespace */
  tagtypeMap["valgrindoutput"]  = VGOUTPUT;
  tagtypeMap["protocolversion"] = PROTOCOL;
  tagtypeMap["preamble"]        = PREAMBLE;
  tagtypeMap["pid"]             = PID;
  tagtypeMap["ppid"]            = PPID;
  tagtypeMap["tool"]            = TOOL;
  tagtypeMap["args"]            = ARGS;
  tagtypeMap["vargv"]           = VARGV;
  tagtypeMap["argv"]            = ARGV;
  tagtypeMap["exe"]             = EXE;
  tagtypeMap["arg"]             = ARG;
  tagtypeMap["status"]          = STATUS;
  tagtypeMap["state"]           = STATE;
  tagtypeMap["time"]            = TIME;
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
  inError = false;
  inStack = false;
  inFrame = false;
  statusPopped  = false;
  inPreamble    = false;
  inVargV       = false;
  inErrorCounts = false;
  inSuppCounts  = false;
  inPair        = false;

   if ( reinit ) {
    info      = new Info();
    topStatus = new TopStatus();
  }
}


/* returns the tag type corresponding to the given tag */
QString XMLParser::acronym( QString kind )
{
  AcronymMap::Iterator it = acronymMap.find( kind );
  if ( it == acronymMap.end() )
    return "???";
  else
    return it.data(); 
}


/* returns the tag type corresponding to the given tag */
XMLParser::TagType XMLParser::tagType( QString tag )
{
  TagTypeMap::Iterator it = tagtypeMap.find( tag );
  if ( it == tagtypeMap.end() )
    return NONE;
  else
    return it.data(); 
}


/* if the output is being displayed in a listview, then don't bother
   to parse 'content' for '<', '>' and '&'.  but if we are going to
   print to file, or if we are comparing 'content' with a view to
   merging errors, then do bother (sigh) */
bool XMLParser::characters( const QString& str )
{
  QString chars = str.simplifyWhiteSpace();
  if ( !chars.isEmpty() ) {
    content = chars;

    if ( escEntities ) {
      content = escapeEntities( content );
    }

  }

  return true;
}


bool XMLParser::startElement( const QString&, const QString&, 
                              const QString& startTag, const QXmlAttributes& )
{
  TagType ttype = tagType( startTag );

  /* Any content before a start tag => client program output */
  if ( !content.isEmpty() ) {
    // TODO: content has lost its spaces/formatting.
    emit loadClientOutput( content );
  }

  switch ( ttype ) {
    case PREAMBLE: 
      inPreamble = true;
      preamble = new Preamble();
      break;
    case VARGV:
      inVargV = true;
      break;
    case ERROR:
      inError = true;
      verror = new Error();
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
      errCounts = new ErrCounts();
      break;
    case PAIR:
      if ( inErrorCounts || inSuppCounts ) {
        inPair = true;
      } break;
    case SUPPCOUNTS:
      inSuppCounts = true;
      suppCounts = new SuppCounts();
      break;
    default:
      break;
  }    

  return true;
}


bool XMLParser::endElement( const QString&, const QString&, 
                            const QString& endTag )
{
  TagType ttype = tagType( endTag );

  switch ( ttype ) {
    case VGOUTPUT:   /* ignore */
      break;
    case PROTOCOL:
      if ( content != "1" ) 
        printf("Fatal Error: wrong protocol version\n");
      info->protocolVersion = content.toInt();
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
      break;

    case ARGS:
      stack.push( info );
      break;
    case VARGV:
      inVargV = false;
      break;  
    case ARG:
      if ( inVargV )
        info->vgInfoList << content;
      else
        info->exInfoList << content;
      break;
    case EXE: {
      if ( inVargV ) {
        info->vgInfoList << content;
      } else {
        /* get the name of the executable to display in TopStatus */
        int pos = content.findRev( '/' );
        QString tmp = (pos == -1) ? content 
                                  : content.right( content.length() - pos-1 );
        topStatus->object = tmp;
        info->exe = content;
        info->exInfoList << content; 
      }
    } break;

    case STATE:
      topStatus->state  = content;
      if ( ! statusPopped ) 
        info->startStatus = content;
      else
        info->endStatus   = content;
      break;
    case TIME:
      if ( ! statusPopped ) {
        topStatus->stime = content;
        info->startTime = content;
      } else {
        topStatus->etime = content;
        info->endTime   = content;
      } break;
    case STATUS:
      if ( statusPopped ) {
        emit updateStatus();
      } else {
        emit loadItem( topStatus );
        statusPopped = true;
        /* pop the others now as well */
        emit loadItem( stack.pop() );   /* Info */
        emit loadItem( stack.pop() );   /* Preamble */
      } break;

    case ERROR:
      inError = false;
      emit loadItem( verror );
      break;
    case UNIQUE:
      if ( inError ) {
        verror->unique = content;
      } else if ( inErrorCounts && inPair ) {
        errCounts->appendPair( content );
      } break;
    case TID:
      if ( inError ) {
        verror->tid = content.toInt();
      } break;
    case KIND:
      if ( inError ) {
        verror->kind  = content;
        verror->acnym = acronym( content );
      } break;
    case WHAT:
      if ( inError ) {
        verror->what = content;
      } break;
    case AUXWHAT:
      if ( inError ) {
        verror->setAux( content );
      } break;
    case STACK:
      inStack = false;
      break;
    case FRAME:
      if ( inError && inStack && inFrame ) {
        inFrame = false;
      } break;
    case IP:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->ip = content;
      } break;
    case OBJ:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->setObj( content );
      } break;
    case FN:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->setFun( content );
      } break;
    case SRCDIR:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->setDir( content );
      } break;
    case SRCFILE:
      if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->setFile( content );
      } break;
    case LINE:
      if ( inPreamble ) {
        preamble->lines << content; 
      } else if ( inError && inStack && inFrame ) {
        verror->currStack->currFrame->setLine( content.toInt() );
      }  break;
    case ERRORCOUNTS:
      inErrorCounts = false;
      emit loadItem( errCounts );
      if ( !errCounts->isEmpty() ) {
        topStatus->num_errs = errCounts->totalErrors;
        emit updateStatus();
        emit updateErrors( errCounts );
      } break;
    case PAIR:
      if ( inErrorCounts || inSuppCounts ) {
        inPair = false;
      } break;
    case COUNT:
      if ( inErrorCounts && inPair ) {
        errCounts->num = content.toInt();
      } else if ( inSuppCounts && inPair ) {
        suppCounts->num = content.toInt();
      } break;
    case NAME:
      if ( inSuppCounts && inPair ) {
        suppCounts->appendPair( content );
      } break;
    case SUPPCOUNTS:
      inSuppCounts = false;
      emit loadItem( suppCounts );
      break;
    case LEAKEDBYTES:
      verror->setLeakedBytes( content.toInt() );
      topStatus->num_leaks += content.toInt();
      emit updateStatus();
      break;
    case LEAKEDBLOCKS:
      verror->setLeakedBlocks( content.toInt() );
      topStatus->num_blocks += content.toInt();
      emit updateStatus();
      break;
    case NONE:
      VK_DEBUG( "no such tag '%s'", endTag.latin1() );
      vk_assert_never_reached();
      break;
	  default:
      break;
  }

  /* reset content, so we can detect client output before a start tag */
  content = "";

  return true;
}



/* Reads a few lines of text from the file to try to ascertain if the
   file is in xml format
   static function
*/
bool XMLParser::xmlFormatCheck( int* err_val, QString fpath )
{
  bool ok = false;
  QFile xmlFile( fpath );
  if ( !xmlFile.open( IO_ReadOnly ) ) {
    *err_val = PERROR_BADFILE;
    goto bye;
  } else {
    QTextStream stream( &xmlFile );
    int n = 0;
    while ( !stream.atEnd() && n < 10 ) {
      QString aline = stream.readLine().simplifyWhiteSpace();
      if ( !aline.isEmpty() ) {      /* found something */
        int pos = aline.find( "<valgrindoutput>", 0 );
        if ( pos != -1 ) {
          ok = true;
          break;
        }
      }
      n++;
    }
    xmlFile.close();
  }

 bye:
  return ok;
}
