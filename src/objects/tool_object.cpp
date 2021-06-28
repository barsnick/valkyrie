/****************************************************************************
** ToolObject class implementation
**
** Essential functionality is contained within a ToolObject.
** Each ToolObject has an associated ToolView for displaying output.
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
The Valgrind process control & log parsing goes like this:

=== Happy flow ===
start() -> vgproc                               ->(writes)-> XML_LOG
        -> logpoller ->(triggers)-> readVgLog() <-(reads )<- XML_LOG

vgproc      ->(finished/died)-> processDone() ->(if parser done)-> DONE
readVgLog() ->(finished parsing log)          ->(if vgproc done)-> DONE

=== Exceptions ===
processDone() ->(parser alive && vgproc error)-> stopProcess()
readVgLog()   ->(parser error && vgproc alive)-> stopProcess()
User Input    ->(Stop command)-> stop()       -> stopProcess()

stopProcess()
  -> cleanup logpoller
  -> cleanup vgproc
       ->(QProc::terminate)->SIGTERM->          -> processDone() -> DONE
       ->(timeout)-> killProc() ->(QtProc::kill)-> processDone() -> DONE
*/

#include "objects/tool_object.h"
#include "toolview/toolview.h"
#include "utils/vk_config.h"
#include "utils/vk_utils.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QString>
#include <QThread>
#include <QStringList>


#if 1
//#include "config.h"
#include "objects/valkyrie_object.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"      // vk_assert, VK_DEBUG, etc.
#include "utils/vglogreader.h"
#include "options/vk_option.h"   // PERROR* and friends
//#include "vk_file_utils.h"       // FileCopy()

#include <QApplication>
#include <QDir>
#include <QTimer>
#endif
#include <unistd.h>	// for usleep

// Waiting for Vg to start:
#define WAIT_VG_START_MAX   1000 // msecs before giving up
#define WAIT_VG_START_SLEEP 100  // msecs to sleep each loop
#define WAIT_VG_START_LOOPS (WAIT_VG_START_MAX / WAIT_VG_START_SLEEP)

// Waiting for Vg to die:
#define TIMEOUT_KILL_PROC       2000 // msec: 'please stop?' to 'die!'
#define TIMEOUT_WAIT_UNTIL_DONE 5000 // msec: 'half done' to 'advise stop'


//TODO: mock a valgrind process, and setup some unit tests (and a test framework!)
//TODO: have popups called from toolview, not object... maybe.
//TODO: make sure not too many popup vkError()'s for any particular state flow.
//TODO: ditto for statusMsg()'s
//TODO: make sure statusMsg()s always correct with current state.




/*!
  class ToolObject
*/
ToolObject::ToolObject( const QString& toolname, VGTOOL::ToolID id )
   : VkObject( toolname ),
     toolView( 0 ), vgRunSaved( true ), processId( VGTOOL::PROC_NONE ),
     toolId( id ), vgreader( 0 ), vgproc( 0 )
{
   // init logpoller
   logpoller = new VkLogPoller( this );
   connect( logpoller, SIGNAL( logUpdated() ),
            this,        SLOT( readVgLog() ) );
}

ToolObject::~ToolObject()
{
   setProcessId( VGTOOL::PROC_NONE );

   if ( vgproc ) {
      // TODO: does this work?
      vgproc->disconnect(); // so no signal calling processDone()

      if ( vgproc->state() != QProcess::NotRunning ) {
         vgproc->terminate();
         //TODO: wait a short while before deleting?
      }

      delete vgproc;
      vgproc = 0;
   }

   if ( vgreader ) {
      delete vgreader;
      vgreader = 0;
   }

   // logpoller auto deleted by Qt when 'this' dies

   // cleanup temp-log
   if ( QFile::exists( tmplogFname ) ) {
      QFile::remove( tmplogFname );
   }
}


