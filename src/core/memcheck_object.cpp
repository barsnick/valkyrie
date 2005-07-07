/* --------------------------------------------------------------------- 
 * Implementation of class Memcheck                  memcheck_object.cpp
 * Memcheck-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "memcheck_object.h"
#include "vk_config.h"
#include "html_urls.h"
#include "vk_messages.h"
#include "logfile.h"

#include <qapplication.h>


/* class Memcheck ------------------------------------------------------ */
Memcheck::~Memcheck()
{
  if ( xmlParser != 0 ) {
    delete xmlParser;
    xmlParser = 0;
  }

}


Memcheck::Memcheck() 
  : ToolObject( MEMCHECK, "Memcheck", "&Memcheck", Qt::SHIFT+Qt::Key_M ) 
{
  /* init vars */
  memcheckView = 0;

	logStream.setEncoding( QTextStream::UnicodeUTF8 );
  xmlParser = new XMLParser( this, true );
  reader.setContentHandler( xmlParser );
  reader.setErrorHandler( xmlParser );
  

  /* initialise memcheck-specific options */
  addOpt( PARTIAL,     Option::ARG_BOOL,   Option::CHECK, 
          "memcheck",  '\0',               "partial-loads-ok",
          "<yes|no>",  "yes|no",           "yes",
          "Ignore errors on partially invalid addresses",
          "too hard to explain here; see manual",
          urlMemcheck::Partial );
  addOpt( FREELIST,    Option::ARG_UINT,   Option::LEDIT, 
          "memcheck",  '\0',               "freelist-vol",
          "<number>",  "",                 "1000000",
          "Volume of freed blocks queue:",
          "volume of freed blocks queue",
          urlMemcheck::Freelist );
  addOpt( LEAK_CHECK,  Option::ARG_STRING, Option::COMBO, 
          "memcheck",  '\0',               "leak-check",
          "<no|summary|full>",  "no|summary|full",  "summary",
          "Search for memory leaks at exit:",
          "search for memory leaks at exit?",
          urlMemcheck::Leakcheck );
  addOpt( LEAK_RES,    Option::ARG_STRING, Option::COMBO, 
          "memcheck",  '\0',               "leak-resolution",
          "<low|med|high>", "low|med|high", "low",
          "Degree of backtrace merging:",
          "how much backtrace merging in leak check", 
          urlMemcheck::Leakres );
  addOpt( SHOW_REACH,  Option::ARG_BOOL,   Option::CHECK, 
          "memcheck",  '\0',               "show-reachable",
          "<yes|no>",  "yes|no",           "no",
          "Show reachable blocks in leak check",
          "show reachable blocks in leak check?",  
          urlMemcheck::Showreach );
  addOpt( GCC_296,     Option::ARG_BOOL,   Option::CHECK, 
          "memcheck",  '\0',               "workaround-gcc296-bugs",
          "<yes|no>",  "yes|no",           "no",
          "Work around gcc-296 bugs",
          "self explanatory",  
          urlMemcheck::gcc296 );
  addOpt( ALIGNMENT,  Option::ARG_UINT,   Option::SPINBOX, 
          "memcheck",  '\0',               "alignment", 
           "<number>", "8|1048576",        "8",
          "Minimum alignment of allocations",
          "set minimum alignment of allocations", 
           urlVgCore::Alignment );
  addOpt( STRLEN,       Option::ARG_BOOL,   Option::CHECK, 
          "memcheck",   '\0',               "avoid-strlen-errors",
          "<yes|no>",   "yes|no",           "yes",
          "Suppress errors from inlined strlen",
          "suppress errors from inlined strlen",  
          urlMemcheck::Strlen );
}


int Memcheck::checkOptArg( int optid, const char* argval, 
                           bool use_gui/*=false*/ )
{
  int errval = PARSED_OK;
  QString argVal( argval );
  Option* opt = findOption( optid );

  switch ( optid ) {

    case PARTIAL:
    case FREELIST:
    case LEAK_CHECK:
    case LEAK_RES:
    case SHOW_REACH:
    case GCC_296:
    case STRLEN:
      opt->isValidArg( &errval, argval );
      break;

    case ALIGNMENT: /* check is really a number, then if is power of two */
      opt->isPowerOfTwo( &errval, argval ); 
      break;

  }

  /* if this option has been called from the cmd-line, save its value
     in valkyrierc if it has passed the checks. */
  if ( errval == PARSED_OK && use_gui == false ) {
    writeOptionToConfig( opt, argval );
  }

  return errval;
}



