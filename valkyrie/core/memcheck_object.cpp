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
#include "vk_option.h"         // PERROR* and friends 
#include "vk_utils.h"          // vk_assert, VK_DEBUG, etc.

#include <qapplication.h>
#include <qtimer.h>
#include <qurloperator.h>


/* class Memcheck ------------------------------------------------------ */
Memcheck::~Memcheck()
{
   if (m_vgproc) {
      if (m_vgproc->isRunning()) {
         m_vgproc->tryTerminate();
         m_vgproc->kill();
      }
      delete m_vgproc;
      m_vgproc = 0;
   }
   if (m_vgreader) {
      delete m_vgreader;
      m_vgreader = 0;
   }

   /* m_logpoller deleted by it's parent: 'this' */

   /* unsaved log... delete our temp file */
   if (!m_fileSaved && !m_saveFname.isEmpty())
      QDir().remove( m_saveFname );
}


Memcheck::Memcheck( int objId ) 
   : ToolObject( "Memcheck", "&Memcheck", Qt::SHIFT+Qt::Key_M, objId ) 
{
   /* init vars */
   m_fileSaved = true;
   m_vgproc    = 0;
   m_vgreader  = 0;
   m_logpoller = 0;

	/* these opts should be kept in exactly the same order as valgrind
      outputs them, as it makes keeping up-to-date a lot easier. */
   addOpt( LEAK_CHECK,  VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO, 
           "memcheck",  '\0',                 "leak-check",
           "<no|summary|full>",  "no|summary|full",  "full",
           "Search for memory leaks at exit:",
           "search for memory leaks at exit?",
           urlMemcheck::Leakcheck );
   addOpt( LEAK_RES,    VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO, 
           "memcheck",  '\0',                 "leak-resolution",
           "<low|med|high>", "low|med|high", "low",
           "Degree of backtrace merging:",
           "how much backtrace merging in leak check", 
           urlMemcheck::Leakres );
   addOpt( SHOW_REACH,  VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "memcheck",  '\0',                 "show-reachable",
           "<yes|no>",  "yes|no",             "no",
           "Show reachable blocks in leak check",
           "show reachable blocks in leak check?",  
           urlMemcheck::Showreach );
   addOpt( PARTIAL,     VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "memcheck",  '\0',                 "partial-loads-ok",
           "<yes|no>",  "yes|no",             "no",
           "Ignore errors on partially invalid addresses",
           "too hard to explain here; see manual",
           urlMemcheck::Partial );
   addOpt( FREELIST,    VkOPTION::ARG_UINT,   VkOPTION::WDG_LEDIT, 
           "memcheck",  '\0',                 "freelist-vol",
           "<number>",  "0|10000000",         "5000000",
           "Volume of freed blocks queue:",
           "volume of freed blocks queue",
           urlMemcheck::Freelist );
   addOpt( GCC_296,     VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "memcheck",  '\0',                 "workaround-gcc296-bugs",
           "<yes|no>",  "yes|no",             "no",
           "Work around gcc-296 bugs",
           "self explanatory",  
           urlMemcheck::gcc296 );
   addOpt( ALIGNMENT,  VkOPTION::ARG_PWR2,   VkOPTION::WDG_SPINBOX, 
           "memcheck",  '\0',                "alignment", 
           "<number>", "8|1048576",          "8",
           "Minimum alignment of allocations",
           "set minimum alignment of allocations", 
           urlVgCore::Alignment );
}


/* check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages -------------------- */
int Memcheck::checkOptArg( int optid, QString& argval )
{
   int errval = PARSED_OK;
   Option* opt = findOption( optid );

   switch ( (Memcheck::mcOpts)optid ) {
   case PARTIAL:
   case FREELIST:
   case LEAK_RES:
   case SHOW_REACH:
   case GCC_296:
   case ALIGNMENT:
      opt->isValidArg( &errval, argval );
      break;

      /* when using xml output from valgrind, this option is preset to
         'full' by valgrind, so this option should not be used. */
   case LEAK_CHECK:
      /* Note: gui options disabled, so only reaches here from cmdline */
      errval = PERROR_BADOPT;
      fprintf(stderr,
              "\nOption disabled '--%s': Memcheck presets this option to 'full' when generating the required xml output.\nSee valgrind/docs/internals/xml_output.txt.\n",
              opt->m_longFlag.latin1());
      break;


   default:
      vk_assert_never_reached();
   }

   return errval;
}