/*!
   Creates this tool's ToolView window,
   and sets up and connections between them
*/
ToolView* ToolObject::createView( QWidget* parent )
{
   toolView = createToolView( parent );

   // signals tool_view --> tool_obj
   connect( toolView, SIGNAL( saveLogFile() ),
            this,       SLOT( fileSaveDialog() ) );

   // signals tool_obj --> tool_view
   connect( this,    SIGNAL( running( bool ) ),
            toolView, SLOT( setState( bool ) ) );

   return toolView;
}


bool ToolObject::isRunning()
{
   return ( processId > VGTOOL::PROC_NONE );
}



/*!
  Set the process id.
  Emits signal for subsequent state: 'running(true|false)' - updates toolview
*/
void ToolObject::setProcessId( int procId )
{
   vk_assert( procId >= VGTOOL::PROC_NONE );
   vk_assert( procId <  VGTOOL::PROC_MAX );
   
   processId = procId;
   emit running( isRunning() );
}

/*!
  get the currently-running process id
*/
int ToolObject::getProcessId()
{
   vk_assert( processId >= VGTOOL::PROC_NONE );
   vk_assert( processId <  VGTOOL::PROC_MAX );
   
   return processId;
}


#if 0
void ToolObject::deleteView()
{
   emit message( "" );  /* clear the status bar */
   vk_assert( m_view != 0 );
   
   m_view->close( true/*alsoDelete*/ );
   m_view = 0;
}
#endif

ToolView* ToolObject::view()
{
   return toolView;
}


/* called from Valkyrie::updateVgFlags() whenever flags have been changed */
QStringList ToolObject::getVgFlags()
{
   QStringList modFlags;
   
   foreach( VkOption * opt, options.getOptionHash() ) {
   
      QString defVal = opt->dfltValue.toString();
      QString cfgVal = vkCfgProj->value( opt->configKey() ).toString();
      
      if ( defVal != cfgVal ) {
         modFlags << "--" + opt->longFlag + "=" + cfgVal;
      }
   }
   return modFlags;
}




/*!
  Start a process
   - Slot, called from the ToolView
*/
bool ToolObject::start( VGTOOL::ToolProcessId procId,
                        QStringList vgflags, QString logfile )
{
   //cerr << "ToolObject::start(): "  << procId << endl;
   vk_assert( procId > VGTOOL::PROC_NONE );
   vk_assert( procId < VGTOOL::PROC_MAX );
   vk_assert( !isRunning() );

   bool ok = false;

   switch ( procId ) {
   case VGTOOL::PROC_VALGRIND:
      tmplogFname = logfile;
      ok = runValgrind( vgflags );
      break;
   case VGTOOL::PROC_PARSE_LOG:
      ok = parseLogFile();
      break;
   default:
      vk_assert_never_reached();
   }

   return ok;
}


/*!
  Parse log file given by [VALKYRIE::VIEW_LOG] entry.
  Called by valkyrie->runTool() if cmdline --view-log=<file> specified.
  ToolView::openLogFile() if gui parse-log selected.
  If 'checked' == true, file perms/format has already been checked
*/
bool ToolObject::parseLogFile()
{
   vk_assert( toolView != 0 );
   // any vg run should have been cleaned up:
   vk_assert( vgRunSaved );
   vk_assert( tmplogFname.isEmpty() );

   setProcessId( VGTOOL::PROC_PARSE_LOG );

   //TODO: pass this via flags from the toolview, or something.
   QString log_file = vkCfgProj->value( "valkyrie/view-log" ).toString();

   statusMsg( "Parsing '" + log_file + "'" );

   // check this is a valid file, and has at least read perms
   int errval = PARSED_OK;
   QString ret_file = fileCheck( &errval, log_file, true );

   if ( errval != PARSED_OK ) {
      vkError( toolView, "File Error", "%s: \n\"%s\"",
               parseErrString( errval ),
               qPrintable( escapeEntities( log_file ) ) );
      setProcessId( VGTOOL::PROC_NONE );
      return false;
   }
   // log file ok
   log_file = ret_file;
   vkCfgProj->setValue( "valkyrie/view-log", log_file );
   
   // Could be a very large file, so at least get ui up-to-date now
   qApp->processEvents( QEventLoop::AllEvents, 1000/*max msecs*/ );

   // Parse the log
   VgLogReader vgLogFileReader( toolView->createVgLogView() );
   bool success = vgLogFileReader.parse( log_file );

   if ( success ) {
      statusMsg( "Loaded Logfile '" + log_file + "'" );
   }
   else {
      statusMsg( "Error Parsing Logfile '" + log_file + "'" );

      VgLogHandler* hnd = vgLogFileReader.handler();
      vkError( toolView, "XML Parse Error",
               "<p>%s</p>", qPrintable( str2html( escapeEntities( hnd->fatalMsg() ) ) ) );
   }

   setProcessId( VGTOOL::PROC_NONE );
   return success;
}