/* returns the ToolView window (memcheckView) for this tool */
ToolView* Memcheck::toolView( QWidget* parent )
{
  usingGui = true;
  memcheckView = new MemcheckView( parent, this );
  memcheckView->setState( is_Running );
  return (ToolView*)memcheckView;
}


void Memcheck::emitRunning( bool run )
{
  is_Running = run;
  emit running( is_Running );

  if ( usingGui ) {
    memcheckView->setState( is_Running );
  }
}


/* in gui mode, outputs a message to the status bar.
   in non-gui mode, prints a msg to stdout */
void Memcheck::statusMsg( QString hdr, QString msg ) 
{ 
  if ( usingGui )
    emit message( hdr + ": " + msg );
  else
    vkInfo( 0, hdr, msg );
}


/* called by MainWin::closeToolView() */
bool Memcheck::closeView()
{
  /* if current process is not yet finished, ask user if they really
     want to close */
  if ( is_Running ) {
    int ok = vkQuery( memcheckView, "Process Running", "&Abort;&Cancel",
                      "<p>The current process is not yet finished.</p>"
                      "<p>Do you want to abort it ?</p>" );
    if ( ok == MsgBox::vkYes ) {
      VK_DEBUG("TODO: stop();");           /* abort */
    } else if ( ok == MsgBox::vkNo ) {
      return false;                        /* continue */
    }
  }

  /* currently loaded / parsed stuff isn't saved to disk */
  if ( !fileSaved ) {
    int ok = vkQuery( memcheckView, "Unsaved File", 
                      "&Save;&Discard;&Cancel",
                      "<p>The current output is not saved."
                      "Do you want to save it ?</p>" );
    if ( ok == MsgBox::vkYes ) {
      VK_DEBUG("TODO: save();\n");         /* save */
    } else if ( ok == MsgBox::vkCancel ) {
      return false;                        /* procrastinate */
    }
  }

  emit message( "" );  /* clear the status bar */
  return memcheckView->close();
}


/* when --view-log=<file> is set on the cmd-line, valkyrie checks the
   file's perms + format, and writes the value to [valkyrie:view-log].
   However, when a file is set via the open-file-dialog in the gui, or
   is contained with in a list of logfiles-to-merge, then perms +
   format checks must be made */
QString Memcheck::validateFile( QString log_file,  bool *ok ) 
{
  int errval = PARSED_OK;

  /* check this is a valid file, and has the right perms */
  QString ret_file = fileCheck( &errval, log_file.latin1(), true, false );
  if ( errval != PARSED_OK ) {
    *ok = vkError( memcheckView, "File Error", 
                   "%s: \n\"%s\"", 
                   parseErrString(errval), 
                   escapeEntities(log_file).latin1() );
    return QString::null;
  }

  /* check the file is readable, and the format is xml */
  bool is_xml = xmlFormatCheck( &errval, log_file );
  if ( errval != PARSED_OK ) {
    *ok = vkError( memcheckView, "File Error", 
                   "%s: \n\"%s\"", 
                   parseErrString(errval), log_file.latin1() );
    return QString::null;
  }

  if ( !is_xml ) {
    *ok = vkError( memcheckView, "File Format Error", 
                   "<p>The file '%s' is not in xml format.</p>",
                   log_file.latin1() );
    return QString::null;
  }

  *ok = true;
  return ret_file;
}


/* called by mergeLogFiles() and parseLogFile().
   opens a logfile and feeds the contents to the parser. 
   the logfile's existence, perms and format have all been
   pre-validated, so no need to re-check. */
bool Memcheck::parseLog( QString log_filename )
{
  QFileInfo fi( log_filename );
  statusMsg( "Parsing", fi.fileName() );

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
      vkError( memcheckView, "Parse Error",
               "<p>Parsing failed on line no %d: '%s'</p>", 
               lineNumber, inputData.latin1() );
      break;
    }
  }

  logFile.close();

  return ok;
}


/* if --view-log=<file> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the open-file-dialog in the gui,
   called by MemcheckView::openLogFile().  either way, the value in
   [valkyrie:view-log] is what we need to know. 
   if 'checked' == true, the file has already been checked
   w.r.t. perms and format. */