/* called from Valkyrie::updateVgFlags() whenever flags have been changed */
QStringList Memcheck::modifiedVgFlags()
{
   QStringList modFlags;
   QString defVal, cfgVal, flag;

   Option* opt;
   for ( opt = m_optList.first(); opt; opt = m_optList.next() ) {
      flag   = opt->m_longFlag.isEmpty() ? opt->m_shortFlag
                                         : opt->m_longFlag;
      defVal = opt->m_defaultValue;     /* opt holds the default */
      cfgVal = vkConfig->rdEntry( opt->m_longFlag, name() );

      switch ( (Memcheck::mcOpts)opt->m_key ) {

      /* when using xml output from valgrind, this option is preset to
         'full' by valgrind, so this option should not be used. */
      case LEAK_CHECK:
         /* ignore this opt */
         break;

      default:
         if ( defVal != cfgVal )
            modFlags << "--" + opt->m_longFlag + "=" + cfgVal;
      }
   }
   return modFlags;
}


/* Creates this tool's ToolView window,
   and sets up and connections between them */
ToolView* Memcheck::createView( QWidget* parent )
{
   m_view = new MemcheckView( parent, this->name() );

   /* signals view --> tool */
   connect( m_view, SIGNAL(saveLogFile()),
            this, SLOT(fileSaveDialog()) );

   /* signals tool --> view */
   connect( this, SIGNAL(running(bool)),
            m_view, SLOT(setState(bool)) );

   setRunState( VkRunState::STOPPED );
   return m_view;
}


/* outputs a message to the status bar. */
void Memcheck::statusMsg( QString hdr, QString msg ) 
{ 
   emit message( hdr + ": " + msg );
}


/* are we done and dusted?
   anything we need to check/do before being deleted/closed?
*/
bool Memcheck::queryDone()
{
   vk_assert( view() != 0 );

   /* if current process is not yet finished, ask user if they really
      want to close */
   if ( isRunning() ) {
      int ok = vkQuery( this->view(), "Process Running", "&Abort;&Cancel",
                        "<p>The current process is not yet finished.</p>"
                        "<p>Do you want to abort it ?</p>" );
      if ( ok == MsgBox::vkYes ) {
         bool stopped = stop();         /* abort */
         vk_assert( stopped );          // TODO: what todo if couldn't stop?
      } else if ( ok == MsgBox::vkNo ) {
         return false;                         /* continue */
      }
   }

   if (!queryFileSave())
      return false;     // not saved: procrastinate.

   return true;
}

/* if current output not saved, ask user if want to save
   returns false if not saved, but user wants to procrastinate.
*/
bool Memcheck::queryFileSave()
{
   vk_assert( view() != 0 );
   vk_assert( !isRunning() );

   /* currently loaded / parsed stuff is saved to tmp file - ask user
      if they want to save it to a 'real' file */
   if ( !m_fileSaved ) {
      int ok = vkQuery( this->view(), "Unsaved File", 
                        "&Save;&Discard;&Cancel",
                        "<p>The current output is not saved, "
                        " and will be deleted.<br/>"
                        "Do you want to save it ?</p>" );
      if ( ok == MsgBox::vkYes ) {            /* save */

         if ( !fileSaveDialog() ) {
            /* user clicked Cancel, but we already have the
               auto-fname saved anyway, so get outta here. */
            return false;
         }

      } else if ( ok == MsgBox::vkCancel ) {  /* procrastinate */
         return false;
      } else {                                /* discard */
         //      fprintf(stderr, "removing tmp file '%s'\n", m_saveFname.latin1() );
         QFile::remove( m_saveFname );
         m_fileSaved = true;
      }
   }
   return true;
}


