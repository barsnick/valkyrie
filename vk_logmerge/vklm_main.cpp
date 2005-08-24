/* ---------------------------------------------------------------------
 * main(): vk_logmerge program entry point                 vklm_main.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qfileinfo.h>
#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include "vklm_main.h"
#include "vk_logmerge.h"

/* Global vars */
const char* progname=0;
int vklm_verbosity=0;

#define SKIP_FILE_ON_ERROR 0  /* 0: die, 1: skip */


/* print message to stderr */
void vklmPrint( const char* msg, ... )
{
  va_list ap;
  va_start( ap, msg );
  va_end( ap );
  fprintf( stderr, "%s: ", progname );
  vfprintf( stderr, msg, ap );
  va_end( ap );
  fprintf( stderr, "\n" );
}

void usage()
{
  fprintf(stderr, "%s, a Valgrind XML log file merger.  Version 0.9.0, 25-July-2005.\n\n", progname);

  fprintf(stderr, "  usage: %s [flags and input files in any order]\n\n", progname);

  fprintf(stderr, "    -h            print this message\n");
  fprintf(stderr, "    -v            be verbose (more -v's give more)\n");
  fprintf(stderr, "    -t            output plain text (non-xml)\n");
  fprintf(stderr, "    -f log_list   obtain input files from log_list (one per line)\n");
  fprintf(stderr, "    -o outfile    write merged output to outfile\n\n");

  fprintf(stderr, "  At least 1 input file must be given.\n\n");

  fprintf(stderr, "  If no '-o outfile' is given, writes to standard output.\n\n");

  fprintf(stderr, "  Example: %s log1.xml -f loglist.fls -o merged.xml\n\n", progname);
}


/*
  Clean log of superfluous leak_errors, and empty errorcounts
*/
void clean_log( QDomDocument& domdoc )
{
  QDomElement docRoot = domdoc.documentElement();
  QDomNode n = docRoot.firstChild();

  /* remove any leak_errors output before status==FINISHED */
  while( !n.isNull() ) {
    QDomElement e = n.toElement();
    if( !e.isNull() ) {

      /* quit after status == FINISHED */
      if (e.tagName() == "status") {
	QString state = e.firstChild().toElement().text();
	if (state == "FINISHED")
	  break;
      }
      else if (e.tagName() == "error") {
	QDomNodeList err_details = e.childNodes();
	assert( err_details.count() >= 4 );
	QString kind = err_details.item(2).toElement().text();
	if ( kind.startsWith( "Leak_" ) ) {
	  /* keep valid n (guaranteed not first child) */
	  n = n.previousSibling();
	  /* remove leak error from tree */
	  docRoot.removeChild( e );
	}
      }
      else if (e.tagName() == "errorcounts") {
	if (e.childNodes().count() == 0) {
	  /* keep valid n (guaranteed not first child) */
	  n = n.previousSibling();
	  /* empty errorcount: remove from tree */
	  docRoot.removeChild( e );
	}
      }
    }
    n = n.nextSibling();
  }
}


/*
  Parse a valgrind xml log file to a QDomDocument
*/
bool parseLog( QString file_path, QDomDocument& vgLog )
{
  QFile file( file_path );
  if ( !file.open( IO_ReadOnly ) ) {
    vklmPrint("error opening file: '%s'", file_path.latin1());
    return false;
  }
  QString errMsg;
  int errLine, errCol;
  if ( !vgLog.setContent( &file, &errMsg, &errLine, &errCol ) ) {
    vklmPrint("error parsing file '%s'", file_path.latin1());
    vklmPrint("%s on line %d, column %d", errMsg.latin1(), errLine, errCol);
    file.close();
    return false;
  }
  file.close();

  /* clean log of unwanted errs, errcounts...  */
  clean_log( vgLog );

  return true;
}


