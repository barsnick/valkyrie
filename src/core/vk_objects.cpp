/* --------------------------------------------------------------------- 
 * Options / flags                                        vk_options.cpp
 * ---------------------------------------------------------------------
 */

#include "vk_objects.h"
#include "vk_config.h"
#include "vk_utils.h"          // vk_assert(), vk_strcmp(), 
#include "vk_popt_option.h"    // PERROR* and friends 
#include "vk_msgbox.h"         // vkInfo() and friends
#include "html_urls.h"

#include <qdir.h>
#include <qfileinfo.h>

/* Example:
addOpt( 
  LEAK_CHECK,                            int opt_key
  Option::ARG_BOOL,                      Option::ArgType arg_type
  Option::CHECK,                         Option::WidgetType w_type
  "memcheck",                            QString cfg_group     // cfgGroup()
  '\0',                                  QChar   short_flag
  "leak-check",                          QString long_flag     // cfgKey()
  "<no|summary|full>",                   QString flag_desc     // cmd-line
  "no|summary|full",                     QString poss_vals
  "summary",                             QString default_val
  "Search for memory leaks at exit",     QString shelp         // gui
  "search for memory leaks at exit?",    QString lhelp         // cmd-line
  "manual.html#leak-check" );            QString url
*/


/* class VkObject ------------------------------------------------------ */
VkObject::~VkObject() 
{ 
  optList.setAutoDelete( true );
  optList.clear();
  optList.setAutoDelete( false );
}


VkObject::VkObject( ObjectId id, const QString& capt, const QString& txt,
                    const QKeySequence& key, bool is_tool ) 
{
  objectId  = id;
  caption   = capt;
  accelText = txt;
  is_Tool   = is_tool;
  accel_Key = key;
}


void VkObject::addOpt( 
     int opt_key,  Option::ArgType arg_type, Option::WidgetType w_type, 
     QString cfg_group, QChar short_flag,         QString long_flag, 
     QString flag_desc, QString poss_vals,        QString default_val, 
     QString shelp,     QString lhelp,            const char* url )
{
  optList.append( new Option( opt_key,   arg_type,   w_type, 
                              cfg_group, short_flag, long_flag, 
                              flag_desc, poss_vals,  default_val, 
                              shelp,     lhelp,      url ) );
}


Option * VkObject::findOption( int optkey )
{
  Option* opt = NULL;
  for ( opt=optList.first(); opt; opt=optList.next() ) {
    if ( opt->key == optkey )
      break;
  }
  vk_assert( opt != NULL );

  return opt;
}


/* writes the argval for this option to vkConfig. */
void VkObject::writeOptionToConfig( Option* opt, QString argval )
{
  opt->modified = true;
  vkConfig->wrEntry( argval, opt->longFlag, opt->cfgGroup() );
}


/* called from VkConfig::mkConfigFile() when we need to create the
   valkyrierc file for the very first time. */
QString VkObject::configEntries()
{
  QString cfgEntry = "\n[" + name() + "]\n";
  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {

    /* skip these entirely */
    if ( opt->key == Valkyrie::HELP_OPT ) continue;

    cfgEntry += opt->longFlag + "=" + opt->defaultValue + "\n";
  }

  return cfgEntry;
}

/* called from VkConfig::modFlags() when a toolview needs to know what
   flags to set || pass to a process. */
QStringList VkObject::modifiedFlags()
{
  QStringList modFlags;
  QString defVal, cfgVal;

  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {

    /* config already has this 'cos we need it to be first in the queue */
    if ( opt->key == Valgrind::TOOL )
      continue;

    defVal = opt->defaultValue;     /* opt holds the default */
    cfgVal = vkConfig->rdEntry( opt->longFlag, name() );

    if ( defVal != cfgVal )
      modFlags << "--" + opt->longFlag + "=" + cfgVal;

  }

  return modFlags;
}


/* Determine the total no. of option structs required by counting the
   no. of entries in the optList.  Note: num_options = optList+1
   because we need a NULL option entry to terminate each option array. */