/*!
  Run a VKProcess, as given by 'flags'.
   - Reads ouput from file, loading this to the listview.
*/
bool ToolObject::runValgrind( QStringList flags )
{
   //VK_DEBUG( "Start Vg run" );
   vk_assert( toolView != 0 );

   setProcessId( VGTOOL::PROC_VALGRIND );
   statusMsg( "Starting Valgrind ..." );

   QString     program = flags.at( 0 );
   QStringList args    = flags.mid( 1 );
   vgRunSaved = false; // reset later if start failed

#if 0//def DEBUG_ON

   for ( int i = 0; i < flags.count(); i++ ) {
      vkPrint( "flag[%d] --> %s", i, qPrintable( flags[i] ) );
   }

#endif

   // new vgreader - view may have been recreated, so need up-to-date ptr
   vk_assert( vgreader == 0 );
   vgreader = new VgLogReader( toolView->createVgLogView() );

   // start a new process, listening on exit signal to call processDone().
   //  - once Vg is done, we can read the remainder of the log in one last go.
   vk_assert( vgproc == 0 );
   vgproc = new QProcess( this->toolView );
   connect( vgproc, SIGNAL( finished( int, QProcess::ExitStatus ) ),
            this,     SLOT( processDone( int, QProcess::ExitStatus ) ) );

   // forward vgproc stdout/err to our stdout/err respectively
   vgproc->setProcessChannelMode( QProcess::ForwardedChannels );

   // set working directory
   vgproc->setWorkingDirectory( vkCfgProj->value( "valkyrie/working-dir" ).toString() );

   // start running process
   vgproc->start( program, args );
   //VK_DEBUG( "Started VgProcess" );

   // Make sure Vg started ok before moving further.
   // Don't bother using QProcess::start():
   //  1) Vg may have finished already(!)
   //  2) QXmlSimpleReader won't start on an empty log: seems to need at least "<?x"
   // So just wait for a while until we find the valgrind output log...
   int nLoops=0;
   for (;nLoops < WAIT_VG_START_LOOPS; nLoops++) {
      if ( QFile::exists( tmplogFname ) ) { break; }
      usleep( WAIT_VG_START_SLEEP * 1000 );
   }
   bool vg_ok = (nLoops < WAIT_VG_START_LOOPS);

   if ( vg_ok ) {
      //VK_DEBUG( "Started Valgrind" );
      statusMsg( "Started Valgrind ..." );

      // poll log regularly to trigger parsing of the latest data via readVgLog()
      // doesn't matter if processDone() or readVgLog() gets called first.
      logpoller->start( 250 );  // msec
   }
   else {
      vgRunSaved = true;  // nothing to save

      VK_DEBUG( "Error: Failed Vg startup: '%s'", qPrintable( flags.join( " " ) ) );
      statusMsg( "Error: Failed to start Valgrind" );
      vkError( toolView, "Process Startup Error",
            "<p>Failed to start valgrind properly.<br>"
            "Please verify Valgrind and Binary paths (via Options->Valkyrie).<br>"
            "Try running Valgrind (with _exactly_ the same arguments) via the command-line"
            "<br><br>%s",
            qPrintable( flags.join( "<br>   " ) ) );

      stopProcess();
      vk_assert( this->getProcessId() == VGTOOL::PROC_NONE );
   }

   return vg_ok;
}


