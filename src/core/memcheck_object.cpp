/* --------------------------------------------------------------------- 
 * Implementation of class Memcheck                  memcheck_object.cpp
 * Memcheck-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "memcheck_object.h"
#include "valkyrie_object.h"
#include "vk_config.h"
#include "html_urls.h"
#include "vk_messages.h"

#include <qapplication.h>
#include <qtimer.h>


/* class Memcheck ------------------------------------------------------ */
Memcheck::~Memcheck()
{
  if ( xmlParser != 0 ) {
    delete xmlParser;
    xmlParser = 0;
  }
}


Memcheck::Memcheck() 
  : ToolObject( "Memcheck", "&Memcheck", Qt::SHIFT+Qt::Key_M ) 
{
  /* init vars */
  fileSaved = true;
  logStream.setEncoding( QTextStream::UnicodeUTF8 );
  xmlParser = new XMLParser( this, true );
  reader.setContentHandler( xmlParser );
  reader.setErrorHandler( xmlParser );
  
	/* these opts should be kept in exactly the same order as valgrind
		 outputs them, as it makes keeping up-to-date a lot easier. */
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
ToolView* Memcheck::createView( QWidget* parent )
{
  m_view = new MemcheckView( parent, this );
  view()->setState( is_Running );
  return m_view;
}


void Memcheck::emitRunning( bool run )
{
  is_Running = run;
  emit running( is_Running );
  view()->setState( is_Running );
}


/* outputs a message to the status bar. */
void Memcheck::statusMsg( QString hdr, QString msg ) 
{ 
  emit message( hdr + ": " + msg );
}


/* called by MainWin::closeToolView() */
bool Memcheck::isDone( Valkyrie::RunMode rmode )
{
  vk_assert( view() != 0 );

  /* if current process is not yet finished, ask user if they really
     want to close */
  if ( is_Running ) {
    int ok = vkQuery( this->view(), "Process Running", "&Abort;&Cancel",
                      "<p>The current process is not yet finished.</p>"
                      "<p>Do you want to abort it ?</p>" );
    if ( ok == MsgBox::vkYes ) {
      bool stopped = stop( rmode );         /* abort */
      vk_assert( stopped );
      // TODO: what todo if can't /  won't stop?
    } else if ( ok == MsgBox::vkNo ) {
      return false;                         /* continue */
    }
  }

  /* currently loaded / parsed stuff is saved to tmp file - ask user
     if they want to save it to a 'real' file */
  if ( !fileSaved ) {
    int ok = vkQuery( this->view(), "Unsaved File", 
                      "&Save;&Discard;&Cancel",
                      "<p>The current output is not saved, "
                      " and will be deleted.<br/>"
                      "Do you want to save it ?</p>" );
    if ( ok == MsgBox::vkYes ) {            /* save */
      VK_DEBUG("TODO: save();\n");
    } else if ( ok == MsgBox::vkCancel ) {  /* procrastinate */
      return false;
    } else {                                /* discard */
      //printf("removing tmp file '%s'\n", saveFname.latin1() );
      QFile file( saveFname );
      file.remove();
      fileSaved = true;
    }
  }

  return true;
}

bool Memcheck::start( Valkyrie::RunMode rm )
{
  vk_assert( !is_Running );
  bool ok = false;
  
  switch ( rm ) {
    case Valkyrie::modeParseLog:
      ok = this->parseLogFile();
      break;

    case Valkyrie::modeMergeLogs:
      ok = this->mergeLogFiles();
      break;

  default:
    vk_assert_never_reached();
  }
  return ok;
}


bool Memcheck::stop( Valkyrie::RunMode rm )
{
  if ( !is_Running ) return true;

  switch ( rm ) {
  case Valkyrie::modeParseLog:
    VK_DEBUG("TODO: %s::stop(parse log)", name().latin1() );
    break;

  case Valkyrie::modeMergeLogs:
    VK_DEBUG("TODO: %s::stop(merge logs)", name().latin1() );
    break;

  case Valkyrie::modeParseOutput:
    proc->tryTerminate();   /* first ask nicely. */
    /* if proc still running after msec_timeout, terminate with prejudice */
    QTimer::singleShot( 2000, proc, SLOT( kill() ) );   // TODO: move N to config
    break;

  default:
    vk_assert_never_reached();
  }

  // TODO: statusMsg() ?

  return true;
}


/* when --view-log=<file> is set on the cmd-line, valkyrie checks the
   file's perms + format, and writes the value to [valkyrie:view-log].
   However, when a file is set via the open-file-dialog in the gui, or
   is contained with in a list of logfiles-to-merge, then perms +
   format checks must be made
   Returns absolute path of log_file, or QString::null on error */
QString Memcheck::validateFile( QString log_file ) 
{
  int errval = PARSED_OK;

  /* check this is a valid file, and has the right perms */
  QString ret_file = fileCheck( &errval, log_file.latin1(), true, false );
  if ( errval != PARSED_OK ) {
    vkError( view(), "File Error", "%s: \n\"%s\"", 
             parseErrString(errval), escapeEntities(log_file).latin1() );
    return QString::null;
  }

  /* check the file is readable, and the format is xml */
  bool is_xml = XMLParser::xmlFormatCheck( &errval, log_file );
  if ( errval != PARSED_OK ) {
    vkError( view(), "File Error", "%s: \n\"%s\"", 
             parseErrString(errval), log_file.latin1() );
    return QString::null;
  }

  if ( !is_xml ) {
    vkError( view(), "File Format Error", 
             "<p>The file '%s' is not in xml format.</p>",
             log_file.latin1() );
    return QString::null;
  }

  return ret_file;
}


/* called by parseLogFile().
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

  while ( ok && !stream.atEnd() ) {
    lineNumber++;
    inputData = stream.readLine();
    source.setData( inputData );
    ok = reader.parseContinue();
  }

  logFile.close();

  if ( !ok ) {
    vkError( view(), "Parse Error",
             "<p>Parsing failed on line no %d: '%s'</p>", 
             lineNumber, inputData.latin1() );
  }
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
  /* tell valkyrie what we are doing */
  emit setRunMode( Valkyrie::modeParseLog );

  QString log_file = vkConfig->rdEntry( "view-log", "valkyrie" );
  if ( !checked ) {
    log_file = validateFile( log_file );
    if ( !log_file.isNull() ) {
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
           this->view(), SLOT(loadItem(XmlOutput *)) );
  connect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
           this->view(), SLOT(updateErrors(ErrCounts*)) );
  connect( xmlParser,    SIGNAL(updateStatus()),
           this->view(), SLOT(updateStatus()) );

  xmlParser->reset();
  source.reset();
  view()->clear();

  bool success = parseLog( log_file);

  disconnect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
              this->view(), SLOT(loadItem(XmlOutput *)) );
  disconnect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
              this->view(), SLOT(updateErrors(ErrCounts*)) );
  disconnect( xmlParser,    SIGNAL(updateStatus()),
              this->view(), SLOT(updateStatus()) );

  emitRunning( false );

  QString hdr = ( success ) ? "Loaded" : "Parse failed";
  statusMsg( hdr, log_file );

  return success;
}


