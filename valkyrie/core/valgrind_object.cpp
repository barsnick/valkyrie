/* --------------------------------------------------------------------- 
 * Implementation of class Valgrind                  valgrind_object.cpp
 * Valgrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "valgrind_object.h"
#include "config.h"            // VK_CFG_DIR, VK_SUPPS_DIR
#include "vk_config.h"
#include "html_urls.h"
#include "memcheck_object.h"
#include "cachegrind_object.h"
#include "massif_object.h"
#include "vk_option.h"         // PERROR* and friends 
#include "vk_utils.h"          // vk_assert, VK_DEBUG, etc.


/* class Valgrind ------------------------------------------------------ */
Valgrind::~Valgrind()
{
   m_toolObjList.setAutoDelete( true );
   m_toolObjList.clear();
}


/* these opts should be kept in exactly the same order as valgrind
   outputs them, as it makes keeping up-to-date a lot easier. */
Valgrind::Valgrind()
   : VkObject( "Valgrind", "Valgrind", Qt::Key_unknown, VkObject::ID_VALGRIND ) 
{ 
   /* init tools */
   initToolObjects();

   /*--------------------------------------------------------------- */
   /* tool */
   addOpt( TOOL,        VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO, 
           "valgrind",  '\0',                 "tool", 
           "<name>",    "memcheck|cachegrind|massif", "memcheck",
           "Main tool:", 
           "use the Valgrind tool named <name>.  Available tools are: memcheck, cachegrind, massif", 
           urlVgCore::mainTool );

   /*--------------------------------------------------------------- */
   /* common options relevant to all tools */
   addOpt( VERBOSITY,   VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "valgrind",  '\0',                 "verbosity", 
           "<0..4>",    "0|4",                "1",
           "Verbosity level:",
           "Be more verbose, include counts of errors", 
           urlVgCore::verbosity );
   addOpt( TRACE_CH,    VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,   
           "valgrind",  '\0',                 "trace-children",
           "<yes|no>",  "yes|no",             "no",
           "Trace child processes: ",
           "Valgrind-ise child processes?",
           urlVgCore::traceChild );
   addOpt( TRACK_FDS,   VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,      
           "valgrind",  '\0',                 "track-fds", 
           "<yes|no>",  "yes|no",             "no",
           "Track open file descriptors:", 
           "track open file descriptors?",
           urlVgCore::trackFds );
   addOpt( TIME_STAMP,  VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,      
           "valgrind",  '\0',                 "time-stamp", 
           "<yes|no>",  "yes|no",             "no",
           "Add timestamps to log messages:", 
           "add timestamps to log messages?",
           urlVgCore::timeStamp );
   addOpt( LOG_FD,      VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "valgrind",  '\0',                 "log-fd", 
           "<1..1024>", "1|1023",             "2",
           "Log to file descriptor:",
           "log messages to file descriptor (1=stdout, 2=stderr)",  
           urlVgCore::logToFd );
   addOpt( LOG_PID,     VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "valgrind",  '\0',                 "log-file", 
           "<file>",    "",                   "",
           "Log to <file>.pid<pid>:",
           "Log messages to <file>.pid<pid>", 
           urlVgCore::logToFilePid );
   addOpt( LOG_FILE,    VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "valgrind",  '\0',                 "log-file-exactly", 
           "<file>",    "",                   "",
           "Log to file:",
           "log messages to <file>", 
           urlVgCore::logToFile );
   addOpt( LOG_QUAL,    VkOPTION::ARG_STRING, VkOPTION::WDG_NONE,
           "valgrind",  '\0',                 "log-file-qualifier",
           "VAR",       "",                   "",
           "<VAR>",     "incorporate $VAR in logfile name",
           urlVgCore::logFileQual );
   addOpt( LOG_SOCKET,  VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "valgrind",  '\0',                 "log-socket", 
           "<ipaddr:port>", "",               "",
           "Log to socket:",
           "log messages to socket ipaddr:port",
           urlVgCore::logToSocket  );

   /*--------------------------------------------------------------- */
   /* uncommon options relevant to all tools */
   addOpt( RUN_LIBC,    VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,      
           "valgrind",  '\0',                 "run-libc-freeres",
           "<yes|no>",  "yes|no",             "yes",
           "Free glibc memory at exit:",
           "Free up glibc memory at exit?",
           urlVgCore::freeGlibc );
   addOpt( SIM_HINTS,   VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO,      
           "valgrind",  '\0',                 "sim-hints", 
           "<hint1,hint2,...>", "none|lax-ioctls|enable-outer", "none",
           "Simulation Hints:",
           "Slightly modify the simulated behaviour. Recognised hints are: lax-ioctls, enable-outer. Use with caution!", 
           urlVgCore::simHints );
   addOpt( KERN_VAR,    VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO,      
           "valgrind",  '\0',                 "kernel-variant", 
           "<variant1,variant2,...>", "none|bproc", "none",
           "Kernel Variants:",
           "Handle non-standard kernel variants. Recognised variants are: bproc. Use with caution!", 
           urlVgCore::kernelVariant );
   addOpt( EM_WARNS,    VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,   
           "valgrind",  '\0',                 "show-emwarns",
           "<yes|no>",  "yes|no",             "no",
           "Show warnings about emulation limits:",
           "show warnings about emulation limits?",
           urlVgCore::showEmWarns );
   addOpt( SMC_CHECK,   VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO,
           "valgrind",  '\0',                 "smc-check",
           "<none|stack|all>",  "none|stack|all",  "stack",
           "Where to check for self-modifying code",
           "checks for self-modifying code: none, only for code on the stack, or all",
           urlVgCore::smcSupport );

   /*--------------------------------------------------------------- */
   /* options relevant to error-reporting tools */
   addOpt( XML_OUTPUT,  VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "valgrind",  '\0',                 "xml",
           "<yes|no>",  "yes|no",             "yes",
           "Output in xml format:", "all output is in XML", 
           urlVgCore::xmlOutput );
   addOpt( XML_COMMENT, VkOPTION::ARG_STRING, VkOPTION::WDG_NONE,
           "valgrind",  '\0',                 "xml-user-comment",
           "<str>",     "",                   "",
           "Insert verbatim in xml output", 
           "copy <str> verbatim to XML output",
           urlVgCore::xmlComment );
   addOpt( DEMANGLE,    VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "valgrind",  '\0',                 "demangle", 
           "<yes|no>",  "yes|no",             "yes",
           "Automatically demangle C++ names",
           "automatically demangle C++ names?",
           urlVgCore::autoDemangle );
   addOpt( NUM_CALLERS, VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "valgrind",  '\0',                 "num-callers", 
           "<1..50>",   "1|50",               "12",
           "Number of stack trace callers:", 
           "show <num> callers in stack traces",
           urlVgCore::numCallers );
   addOpt( ERROR_LIMIT, VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,   
           "valgrind",  '\0',                 "error-limit", 
           "<yes|no>",  "yes|no",             "yes",
           "Limit the number of errors shown",
           "Stop showing new errors if too many?",
           urlVgCore::errorLimit );
   addOpt( SHOW_BELOW,  VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,   
           "valgrind",  '\0',                 "show-below-main", 
           "<yes|no>",  "yes|no",             "no",
           "Continue stack traces below main()",
           "continue stack traces below main()", 
           urlVgCore::stackTraces );

   /*--------------------------------------------------------------- */
   /* Can't access vkConfig->suppDir(): config created after us! */
   QString defSuppDir = QDir::homeDirPath() + "/" + VK_CFG_DIR + VK_SUPPS_DIR;
   /* list of dirs holding suppression files */
   addOpt( SUPPS_DIRS,  VkOPTION::NOT_POPT,   VkOPTION::WDG_LISTBOX, 
           "valgrind",  '\0',                 "supps-dirs",
           "",          "",                   defSuppDir,
           "Suppression Dirs:", "",           urlValkyrie::suppsTab );

   /* list of all suppression files found in option SUPP_DIRS */
   addOpt( SUPPS_AVAIL, VkOPTION::NOT_POPT,   VkOPTION::WDG_LISTBOX, 
           "valgrind",  '\0',                 "supps-avail",
           "",          "",                   "",
           "Available error-suppression file(s):",
           "",          urlValkyrie::suppsTab );

   /* list of selected suppression files */
   addOpt( SUPPS_SEL,   VkOPTION::ARG_STRING, VkOPTION::WDG_LISTBOX,
           "valgrind",  '\0',                 "suppressions",
           "<file1,...>", "",                 "",
           "Selected error-suppression file(s):",
           "suppress errors described in suppressions file(s)", 
           urlValkyrie::suppsTab );

   /*--------------------------------------------------------------- */
   addOpt( GEN_SUPP,    VkOPTION::ARG_STRING, VkOPTION::WDG_COMBO,
           "valgrind",  '\0',                 "gen-suppressions",
           "<yes|no|all>",  "yes|no|all",     "no",
           "Print suppressions for errors",
           "print suppressions for errors?",
           urlVgCore::genSuppressions );
   addOpt( DB_ATTACH,   VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "valgrind",  '\0',                 "db-attach", 
           "<yes|no>",  "yes|no",             "no",
           "Start debugger on error detection",
           "start debugger when errors detected?",
           urlVgCore::startDebugger );
   addOpt( DB_COMMAND,  VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "valgrind",  '\0',                 "db-command", 
           "<command>", "",                   "/usr/bin/gdb -nw %f %p",
           "Debugger:", 
           "command to start debugger",
           urlVgCore::whichDebugger );
   addOpt( INPUT_FD,    VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "valgrind",  '\0',                 "input-fd",
           "<0..1024>", "0|1023",             "0",
           "Input file descriptor:", 
           "File descriptor for (db) input (0=stdin, 1=stdout, 2=stderr)",
           urlVgCore::inputFd );
   addOpt( MAX_SFRAME,  VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "valgrind",  '\0',                 "max-stackframe",
           "<number>",  "0|2000000",          "2000000",
           "Stack switch on SP changes at:", 
           "assume stack switch for stack pointer changes larger than <number> bytes",
           urlVgCore::maxSFrames );
}


