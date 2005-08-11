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

#include "vklm_main.h"
#include "vk_logmerge.h"

/* Global vars */
const char* progname=0;
int vklm_verbosity=0;

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
  fprintf(stderr, "%s, a valgrind log file merger.  Version 0.9.0, 25-July-2005.\n\n", progname);

  fprintf(stderr, "  usage: %s [flags and input files in any order]\n\n", progname);

  fprintf(stderr, "    -h            print this message\n");
  fprintf(stderr, "    -v            be verbose (more -v's give more)\n");
  fprintf(stderr, "    -f log_list   obtain input files from log_list (one per line)\n");
  fprintf(stderr, "    -o outfile    write merged output to outfile\n\n");

  fprintf(stderr, "  At least 2 input files must be given.\n\n");

  fprintf(stderr, "  If no '-o outfile' is given, writes to standard output.\n\n");

  fprintf(stderr, "  Example: %s log1.xml -f loglist.fls -o merged.xml\n\n", progname);
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
  return true;
}


/*
  Merge all valid Valgrind logs in log_files to master_log
*/
bool mergeVgLogList( QStringList& log_files, 
		     QDomDocument& master_log/*out*/ )
{
  /* check there's a minimum of two files to merge */
  if ( log_files.count() < 1 ) {
    vklmPrint("no input logs given\n");
    return false;
  }
  if ( log_files.count() < 2 ) {
    vklmPrint("need minimum of 2 files to merge\n");
    return false;
  }

  if (vklm_verbosity > 0)
    vklmPrint("merging logs...");

  /* read first parseable file into master */
  QString master_fname;
  unsigned int lognum;
  for (lognum=0; lognum<log_files.count()-1; lognum++) {
    /* get simple filename */
    master_fname = QFileInfo( log_files[0] ).fileName();

    if ( parseLog( log_files[lognum], master_log ) )
      break;
    vklmPrint("skipping merge of file: '%s'\n", master_fname.latin1());
  }
  if (lognum >= log_files.count()-1) {
    vklmPrint("need minimum of 2 files to merge\n");
    return false;
  }

  /* loop over the rest of the files in the list, and merge one-by-one */
  bool merged_once = false;
  for (lognum++; lognum<log_files.count(); lognum++) {
    /* get simple filename */
    QString slave_fname = QFileInfo( log_files[lognum] ).fileName();

    QDomDocument slave_log;
    // TODO: give DTD to use
    //  QDomDocument slave_log( "valgrind" );

    bool ok = parseLog( log_files[lognum], slave_log );
    if (ok) {    
      if (vklm_verbosity > 0)
	vklmPrint("merging %s <- %s", master_fname.latin1(), slave_fname.latin1());

      /* --- merge the logs --- */
      ok = mergeVgLogs( master_log, slave_log );
    }
    if (!ok) {   /* failed parse/merge of slave file => skipped file */
      vklmPrint("skipping merge of file: '%s'\n", slave_fname.latin1());
    } else {
      merged_once = true;
    }
  }
  if (!merged_once) {
    vklmPrint("need minimum of 2 files to merge\n");
    return false;
  }
  if (vklm_verbosity > 0)
    vklmPrint("merge complete\n");

  return true;
}



/*
  Main
*/
int main ( int argc, char* argv[] )
{
  QString outfile;
  QStringList log_files;

  QString fname_nopath = QFileInfo(argv[0]).fileName();
  progname = fname_nopath.latin1();

  /*
    parse command-line args
  */
  int c;
  while ((c = getopt (argc, argv, "-hvf:o:")) != -1) {
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
    return 1;
  }

  /*
    output result
  */
  QString xml_string = mergedLog.toString( 2/*indent*/ );

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
    outStream << xml_string;
    outFile.close();
    vklmPrint("output saved to '%s'\n", outfile.latin1());

  } else {
    /* write to stdout */
    fprintf( stdout, "%s", xml_string.latin1() );
  }

  return 0;
}