vkPoptOption * VkObject::poptOpts()
{
  //printOptList();
  int num_options  = optList.count() + 1;
  size_t nbytes    = sizeof(vkPoptOption) * num_options;
  vkPoptOption * vkopts = (vkPoptOption*)malloc(nbytes);

  int idx = 0;
  Option *opt;
  QString tmp;
  for ( opt = optList.first(); opt; opt = optList.next() ) {
    vk_assert( opt != NULL );
    vk_assert( vkopts != NULL );

    /* not a popt option */
    if ( opt->argType != Option::NOT_POPT ) {
      vkopts[idx].argType    = opt->argType;
      vkopts[idx].shortFlag  = opt->shortFlag.latin1();
      vkopts[idx].longFlag   = opt->longFlag.latin1();
      vkopts[idx].arg        = 0;
      if ( opt->key == Valkyrie::HELP_OPT ) {
        vkopts[idx].val      = opt->shortFlag.latin1();
      } else {
        vkopts[idx].val      = opt->key;
      }
      if ( opt->defaultValue.isEmpty() ) {
        vkopts[idx].helptxt  = opt->longHelp;
      } else {
        tmp = opt->longHelp + " [" + opt->defaultValue + "]";
        vkopts[idx].helptxt  = vk_strdup( tmp.ascii() );
      }
      vkopts[idx].helpdesc   = opt->flagDescrip.ascii();
      idx++;
    }
  }
  /* we need a null entry to terminate the table */
  vkopts[idx].argType   = 0;
  vkopts[idx].shortFlag = '\0';
  vkopts[idx].longFlag  = NULL;
  vkopts[idx].arg       = 0;
  vkopts[idx].val       = 0;
  vkopts[idx].helptxt   = NULL;
  vkopts[idx].helpdesc  = NULL;

  return vkopts;
}


/* Tread carefully: when creating the struct in poptOpts(), the
   helpdesc field was only malloc'd if there was a default value.
   So we check to see if the original string and the copy are the
   same.  If yes, leave it alone, as it only contains a ptr to the
   original string.  */
void VkObject::freePoptOpts( vkPoptOption * vkopts )
{
  int num_options  = optList.count();
  if ( num_options == PARSED_OK )
    return;

  int xx = 0;
  for ( int yy=0; yy<num_options; yy++ ) {

    /* beware: don't use strcmp on empty strings :( */
    if ( optList.at(yy)->argType != Option::NOT_POPT && 
         !optList.at(yy)->longHelp.isEmpty()  ) {
      const char* tmp = optList.at(yy)->longHelp.ascii();
      if ( !vk_strcmp( vkopts[xx].helptxt, tmp ) ) {
        //printf("tmp: -->%s<--\n", tmp);
        //printf("\thelptext: -->%s<--\n", vkopts[xx].helptxt );
        vk_str_free( vkopts[xx].helptxt );
      }
      xx++;
    }
  }

  vk_free( vkopts );
  vkopts = NULL;
}


/* static fn, called from parseCmdArgs() in parse_cmd_args.cpp */
int VkObject::checkArg(int optid, const char* argval, bool use_gui/*=false*/)
{
  VkObject * vkObj = NULL;
  if ( optid >= Valkyrie::HELP_OPT && optid <= Valkyrie::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "valkyrie" );
  else if ( optid >= Valgrind::FIRST_CMD_OPT && optid <= Valgrind::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "valgrind" );
  else if ( optid >= Memcheck::FIRST_CMD_OPT && optid <= Memcheck::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "memcheck" );
  else if ( optid >= Cachegrind::FIRST_CMD_OPT && optid <= Cachegrind::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "cachegrind" );
  else if ( optid >= Massif::FIRST_CMD_OPT && optid <= Massif::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "massif" );
  else
    vk_assert_never_reached();

  vk_assert( vkObj != NULL );
  return vkObj->checkOptArg( optid, argval, use_gui );
}


/* class Valkyrie ------------------------------------------------------ 
   The key value of HELP_OPT is used:
   - to exclude the option from configEntries;
   - to determine when a shortFlag is needed for popt.
   The key value of NOT_POPT is used:
   - to exclude the option from popt options.  */
