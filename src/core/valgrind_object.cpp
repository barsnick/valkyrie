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
#include "vk_config.h"
#include "html_urls.h"



/* class Valgrind ------------------------------------------------------ */
Valgrind::~Valgrind() { }


Valgrind::Valgrind()
  : VkObject( "Valgrind", "Valgrind", Qt::Key_unknown, false ) 
{ 
  addOpt( TOOL,        Option::ARG_STRING, Option::COMBO, 
          "valgrind",  '\0',               "tool", 
          "<name>",    "memcheck|cachegrind|massif", "memcheck",
          "Main tool:", 
          "use the Valgrind tool named <name>.  Available tools are: memcheck, cachegrind, massif", 
          urlNone );
  /* common options relevant to all tools */
  addOpt( VERBOSITY,   Option::ARG_UINT,   Option::SPINBOX, 
          "valgrind",  '\0',               "verbosity", 
          "<0..4>",    "0|4",              "1",
          "Verbosity level:",
          "Be more verbose, include counts of errors", 
          urlNone );
  addOpt( XML_OUTPUT,  Option::ARG_BOOL,   Option::CHECK, 
          "valgrind",  '\0',               "xml",
          "<yes|no>",  "yes|no",           "yes",
          "Output in xml format:", "all output is in XML", 
          urlNone );
  addOpt( TRACE_CH,    Option::ARG_BOOL,   Option::CHECK,   
          "valgrind",  '\0',               "trace-children",
          "<yes|no>",  "yes|no",           "no",
          "Trace child processes: ",
          "Valgrind-ise child processes?",
          urlNone );
  addOpt( TRACK_FDS,   Option::ARG_BOOL,   Option::CHECK,      
          "valgrind",  '\0',               "track-fds", 
          "<yes|no>",  "yes|no",           "no",
          "Track open file descriptors:", 
          "track open file descriptors?",
          urlNone );
  addOpt( TIME_STAMP,  Option::ARG_BOOL,   Option::CHECK,      
          "valgrind",  '\0',               "time-stamp", 
          "<yes|no>",  "yes|no",           "no",
          "Add timestamps to log messages:", 
          "add timestamps to log messages?",
          urlNone );
  /* uncommon options relevant to all tools */
  addOpt( RUN_LIBC,    Option::ARG_BOOL,   Option::CHECK,      
          "valgrind",  '\0',               "run-libc-freeres",
          "<yes|no>",  "yes|no",           "yes",
          "Free glibc memory at exit:",
          "Free up glibc memory at exit?",
          urlNone );
  addOpt( WEIRD,       Option::ARG_STRING, Option::COMBO,      
          "valgrind",  '\0',               "weird-hacks", 
          "<hack1,...>", "none|lax-ioctls|ioctl-mmap", "none",
          "Weird hacks:",
          "Slightly modify the simulated behaviour. Recognised hacks are: lax-ioctls,ioctl-mmap. Use with caution!", 
          urlNone );
  addOpt( PTR_CHECK,   Option::ARG_BOOL,   Option::CHECK,   
          "valgrind",  '\0',               "pointercheck",
          "<yes|no>",  "yes|no",           "yes",
          "Enforce client address space limits:",
          "enforce client address space limits",
          urlNone );
  addOpt( EM_WARNS,    Option::ARG_BOOL,   Option::CHECK,   
          "valgrind",  '\0',               "show-emwarns",
          "<yes|no>",  "yes|no",           "no",
          "Show warnings about emulation limits:",
          "show warnings about emulation limits?",
          urlNone );
  addOpt( ELAN_HACKS,  Option::ARG_BOOL,   Option::CHECK,
          "valgrind",  '\0',               "support-elan3",
          "<yes|no>",  "yes|no",           "no",
          "Support Quadrics Elan3:",
          "hacks for Quadrics Elan3 support",
          urlNone );
  /* options relevant to error-reporting tools */
  addOpt( LOG_FD,      Option::ARG_UINT,   Option::SPINBOX, 
          "valgrind",  '\0',               "log-fd", 
          "<1..1024>", "1|1023",           "2",
          "Log to file descriptor:",
          "log messages to file descriptor (1=stdout, 2=stderr)",  
          /* TODO: subclass QProcess so we can use fd > 2 */
          urlNone );
  addOpt( LOG_PID,     Option::ARG_STRING, Option::LEDIT, 
          "valgrind",  '\0',               "log-file", 
          "<file>",    "",                 "",
          "Log to <file>.pid<pid>:",
          "Log messages to <file>.pid<pid>", 
          urlNone );
  addOpt( LOG_FILE,    Option::ARG_STRING, Option::LEDIT, 
          "valgrind",  '\0',               "log-file-exactly", 
          "<file>",    "",                 "",
          "Log to file:",
          "log messages to <file>", 
          urlNone );
  addOpt( LOG_SOCKET,  Option::ARG_STRING, Option::LEDIT, 
          "valgrind",  '\0',               "log-socket", 
          "<ipaddr:port>", "",             "",
          "Log to socket:",
          "log messages to socket ipaddr:port",
          urlNone );
  addOpt( DEMANGLE,    Option::ARG_BOOL,   Option::CHECK, 
          "valgrind",  '\0',               "demangle", 
          "<yes|no>",  "yes|no",           "yes",
          "Automatically demangle C++ names",
          "automatically demangle C++ names?",
          urlNone );
  addOpt( NUM_CALLERS, Option::ARG_UINT,   Option::SPINBOX, 
          "valgrind",  '\0',               "num-callers", 
          "<1..50>",   "1|50",             "12",
          "Number of stack trace callers:", 
          "show <num> callers in stack traces",
          urlNone );
  addOpt( ERROR_LIMIT, Option::ARG_BOOL,   Option::CHECK,   
          "valgrind",  '\0',               "error-limit", 
          "<yes|no>",  "yes|no",           "yes",
          "Limit the number of errors shown",
          "Stop showing new errors if too many?",
          urlNone );
  addOpt( SHOW_BELOW,  Option::ARG_BOOL,   Option::CHECK,   
          "valgrind",  '\0',               "show-below-main", 
          "<yes|no>",  "yes|no",           "no",
          "Continue stack traces below main()",
          "continue stack traces below main()", 
          urlNone );
  /*--------------------------------------------------------------- */
  /* this holds a list of *all* suppression files ever found.
     it is seeded with any suppression files found by configure */
  addOpt( SUPPS_ALL,   Option::NOT_POPT,   Option::LISTBOX, 
          "valgrind",  '\0',               "supps-all",
          "",          "",                 "",
          /*"default.supp;xfree-3.supp;xfree-4.supp;glibc-2.1.supp|glibc-2.2.supp;glibc-2.3.supp",*/
          "Available error-suppression file(s):",
          "",          urlNone );
  /* need to keep a list of suppression files found by configure
     which is never changed; only used to reset default values */
  addOpt( SUPPS_DEF,   Option::NOT_POPT,   Option::NONE, 
          "valgrind",  '\0',               "supps-def",
          "",          "",                 "",
          "",          "",                 urlNone );
  addOpt( SUPPS_SEL,    Option::ARG_STRING, Option::LISTBOX,
          "valgrind",   '\0',              "suppressions",
          "<file1,...>", "",               "default.supp",
          "Selected error-suppression file(s):",
          "suppress errors described in suppressions file(s)", 
          urlNone );
  /*--------------------------------------------------------------- */
  addOpt( GEN_SUPP,    Option::ARG_BOOL,   Option::CHECK, 
          "valgrind",  '\0',               "gen-suppressions",
          "<yes|no|all>",  "yes|no|all",   "no",
          "Print suppressions for errors",
          "print suppressions for errors?",
          urlNone );
  addOpt( DB_ATTACH,   Option::ARG_BOOL,   Option::CHECK, 
          "valgrind",  '\0',               "db-attach", 
          "<yes|no>",  "yes|no",           "no",
          "Start debugger on error detection",
          "start debugger when errors detected?",
          urlNone );
  addOpt( DB_COMMAND,  Option::ARG_STRING, Option::LEDIT, 
          "valgrind",  '\0',               "db-command", 
          "<command>", "",                 "/usr/bin/gdb -nw %f %p",
          "Debugger:", 
          "command to start debugger",
          urlNone );
  addOpt( INPUT_FD,    Option::ARG_UINT,   Option::SPINBOX, 
          "valgrind",  '\0',               "input-fd",
          "<0..1024>", "0|1023",           "0",
          "Input file descriptor:", 
          "File descriptor for (db) input (0=stdin, 1=stdout, 2=stderr)",
          urlNone );
  addOpt( MAX_SFRAME,  Option::ARG_UINT,   Option::SPINBOX, 
          "valgrind",  '\0',               "max-stackframe",
          "<number>",  "0|2000000",        "2000000",
          "Stack switch on SP changes at:", 
          "assume stack switch for stack pointer changes larger than <number> bytes",
          urlNone );
}