/* check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages -------------------- */
int Valgrind::checkOptArg( int optid, QString& argval )
{
   vk_assert( optid >= 0 && optid < NUM_OPTS );

   int errval = PARSED_OK;
   Option* opt = findOption( optid );

   switch ( (Valgrind::vgOpts)optid ) {

   case TOOL:
   case SIM_HINTS:
   case RUN_LIBC:
   case NUM_CALLERS:
   case DEMANGLE:
//   case INPUT_FD: // TODO
   case SHOW_BELOW:
   case MAX_SFRAME:
   case SMC_CHECK:
      opt->isValidArg( &errval, argval );
      break;

   case VERBOSITY:
   case TRACK_FDS:
   case TIME_STAMP:
   case EM_WARNS:
   case GEN_SUPP:
   case ERROR_LIMIT:
   case DB_COMMAND:
   case DB_ATTACH:
      /* Note: gui option disabled, so only reaches here from cmdline */
      errval = PERROR_BADOPT;
      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      vkPrintErr(" - Valgrind presets these options for XML output.");
      vkPrintErr(" - See valgrind/docs/internals/xml_output.txt.");
      break;

   case XML_COMMENT:
      /* don't wan't xml in comment: escape '<','&',etc */
      argval = escapeEntities( argval );
      break;

   case SUPPS_DIRS:
   case SUPPS_AVAIL:
      /* Not popts: only reaches here from gui */
      vkPrint("TODO: check Valgrind opts");
      break;

   case SUPPS_SEL:
      /* check valid suppression files */
      vkPrint("TODO: check Valgrind opts");
      break;

   case TRACE_CH: {
#if 0
      if ( opt->isValidArg( &errval, argval ) ) {
         if ( argval == "yes" ) {
            if ( vkConfig->rdBool( "db-attach", "valgrind" ) )
               errval = PERROR_DB_CONFLICT;
         }
      }
#else
      /* Disabled for now - can't deal with the multiple xml files this generates */
      /* Note: Also disabled in ValgrindOptionsPage() */
      errval = PERROR_BADOPT;
      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      vkPrintErr(" - Valkyrie can't yet handle the multiple xml files this generates.");
#endif
   } break;


   case XML_OUTPUT:
      /* Note: gui option disabled, so only reaches here from cmdline */
      errval = PERROR_BADOPT;
      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      vkPrintErr(" - Valkyrie always requires xml output from Valgrind.");
      break;


#if 0 // TODO: Fix Valgrind to allow gdb attaching with XML output
   case DB_COMMAND: {   /* gdb -nw %f %p */
      int pos = argval.find( ' ' );
      QString tmp = argval.left( pos );
      argval = binaryCheck( &errval, tmp );
      argval += tmp.right( tmp.length() - pos+1 );
      // vkPrint("db_command: %s", argval.latin1() );
   } break;

   /* check for conflict with --trace-children */
   case DB_ATTACH:
      if ( opt->isValidArg( &errval, argval ) ) {
         if ( argval == "yes" ) {
            if ( vkConfig->rdBool( "trace-children","valgrind" ) )
               errval = PERROR_DB_CONFLICT;
         }
      } break;
#endif

   case KERN_VAR:
      break;

   /* logging options */
   /* for all tools we use --log-file-exactly=xyz.
      this is set in Valkyrie::runTool(), and updated by the tool.
      all logging options are therefore ignored */
   case LOG_FILE:
   case LOG_FD:
   case LOG_PID:
   case LOG_QUAL:
   case LOG_SOCKET:
      /* Note: gui options disabled, so only reaches here from cmdline */
      errval = PERROR_BADOPT;
      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      vkPrintErr(" - Valkyrie sets its own logging options to gather data from Valgrind.");
      break;


   /* Not yet implemented */
   case INPUT_FD:
      /* Note: gui option disabled, so only reaches here from cmdline */
      errval = PERROR_BADOPT;
      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      vkPrintErr(" - Not yet implemented.");
      break;

   default:
      vk_assert_never_reached();
   }

   return errval;
}