/*!
  Stop a process.
  Try to be nice, but if nice don't get the job done, hire a
  one off timer to singleShoot the bugger.

  Note: We may wait around in this function for Vg to die.
    - but we do call qApp->processEvents()
*/
void ToolObject::stopProcess()
{
   // dont try to stop a stopped process.
   if ( !isRunning() ) {
      return;
   }

   VK_DEBUG( "Stopping VgProcess" );
   statusMsg( "Stopping Valgrind process ..." );

   // first things first: stop trying to read from the log.
   if ( logpoller != 0 && logpoller->isActive() ) {
      logpoller->stop();
   }

   if ( vgreader != 0 ) {
      delete vgreader;
      vgreader = 0;
   }

   switch ( getProcessId() ) {
   case VGTOOL::PROC_VALGRIND: {
      // if vgproc is alive, shut it down
      if ( vgproc && ( vgproc->state() != QProcess::NotRunning ) ) {
         VK_DEBUG( "VgProcess starting/running: Terminate." );
         vkPrint( "ToolObject::stopProcess(): process starting/running: terminate." );

         vgproc->terminate();  // if & when succeeds: signal -> processDone()

         // in case doesn't want to stop, start timer to really kill it off.
         QTimer::singleShot( TIMEOUT_KILL_PROC, this, SLOT( killProcess() ) );

         // Now it's a race between killProcess() and processDone()
         // either way, hang around until vgproc is cleaned up.

         VK_DEBUG( "Waiting for VgProcess to die." );
         //TODO: some bad joke re waiting for the inheritance.

         int sleepDuration = 100; // msec
         int nCycles = ( TIMEOUT_KILL_PROC / sleepDuration ) + 1; // +1: ensure Vg gone

         for ( int i = 0; i < nCycles; i++ ) {
            qApp->processEvents( QEventLoop::AllEvents, 10/*max msecs*/ );
            usleep( 1000 * sleepDuration );

            if ( vgproc == 0 ) {
               break;
            }
         }

         // all should now be well.
         vk_assert( vgproc == 0 );
         vk_assert( !this->isRunning() );
      }
      else {
         VK_DEBUG( "VgProcess already stopped (or never started)." );

         if ( vgproc ) {
            delete vgproc;
            vgproc = 0;
         }

         setProcessId( VGTOOL::PROC_NONE );
      }
   }
   break;

   case VGTOOL::PROC_PARSE_LOG: {   // parse log
      // TODO
      VK_DEBUG( "TODO: ToolObject::stop(parse log)" );
      setProcessId( VGTOOL::PROC_NONE );
   }
   break;

   default:
      vk_assert_never_reached();
   }
}