Valkyrie::Valkyrie()
  : VkObject( VALKYRIE, "Valkyrie", "Valkyrie", Qt::Key_unknown, false ) 
{
  runMode = modeNotSet;

  addOpt( HELP_OPT,    Option::ARG_NONE,   Option::NONE,
          "",          'h',                "help", 
          "",          "",                 "", 
          "",          "show this help message and exit", urlNone );
  addOpt( HELP_OPT,    Option::ARG_NONE,   Option::NONE,
          "",          'v',                "version", 
          "",          "",                 "", 
          "",          "display version information and exit", urlNone );
  addOpt( HELP_OPT,    Option::ARG_NONE,   Option::NONE,
          "",          'V',                "valgrind-opts", 
          "",          "",                 "", 
          "",          "Show valgrind options too, and exit", urlNone );
  addOpt( TOOLTIP,     Option::NOT_POPT,   Option::CHECK, 
          "valkyrie",  '\0',               "show-tooltips", 
          "",          "true|false",       "true", 
          "Show tooltips",      "",          urlNone );
  addOpt( PALETTE,     Option::NOT_POPT,   Option::CHECK, 
          "valkyrie",  '\0',               "use-vk-palette", 
          "",          "true|false",       "true", 
          "Use valkyrie's palette",   "",  urlNone );
  addOpt( ICONTXT,     Option::NOT_POPT,   Option::CHECK, 
          "valkyrie",  '\0',               "show-butt-text", 
          "",          "true|false",       "true", 
          "Show toolbar text labels",  "", urlNone );
  addOpt( FONT_SYSTEM,  Option::NOT_POPT,   Option::CHECK,
          "valkyrie",   '\0',               "use-system-font", 
          "",          "true|false",       "true", 
          "Use default system font",  "",  urlNone );
  addOpt( FONT_USER,   Option::NOT_POPT,   Option::LEDIT, 
          "valkyrie",  '\0',               "user-font", 
          "",          "",  "Helvetica [Cronyx],12,-1,5,50,0,0,0,0,0", 
          "",          "",                 urlNone );
  addOpt( SRC_EDITOR,  Option::NOT_POPT,   Option::LEDIT, 
          "valkyrie",  '\0',               "src-editor", 
          "",          "",                 "/usr/bin/emacs", 
          "Src Editor:",   "",     urlNone );
  addOpt( SRC_LINES,   Option::NOT_POPT,   Option::SPINBOX, 
          "valkyrie",  '\0',               "src-lines",
          "",          "1|10",             "2", 
          "Extra lines shown above/below the target line:", "",
          urlNone );
  /* path to valgrind executable (found by configure) */
  addOpt( VG_EXEC,     Option::NOT_POPT,   Option::LEDIT, 
          "valkyrie",  '\0',               "vg-exec",
          "",          "",                 "",
          "Valgrind:", "",                 urlNone );
  /* path to a supp files dir. this is initially found by configure,
     but can be changed later via valkyrie's Option page to point to
     another suppression files dir */
  addOpt( VG_SUPPS_DIR,   Option::NOT_POPT,   Option::LEDIT, 
          "valkyrie",  '\0',               "vg-supps-dir",
          "",          "",                 "",
          "Supps.Dir:",  "",                 urlNone );
  addOpt( BINARY,      Option::NOT_POPT,   Option::LEDIT,
          "valkyrie",  '\0',               "binary", 
          "",          "",                 "", 
          "Binary:",   "",                 urlNone );
  addOpt( BIN_FLAGS,   Option::NOT_POPT,   Option::LEDIT,
          "valkyrie",  '\0',               "binary-flags", 
          "",          "",                 "", 
          "Binary flags:", "",             urlNone );
  addOpt( USE_GUI,     Option::ARG_BOOL,   Option::NONE,
          "valkyrie",  '\0',               "gui", 
          "<yes|no>",  "yes|no",           "yes",
          "xxxxxxx",
          "use the graphical interface",
          urlNone );
  addOpt( VIEW_LOG,    Option::ARG_STRING, Option::LEDIT, 
          "valkyrie",  '\0',               "view-log", 
          "<file>",    "",                 "",
          "View logfile:", "parse and view a valgrind logfile",
          urlNone );
  addOpt( MERGE_LOGS,  Option::ARG_STRING, Option::NONE, 
          "valkyrie",  '\0',               "merge", 
          "<loglist>", "",                 "",
          "View logfiles:", "merge multiple logfiles, discarding duplicate errors",
          urlNone );
}