bool Memcheck::start( VkRunState::State rs, QStringList vgflags )
{
   bool ok = false;
   vk_assert( rs != VkRunState::STOPPED );
   vk_assert( !isRunning() );
  
   switch ( rs ) {
   case VkRunState::VALGRIND: { ok = runValgrind( vgflags ); break; }
   case VkRunState::TOOL1:    { ok = parseLogFile();         break; }
   case VkRunState::TOOL2:    { ok = mergeLogFiles();        break; }
   default:
      vk_assert_never_reached();
   }
   return ok;
}


bool Memcheck::stop()
{
   vk_assert( isRunning() );

   switch ( runState() ) {
   case VkRunState::VALGRIND: {
      vk_assert( m_vgproc != 0 );
      vk_assert( m_vgproc->isRunning() );
      m_vgproc->stop();

      vk_assert( m_logpoller != 0 );
      vk_assert( m_logpoller->isActive() );
      m_logpoller->stop();
      break;
   }

   case VkRunState::TOOL1:
      /* TODO: make log parsing a VkProcess.  This will allow
         - valkyrie to stay responsive
         - the ability to interrupt the process if taking too long */
      VK_DEBUG("TODO: %s::stop(parse log)", name().latin1() );
      break;

   case VkRunState::TOOL2:
      // TODO: stop merge 
      VK_DEBUG("TODO: %s::stop(merge logs)", name().latin1() );
      break;

   default:
      vk_assert_never_reached();
   }

   return true;
}


/* if --vg-opt=<arg> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the run-button in the gui, 
   then MainWindow::run() calls valkyrie->runTool().  */
bool Memcheck::runValgrind( QStringList vgflags )
{
   m_saveFname = vk_mkstemp( vkConfig->logsDir() + "mc_log", "xml" );
   vk_assert( !m_saveFname.isEmpty() );

   /* fill in filename in flags list */
#if (QT_VERSION-0 >= 0x030200)
   vgflags.gres( "--log-file-exactly", "--log-file-exactly=" + m_saveFname );
#else // QT_VERSION < 3.2
   QStringList::iterator it_str = vgflags.find("--log-file-exactly");
   if (it_str != vgflags.end())
      (*it_str) += ("=" + m_saveFname);

#endif
  
   setRunState( VkRunState::VALGRIND );
   m_fileSaved = false;
   statusMsg( "Memcheck", "Running ... " );

   bool ok = runProcess( vgflags );

   if (!ok) {
      statusMsg( "Memcheck", "Failed" );
      m_fileSaved = true;
      setRunState( VkRunState::STOPPED );
   }
   return ok;
}


/* Parse log file given by [valkyrie::view-log] entry.
   Called by valkyrie->runTool() if cmdline --view-log=<file> specified.
   MemcheckView::openLogFile() if gui parse-log selected.
   If 'checked' == true, file perms/format has already been checked */
bool Memcheck::parseLogFile()
{
   vk_assert( view() != 0 );

   QString log_file = vkConfig->rdEntry( "view-log", "valkyrie" );
   statusMsg( "Parsing", log_file );

   /* check this is a valid file, and has the right perms */
   int errval = PARSED_OK;
   QString ret_file = fileCheck( &errval, log_file, true, false );
   if ( errval != PARSED_OK ) {
      vkError( view(), "File Error", "%s: \n\"%s\"", 
               parseErrString(errval),
               escapeEntities(log_file).latin1() );
      return false;
   }
   log_file = ret_file;
  
   /* fileSaved is always true here 'cos we are just parsing a file
      which already exists on disk */
   m_fileSaved = true;
   setRunState( VkRunState::TOOL1 );

   /* Could be a very large file, so at least get ui up-to-date now */
   qApp->processEvents( 1000/*max msecs*/ );

   /* Parse the log */
   VgLogReader vgLogFileReader( view()->vgLogPtr() );
   bool success = vgLogFileReader.parse( log_file );
   if (!success) {
      VgLogHandler* hnd = vgLogFileReader.handler();
      statusMsg( "Parsing", "Error" );
      vkError( view(), "XML Parse Error",
               "<p>%s</p>", escapeEntities(hnd->errorString()).latin1() );
   }

   if (success) {
      m_saveFname = log_file;
      statusMsg( "Loaded", log_file );
   } else {
      statusMsg( "Parse failed", log_file );
   }
   setRunState( VkRunState::STOPPED );
   return success;
}