/* are we done and dusted?
   anything we need to check/do before being deleted/closed?
   return true -> all done.
   return false -> not quite yet.
*/
bool ToolObject::queryDone()
{
   vk_assert( toolView != 0 );

   // if current process busy, ask user if they really want to close
   if ( isRunning() ) {
      int ok = vkQuery( toolView, "Process Running", "&Abort;&Cancel",
                        "<p>The current process is not yet finished.</p>"
                        "<p>Do you want to abort it ?</p>" );

      // Note: process may have finished while waiting for user
      if ( ok == MsgBox::vkYes ) {
         stopProcess();                       // abort
         vk_assert( !isRunning() );
      }
      else if ( ok == MsgBox::vkNo ) {
         return false;                        // continue
      }
   }

   // if current output not saved, ask user if want to save
   bool discardLog = true;
   if ( !vgRunSaved ) {
      int ok = vkQuery( toolView, "Unsaved Run Log",
                        "&Save;&Discard;&Cancel",
                        "<p>The current output is not saved, "
                        " and will be deleted.<br/>"
                        "Do you want to save it ?</p>" );

      if ( ok == MsgBox::vkYes ) {            // Save log
         discardLog = fileSaveDialog();       // not saved -> procrastinate
      }
      else if ( ok == MsgBox::vkCancel ) {    // Cancelled: procrastinate
         discardLog = false;
      }
      // else discard log
   }

   if ( discardLog ) {
      if ( QFile::exists( tmplogFname ) ) {
         QFile::remove( tmplogFname );
      }
      tmplogFname = QString();
      vgRunSaved = true; // nothing more to save

      // TODO: clear View
   }

   vk_assert( vgproc == 0 );
   vk_assert( !this->isRunning() );
   return discardLog;
}





/*!
  Stop a process
   - Slot, called from the ToolView

   Note: could take a while (TIMEOUT_KILL_PROC) before we return
    - but we do call qApp->processEvents()
*/
void ToolObject::stop()
{
   //cerr << "ToolObject::stop() " << endl;

   // could take a while (TIMEOUT_KILL_PROC) before really done,
   stopProcess();
   vk_assert( vgproc == 0 );
   vk_assert( !this->isRunning() );
}






/*!
  Kills a process immediately
   - only called as a result of kill timer timeout from stopProcess()

  If process already stopped, cleans it up. Else kills it, which
  signals processDone(), which cleans it up.
*/
void ToolObject::killProcess()
{
   if ( vgproc == 0 ) {
      // vgproc died and called processDone() already
      VK_DEBUG( "VgProcess already cleaned up." );
      return;
   }

   if ( vgproc->state() == QProcess::NotRunning ) {
      VK_DEBUG( "VgProcess already stopped." );

      // cleanup already.
      delete vgproc;
      vgproc = 0;
      setProcessId( VGTOOL::PROC_NONE );

   }
   else {
      VK_DEBUG( "VgProcess still running: kill it!" );
      vgproc->kill();
      // process will die, signalling processDone(),
      // which will cleanup vgproc.
   }
}


/*!
  Process exited, from one of:
   - process finished/died/killed external to valkyrie
   - startup failed, and called stopProcess()
   - user got bored and called stopProcess()

   - stopProcess() called terminate(), which called this directly
   - stopProcess() started a timer to call killProcess(), to call kill(),
     which called this directly.

  Since both processDone() and killProcess() must clean up vgproc,
  be careful to check who's cleaned up already.

  Don't worry about the parser state: just deal with the Vg process.
   - unless Vg quit with an error & parser is still runnning,
     in which case, stop the parser too, via stopProcess()
*/
void ToolObject::processDone( int exitCode, QProcess::ExitStatus exitStatus )
{
   //vkDebug( "ToolObject::processDone( %d, %d )", exitCode, exitStatus );
   vk_assert( toolView != 0 );

   if ( vgproc == 0 ) {
      // Tiny chance that the timeout called killProcess(), cleaning up vgproc,
      // but vgproc already sent an (buffered) call to processDone() directly?
      // Don't know if that's even possible with Qt signal/slots, but just in case.
      VK_DEBUG( "VgProcess already cleaned up." );
      return;
   }

   // cleanup first -------------------------------------------------
   delete vgproc;
   vgproc = 0;

   // ---------------------------------------------------------------
   // check process exit status - valgrind might have bombed
   bool ok = true;

   if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
      //VK_DEBUG( "Clean VgProcess stop: process exited ok" );
      statusMsg( "Finished Valgrind process!" );

   }
   else {
      ok = false;
      VK_DEBUG( "Error running VgProcess: process failed (%d)", exitStatus );
      statusMsg( "Error running Valgrind process" );

      // Note: when calling a QDialog, qApp still processes events,
      // so killProcess() may get called here.
      vkError( toolView, "Run Error",
               "<p>Valgrind process %s, giving return value %d.<br><br>"
               "Most likely, you either pressed 'Stop', or the (client)<br>"
               "process running under Valgrind died.<br><br>"
               "If, however, you suspect Valgrind itself may have crashed,<br>"
               "please click 'OK', 'Save Log' and examine for details.</p>",
               ( exitStatus == QProcess::NormalExit ) ? "exited normally" :
               "crashed or was killed",
               exitCode );
   }

   // if log reader not active anymore, we're done
   if ( vgreader == 0 ) {
      //VK_DEBUG( "All done." );
      statusMsg( "Finished running Valgrind successfully!" );
      setProcessId( VGTOOL::PROC_NONE );
   }
   else {
      // For a number of reasons, vgreader may continue on a while after
      // vgproc has gone (e.g. Vg dies, leaving incomplete xml)
      if ( !ok ) {
         // process error: stop reader now.
         VK_DEBUG( "VgProcess finished with error: stop VgReader" );
         stopProcess();
      }
      else {
         // Vg finished happily.
         // In principle, want pargser also to stop when it's happy,
         // but "just in case" Vg doesn't output complete XML (surely not!)
         // or the parser is taking too long with too much data,
         // wait a while, and, if same state, inform the user what's going on.

         //VK_DEBUG( "VgProcess finished happy: take another look at "
         //          "VgReader after timeout (%d)", TIMEOUT_WAIT_UNTIL_DONE );
         QTimer::singleShot( TIMEOUT_WAIT_UNTIL_DONE,
                             this, SLOT( checkParserFinished() ) );
      }
   }
}