/*
  Merge all valid Valgrind logs in log_files to master_log
*/
bool mergeVgLogList( QStringList& log_files, 
		     QDomDocument& master_log/*out*/ )
{
  /* check there's a minimum of one file */
  if ( log_files.count() < 1 ) {
    vklmPrint("No input files specified.  For help, try -h.");
    return false;
  }

  if (vklm_verbosity > 0)
    vklmPrint("merging logs...");

  QString master_fname;
  unsigned int lognum=0;

#if SKIP_FILE_ON_ERROR
  /* read first parseable file into master */
  for (lognum=0; lognum<log_files.count(); lognum++) {
    /* get simple filename */
    master_fname = QFileInfo( log_files[0] ).fileName();

    if ( parseLog( log_files[lognum], master_log ) )
      break;
    vklmPrint("skipping file: '%s'\n", master_fname.latin1());
  }
#else // die_on_error
  /* read first file into master */
  master_fname = QFileInfo( log_files[0] ).fileName();
  if ( ! parseLog( log_files[0], master_log ) ) {
    vklmPrint("failed to parse file: '%s'\n", master_fname.latin1());
    return false;
  }
#endif

  lognum++;

  if (lognum >= log_files.count()) {
    if (vklm_verbosity > 0)
      vklmPrint("no logs to merge\n");
  }

  /* loop over the rest of the files in the list, and merge one-by-one */
  for (; lognum<log_files.count(); lognum++) {
    /* get simple filename */
    QString slave_fname = QFileInfo( log_files[lognum] ).fileName();

    QDomDocument slave_log;
    // TODO: give DTD to use
    //  QDomDocument slave_log( "valgrind" );

    bool ok = parseLog( log_files[lognum], slave_log );
    if (!ok) {   /* failed parse/merge of slave file => skipped file */
#if SKIP_FILE_ON_ERROR
      vklmPrint("skipping file: '%s'\n", slave_fname.latin1());
      continue;
#else // DIE
      vklmPrint("failed to parse file: '%s'\n", slave_fname.latin1());
      return false;
#endif
    }
    if (vklm_verbosity > 0)
      vklmPrint("merging %s <- %s", master_fname.latin1(), slave_fname.latin1());
    
    /* --- merge the logs --- */
    ok = mergeVgLogs( master_log, slave_log );
    if (!ok) {   /* failed parse/merge of slave file => skipped file */
#if SKIP_FILE_ON_ERROR
      vklmPrint("skipping file: '%s'\n", slave_fname.latin1());
      continue;
#else // DIE
      vklmPrint("merge failed for file: '%s'\n", slave_fname.latin1());
      return false;
#endif
    }
  }
  if (vklm_verbosity > 0)
    vklmPrint("done.\n");

  return true;
}



#if 0
void printDomTree( const QDomElement &parentElement,
		   QString indent=QString("") )
{
  QDomNode n = parentElement.firstChild();

  while( !n.isNull() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString pre = indent + e.tagName();

      if (e.hasChildNodes() && e.firstChild().isElement()) {
	/* branch node */
	fprintf(stderr, "%s\n", pre.latin1());
      } else {
	/* leaf node */
	QString content = e.text();
	fprintf(stderr, "%s: %s\n", pre.latin1(), content.latin1());
      }
    }
    n = n.nextSibling();
  }
}
#endif


QString plaintxtVgPreamble( const QDomElement& preamble )
{
  QString output_str;
  QDomNodeList lines = preamble.childNodes();
  for (unsigned int i=0; i<lines.count(); i++) {
    QString content = lines.item(i).toElement().text();
    output_str += content + "\n";
  }
  output_str += "\n";
  return output_str;
}


/* ref: coregrind/m_debuginfo/symtab.c :: VG_(describe_IP) */
/* CAB: why don't print dirname for non-xml ? */
QString describe_IP( QDomNode frame )
{
  QDomNodeList frame_details = frame.childNodes();
  assert( frame_details.count() >= 1 );  /* only ip guaranteed */
  QDomElement ip     = frame_details.item( 0 ).toElement();
  QDomElement obj    = frame_details.item( 1 ).toElement();
  QDomElement fn     = frame_details.item( 2 ).toElement();
//  QDomElement dir    = frame_details.item( 3 ).toElement();
  QDomElement srcloc = frame_details.item( 4 ).toElement();
  QDomElement line   = frame_details.item( 5 ).toElement();

  bool  know_fnname  = !fn.isNull();
  bool  know_objname = !obj.isNull();
  bool  know_srcloc  = !srcloc.isNull() && !line.isNull();
//  bool  know_dirinfo = !dir.isNull();

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
    str += " (" + srcloc.text() + ":" + line.text() + ")";
  }
  return str;
}