int Valkyrie::checkOptArg( int optid, const char* argval, 
													 bool use_gui/*=false*/ )
{ 
  int errval = PARSED_OK;
  QString argVal( argval );
  Option* opt = findOption( optid );
  switch ( optid ) {

  /* these options are _only_ set via the gui, and are either (a)
     limited to a set of available values, or (b) have already been
     checked, so no need to re-check them. */
    case TOOLTIP:
    case PALETTE:
    case ICONTXT:
    case FONT_SYSTEM:
    case FONT_USER:
    case SRC_LINES:
    case VG_SUPPS_DIR:
      return errval;
      break;

    case SRC_EDITOR:
    case VG_EXEC:
      argVal = opt->binaryCheck( &errval, argval );
      break;

    case USE_GUI:
      opt->isValidArg( &errval, argval );
      break;

    case VIEW_LOG:
      if ( runMode != modeNotSet ) {
        errval = PERROR_BADOPERATION;
      } else {
        argVal = opt->fileCheck( &errval, argval, true, false );
				if ( errval == PARSED_OK ) {
					/* check the file format is xml */
					bool ok = opt->xmlFormatCheck( &errval, argVal );
					if ( ok && errval == PARSED_OK ) {
						runMode = modeParseLog;
					}
				}
      } break;

    /* Expects a single file which contains a list of
       logfiles-to-be-merged, each on a separate line, with a minimum
       of two logfiles. Validating each file is done at merge-time, as
       we will then skip any files which we can't read. */
    case MERGE_LOGS:
      if ( runMode != modeNotSet ) {
        errval = PERROR_BADOPERATION;
      } else {
				argVal = opt->fileCheck( &errval, argval, true, false );
				if ( errval == PARSED_OK )
					runMode = modeMergeLogs;
			} break;

    case BINARY:
      if ( runMode != modeNotSet ) {
        errval = PERROR_BADOPERATION;
      } else {
        argVal = opt->binaryCheck( &errval, argval );
				if ( errval == PARSED_OK ) 
					runMode = modeParseOutput;
      } break;

    case BIN_FLAGS:
      argVal = argval;
      break;
  }

  /* if this option has been called from the cmd-line, save its value
     in valkyrierc if it has passed the checks. */
  if ( errval == PARSED_OK && use_gui == false ) {
    writeOptionToConfig( opt, argVal );
  }

  return errval; 
}


/* called from VkConfig::modFlags() 
   see if valkyrie was told what to do:
   - on the cmd-line
   - via a tool
   - via the gui options page */
QStringList Valkyrie::modifiedFlags()
{
  Option* opt;
  QStringList modFlags;
  QString defVal, cfgVal;

  switch ( runMode ) {

    case modeNotSet:
      break;

    case modeParseLog: {
      opt    = findOption( VIEW_LOG );
      defVal = opt->defaultValue;
      cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
      if ( defVal != cfgVal )
        modFlags << "--" + opt->longFlag + "=" + cfgVal;
    } break;

    case modeParseOutput: {
      opt    = findOption( BINARY );
      defVal = opt->defaultValue;
      cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
      if ( defVal != cfgVal ) {
        modFlags << cfgVal;
        /* see if there were any flags given for the binary */
        opt = findOption( BIN_FLAGS );
        defVal = opt->defaultValue;
        cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
        if ( defVal != cfgVal )
          modFlags << cfgVal;
      }
    } break;

		case modeMergeLogs:
			opt    = findOption( MERGE_LOGS );
			defVal = opt->defaultValue;
			cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
      if ( defVal != cfgVal )
        modFlags << "--" + opt->longFlag + "=" + cfgVal;
			break;
  }

  return modFlags;
}





/* class Valgrind ------------------------------------------------------ */
Valgrind::Valgrind()
  : VkObject( VALGRIND, "Valgrind", "Valgrind", Qt::Key_unknown, false ) 
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
          "<number>",  "1|2",              "2",
          "Log to file descriptor:",
          "log messages to file descriptor (1=stdout, 2=stderr)",  
					/* TODO: subclass QProcess so we can use fd > 2 */
          urlNone );
  addOpt( LOG_PID,     Option::ARG_STRING, Option::LEDIT, 
          "valgrind",  '\0',               "log-file", 
          "<file>",    "",                 "",
          "Log to <file>.pid:",
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
          "<number>",  "0|1|2",            "0",
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
  vk_assert( optid >= FIRST_CMD_OPT && optid <= LAST_CMD_OPT );

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
      argVal = opt->binaryCheck( &errval, tmp );
      argVal += tmp.right( tmp.length() - pos+1 );
      printf("db_command: %s\n", argVal.ascii() );
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

      case LOG_PID:        // log to file.pid
      case LOG_FILE:       // log to file.name
      case LOG_FD:         // ?? log to file descriptor = 2
      case LOG_SOCKET:     // ?? log to socket ipaddr:port
        break;

      case XML_OUTPUT:      // the sine qua non
        //modFlags << "--" + opt->longFlag + "=" + vkConfig->rdEntry( opt->longFlag, name() );
        modFlags << "--" + opt->cfgKey() + "=" 
                         + vkConfig->rdEntry( opt->cfgKey(), name() );
        break;

      default:
        defVal = opt->defaultValue;
        //RM: cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
        cfgVal = vkConfig->rdEntry( opt->cfgKey(), name() );
        if ( defVal != cfgVal ) {
          //modFlags << "--" + opt->longFlag + "=" + cfgVal;
          modFlags << "--" + opt->cfgKey() + "=" + cfgVal;
        }
        break;
    }
  }

  return modFlags;
}