/*!
  Read Valgrind XML
   - Called by logpoller signals only.

  Don't worry about Valgrind process state: just read the log.
   - unless we have a parser error & valgrind is still runnning,
     in which case, stop the process too, via stopProcess().
*/
void ToolObject::readVgLog()
{
   vk_assert( toolView != 0 );
   vk_assert( vgreader != 0 );
   vk_assert( logpoller != 0 );
   vk_assert( !tmplogFname.isEmpty() );

   // Note: not calling qApp->processEvents(), since parser only
   // reads in a limited amount in one go anyway.

   // try parsing vg xml log ----------------------------------------
   bool ok = true;
   QString errHeader;
   statusMsg( "Parsing Valgrind XML log..." );

   if ( !vgreader->handler()->started() ) {
      // first time around...
      //VK_DEBUG( "Start parsing Valgrind XML log" );

      ok = vgreader->parse( tmplogFname, true/*incremental*/ );

      if ( !ok ) {
         VK_DEBUG( "Error: parse() failed" );
         errHeader = "XML Parse-Startup Error";
      }
   }
   else {
      // we've started, so we'll continue...
      //VK_DEBUG( "Continue parsing Valgrind XML log" );

      ok = vgreader->parseContinue();

      if ( !ok ) {
         VK_DEBUG( "Error: parseContinue() failed" );
         errHeader = "XML Parse-Continue Error";
      }
   }

   if ( !vgreader->handler()->fatalMsg().isEmpty() ) {
      ok = false;
   }

   if ( vgreader->handler()->finished() ) {
      //VK_DEBUG( "Reached end of XML log" );
   }


   // deal with failures --------------------------------------------
   if ( !ok ) {
      // parsing failed :-(
      VK_DEBUG( "Error parsing Valgrind log" );
      statusMsg( "Error parsing Valgrind log" );

      // Failed: print error & stop everything.
      QString errMsg = vgreader->handler()->fatalMsg();
      vkError( toolView, errHeader,
               "<p>Failed to parse Valgrind XML output:<br>%s</p>",
               qPrintable( str2html( errMsg ) ) );
   }

   // cleanup -------------------------------------------------------
   // if parsing failed, or this was a last call, then cleanup
   if ( !ok || vgreader->handler()->finished() ) {
      //VK_DEBUG( "Cleaning up logpoller & reader" );
      vk_assert( vgreader != 0 );
      vk_assert( logpoller != 0 );
      vk_assert( logpoller->isActive() );

      // cleanup.
      logpoller->stop();
      delete vgreader;
      vgreader = 0;

      // if vgproc not active anymore, we're done!
      if ( vgproc == 0 ) {
         //VK_DEBUG( "All done." );
         statusMsg( "Finished running Valgrind successfully!" );
         setProcessId( VGTOOL::PROC_NONE );
      }
      else {
         // vgproc is still alive...
         if ( !ok ) {
            // parse error: stop vgproc now
            VK_DEBUG( "VgReader finished with error: stop VgProcess" );
            stopProcess();
         }

         // else: parser finished happily. Allow Vg to stop when it's also happy.
         // TODO: Any reason why Vg might need stopping from this state?
         //  - if any good reason, then dup checkParserFinished() functionality.
      }
   }
}