bool Memcheck::parseLogFile( bool checked/*=true*/ )
{
  if ( usingGui ) {
    /* tell valkyrie what we are doing */
    ;//FIXME: emit setRunMode( Valkyrie::modeParseLog );
  }

  QString log_file = vkConfig->rdEntry( "view-log", "valkyrie" );
  if ( !checked ) {
    bool valid = false;
    log_file = validateFile( log_file, &valid );
    if ( valid ) {
      vkConfig->wrEntry( log_file, "view-log", "valkyrie" );
    } else {
      vkConfig->wrEntry( "", "view-log", "valkyrie" );
      return false;
    }
  }

  /* fileSaved is always true here 'cos we are just parsing a file
     which already exists on disk */
  fileSaved = true;
  emitRunning( true );

  connect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
           memcheckView, SLOT(loadItem(XmlOutput *)) );
  connect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
           memcheckView, SLOT(updateErrors(ErrCounts*)) );
  connect( xmlParser,    SIGNAL(updateStatus()),
           memcheckView, SLOT(updateStatus()) );

  xmlParser->reset();
  source.reset();
  memcheckView->clear();

  bool success = parseLog( log_file);

  disconnect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
              memcheckView, SLOT(loadItem(XmlOutput *)) );
  disconnect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
              memcheckView, SLOT(updateErrors(ErrCounts*)) );
  disconnect( xmlParser,    SIGNAL(updateStatus()),
              memcheckView, SLOT(updateStatus()) );

  emitRunning( false );

  QString hdr = ( success ) ? "Loaded" : "Parse failed";
  statusMsg( hdr, log_file );

  /* if we are in non-gui mode, tell valkyrie we are done */
  emit finished();

  return success;
}


/* if --merge=<file_list> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the open-file-dialog in the gui,
   called by MemcheckView::openMergeFile().  either way, the value in
   [valkyrie:merge] is what we need to know */
bool Memcheck::mergeLogFiles()
{
  if ( usingGui ) {
    /* tell valkyrie what we are doing */
    ;//FIXME: emit setRunMode( Valkyrie::modeMergeLogs );
  }

  fileSaved = false;
  emitRunning( true );

  QString log_list = vkConfig->rdEntry( "merge", "valkyrie" );
  QFile logFile( log_list );
  if ( !logFile.open( IO_ReadOnly ) ) {
    emitRunning( false );
    return vkError( memcheckView, "Open File Error", 
           "<p>Unable to open logfile '%s'</p>", log_list.latin1() );
  }
  QFileInfo fi( log_list );
  statusMsg( "Merging", fi.fileName() );
  qApp->processEvents();

  /* iterate through the list of files, validating each one as we go.
     if a file is valid, bung it in the list, else skip it.
     if we bomb out > 5 times, offer a chance to cancel the operation. */
  QString temp;
  QStringList logFileList;
  bool valid;
  int max_errs = 5;

  QTextStream stream( &logFile );
  while ( !stream.atEnd() ) {
    temp = stream.readLine().simplifyWhiteSpace();
    /* skip empty lines */
    if ( temp.isEmpty() )
      continue;
    /* check re file perms and format */
    temp = validateFile( temp, &valid );
    if ( valid )
      logFileList << temp;
    else {
      max_errs = max_errs - 1;
    }
    if ( max_errs <= 0 ) {
      int val = vkQuery( memcheckView, 2, "Invalid Files",
                "<p>This doesn't look like a valid list of logfiles.</p>"
                "<p>Would you like to cancel the operation?</p>" );
      if ( val == MsgBox::vkYes ) {        /* abort */
        logFile.close();
        statusMsg( "Merge aborted", "(" + fi.fileName()+ ")" );
        fileSaved = true;                  /* nothing to save */
        emitRunning( false );
        return true;
      } else if ( val == MsgBox::vkNo ) {  /* continue */
        max_errs = 500;
      }
    }   
  }
  logFile.close();

  /* check there's a minimum of two files-to-merge */
  if ( logFileList.count() < 2 ) {
    emitRunning( false );
    return vkError( memcheckView, "Merge LogFiles Error", 
           "<p>The minimum number of files required is 2.</p>"
           "<p>The file '%s' contains %d valid xml logfiles.</p>", 
           log_list.latin1(), logFileList.count() );
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
    statusMsg( "Merging",  master_fname + " <- " + slave_fname );

    qApp->processEvents();

    masterLogFile->merge( slaveLogFile );

    /* disconnect the logFile from the parser */
    disconnect( xmlParser,    SIGNAL(loadItem(XmlOutput *)), 
                slaveLogFile, SLOT(loadItem(XmlOutput *)) );

    /* delete the slave and free memory */
    delete slaveLogFile;
    slaveLogFile = 0;
  }

  /* create a unique filename for saving the merged files to */
  QString fname = vk_mkstemp( "merged", vkConfig->logsDir(), ".xml" );
  /* fallback in case mkstemp bombs out */
  if ( fname.isNull() )
    fname = vkConfig->logsDir() + fi.baseName(true) + "-merged.xml";

  fileSaved = masterLogFile->save( fname );
  if ( fileSaved ) vkConfig->wrEntry( fname, "view-log", "valkyrie" );
  else             vkConfig->wrEntry( "", "view-log", "valkyrie" );

  QString msg = "("+fi.fileName()+")";
  if ( !fileSaved ) msg += "<p> unable to save result</p>";
  else              msg += "<p>Output saved to '" +fname+ "'</p>";
  statusMsg( "Merge Complete", msg );

  /* delete the master and free memory, as we are done */
  delete masterLogFile;
  masterLogFile = 0;

  /* load the result, if in gui-mode */
  if ( usingGui && fileSaved ) {
    parseLogFile();
    emit message( fname );
  }

  emitRunning( false );

  /* if we are in non-gui mode, tell valkyrie we are done */
  emit finished();

  return true;
}