/* class Memcheck ------------------------------------------------------ */
Memcheck::Memcheck() 
  : VkObject( MEMCHECK, "Memcheck", "&Memcheck", Qt::SHIFT+Qt::Key_M ) 
{
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



/* valkyrie hijacks any log-to-file flags; these are not passed to
   valgrind, but are used after parsing has finished to save to.
   FIXME: not sure yet how to handle LOG_FD and LOG_SOCKET */
QStringList Memcheck::modifiedFlags()
{
  QStringList modFlags;
  QString defVal, cfgVal;

  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {
  
    switch ( opt->key ) {

      default:
        defVal = opt->defaultValue;
        cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
        if ( defVal != cfgVal )
          modFlags << "--" + opt->longFlag + "=" + cfgVal;
        break;
    }
  }

  return modFlags;
}



/* class Cachegrind ---------------------------------------------------- */
Cachegrind::Cachegrind()
  : VkObject( CACHEGRIND, "Cachegrind", "&Cachegrind", Qt::SHIFT+Qt::Key_C ) 
{ 
  /* cachegrind flags */
  addOpt( I1_CACHE,     Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "I1",
          "<size,assoc,line_size>", "",     "0,0,0",
          "I1 cache configuration:",
          "set I1 cache manually",
          urlCachegrind::Cacheopts );
  addOpt( D1_CACHE,     Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "D1", 
          "<size,assoc,line_size>", "",     "0,0,0",
          "D1 cache configuration:",        
          "Set D1 cache manually",
          urlCachegrind::Cacheopts );
  addOpt( L2_CACHE,     Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "L2",
          "<size,assoc,line_size>", "",     "0,0,0",
          "L2 cache configuration:",
          "set L2 cache manually",
          urlCachegrind::Cacheopts );
  /* cachegrind annotate script flags */
  addOpt( PID_FILE,     Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", '\0',               "pid", 
          "<file.pid>", "",                 "",
          "File to read:",
          "Which <cachegrind.out.pid> file to read (required)",
          urlCachegrind::Pid );
  addOpt( SORT,         Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", '\0',               "sort", 
          "<A,B,C>",    "",                 "event column order",
          "Sort columns by:",
          "sort columns by events A,B,C",
          urlCachegrind::Sort );
  addOpt( SHOW,         Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", '\0',               "show",
          "<A,B,C>",    "",                 "all",
          "Show figures for events:",
          "only show figures for events A,B,C",
          urlCachegrind::Show );
  addOpt( THRESH,       Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "threshold", 
          "<%>",        "0|100",            "99",
          "Threshold percentage:",
          "percentage of counts (of primary sort event) we are interested in",
          urlCachegrind::Threshold );
  addOpt( AUTO,         Option::ARG_BOOL,   Option::CHECK, 
          "cachegrind", '\0',               "auto",
          "<yes|no>",   "yes|no",           "no",
          "Automatically annotate all relevant source files",
          "Annotate all source files containing functions that helped reach the event count threshold",
          urlCachegrind::Auto );
  addOpt( CONTEXT,      Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "context",
          "<number>",   "0|50",             "8",
          "Number of context lines to print:",
          "print <number> lines of context before and after annotated lines",
          urlCachegrind::Context );
  addOpt( INCLUDE,      Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", 'I',                "include", 
          "<dir1,dir2>", "",                "",
          "Source dirs:",
          "List of directories to search for source files",
          urlCachegrind::Include );
}


int Cachegrind::checkOptArg( int optid, const char* argval, 
														 bool use_gui/*=false*/ )
{ 
  vk_assert( optid >= FIRST_CMD_OPT && optid <= LAST_CMD_OPT );

  int errval = PARSED_OK;
  QString argVal( argval );
  Option* opt = findOption( optid );

  switch ( optid ) {

  case I1_CACHE:
  case D1_CACHE:
  case L2_CACHE: {
    QStringList aList = QStringList::split( ",", argVal );
    if ( aList.count() != 3 ) {
      errval = PERROR_BADARG;
    }  else {
      QStringList possvals;  
      possvals << "4" << "1048576";  // << "8"<< "8192";
      /* 1st number */
      opt->setPossibleValues( possvals );
      if ( opt->isPowerOfTwo( &errval, aList[0].latin1() ) ) {
        /* 2nd number */
        possvals[0] = "0";
        possvals[1] = "8";
        opt->setPossibleValues( possvals );
        if ( opt->isValidArg( &errval, aList[1].latin1() ) ) {
          /* 3rd number */
          possvals[0] = "4";
          possvals[1] = "8192";
          opt->setPossibleValues( possvals );
          opt->isPowerOfTwo( &errval, aList[2].latin1() );
        }
      }
    }
  } break;

  case PID_FILE:
    argVal = opt->fileCheck( &errval, argVal, true, false );
    break;

  case THRESH:
  case AUTO:
  case CONTEXT:
    opt->isValidArg( &errval, argVal );
    break;

  /* not sure how to handle these just yet :( */
  case SHOW:
  case SORT:
    break;

  case INCLUDE: {
    QStringList aList = QStringList::split( ",", argVal );
    for ( unsigned int i=0; i<aList.count(); i++ ) {
      QString tmp    = aList[i].simplifyWhiteSpace();
      QString srcdir = opt->dirCheck( &errval, tmp.ascii(), true, false );
      if ( errval == PARSED_OK ) {
        aList[i] = srcdir;
      } else {
        if ( !use_gui ) {
          vkPrint( "Parse error [--include=%s] : invalid dir path\n"
                   "The offending directory is '%s'", 
                   argVal.ascii(), tmp.ascii() );
        }
        errval = PERROR_DEFAULT;
        break;
      } 
      argVal = aList.join( "," );
    } 
  } break;

  }

  /* if this option has been called from the cmd-line, save its value
     in valkyrierc if it has passed the checks. */
  if ( errval == PARSED_OK && use_gui == false ) {
    writeOptionToConfig( opt, argVal );
  }

  return errval;
}



/* class Massif -------------------------------------------------------- */
Massif::Massif()
  : VkObject( MASSIF, "Massif", "Ma&ssif", Qt::SHIFT+Qt::Key_S ) 
{
  addOpt( HEAP,        Option::ARG_BOOL,   Option::CHECK,
          "massif",    '\0',               "heap",
          "<yes|no>",  "yes|no",           "yes",
          "Profile heap blocks",
          "profile heap blocks",           
          urlMassif::Heap );
  addOpt(  HEAP_ADMIN, Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "heap-admin", 
          "<number>",  "4|15",             "8",
          "Average admin bytes per heap block:",
          "average admin bytes per heap block", 
           urlMassif::HeapAdmin );
  addOpt( STACKS,      Option::ARG_BOOL,   Option::CHECK,
          "massif",    '\0',               "stacks",
          "<yes|no>",  "yes|no",           "yes",
          "Profile stack(s)",
          "profile stack(s)",
          urlMassif::Stacks );
  addOpt(  DEPTH,      Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "depth", 
          "<number>",  "1|20",             "3",
          "Depth of contexts:",
          "depth of contexts", 
           urlMassif::Depth );
  addOpt( ALLOC_FN,    Option::ARG_STRING, Option::LEDIT, 
          "massif",    '\0',               "alloc-fn", 
          "<name>",    "",               "empty",
          "Specify <fn> as an alloc function:",
          "specify <fn> as an alloc function", 
          urlMassif::AllocFn );
  addOpt( FORMAT,      Option::ARG_STRING,   Option::COMBO,  
          "massif",    '\0',                 "format",
          "<text|html|xml>", "text|html|xml", "text",
          "Format of textual output:",
          "format of textual output",
          urlMassif::Format );
  addOpt( ALIGNMENT,  Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "alignment", 
          "<number>",  "8|1048576",        "8",
          "Minimum alignment of allocations:",
          "set minimum alignment of allocations", 
           urlVgCore::Alignment );
}


int Massif::checkOptArg( int optid, const char* argval, 
												 bool use_gui/*=false*/ )
{
  vk_assert( optid >= FIRST_CMD_OPT && optid <= LAST_CMD_OPT );

  int errval = PARSED_OK;
  Option * opt = findOption( optid );

  switch ( optid ) {

    /* how on earth can this be checked ? */
    case ALLOC_FN:
      break;

    case HEAP:
    case HEAP_ADMIN:
    case STACKS:
    case DEPTH:
    case FORMAT:
      opt->isValidArg( &errval, argval );
      break;

    /* check it's really a number ... then if is a power of two */
    case ALIGNMENT:
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