/* if --merge=<file_list> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the open-file-dialog in the gui,
   called by MemcheckView::openMergeFile().  either way, the value in
   [valkyrie:merge] is what we need to know */
bool Memcheck::mergeLogFiles()
{
  /* tell valkyrie what we are doing */
  emit setRunMode( Valkyrie::modeMergeLogs );
 
  QString fname_logList = vkConfig->rdEntry( "merge", "valkyrie" );
  statusMsg( "Merging", fname_logList );
 
  QStringList flags;
  flags << vkConfig->rdEntry( "merge-exec","valkyrie");
  flags << "-f";
  flags << fname_logList;
 
  /* read from stdout */
  int log_fd =  1;
 
  return runProcess( flags, log_fd, "mc_merged" );
}


/* if --vg-opt=<arg> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the run-button in the gui, 
   then MainWindow::run() calls valkyrie->runTool().  */
bool Memcheck::run( QStringList flags )
{
  /* tell valkyrie what we are doing */
  emit setRunMode( Valkyrie::modeParseOutput );

  /* Read from log_fd */
  int log_fd =  vkConfig->rdInt( "log-fd", "valgrind" );

  return runProcess( flags, log_fd, "mc_output" );
}


/* Run a VKProcess, as given by 'flags'.
   Listens to output on 'log_fd', loading this output to the toolView
   via xmlParser::loadItem(XmlOutput*).
   Output to stdout, stderr is also gathered and sent to toolView via
   loadClientOutput().
   Note: 'log_fd' takes precedence over stdout/stderr, so if the fd's
   overlap, output will be taken from 'log_fd', not stdout/stderr.
   Auto-saves log_fd output to a unique fname starting with 'fbasename'.
*/
bool Memcheck::runProcess( QStringList flags, int log_fd,
                           QString fbasename )
{
#if 0
  for ( unsigned int i=0; i<flags.count(); i++ )
    printf("flag[%d] --> %s\n", i, flags[i].latin1() );
#endif

  fileSaved = false;
  emitRunning( true );

  /* init the auto-save filename and stream */
  saveFname = vk_mkstemp( fbasename, vkConfig->logsDir(), ".xml" );
  //printf("saveFname = %s\n", saveFname.latin1() );
  if ( ! setupFileStream( true ) ) {
    emitRunning( false );
    setupFileStream( false );
    vkError( this->view(), "Open File Error", 
             "<p>Unable to open file '%s' for writing.</p>", 
             saveFname.latin1() );
    return false;
  }

  setupParser( true );
  /* fork a new process in non-blocking mode */
  setupProc( true, log_fd );

  /* stuff the cmd-line flags/non-default opts into the process */
  proc->setArguments( flags );

  if ( ! proc->start() ) {
    setupProc( false );
    setupParser( false );
    setupFileStream( false );
    vkError( this->view(), "Error", "<p>Process failed to start</p>" );
    return false;
  }

  return true;
}


