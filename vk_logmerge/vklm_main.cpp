/* ---------------------------------------------------------------------
 * main(): vk_logmerge program entry point                 vklm_main.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <sys/types.h>   // getpid
#include <unistd.h>      // getpid
#include <stdlib.h>      // exit

#include <qfileinfo.h>
#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

#include "vklm_main.h"
#include "vglogreader.h"
#include "vglog.h"



/* Global vars */
const char* progname=0;
int vklm_verbosity=0;

#define SKIP_FILE_ON_ERROR 0  /* 0: die, 1: skip */


/* print message to stderr */
void vklmPrint( int verb, const char* msg, ... )
{
   if (vklm_verbosity < verb)
      return;
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stderr, "===%s:%d=== ", progname, (int)getpid() ); 
   vfprintf( stderr, msg, ap );
   va_end( ap );
   fprintf( stderr, "\n" );
}

/* print message to stderr */
void vklmPrintErr( const char* msg, ... )
{
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stderr, "===%s:%d=== Error: ", progname, (int)getpid() ); 
   vfprintf( stderr, msg, ap );
   va_end( ap );
   fprintf( stderr, "\n" );
}


void usage()
{
   fprintf(stderr, "%s, a Valgrind XML log file merger.  Version 1.1.0, 27-Nov-2005.\n\n", progname);

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
  Parse a valgrind xml log file to a VgLog
*/
bool parseLog( QString file_path, VgLog& vgLog )
{
#if 1  // TODO: validate file
   QFile file( file_path );
   if ( !file.open( IO_ReadOnly ) ) {
      vklmPrint( 0, "error opening file: '%s'", file_path.latin1() );
      return false;
   }
   file.close();
#endif

   VgLogReader reader( &vgLog );
   bool ok = reader.parse( file_path );
   if (!ok) {
      VgLogHandler* hnd = reader.handler();
      vklmPrint( 0, "error parsing file '%s'", file_path.latin1() );
      vklmPrint( 0, " - %s", hnd->fatalMsg().latin1() );
      return false;
   }

   return true;
}


/*
  Merge all valid Valgrind logs in log_files to master_log
*/
bool mergeVgLogList( QStringList& log_files, 
                     VgLog& master_log/*out*/ )
{
   /* check there's a minimum of one file */
   if ( log_files.count() < 1 ) {
      vklmPrint( 0, "No input files specified.  For help, try -h." );
      return false;
   }

   vklmPrint( 1, "merging logs..." );

   QString master_fname;
   unsigned int lognum=0;

#if SKIP_FILE_ON_ERROR
   /* read first parseable file into master */
   for (lognum=0; lognum<log_files.count(); lognum++) {
      /* get simple filename */
      master_fname = QFileInfo( log_files[0] ).fileName();

      if ( parseLog( log_files[lognum], master_log ) )
         break;
      vklmPrint( 0, "skipping file: '%s'", master_fname.latin1() );
      vklmPrint( 0, " " );
   }
#else // DIE_ON_ERROR
   /* read first file into master */
   master_fname = QFileInfo( log_files[0] ).fileName();
   if ( ! parseLog( log_files[0], master_log ) ) {
      vklmPrint( 0, "failed to parse file: '%s'", master_fname.latin1() );
      vklmPrint( 0, " " );
      return false;
   }
#endif

   lognum++;

   if (lognum >= log_files.count()) {
      vklmPrint( 1, "no logs to merge" );
      vklmPrint( 1, " " );
   }

   /* loop over the rest of the files in the list, and merge one-by-one */
   for (; lognum<log_files.count(); lognum++) {
      /* get simple filename */
      QString slave_fname = QFileInfo( log_files[lognum] ).fileName();

      VgLog slave_log;
      // TODO: give DTD to use
      //  VgLog slave_log( "valgrind" );

      bool ok = parseLog( log_files[lognum], slave_log );
      if (!ok) {   /* failed parse/merge of slave file => skipped file */
#if SKIP_FILE_ON_ERROR
         vklmPrint( 0, "skipping file: '%s'", slave_fname.latin1() );
         vklmPrint( 0, " " );
         continue;
#else // DIE_ON_ERROR
         vklmPrint( 0, "failed to parse file: '%s'", slave_fname.latin1() );
         vklmPrint( 0, " " );
         return false;
#endif
      }

      vklmPrint( 2, "***********************************************");
      vklmPrint( 1, "merging %s <- %s", master_fname.latin1(), slave_fname.latin1() );

      /* --- merge the logs --- */
      ok = master_log.merge( slave_log );

      if (!ok) {   /* failed parse/merge of slave file => skipped file */
#if SKIP_FILE_ON_ERROR
         vklmPrint( 0, "skipping file: '%s'", slave_fname.latin1() );
         vklmPrint( 0, " " );
         continue;
#else // DIE_ON_ERROR
         vklmPrint( 0, "merge failed for file: '%s'", slave_fname.latin1() );
         vklmPrint( 0, " " );
         return false;
#endif
      }
   }
   vklmPrint( 1, "done." );

   return true;
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
            vklmPrint( 0, "unable to open logfile '%s'.", optarg );
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
   VgLog mergedLog;    // TODO: give DTD: VgLog( "valgrind" );
   if ( ! mergeVgLogList( log_files, mergedLog ) ) {
      vklmPrint( 0, "quitting..." );
      return 1;
   }

   /*
     output result
   */
   QString output_str;
   if (plaintext_output)
      output_str = mergedLog.toPlainTxt();
   else
      output_str = mergedLog.toString();

   if ( ! outfile.isEmpty() ) {
      /* write to file */
      QFile outFile( outfile );
      if ( !outFile.open( IO_WriteOnly ) ) {
         vklmPrint( 0, "error: unable to open file for writing: '%s'", 
                    outfile.latin1() );
         vklmPrint( 0, "failed to save merge result\n" );
         return 1;
      }

      QTextStream outStream( &outFile );
      outStream << output_str;
      outFile.close();
      vklmPrint( 1, "output saved to '%s'\n", outfile.latin1() );

   } else {
      /* write to stdout */
      fprintf( stdout, "%s", output_str.latin1() );
   }

   return 0;
}