/* valkyrie hijacks any log-to-file flags; these are not passed to
   valgrind, but are used after parsing has finished to save to. */
QStringList Valgrind::modifiedVgFlags( const ToolObject* tool_obj )
{
   QStringList modFlags;
   QString defVal, cfgVal, flag;

   for ( Option* opt = m_optList.first(); opt; opt = m_optList.next() ) {
      flag = opt->m_longFlag.isEmpty() ? opt->m_shortFlag
                                       : opt->m_longFlag;
      defVal = opt->m_defaultValue;
      cfgVal = vkConfig->rdEntry( opt->cfgKey(), name() );

      switch ( (Valgrind::vgOpts)opt->m_key ) {

      /* we never want these included */
      case TOOL:            /* tool set by valkyrie */
      case SUPPS_DIRS:      /* false option */
      case SUPPS_AVAIL:     /* false option */
         break;

      /* only error-reporting tools have suppressions */
      case SUPPS_SEL: {
         if ( tool_obj->name() == "memcheck" ) {
            /* we need '--suppressions=' before each and every filename */
            QString optEntry = vkConfig->rdEntry( opt->cfgKey(), name() );
            QStringList files = QStringList::split( ",", optEntry );
            for ( unsigned int i=0; i<files.count(); i++ ) {
               modFlags << "--" + opt->cfgKey() + "=" + files[i];
            }
         }
      } break;


      /* for memcheck we always need xml=yes */
      case XML_OUTPUT:
         if ( tool_obj->name() == "memcheck")
            modFlags << "--" + opt->cfgKey() + "=yes";
         else
            if ( defVal != cfgVal )
               modFlags << "--" + opt->cfgKey() + "=" + cfgVal;
         break;

      case VERBOSITY:
      case TRACK_FDS:
      case TIME_STAMP:
      case EM_WARNS:
      case GEN_SUPP:
      case ERROR_LIMIT:
      case DB_ATTACH:
      case DB_COMMAND:
         if ( defVal != cfgVal ) {
            // disabled for now: /* gui options not disabled: other tools use these options */
            if ( tool_obj->name() == "memcheck") {
               /* memcheck presets/ignores these options for xml output
                  - ignore these opts
                  - see valgrind/docs/internals/xml_output.txt */
            } else {
               modFlags << "--" + opt->cfgKey() + "=" + cfgVal;
            }
         }
         break;

      /* for all tools we use --log-file-exactly=xyz.
         this is set in Valkyrie::runTool(), and updated by the tool.
         all logging options should therefore not be used */
      case LOG_FILE:
      case LOG_FD:
      case LOG_PID:
      case LOG_QUAL:
      case LOG_SOCKET:
         /* ignore these opts */
         break;

      default:
         if ( defVal != cfgVal ) {
            modFlags << "--" + opt->cfgKey() + "=" + cfgVal;
         }
         break;
      }
   }

   return modFlags;
}