/* if --merge=<file_list> was specified on the cmd-line, called by
   valkyrie->runTool(); if set via the open-file-dialog in the gui,
   called by MemcheckView::openMergeFile().  either way, the value in
   [valkyrie:merge] is what we need to know */
bool Memcheck::mergeLogFiles()
{
   QString fname_logList = vkConfig->rdEntry( "merge", "valkyrie" );
   statusMsg( "Merging logs in file-list", fname_logList );
 
   m_saveFname = vk_mkstemp( vkConfig->logsDir() + "mc_merged", "xml" );
   vk_assert( !m_saveFname.isEmpty() );

   QStringList flags;
   flags << vkConfig->rdEntry( "merge-exec","valkyrie");
   flags << "-f";
   flags << fname_logList;
   flags << "-o";
   flags << m_saveFname;

   setRunState( VkRunState::TOOL2 );
   m_fileSaved = false;
   statusMsg( "Merge Logs", "Running ... " );

   bool ok = runProcess( flags );

   if (!ok) {
      statusMsg( "Merge Logs", "Failed" );
      m_fileSaved = true;
      setRunState( VkRunState::STOPPED );
   }
   return ok;
}


/* Run a VKProcess, as given by 'flags'.
   Reads ouput from file, loading this to the listview.
*/
bool Memcheck::runProcess( QStringList flags )
{
   //  fprintf(stderr, "\nMemcheck::runProcess()\n");
   //  for ( unsigned int i=0; i<flags.count(); i++ )
   //    fprintf(stderr, "flag[%d] --> %s\n", i, flags[i].latin1() );
   vk_assert( view() != 0 );

   /* new m_vgreader - view() may be recreated, so need up-to-date ptr */
   vk_assert( m_vgreader == 0 );
   m_vgreader = new VgLogReader( view()->vgLogPtr() );

   /* start the log parse - nothing written yet tho */
   if (!m_vgreader->parse( m_saveFname, true )) {
      QString errMsg = m_vgreader->handler()->errorString();
      if (m_vgreader != 0) {
         delete m_vgreader;
         m_vgreader = 0;
      }
      VK_DEBUG("m_vgreader failed to start parsing empty log\n");
      vkError( view(), "XML Parse Error",
               "<p>%s</p>", errMsg.ascii() );
      return false;
   }

   /* start a new process, listening on exit signal */
   vk_assert( m_vgproc == 0 );
   m_vgproc = new VKProcess( flags, this );
   connect( m_vgproc, SIGNAL(processExited()),
            this, SLOT(processDone()) );

   if ( !m_vgproc->start() ) {
      if (m_vgreader != 0) {
         delete m_vgreader;
         m_vgreader = 0;
      }
      if (m_vgproc != 0) {
         delete m_vgproc;
         m_vgproc = 0;
      }
      VK_DEBUG("m_vgproc failed to start\n");
      vkError( this->view(), "Error", "<p>VG Process failed to start: <br>%s</p>",
               flags.join(" ").latin1() );
      return false;
   }

   /* poll log for updates */
   m_logpoller = new VkLogPoller( this, "memcheck logpoller" );
   //   m_logpoller->setLog(  );
   connect( m_logpoller, SIGNAL(logUpdated()), this, SLOT(readVgLog()) );
   m_logpoller->start();

   //  fprintf(stderr, "\n - END MC::runProcess()\n" );
   return true;
}