int Valgrind::checkOptArg( int optid, const char* argval, 
                           bool use_gui/*=false*/ )
{
  vk_assert( optid >= 0 && optid <= LAST_CMD_OPT );

  int errval = PARSED_OK;
  QString argVal( argval );
  Option* opt = findOption( optid );

  switch ( optid ) {

    case TOOL:
    case XML_OUTPUT:
    case VERBOSITY:
    case WEIRD:
    case RUN_LIBC:
    case NUM_CALLERS:
    case ERROR_LIMIT:
    case GEN_SUPP:
    case DEMANGLE:
    case INPUT_FD:
    case SHOW_BELOW:
    case MAX_SFRAME:
    case LOG_FD:
      opt->isValidArg( &errval, argval );
      break;

    case TRACE_CH: {
      if ( opt->isValidArg( &errval, argval ) ) {
        if ( argVal == "yes" ) {
          if ( vkConfig->rdBool( "gdb-attach", "valgrind" ) )
            errval = PERROR_DB_CONFLICT;
        }
      }
    } break;

    case LOG_FILE:    /* let valgrind handle the checking here */
    case LOG_PID:
    case LOG_SOCKET:
      if ( vkConfig->rdBool( "db-attach","valgrind" ) ) {
        errval = PERROR_DB_OUTPUT;
      } else {
        // FIXME re this stuff 
        //config->wrEntry( "false", "logfile-fd-use", "valgrind" );
        //vkConfig->wrEntry( "true",  "logfile-use",    "valgrind" );
        //config->wrEntry( "false", "logsocket-use",  "valgrind" );
      } break;

    case DB_COMMAND: {   /* gdb -nw %f %p */
      int pos = argVal.find( ' ' );
      QString tmp = argVal.left( pos );
      argVal = binaryCheck( &errval, tmp );
      argVal += tmp.right( tmp.length() - pos+1 );
      printf("db_command: %s\n", argVal.latin1() );
    } break;

    /* check for conflict with --trace-children */
    case DB_ATTACH:
      printf("FIXME: this doesn't work with xml output\n");
      if ( opt->isValidArg( &errval, argval ) ) {
        if ( vk_strcmp( argval, "yes" ) ) {
          if ( vkConfig->rdBool( "trace-children","valgrind" ) )
            errval = PERROR_DB_CONFLICT;
          // FIXME re this stuff 
          //if ( vkConfig->rdBool( "logfile-use",  "valgrind" ) ||
          //     vkConfig->rdBool( "logsocket-use","valgrind" ) )
          //  errval = PERROR_DB_OUTPUT;
        }
      } break;

    case TRACK_FDS:
    case TIME_STAMP:
    case PTR_CHECK:
    case EM_WARNS:
    case SUPPS_SEL:
      printf("TODO: check Valgrind opts\n");
      break;

  }

  /* if this option has been called from the cmd-line, save its value
     in valkyrierc if it has passed the checks. */
  if ( errval == PARSED_OK && use_gui == false ) {
    writeOptionToConfig( opt, argval );
  }

  return errval;
}