/*!
  If for some reason Valgrind has finished and the parser still hasn't,
  inform the user and remind of option to stopping by hand.

  Notes:
  * vgreader->parse() and parseContinue() call QXmlInputSource::fetchData(),
    which reads in only a limited amount (512B for Qt3.3.6) from the logfile.
    Valgrind, after finishing up, can write a whole bunch of data in one go
    to the logfile, which takes some iterations of parserContinue() to read in.
  * If Valgrind doesn't write a complete XMLfile (!), this would leave the
    parser with incomplete XML, trying to parse it indefinitely.
*/
void ToolObject::checkParserFinished()
{
   if ( vgproc == 0 && vgreader != 0 ) {
      VK_DEBUG( "Timeout waiting for parser to finish: Parser _still_ alive." );
      vkInfo( toolView, "Valgrind finished, but log-reader alive",
              "<p>The Valgrind process finished some time ago,<br>"
              "but the log reader is still active.<br><br>"
              "This could indicate that Valgrind didn't finish<br>"
              "writing the XML log properly, or just that<br>"
              "there is a huge amount of XML to read.<br><br>"
              "Press Stop if you want to force the process to stop.</p>" );
   }
   else {
      VK_DEBUG( "Timeout waiting for parser to finish: Parser finished." );
   }
}


/*!
  1. Brings up a Save File Dialog to choose a filename to save to
  2. Gets log to read from
  3. Copies log to given filename
  returns false on: user pressing Cancel, src==dst, copy fail
  else true.
*/
bool ToolObject::fileSaveDialog()
{
   cerr << "ToolObject::fileSaveDialog()" << endl;
   vk_assert( toolView != 0 );

   QString fname = vkDlgCfgGetFile( toolView, "valkyrie/view-log",
                                    QFileDialog::AcceptSave );

   if ( fname.isEmpty() ) { // Cancelled
      return false;
   }

   // --- Get appropriate source log
   // TODO: this is horrible, but good enough for now.
   //  - relies on empty tmplogFname to indicate not a vg run but a loaded log
   QString srcFname = tmplogFname;
   if ( tmplogFname.isEmpty() ) {
      srcFname = vkCfgProj->value( "valkyrie/view-log" ).toString();
   }

   // trying to copy src to src?
   if ( QFileInfo( srcFname ) == QFileInfo( fname ) ) {
      return false;
   }

   // --- Copy src log to given filename ---
   // first delete if already exists
   if ( QFile::exists( fname ) ) {
      QFile::remove( fname );
   }
   bool ok = QFile::copy( srcFname, fname );

   if ( ok ) {
      vgRunSaved = true;
      statusMsg( "Saved: " + srcFname );
   }
   else {
      // nogo: return and try again
      vkInfo( toolView, "Save Failed",
              "<p>Failed to save file to '%s'", qPrintable( fname ) );
      statusMsg( "Failed Save: " + srcFname );
   }

   return ok;
}