/* slot, connected to proc's signal readyReadStd***().
   read and process the data, which might be output in chunks.
   output is auto-saved to a logfile in ~/.valkyrie-X.X.X/logs/
   Note: This function must not block, else vkprocess can finish
   and be deleted before this unblocks and returns to vkprocess,
   leading to a segfault.
*/
void Memcheck::parseOutput()
{
  statusMsg( "Memcheck", "Parsing output ... " );

  bool ok = true;
  QString data;
  int lineNumber = 0;

  //  int log_fd = proc->getFDout();

  // TODO: proc->readLineXXX() doesn't respect client output formatting

  /* FDout: takes precedence over stdout/stderr,
     so even if FDout == 1|2, we'll still get the data here. */
  while ( proc->canReadLineFDout() ) {
    lineNumber++;
    data = proc->readLineFDout();
    //printf("MC::parseOutput(): FDout(%d): '%s'\n", log_fd, data.latin1() );
    logStream << data << "\n";
    source.setData( data );
    bool ok = reader.parseContinue();
    if ( !ok ) {
      /* if we get here, it means either:
         a) Output from valgrind run is bad
         b) Output from vk_logmerge run is bad
          - neither should happen, so die.
         Not very nice, but output-to-date is saved in the auto-log file.
         TODO: do sthng nicer here - but rem not to block this function!
       */
      vk_assert_never_reached();
    }
  }

  /* Stdout: anything read here will be from client prog */
  while ( proc->canReadLineStdout() ) {
    data = proc->readLineStdout();
    //printf("\nMC::parseOutput(): Stdout: '%s'\n", data.latin1() );
    loadClientOutput( data, 1 );
  }

  /* Stderr: if valgrind failed to start, may output to stderr;
     else will be client prog output. */
  while ( proc->canReadLineStderr() ) {
    data = proc->readLineStderr();
    //printf("\nMC::parseOutput(): Stderr: '%s'\n", data.latin1() );
    loadClientOutput( data, 2 );
  }
}


/* slot, connected to proc's signal processExited().
   parsing of valgrind's output is finished, successfully or otherwise.
   if the user passed either 'log-file' or 'log-file-exactly' on the
   cmd-line, save the output immediately to whatever they specified.
   log-file == <file>.pid<pid> || log-file-exactly == <file> */
void Memcheck::processDone()
{
  statusMsg( "Memcheck", "Parsing complete" );
  /* did valgrind exit normally ? */
  if ( proc->normalExit() )
    ;//VK_DEBUG("normalExit() == true");

  /* grab the process id in case we need it later */
  long currentPid = proc->processIdentifier();

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
      fname += ".pid" + QString::number( currentPid );
    }
  }

  /* Save output to logfile <fname> */
  saveParsedOutput( fname );

  emitRunning( false );
}


