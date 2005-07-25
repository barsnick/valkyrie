/* ---------------------------------------------------------------------
 * Implementation of VKLogMerge                          vk_logmerge.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_logmerge.h"
#include "logfile.h"
#include "vk_popt_option.h"
#include "vk_option.h"
#include "vk_utils.h"

#include <qfile.h>
#include <qfileinfo.h>


VKLogMerge::VKLogMerge( QObject* parent, const char* name )
  : QObject( parent, name )
{
  logStream.setEncoding( QTextStream::UnicodeUTF8 );
  xmlParser = new XMLParser( this, true );
  reader.setContentHandler( xmlParser );
  reader.setErrorHandler( xmlParser );
}

VKLogMerge::~VKLogMerge() { }



QString VKLogMerge::validateFile( QString& log_file,  bool& ok ) 
{
  int errval = PARSED_OK;

  /* check this is a valid file, and has the right perms */
  QString ret_file = fileCheck( &errval, log_file.latin1(), true, false );
  if ( errval != PARSED_OK ) {
    ok = false;
    fprintf(stderr ,"File Error: %s: \n\"%s\"", 
            parseErrString(errval), 
            escapeEntities(log_file).latin1() );
    return QString::null;
  }

  /* check the file is readable, and the format is xml */
  bool is_xml = XMLParser::xmlFormatCheck( &errval, log_file );
  if ( errval != PARSED_OK ) {
    ok = false;
    fprintf(stderr, "File Error: %s: \n\"%s\"", 
	    parseErrString(errval), log_file.latin1() );
    return QString::null;
  }

  if ( !is_xml ) {
    ok = false;
    fprintf(stderr, "File Format Error: File '%s' not in xml format.",
	    log_file.latin1() );
    return QString::null;
  }

  ok = true;
  return ret_file;
}



/* opens a logfile and feeds the contents to the parser. 
   the logfile's existence, perms and format have all been
   pre-validated, so no need to re-check. */
bool VKLogMerge::parseLog( QString log_filename )
{
  QFileInfo fi( log_filename );
  fprintf(stderr, "Parsing %s\n", fi.fileName().latin1());

  QFile logFile( log_filename );
  logFile.open( IO_ReadOnly );

  int lineNumber = 1;
  QTextStream stream( &logFile );
  stream.setEncoding( QTextStream::UnicodeUTF8 );
  QString inputData = stream.readLine();
  source.setData( inputData );
  bool ok = reader.parse( &source, true );

  while ( !stream.atEnd() ) {
    lineNumber++;
    inputData = stream.readLine();
    source.setData( inputData );
    ok = reader.parseContinue();
    if ( !ok ) {
      fprintf(stderr, "Parse Error: Parsing failed on line no %d: '%s'</p>", 
	      lineNumber, inputData.latin1() );
      break;
    }
  }

  logFile.close();

  return ok;
}



/* if --merge=<file_list> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the open-file-dialog in the gui,
   called by MemcheckView::openMergeFile().  either way, the value in
   [valkyrie:merge] is what we need to know */
bool VKLogMerge::mergeLogFiles( QString& log_list, QString& fname_out )
{
  QFile logFile( log_list );
  QFileInfo fi( log_list );

  if ( !(fi.isFile() && logFile.open( IO_ReadOnly )) ) {
    fprintf(stderr, "Open File Error: Unable to open logfile '%s'.\n", log_list.latin1() );
    return false;
  }

  fprintf(stderr, "Merging logs in %s\n", fi.fileName().latin1());

  /* iterate through the list of files, validating each one as we go.
     if a file is valid, bung it in the list, else skip it.
     if we bomb out > 5 times, offer a chance to cancel the operation. */
  QStringList logFileList;

  QTextStream stream( &logFile );
  while ( !stream.atEnd() ) {
    QString temp = stream.readLine().simplifyWhiteSpace();
    /* skip empty lines */
    if ( temp.isEmpty() )
      continue;
    /* check re file perms and format */
    bool valid;
    temp = validateFile( temp, valid );
    if ( valid ) {
      logFileList << temp;
    } else {
      /* die on first error */
      return false;
    }
  }
  logFile.close();

  /* check there's a minimum of two files to merge */
  if ( logFileList.count() < 2 ) {
    fprintf(stderr, "Merge LogFiles Error: Need minimum of 2 files to merge.\n");
    return false;
  }

  /* create a 'master' LogFile, and parse the contents of file #1 into it.
     subsequent files are parsed one-by-one, and compared with the
     'master'; duplicates are merged where found, and stuff like
     <errcounts> and <suppcounts> are incremented to reflect any
     merges.  the 'master' contains the final output */
  LogFile* masterLogFile = new LogFile( logFileList[0] );
  xmlParser->reset();
  source.reset();
  connect( xmlParser,     SIGNAL(loadItem(XmlOutput *)), 
           masterLogFile, SLOT(loadItem(XmlOutput *)) );

  fi.setFile( logFileList[0] );
  QString master_fname = fi.fileName();
  /* parse the first xml_logfile into the master */
  parseLog( logFileList[0] );

  /* disconnect the master so it no longer communicates with the parser */
  disconnect( xmlParser,     SIGNAL(loadItem(XmlOutput *)), 
              masterLogFile, SLOT(loadItem(XmlOutput *)) );

  /* loop over the rest of the files in the list, and merge one-by-one */
  QString slave_fname;
  LogFile* slaveLogFile;
  for ( unsigned int i=1; i<logFileList.count(); i++ ) {

    /* reset everything */
    fi.setFile( logFileList[i] );
    slave_fname = fi.fileName();    
    xmlParser->reset();
    source.reset();

    /* create a new LogFile */
    slaveLogFile = new LogFile( logFileList[i] );
    connect( xmlParser,    SIGNAL(loadItem(XmlOutput *)), 
             slaveLogFile, SLOT(loadItem(XmlOutput *)) );

    /* parse the next file in the list into logFile */
    parseLog( logFileList[i] );

    /* tell user we are merging the slave into the master */
    fprintf(stderr, "Merging %s <- %s\n", master_fname.latin1(), slave_fname.latin1());

    masterLogFile->merge( slaveLogFile );

    /* disconnect the logFile from the parser */
    disconnect( xmlParser,    SIGNAL(loadItem(XmlOutput *)), 
                slaveLogFile, SLOT(loadItem(XmlOutput *)) );

    /* delete the slave and free memory */
    delete slaveLogFile;
    slaveLogFile = 0;
  }

  bool fileSaved = masterLogFile->save( fname_out );

  fprintf(stderr, "Merge Complete\n");
  if ( fileSaved ) {
    if (!fname_out.isEmpty()) {
      fprintf(stderr, "Output saved to '%s'\n", fname_out.latin1());
    }
  } else {
    fprintf(stderr, "Failed to save result\n");
  }

  /* delete the master and free memory, and we're done */
  delete masterLogFile;
  masterLogFile = 0;

  return true;
}