/* valkyrie hijacks any log-to-file flags; these are not passed to
   valgrind, but are used after parsing has finished to save to.
   FIXME: not sure yet how to handle LOG_FD and LOG_SOCKET */
QStringList Valgrind::modifiedFlags()
{
  QStringList modFlags;
  QString defVal, cfgVal;

  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {
  
    switch ( opt->key ) {

      /* we never want these included */
      case SUPPS_ALL:
      case SUPPS_DEF:
        break;

      /* we need '--suppressions=' before each and every filename */
      case SUPPS_SEL: {
        QStringList files = QStringList::split( ",", vkConfig->rdEntry( opt->cfgKey(), name() ) );
        for ( unsigned int i=0; i<files.count(); i++ ) {
          modFlags << "--" + opt->cfgKey() + "=" + files[i];
        }
      } break;

      case LOG_PID:         /* log to <file>.pid<pid>    */
      case LOG_FILE:        /* log to <file>             */
      case LOG_SOCKET:      /* log to socket ipaddr:port */
        break;

      case XML_OUTPUT:      /* the sine qua non */
        modFlags << "--" + opt->cfgKey() + "=" 
                         + vkConfig->rdEntry( opt->cfgKey(), name() );
        break;

      default:
        defVal = opt->defaultValue;
        cfgVal = vkConfig->rdEntry( opt->cfgKey(), name() );
        if ( defVal != cfgVal ) {
          modFlags << "--" + opt->cfgKey() + "=" + cfgVal;
        }
        break;
    }
  }

  return modFlags;
}