/* if --vg-opt=<arg> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the run-button in the gui, 
   then MainWindow::run() calls valkyrie->runTool().  */
bool Memcheck::run( QStringList flags )
{
  if ( usingGui ) {
    /* tell valkyrie what we are doing */
    ;//FIXME: emit setRunMode( Valkyrie::modeParseOutput );
  }

#if 0
  for ( unsigned int i=0; i<flags.count(); i++ )
    printf("flag[%d] --> %s\n", i, flags[i].latin1() );
#endif

  fileSaved = false;
  emitRunning( true );

  /* init the auto-save filename and stream */
  save_fname = vk_mkstemp( "mc_output", vkConfig->logsDir(), ".xml" );
  logFile.setName( save_fname );
  //printf("save_fname = %s\n", save_fname.latin1() );
  //logStream.setEncoding( QTextStream::UnicodeUTF8 );
  //logStream.setDevice( &logFile );
  //if ( !logFile.open( IO_WriteOnly ) ) {
	if ( ! setupFileStream( true ) ) {
    emitRunning( false );
		setupFileStream( false );
    return vkError( memcheckView, "Open File Error", 
                    "<p>Unable to open file '%s' for writing.</p>", 
                    save_fname.latin1() );
  }

  setupParser( true );
  /* fork a new process in non-blocking mode */
  setupProc( true );

  /* stuff the cmd-line flags/non-default opts into the process */
  proc->setArguments( flags );

  if ( ! proc->start() ) {
    setupProc( false );
    setupParser( false );
    setupFileStream( false );
    return vkError( memcheckView, "Error", 
                    "<p>Process failed to start</p>" );
  }

  return true;
}


/* slot, connected to proc's signal readyReadStd***().
   read and process the data, which might be output in chunks.
   output is auto-saved to a logfile in ~/.valkyrie-X.X.X/logs/ */
void Memcheck::parseOutput()
{
  statusMsg( "Memcheck", "Parsing output ... " );

  bool ok;
  QString data;
	int lineNumber = 0;

  if ( log_fd == 1 ) {                    /* stdout */
    while ( proc->canReadLineStdout() ) {
			lineNumber++;
      data = proc->readLineStdout();
      // printf("data: -->%s<--\n", data.latin1() );
      logStream << data << "\n";
      if ( usingGui ) {
        source.setData( data );
        ok = reader.parseContinue();
      }
      if ( !ok ) break;
    }
  } else if ( log_fd == 2 ) {             /* stderr */
    while ( proc->canReadLineStderr() ) {
			lineNumber++;
      data = proc->readLineStderr();
      // printf("data: -->%s<--\n", data.latin1() );
      logStream << data << "\n";
      if ( usingGui ) {
        source.setData( data );
        ok = reader.parseContinue();
      }
      if ( !ok ) break;
    }
  }
#if 1
  if ( !ok ) {
    vkError( memcheckView, "Parse Error", 
             "<p>Parsing failed on line #%d: '%s'</p>", 
						 lineNumber, data.latin1() );
#endif
  }

}


/* slot, connected to proc's signal processExited().
   parsing of valgrind's output is finished, successfully or otherwise.
   if the user passed either 'log-file' or 'log-file-exactly' on the
   cmd-line, save the output immediately to whatever they specified.
   log-file == file.pid || log-file-exactly == file.name */