/* Register tools */
void Valgrind::initToolObjects()
{ 
   int objId = VkObject::ID_TOOL0;
   m_toolObjList.append( new Memcheck  ( objId++ ) );

   // TODO: I need another lifetime!
//   m_toolObjList.append( new Cachegrind( objId++ ) );
//   m_toolObjList.append( new Massif    ( objId++ ) );
}


/* ToolObject access -------------------------------------------=---- */ 

/* Returns a list of all ptrs to ToolObjects */
ToolObjList Valgrind::toolObjList()
{
   return m_toolObjList;
}

/* Returns a ToolObject's objectId based on its name */
int Valgrind::toolObjId( const QString& name )
{
   return toolObj(name)->objId();
}

/* Returns a ToolObject based on its objectId */
ToolObject* Valgrind::toolObj( int tid )
{
   //   vkPrint("Valgrind::toolObj( int tid=%d )", tid);
   vk_assert( tid >= ID_TOOL0 );
   ToolObject* tool = m_toolObjList.at( tid - ID_TOOL0 );
   vk_assert( tool != 0 );
   vk_assert( tool->objId() == tid );
   return tool;
}

/* Returns a ToolObject based on its name */
ToolObject* Valgrind::toolObj( const QString& name ) {
   ToolObject* tool;
   for ( tool = m_toolObjList.first(); tool; tool = m_toolObjList.next() ) {
      if ( tool->name() == name )
         return tool;
   }
   vk_assert_never_reached();
   return NULL;
}