/* Called on m_vgproc exit - stop logpoller, but send one more signal.
   m_vgproc may:
   - exit from self
   - be terminated by the user via valkyrie
   - be terminated from vgReadLog because of an xml parse error
*/
void Memcheck::processDone()
{
   //   fprintf(stderr, "\nMemcheck::processDone()\n");
   vk_assert( m_vgproc != 0 );
   vk_assert( m_logpoller != 0 );

   /* Stop signals to readVgLog()... */
   m_logpoller->stop( /*...bar one last chance:*/true );
}


/* Read memcheck xml log output
   Called by m_logpoller signals, and on m_vgproc exit signal

   Deals with cleanup of m_vgproc and m_vgreader, and only emits
   running(false) when both of these are done and dusted.
   - we assume that at some point m_vgproc will die, and use this as
   a condition to do a final cleanup if necessary.
*/
void Memcheck::readVgLog()
{
   //   fprintf(stderr, "\nMemcheck::readVgLog()\n");
   vk_assert( view() != 0 );

   if (m_vgreader != 0) {
      //      fprintf(stderr, " - parseContinue\n");
      VgLogHandler* hnd = m_vgreader->handler();

      /* Note: xml log may not be completed by valgrind,
         meaning hnd never reaches 'finished' - this is taken care of by
         the final cleanup on m_vgproc death
      */
      if (m_vgreader->parseContinue()) {
         /* Parsing succeeded */
         if (hnd->finished) {  /* We're done */
            //            fprintf(stderr, " - finished\n");

            /* cleanup reader */
            delete m_vgreader;
            m_vgreader = 0;

            if (runState() == VkRunState::VALGRIND)
               statusMsg( "Memcheck", "Finished" );
            else
               statusMsg( "Merge Logs", "Finished" );

            /* Only stop running if m_vgproc done and dusted */
            if (m_vgproc == 0)
               setRunState( VkRunState::STOPPED );
         }
      } else {
         //            fprintf(stderr, " - parse failed on line %d\n", hnd->errorLine);
         /* Parsing failed: kill m_vgproc, if alive
            - will trigger another read, but m_vgreader == 0 */

         /* gather first stderr line */
         QString str_err = m_vgproc->readLineStderr();

         /* cleanup reader */
         QString errMsg = hnd->errorString();
         delete m_vgreader;
         m_vgreader = 0;

         if (m_vgproc->isRunning())
            stop();

         if (runState() == VkRunState::VALGRIND) {
            statusMsg( "Memcheck", "Error parsing output log" );
            vkError( view(), "XML Parse Error",
                     "<p>%s</p>", errMsg.ascii() );
         } else {
            statusMsg( "Merge Logs", "Error parsing output log" );
            if (str_err.isEmpty()) str_err = "";
            vkError( view(), "Parse Error",
                     "<p>%s</p>", escapeEntities(str_err).latin1() );
         }

         /* Only stop running if m_vgproc done and dusted */
         if (m_vgproc == 0)
            setRunState( VkRunState::STOPPED );
      }
   }

   /* If m_vgproc stopped but not cleaned up, cleanup. */
   if (m_vgproc != 0 && !m_vgproc->isRunning()) {
      //      fprintf(stderr, " - cleaning up m_vgproc\n");

      /* gather first stderr line */
      QString str_err = m_vgproc->readLineStderr();

      delete m_vgproc;
      m_vgproc = 0;

      /* m_vgproc is dead, but it's possible the last signals haven't
         called us yet.  However, we can't wait and see else m_vgreader
         may not get deallocated. */
      if (m_vgreader != 0) {
         readVgLog();

         /* still not finished => error
            valgrind xml output has not been completed properly, or merge failed */
         if (m_vgreader != 0) {
            delete m_vgreader;
            m_vgreader = 0;

            if (runState() == VkRunState::VALGRIND) {
               statusMsg( "Memcheck", "Error - incomplete output log" );
               vkError( view(), "XML Parse Error",
                        "<p>valgrind xml output is incomplete</p>" );
            } else {
               statusMsg( "Merge Logs", "Error - incomplete output log" );
               if (str_err.isEmpty()) str_err = "";
               vkError( view(), "Parse Error",
                        "<p>%s</p>", escapeEntities(str_err).latin1() );
            }
         }
      }

      /* Ok, we're sure nothing is alive anymore */
      setRunState( VkRunState::STOPPED );
   }
}


