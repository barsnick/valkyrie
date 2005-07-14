/* --------------------------------------------------------------------- 
 * Implementation of class Valkyrie                  valkyrie_object.cpp
 * Valkyrie-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "valkyrie_object.h"
#include "valgrind_object.h"
#include "tool_object.h"

#include "vk_config.h"
#include "vk_messages.h"
#include "vk_utils.h"          // vk_assert(), vk_strcmp(), vkPrint()
#include "html_urls.h"

#include <qapplication.h>


/* class Valkyrie ------------------------------------------------------ 
   The key value of HELP_OPT is used:
   - to exclude the option from configEntries;
   - to determine when a shortFlag is needed for popt.
   The key value of NOT_POPT is used:
   - to exclude the option from popt options.  
*/
Valkyrie:: ~Valkyrie() { }


Valkyrie::Valkyrie()
  : VkObject( VALKYRIE, "Valkyrie", "Valkyrie", Qt::Key_unknown, false ) 
{
  /* init vars */
  runMode = modeNotSet;

  valgrind   = 0;


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
      argVal = binaryCheck( &errval, argval );
      break;

    case USE_GUI:
      opt->isValidArg( &errval, argval );
      break;

    case VIEW_LOG:
      argVal = fileCheck( &errval, argval, true, false );
      if ( errval == PARSED_OK ) {
        /* check the file format is xml */
        bool ok = xmlFormatCheck( &errval, argVal );
        if ( ok && errval == PARSED_OK ) {
          runMode = modeParseLog;
        }
      }
      break;

    /* expects a single file which contains a list of
       logfiles-to-be-merged, each on a separate line, with a minimum
       of two logfiles. Validating each file is done at merge-time, as
       we will then skip any files which we can't read. */
    case MERGE_LOGS:
      argVal = fileCheck( &errval, argval, true, false );
      if ( errval == PARSED_OK )
        runMode = modeMergeLogs;
      break;

    case BINARY:
      argVal = binaryCheck( &errval, argval );
      if ( errval == PARSED_OK ) 
        runMode = modeParseOutput;
      break;

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


/* set usingGui + get ptrs to all tools. 
   if usingGui=true, create a list of tool-only objects.
   if usingGui=false, connect tool objects up to quit slot */
void Valkyrie::init()
{
  usingGui = vkConfig->rdBool( "gui","valkyrie" );

  valgrind   = (Valgrind*)vkConfig->vkObject( "valgrind" );

  ToolList toolList = vkConfig->vkToolList();

  for ( ToolObject* tool=toolList.first(); tool; tool=toolList.next() ) {
    if ( !usingGui ) {
      connect( tool, SIGNAL( finished() ), this, SLOT( quit() ) );
    }
    connect( tool, SIGNAL( fatal() ), this, SLOT( quit() ) );
  }
}


/* slot: called by a tools when it has finished() its task */
void Valkyrie::quit()
{ exit(0); }

/* called from MainWindow::run() */
void Valkyrie::setRunMode( Valkyrie::RunMode rm )
{ runMode = rm; }


/* called from Valkyrie::currentFlags() 
   see if valkyrie was told what to do:
   - on the cmd-line
   - via a tool
   - via the gui options page */
QStringList Valkyrie::modifiedFlags()
{
  Option* opt;
  QStringList modFlags;
  QString cfgVal;

  switch ( runMode ) {

    case modeParseLog: {
      opt    = findOption( VIEW_LOG );
      cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
      if ( cfgVal != opt->defaultValue )
        modFlags << "--" + opt->longFlag + "=" + cfgVal;
    } break;

    case modeNotSet:
    case modeParseOutput: {
      opt    = findOption( BINARY );
      cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
      if ( cfgVal != opt->defaultValue ) {
        modFlags << cfgVal;
        /* see if there were any flags given for the binary */
        opt = findOption( BIN_FLAGS );
        cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
        if ( cfgVal != opt->defaultValue )
          modFlags << cfgVal;
      }
    } break;

    case modeMergeLogs:
      opt    = findOption( MERGE_LOGS );
      cfgVal = vkConfig->rdEntry( opt->longFlag, name() );
      if ( cfgVal != opt->defaultValue )
        modFlags << "--" + opt->longFlag + "=" + cfgVal;
      break;
  }

  return modFlags;
}


/* called by:
   (a) MainWindow::showFlagsWidget() so user can see 
       exactly what is being fed to valgrind on the cmd-line;
   (b) by Valkyrie::runTool().  
   returns a '\n' separated list of current relevant flags */
QString Valkyrie::currentFlags( ToolObject* tool_obj )
{
  flags.clear();

  switch ( runMode ) {

    /* if nothing was specified on the cmd-line, or we're about to run
       valgrind, return the flags which would be used if the current
       tool were run */
    case modeNotSet:
    case modeParseOutput:
      /* get the /path/to/valgrind */
      flags << vkConfig->rdEntry( "vg-exec","valkyrie");
       /* set the tool we are using */
#if 1
      flags << "--tool=" + tool_obj->name();
#else
      flags << "--tool=none";
#endif
      /* check if any valgrind core opts have been modified.
         'modified' means 'set to anything other than default' */
      flags += valgrind->modifiedFlags();
      /* now get flags which have been specified / modified for this
         tool _only_ */
      flags += tool_obj->modifiedFlags();
      /* finally, check for valkyrie-specific flags */
      flags += modifiedFlags();
      break;

    case modeParseLog:
    case modeMergeLogs:
      flags += modifiedFlags();
      break;
  }

	/* ## hack alert: unfortunately, we have to pass each arg to
		 QProcess as a separate string, and this includes any binary
		 flags; but for display purposes in the flagWidget, concat the
		 binary together with its flags 'cos its prettier. */
	QString flags2 = "";
	int num_flags = flags.count();
	for ( int i=0; i<num_flags-2; i++ )
		flags2 += flags[i] +  "\n";
	if ( !vkConfig->rdEntry("binary-flags", "valkyrie").isEmpty() ) {
		flags2 += flags[num_flags-2] + " " + flags[num_flags-1];
	} else {
		flags2 += flags[num_flags-1];
	}

	return flags2;
}


/* called from MainWin when user clicks stopButton */
void Valkyrie::stopTool( ToolObject* activeTool/*=0*/ )
{
  if ( activeTool == 0 ) {
    activeTool = vkConfig->tool();
    vk_assert( activeTool != 0 );
  }

  bool success = activeTool->stop( runMode );

  vk_assert( success );  // TODO: what to do if couldn't stop?
}


/* If called from MainWin, then activeTool is set.
   If called from main(), then activeTool == 0, 
   so find out which tool is supposed to be running.  */
bool Valkyrie::runTool( ToolObject* activeTool/*=0*/ )
{
  if ( activeTool == 0 ) {
    activeTool = vkConfig->tool();
    vk_assert( activeTool != 0 );
  }

  bool success = true;

  /* find out what we are supposed to be doing */
  switch ( runMode ) {

  case modeNotSet:       /* no flags given on cmd-line */
    if ( !usingGui ) {
      vkInfo( 0, "Error", "</p>You haven't told me what to do.</p>" );
      quit();
    } break;
    
  /* run valgrind --tool=tool_name, with all flags */
  case modeParseOutput:
    currentFlags( activeTool ); /* ensure flags up-to-date */
    success = activeTool->run( flags );
    break;

  /* let tool decide what to do for other runModes */
  default:
    success = activeTool->start( runMode );
  }


  /* we've done what we were asked to do, so relax */
  //runMode = modeNotSet;

  return success;
}


