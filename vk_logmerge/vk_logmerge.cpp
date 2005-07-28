/* ---------------------------------------------------------------------
 * Implementation of VKLogMerge                          vk_logmerge.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_logmerge.h"
#include "vk_popt_option.h"
#include "vk_option.h"
#include "vk_utils.h"

#include <qfile.h>
#include <qfileinfo.h>


VKLogMerge::VKLogMerge( QObject* parent, const char* name )
  : QObject( parent, name )
{ }

VKLogMerge::~VKLogMerge() { }


/* check is valid file, correct perms, and format is xml.
   returns absolute path of log_file.
   returns QString::null on error.
*/
QString VKLogMerge::validateFile( QString& log_file ) 
{
  int errval = PARSED_OK;

  /* check this is a valid file, and has the right perms */
  QString ret_file = fileCheck( &errval, log_file.latin1(), true, false );
  if ( errval != PARSED_OK ) {
    fprintf(stderr ,"File Error (%s): '%s'\n", 
            parseErrString(errval), 
            escapeEntities(log_file).latin1() );
    return QString::null;
  }

  /* check the file is readable, and the format is xml */
  bool is_xml = XMLParser::xmlFormatCheck( &errval, log_file );
  if ( errval != PARSED_OK ) {
    fprintf(stderr, "File Error (%s): '%s'\n", 
            parseErrString(errval),
            log_file.latin1() );
    return QString::null;
  }

  if ( !is_xml ) {
    fprintf(stderr, "File Format Error: File '%s' not in xml format.\n",
	    log_file.latin1() );
    return QString::null;
  }

  return ret_file;
}



/* opens a logfile and feeds the contents to the parser. 
   the logfile's existence, perms and format have all been
   pre-validated, so no need to re-check. */
bool VKLogMerge::parseLog( XMLParser* xmlParser,
                           VGLog* vgLog,
                           QXmlSimpleReader& reader,
                           QString& log_fname )
{
  connect( xmlParser, SIGNAL(loadItem(XmlOutput *)), 
           vgLog,       SLOT(loadItem(XmlOutput *)) );

  QXmlInputSource source;

  QString fname = QFileInfo( log_fname ).fileName();
  fprintf(stderr, "Parsing %s\n", fname.latin1());

  QFile logFile( log_fname );
  logFile.open( IO_ReadOnly );

  int lineNumber = 1;
  QTextStream stream( &logFile );
  stream.setEncoding( QTextStream::UnicodeUTF8 );
  QString inputData = stream.readLine();
  source.setData( inputData );
  bool ok = reader.parse( &source, true );

  while ( ok && !stream.atEnd() ) {
    lineNumber++;
    inputData = stream.readLine();
    source.setData( inputData );
    ok = reader.parseContinue();
  }

  logFile.close();

  if ( !ok ) {
    fprintf(stderr, "Parse Error: Parsing failed on line no %d: '%s'\n", 
	    lineNumber, inputData.latin1() );
  }

  /* disconnect the parser so it no longer communicates with the logfile */
  disconnect( xmlParser, SIGNAL(loadItem(XmlOutput *)), 
              vgLog,       SLOT(loadItem(XmlOutput *)) );
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
    temp = validateFile( temp );
    if ( !temp.isNull() ) {
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
  VGLog masterLog( logFileList[0] );

  QString master_fname = QFileInfo( logFileList[0] ).fileName();

  /* Note: Need separate XMLParser for masterLog,
     since XMLParser::reset() will delete the object pointers */
  XMLParser xmlParser_mastr( this, true );
  xmlParser_mastr.reset();
  QXmlSimpleReader reader_mastr;
  reader_mastr.setContentHandler( &xmlParser_mastr );
  reader_mastr.setErrorHandler( &xmlParser_mastr );

  /* parse the first logfile into the master */
  bool parse_ok = parseLog( &xmlParser_mastr, &masterLog, 
                            reader_mastr, logFileList[0] );

  if (!parse_ok) {   /* failed parse on master file: die */
    fprintf(stderr, "Failed merge.\n");
    return false;
  }

  /* loop over the rest of the files in the list, and merge one-by-one */
  QString slave_fname;
  for ( unsigned int i=1; i<logFileList.count(); i++ ) {

    XMLParser xmlParser_slave( this, true );
    xmlParser_slave.reset();
    QXmlSimpleReader reader_slave;
    reader_slave.setContentHandler( &xmlParser_slave );
    reader_slave.setErrorHandler( &xmlParser_slave );

    /* set filename */
    slave_fname = QFileInfo( logFileList[i] ).fileName();

    /* create a new VGLog */
    VGLog slaveLog( logFileList[i] );

    /* parse the next file in the list into slaveLog */
    parse_ok = parseLog( &xmlParser_slave, &slaveLog, 
                         reader_slave, logFileList[i] );

    if (!parse_ok) {   /* failed merge on slave file: skip file */
      fprintf(stderr, "Skipping merge of slave file: %s\n", slave_fname.latin1());
      continue;
    }

    /* tell user we are merging the slave into the master */
    fprintf(stderr, "Merging %s <- %s\n", master_fname.latin1(), slave_fname.latin1());

    bool merge_ok = masterLog.merge( &slaveLog );

    if (!merge_ok) {   /* failed merge on slave file: skipped file */
      fprintf(stderr, "Skipping merge of slave file: %s\n", slave_fname.latin1());
    }
  }

  bool fileSaved = masterLog.save( fname_out );

  fprintf(stderr, "Merge Complete\n");
  if ( fileSaved ) {
    if (!fname_out.isEmpty()) {
      fprintf(stderr, "Output saved to '%s'\n", fname_out.latin1());
    }
  } else {
    fprintf(stderr, "Failed to save result\n");
  }

  return true;
}