/* brings up a fileSaveDialog until successfully saved,
   or user pressed Cancel.
   if fname.isEmpty, ask user for a name first.
   returns false on user pressing Cancel, else true.
*/
bool Memcheck::fileSaveDialog( QString fname/*=QString()*/ )
{
   vk_assert( view() != 0 );

   QFileDialog dlg;
   dlg.setShowHiddenFiles( true );
   QString flt = "XML Files (*.xml);;Log Files (*.log.*);;All Files (*)";
   QString cptn = "Save Log File As";

   /* Ask fname if don't have one already */
   if ( fname.isEmpty() ) {
      /* start dlg in dir of last saved logfile */
      QString start_path = QFileInfo( m_saveFname ).dirPath();
      fname = dlg.getSaveFileName( start_path, flt, this->view(), "fsdlg", cptn );
      if ( fname.isEmpty() )
         return false;
   }

   /* try to save file until succeed, or user Cancels */
   while ( !saveParsedOutput( fname ) ) {
      QString start_path = QFileInfo( fname ).dirPath();
      fname = dlg.getSaveFileName( start_path, flt, this->view(), "fsdlg", cptn );
      if ( fname.isEmpty() )   /* Cancelled */
         return false;
   }

   return true;
}

/* Save to file
   - we already have everything in m_saveFname logfile, so just copy that
*/
bool Memcheck::saveParsedOutput( QString& fname )
{
   //printf("saveParsedOutput(%s)\n", fname.latin1() );
   vk_assert( view() != 0 );
   vk_assert( !fname.isEmpty() );

   /* checks on destination fname */
   if ( fname.find('/') == -1 ) {
      /* no abs or rel path given, so save in default dir */
      /* TODO: Not sure about this...
         - normally, even if no './' is given, it is still implied... */
      fname = vkConfig->logsDir() + fname;
   }
   /* make sure path is absolute */
   fname = QFileInfo( fname ).absFilePath();

   /* if this filename already exists, check if we should over-write it */
   if ( QFile::exists( fname ) ) {
      int ok = vkQuery( this->view(), 2, "Overwrite File",
                        "<p>Over-write existing file '%s' ?</p>", 
                        fname.latin1() );
      if ( ok == MsgBox::vkNo ) {
         /* nogo: return and try again */
         return false;
      }
   }

   /* save log (=copy/rename) */
   bool ok;
   if (!m_fileSaved) {
      /* first save after a run, so just rename m_saveFname => fname */
      //printf("renaming: '%s' -> '%s'\n", m_saveFname.latin1(), fname.latin1() );
      ok = QDir().rename( m_saveFname, fname );
   } else {
      /* we've saved once already: must now copy m_saveFname => fname */
      //printf("copying: '%s' -> '%s'\n", m_saveFname.latin1(), fname.latin1() );
      QUrlOperator *op = new QUrlOperator();
      op->copy( m_saveFname, fname, false, false ); 
      /* TODO: check copied ok */
      ok = true;
   }
   if (ok) {
      m_saveFname = fname;
      m_fileSaved = true;
      statusMsg( "Saved", m_saveFname );
   } else {
      /* nogo: return and try again */
      vkInfo( this->view(), "Save Failed", 
              "<p>Failed to save file to '%s'",  fname.latin1() );
      statusMsg( "Failed Save", m_saveFname );
   }
   return ok;
}