QString plaintxtVgError( const QDomElement& errorcount,
                         const QDomElement& error )
{
  QString output_str;

  /* for non-leak errors, add count */
  QDomElement err_kind   = error.childNodes().item(2).toElement();
  if (!err_kind.text().startsWith("Leak_")) {
    QDomElement err_unique = error.childNodes().item(0).toElement();
    
    /* find count from matching pair in errorcount */
    QString count = "?";
    QDomNodeList pairs = errorcount.elementsByTagName( "pair" );
    for (unsigned int i=0; i<pairs.count(); i++) {
      QDomNodeList pair_details = pairs.item(i).childNodes();
      QDomElement unique = pair_details.item(1).toElement();
      if (unique.text() == err_unique.text() ) {
	QDomElement cnt = pair_details.item(0).toElement();
	count = cnt.text();
	break;
      }
    }
    output_str += "Occurred " + count + " times:\n";
  }

  QDomNode n = error.firstChild();
  while( !n.isNull() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tag = e.tagName();

      if (tag == "what") {
	output_str += e.text() + "\n";
      }
      else if (tag == "stack") {
	QDomNodeList frames = e.childNodes();
	for (unsigned int i=0; i<frames.count(); i++) {
	  QString ip_desc = describe_IP( frames.item(i) );
	  output_str += (i==0 ? "   at " : "   by ") + ip_desc + "\n";
	}
      }
      else if (tag == "auxwhat") {
	output_str += "  " + e.text() + "\n";
      }
    }
    n = n.nextSibling();
  }
  output_str += "\n";
  return output_str;
}


/* ref: memcheck/mac_leakcheck.c :: MAC_(do_detect_memory_leaks) */
QString plaintxtVgErrorSummary( QDomElement& errcnt, QDomElement& suppcnt )
{
  int errs=0, err_contexts=0, supps=0, supp_contexts=0;
  QDomNodeList pairs;

  /* count errors */
  pairs = errcnt.childNodes();
  err_contexts = pairs.count();
  for (unsigned int i=0; i<pairs.count(); i++) {
    QDomElement count = pairs.item(i).firstChild().toElement();
    errs += count.text().toULong();
  }

  /* count suppressions */
  pairs = suppcnt.childNodes();
  supp_contexts = pairs.count();
  for (unsigned int i=0; i<pairs.count(); i++) {
    QDomElement count = pairs.item(i).firstChild().toElement();
    supps += count.text().toULong();
  }

  QString output_str = "ERROR SUMMARY: " +
    QString::number(errs) + " errors from " +
    QString::number(err_contexts) + " contexts " +
    "(suppressed: " + QString::number(supps) + " from " +
    QString::number(supp_contexts) + ")\n\n\n";

  return output_str;
}


/* ref: memcheck/mac_leakcheck.c :: MAC_(do_detect_memory_leaks) */
/* print leak error summary (only for errs after suppcounts output) */
/* CAB: suppressed leak errs? - not printed for xml */
QString plaintxtVgLeakSummary( const QDomDocument& log )
{
  enum ErrKind { EK_UNREACHED=0, EK_INDIRECT, EK_INTERIOR, EK_PROPER };

  QMap<QString,int> errkind_map;
  errkind_map["Leak_DefinitelyLost"] = EK_UNREACHED;
  errkind_map["Leak_IndirectlyLost"] = EK_INDIRECT;
  errkind_map["Leak_PossiblyLost"  ] = EK_INTERIOR;
  errkind_map["Leak_StillReachable"] = EK_PROPER;

  typedef struct {
    unsigned long bytes, blocks;
  } Leak;

  Leak leaks[ errkind_map.count() ];
  for (unsigned int i=0; i<errkind_map.count(); i++) {
    leaks[i].bytes  = 0;
    leaks[i].blocks = 0;
  }

  /* gather details from leak errors (those after 'suppcounts') */
  bool start_leak_errors = false;
  QDomNode n = log.documentElement().firstChild();
  while( !n.isNull() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tag = e.tagName();

      if (tag == "suppcounts") {
	start_leak_errors = true;
      }
      else if (start_leak_errors && tag == "error") {
	QDomNodeList err_details = e.childNodes();
	assert( err_details.count() >= 6 ); /* leak errs only */
	QString kind   = err_details.item( 2 ).toElement().text();
	QString bytes  = err_details.item( 4 ).toElement().text();
	QString blocks = err_details.item( 5 ).toElement().text();

	leaks[ errkind_map[ kind ] ].bytes  += bytes.toULong();
	leaks[ errkind_map[ kind ] ].blocks += blocks.toULong();
      }
    }
    n = n.nextSibling();
  }