void Memcheck::saveParsedOutput( QString& fname )
{ 
  if ( fname.isEmpty() || fname == saveFname ) {
    /* nothing to do: log already saved to saveFname */
    statusMsg( "Saved to default", saveFname );
  } else {
    QFileInfo fi( fname );
    if ( fname[0] != '/' && fname[0] != '.' ) {
      /* CAB: TODO: Not sure about this...
          - normally, even if no './' is given, it is still implied... */
      /* no abs or rel path given, so save in default dir */
      fname = vkConfig->logsDir() + fname;
    } else {
      /* found a path: make sure it's the absolute version */
      fname = fi.dirPath( true ) + "/" + fi.fileName();
    }
    fi.setFile( fname );
    /* if this filename already exists, check if we should over-write it */
    if ( fi.exists() ) {
      int ok = vkQuery( this->view(), 2, "Overwrite File",
                        "<p>Over-write existing file '%s' ?</p>", 
                        fname.latin1() );
      if ( ok == MsgBox::vkNo ) {
        // TODO: Allow user to enter new logfile
        statusMsg( "Saved to default", saveFname );
        return;
      }
    }

    /* save (rename, actually) the auto-named log-file */
    QDir dir( fi.dir() );
    if ( dir.rename( saveFname, fname ) ) {
      statusMsg( "Saved", fname );
    } else {
      vkInfo( this->view(), "Save Failed", 
              "<p>Failed to save file to '%s'",  fname.latin1() );
      statusMsg( "Saved to default", saveFname );
    }
  }
}


/* fork a new process in non-blocking mode; connect the pipes:
   log_fd || stdout || stderr
   on init, sets proc to use file descriptor log_fd
*/
void Memcheck::setupProc( bool init, int log_fd/*=-1*/ )
{
  if ( init ) {                   /* starting up */
    vk_assert( proc == 0 );
    vk_assert( log_fd >= 1 );
    proc = new VKProcess( this, "mc_proc" );
    proc->setCommunication( VKProcess::Stdin  | VKProcess::Stdout | 
                            VKProcess::Stderr | VKProcess::FDin   |
                            VKProcess::FDout );
    proc->setFDout( log_fd );
    connect( proc, SIGNAL( processExited() ),
             this, SLOT( processDone() ) );
    connect( proc, SIGNAL( readyReadFDout() ),
             this, SLOT( parseOutput() ) );
    connect( proc, SIGNAL( readyReadStdout() ),
             this, SLOT( parseOutput() ) );
    connect( proc, SIGNAL( readyReadStderr() ),
             this, SLOT( parseOutput() ) );
  } else {                        /* closing down */
    vk_assert( log_fd == -1 );    /* just ensure proper use */
    disconnect( proc, SIGNAL( processExited() ),
                this, SLOT( processDone() ) );
    disconnect( proc, SIGNAL( readyReadFDout() ),
                this, SLOT( parseOutput() ) );
    disconnect( proc, SIGNAL( readyReadStdout() ),
                this, SLOT( parseOutput() ) );
    disconnect( proc, SIGNAL( readyReadStderr() ),
                this, SLOT( parseOutput() ) );
    delete proc;
    proc = 0;
  }
}


/* connect this lot up, reset source and parser,
   and tell the reader we are parsing incrementally */
void Memcheck::setupParser( bool init )
{
  if ( init ) {                   /* starting up */
    view()->clear();
    xmlParser->reset();
    source.reset();
  
    connect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
             this->view(), SLOT(loadItem(XmlOutput *)) );
    connect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
             this->view(), SLOT(updateErrors(ErrCounts*)) );
    connect( xmlParser,    SIGNAL(updateStatus()),
             this->view(), SLOT(updateStatus()) );
    connect( xmlParser,    SIGNAL(loadClientOutput(const QString&)),
       this,         SLOT(loadClientOutput(const QString&)) );

    source.setData( "" );
    reader.parse( &source, true );
  } else {                        /* closing down */
    disconnect( xmlParser,    SIGNAL(loadItem(XmlOutput *)),
                this->view(), SLOT(loadItem(XmlOutput *)) );
    disconnect( xmlParser,    SIGNAL(updateErrors(ErrCounts*)),
                this->view(), SLOT(updateErrors(ErrCounts*)) );
    disconnect( xmlParser,    SIGNAL(updateStatus()),
                this->view(), SLOT(updateStatus()) );
    disconnect( xmlParser,    SIGNAL(loadClientOutput(const QString&)),
    this,         SLOT(loadClientOutput(const QString&)) );
  }

}


bool Memcheck::setupFileStream( bool init )
{
  bool ok = true;

  if ( init ) {
    QFile* logFile = new QFile( saveFname );
    ok = logFile->open( IO_WriteOnly );
    logStream.setDevice( logFile );
  } else {
    QFile* logFile = (QFile*)logStream.device();
    logStream.unsetDevice();
    logFile->close();
    delete logFile;
  }
  
  return ok;
}



void Memcheck::loadClientOutput( const QString& client_output, int log_fd/*=-1*/ )
{
  if (log_fd == -1)
    log_fd = vkConfig->rdInt( "log-fd", "valgrind" );

  this->view()->loadClientOutput(client_output, log_fd);
}