void Memcheck::saveParsedOutput()
{ 
  statusMsg( "Memcheck", "Parsing complete" );
  /* did valgrind exit normally ? */
  if ( proc->normalExit() )
    ;//VK_DEBUG("normalExit() == true");

  /* grab the process id in case we need it later */
  long currentPid = proc->processIdentifier();

  //logFile.close();
  //logStream.unsetDevice();
	setupFileStream( false );  /* close down auto-save log stuff */
  setupParser( false );      /* disconnect the parser */
  setupProc( false );        /* disconnect and delete the proc */

  /* try for --log-file-exactly first */
  QString fname = vkConfig->rdEntry( "log-file-exactly","valgrind" );
  if ( fname.isEmpty() ) {
    /* try with --log-file */
    fname = vkConfig->rdEntry( "log-file","valgrind" );
    if ( !fname.isEmpty() && currentPid != -1 ) {
      /* tack the pid on the end */
      fname += "." + QString::number( currentPid );
    }
  }

  /* there's nothing else we can do; let the user decide */
  if ( fname.isEmpty() ) {
    fname = save_fname;
  } else {
    QFileInfo fi( fname );
    if ( fi.dirPath() == "." ) {
      /* no filepath given, so save in default dir */
      fname = vkConfig->logsDir() + fname;
    } else {
      /* found a path: make sure it's the absolute version */
      fname = fi.dirPath( true ) + "/" + fi.fileName();
    }
    fi.setFile( fname );
    /* if this filename already exists, check if we should over-write it */
    if ( fi.exists() ) {
      int ok = vkQuery( memcheckView, 2, "Overwrite File",
                        "<p>Over-write existing file '%s' ?</p>", 
                        fname.latin1() );
      if ( ok == MsgBox::vkNo ) {
        return;
      }
    }

    /* save (rename, actually) the auto-named log-file */
    QDir dir( fi.dir() );
    if ( dir.rename( fi.fileName(), fname ) ) {
      ;//logFilename = save_fname;
    } else {
      vkInfo( memcheckView, "Save Failed", 
              "<p>Failed to save file to '%s'",  fname.latin1() );
    }

  }

  statusMsg( "Saved", fname );
  emitRunning( false );

  /* if we are in non-gui mode, tell valkyrie we are done */
  emit finished();
}


/* fork a new process in non-blocking mode; connect up the pipe to
   stdout || stderr || ...  
   TODO: re-implement QProcess so we can connect to fd > 2 */
void Memcheck::setupProc( bool init )
{
  if ( init ) {                   /* starting up */
    vk_assert( proc == 0 );
    proc = new QProcess( this, "mc_proc" );
    connect( proc, SIGNAL( processExited() ),
             this, SLOT( saveParsedOutput() ) );
    log_fd = vkConfig->rdInt( "log-fd", "valgrind" );
    if ( log_fd == 1 ) {           /* stdout */
      connect( proc, SIGNAL( readyReadStdout() ),
               this, SLOT( parseOutput() ) );
    } else if ( log_fd == 2 ) {    /* stderr */
      connect( proc, SIGNAL( readyReadStderr() ),
               this, SLOT( parseOutput() ) );
    } else {
      vk_assert_never_reached();
    }
  } else {                      /* closing down */
    disconnect( proc, SIGNAL( processExited() ),
                this, SLOT( saveParsedOutput() ) );
    if ( log_fd == 1 ) {           /* stdout */
      disconnect( proc, SIGNAL( readyReadStdout() ),
                  this, SLOT( parseOutput() ) );
    } else if ( log_fd == 2 ) {    /* stderr */
      disconnect( proc, SIGNAL( readyReadStderr() ),
                  this, SLOT( parseOutput() ) );
    } else {
      vk_assert_never_reached();
    }
    delete proc;
    proc = 0;
  }
}


/* connect this lot up if we are in gui mode; reset source and parser,
   and tell the reader we are parsing incrementally */
void Memcheck::setupParser( bool init )
{
  if ( !usingGui ) return;

  if ( init ) {                   /* starting up */
    memcheckView->clear();
    xmlParser->reset();
    source.reset();
  
    connect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
             memcheckView, SLOT(loadItem(XmlOutput *)) );
    connect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
             memcheckView, SLOT(updateErrors(ErrCounts*)) );
    connect( xmlParser,    SIGNAL(updateStatus()),
             memcheckView, SLOT(updateStatus()) );

    source.setData( "" );
    reader.parse( &source, true );
  } else {                        /* closing down */
    disconnect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
                memcheckView, SLOT(loadItem(XmlOutput *)) );
    disconnect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
                memcheckView, SLOT(updateErrors(ErrCounts*)) );
    disconnect( xmlParser,    SIGNAL(updateStatus()),
                memcheckView, SLOT(updateStatus()) );
  }

}


bool Memcheck::setupFileStream( bool init )
{
	bool ok = true;

	if ( init ) {
		ok = logFile.open( IO_WriteOnly );
		logStream.setDevice( &logFile );
	} else {
		logFile.close();
		logStream.unsetDevice();
	}

	return ok;
}