#define LEAK_STR( lk ) QString( \
  QString::number(lk.bytes) + " bytes in " + \
  QString::number(lk.blocks) + " blocks.\n" )

  QString output_str = "LEAK SUMMARY\n";
  output_str += "   definitely lost: " + LEAK_STR(leaks[EK_UNREACHED]);
  if (leaks[EK_INDIRECT].blocks > 0)
  output_str += "   indirectly lost: " + LEAK_STR(leaks[EK_INDIRECT]);
  output_str += "     possibly lost: " + LEAK_STR(leaks[EK_INTERIOR]);
  output_str += "   still reachable: " + LEAK_STR(leaks[EK_PROPER]);

  return output_str;
}

/* print entire log to plain text string,
   a-la valgrind non-xml output */
QString plaintxtVgLog( const QDomDocument& log )
{
  QDomElement last_errcnt;
  QString output_str;

  /* get last errorcounts element */
  QDomNode node = log.documentElement().lastChild();
  while( !node.isNull() ) {
    if ( node.isElement() ) {
      QDomElement e = node.toElement();
      QString tag = e.tagName();

      if (tag == "errorcounts") {
 	last_errcnt = e;
	break;
      }
    }
    node = node.previousSibling();
  }

  QDomNode n = log.documentElement().firstChild();
  while( !n.isNull() ) {
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tag = e.tagName();

      if (tag == "preamble") {
	output_str += plaintxtVgPreamble( e );
      }
      else if (tag == "error") {
	output_str += plaintxtVgError( last_errcnt, e );
      }
      else if (tag == "suppcounts") {
	output_str += plaintxtVgErrorSummary( last_errcnt, e );
      }
    }
    n = n.nextSibling();
  }

  /* print leak summary (memcheck only) */
  output_str += plaintxtVgLeakSummary( log );

  return output_str;
}




/*
  Main
*/
int main ( int argc, char* argv[] )
{
  QString outfile;
  QStringList log_files;
  bool plaintext_output = false;

  QString fname_nopath = QFileInfo(argv[0]).fileName();
  progname = fname_nopath.latin1();

  /*
    parse command-line args
  */
  int c;
  while ((c = getopt (argc, argv, "-hvtf:o:")) != -1) {
    switch (c) {
    case 'h':  /* help */
      usage();
      return 0;
      
    case 'v':  /* be verbose */
      vklm_verbosity++;
      break;
      
    case 1:  /* the '-' in optstring puts non-option args here */
      log_files << optarg;    /* we validate the file later */
      break;

    case 'f': {
      QFile loglist( optarg );
      if ( !loglist.open( IO_ReadOnly ) ) {
	vklmPrint("unable to open logfile '%s'.", optarg);
	return 1;
      }

      /* add each logfile in loglist to our list */
      QTextStream stream( &loglist );
      while ( !stream.atEnd() ) {
	QString logfile = stream.readLine().simplifyWhiteSpace();
	if ( logfile.isEmpty() )   /* skip empty lines */
          continue;
	log_files << logfile;      /* validate the file later. */
      }
      loglist.close();
      break;
    }

    case 'o':
      outfile = optarg;
      break;
      
    case 't':
      plaintext_output = true;
      break;
      
    case '?':
      usage();
      return 1;
    default:
      abort();
    }
  }

  /*
    merge
  */
  QDomDocument mergedLog;    // TODO: give DTD: QDomDocument( "valgrind" );
  if ( ! mergeVgLogList( log_files, mergedLog ) ) {
    vklmPrint("quitting...\n");
    return 1;
  }

  /*
    output result
  */
  QString output_str;
  if (plaintext_output)
    output_str = plaintxtVgLog( mergedLog );
  else
    output_str = mergedLog.toString( 2/*indent*/ );

  if ( ! outfile.isEmpty() ) {
    /* write to file */
    QFile outFile( outfile );
    if ( !outFile.open( IO_WriteOnly ) ) {
      vklmPrint("error: unable to open file for writing: '%s'", 
                outfile.latin1() );
      vklmPrint("failed to save merge result\n");
      return 1;
    }

    QTextStream outStream( &outFile );
    outStream << output_str;
    outFile.close();
    if (vklm_verbosity > 0)
      vklmPrint("output saved to '%s'\n", outfile.latin1());

  } else {
    /* write to stdout */
    fprintf( stdout, "%s", output_str.latin1() );
  }

  return 0;
}

