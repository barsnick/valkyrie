/****************************************************************************
** Valgrind implementation
**  - Valgrind-specific: options / flags / functionality
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

#include <QDir>

#include "help/help_urls.h"
#include "objects/valgrind_object.h"
#include "options/valgrind_options_page.h"  // createVkOptionsPage()

#include "utils/vk_config.h"

//#include "config.h"
#include "objects/helgrind_object.h"
#include "objects/memcheck_object.h"
//#include "cachegrind_object.h"
//#include "massif_object.h"
#include "options/vk_option.h"
#include "utils/vk_utils.h"          // vk_assert, VK_DEBUG, etc.


//#include <ctype.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>


/***************************************************************************/
/*!
    Constructs a Valkyrie object
*/
Valgrind::Valgrind()
   : VkObject( "valgrind" )
{
   initToolObjects();
   
   setupOptions();
}


Valgrind::~Valgrind()
{
   // clean up tool list
   while ( !toolObjList.isEmpty() ) {
      delete toolObjList.takeFirst();
   }
}



/*!
   Setup the options for this object.

   Note: These opts should be kept in exactly the same order as valgrind
   outputs them, as it makes keeping up-to-date a lot easier.
*/
void Valgrind::setupOptions()
{
   // ------------------------------------------------------------
   // tool
   options.addOpt(
      VALGRIND::TOOL,
      this->objectName(),
      "tool",
      '\0',
      "<name>",
      "memcheck|cachegrind|massif",
      "memcheck",
      "Main tool:",
      "use the Valgrind tool named <name>.  Available tools are: memcheck, cachegrind, massif",
      urlVgCore::mainTool,
      VkOPT::ARG_STRING,
      VkOPT::WDG_COMBO
   );
   
   
   // ------------------------------------------------------------
   // common options relevant to all tools
   options.addOpt(
      VALGRIND::VERBOSITY,
      this->objectName(),
      "verbosity",
      '\0',
      "<0..4>",
      "0|4",
      "1",
      "Verbosity level:",
      "Be more verbose, include counts of errors",
      urlVgCore::verbosity,
      VkOPT::ARG_UINT,
      VkOPT::WDG_SPINBOX
   );
   
   options.addOpt(
      VALGRIND::TRACE_CH,
      this->objectName(),
      "trace-children",
      '\0',
      "<yes|no>",
      "yes|no",
      "no",
      "Trace child processes: ",
      "Valgrind-ise child processes (follow execve)?",
      urlVgCore::traceChild,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::SILENT_CH,
      this->objectName(),
      "child-silent-after-fork",
      '\0',
      "<yes|no>",
      "yes|no",
      "yes",   /* currently necessary for clean XML output */
      "Omit child output between fork and exec: ",
      "omit child output between fork & exec?",
      urlVgCore::silentChild,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::TRACK_FDS,
      this->objectName(),
      "track-fds",
      '\0',
      "<yes|no>",
      "yes|no",
      "no",
      "Track open file descriptors:",
      "track open file descriptors?",
      urlVgCore::trackFds,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::TIME_STAMP,
      this->objectName(),
      "time-stamp",
      '\0',
      "<yes|no>",
      "yes|no",
      "no",
      "Add timestamps to log messages:",
      "add timestamps to log messages?",
      urlVgCore::timeStamp,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::LOG_FD,
      this->objectName(),
      "log-fd",
      '\0',
      "<1..1024>",
      "1|1023",
      "2",
      "Log to file descriptor:",
      "log messages to file descriptor (1=stdout, 2=stderr)",
      urlVgCore::logToFd,
      VkOPT::ARG_UINT,
      VkOPT::WDG_SPINBOX
   );
   
   options.addOpt(
      VALGRIND::LOG_FILE,
      this->objectName(),
      "log-file",
      '\0',
      "<file>",
      "",
      "",
      "Log to file:",
      "log messages to <file>",
      urlVgCore::logToFile,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALGRIND::LOG_SOCKET,
      this->objectName(),
      "log-socket",
      '\0',
      "<ipaddr:port>",
      "",
      "",
      "Log to socket:",
      "log messages to socket ipaddr:port",
      urlVgCore::logToSocket,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LEDIT
   );
   
   
   // ------------------------------------------------------------
   // uncommon options relevant to all tools
   options.addOpt(
      VALGRIND::RUN_LIBC,
      this->objectName(),
      "run-libc-freeres",
      '\0',
      "<yes|no>",
      "yes|no",
      "yes",
      "Free glibc memory at exit:",
      "Free up glibc memory at exit?",
      urlVgCore::freeGlibc,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::SIM_HINTS,
      this->objectName(),
      "sim-hints",
      '\0',
      "<hint1,hint2,...>",
      "none|lax-ioctls|enable-outer",
      "none",
      "Simulation Hints:",
      "Slightly modify the simulated behaviour. Recognised hints are: lax-ioctls, enable-outer. Use with caution!",
      urlVgCore::simHints,
      VkOPT::ARG_STRING,
      VkOPT::WDG_COMBO
   );
   
   options.addOpt(
      VALGRIND::KERN_VAR,
      this->objectName(),
      "kernel-variant",
      '\0',
      "<variant1,...>",
      "none|bproc",
      "none",
      "Kernel Variants:",
      "Handle non-standard kernel variants. Recognised variants are: bproc. Use with caution!",
      urlVgCore::kernelVariant,
      VkOPT::ARG_STRING,
      VkOPT::WDG_COMBO
   );
   
   options.addOpt(
      VALGRIND::EM_WARNS,
      this->objectName(),
      "show-emwarns",
      '\0',
      "<yes|no>",
      "yes|no",
      "no",
      "Show warnings about emulation limits:",
      "show warnings about emulation limits?",
      urlVgCore::showEmWarns,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::SMC_CHECK,
      this->objectName(),
      "smc-check",
      '\0',
      "<none|stack|all>",
      "none|stack|all",
      "stack",
      "Where to check for self-modifying code",
      "checks for self-modifying code: none, only for code on the stack, or all",
      urlVgCore::smcSupport,
      VkOPT::ARG_STRING,
      VkOPT::WDG_COMBO
   );
   
   
   // ------------------------------------------------------------
   // options relevant to error-reporting tools
   options.addOpt(
      VALGRIND::XML_OUTPUT,
      this->objectName(),
      "xml",
      '\0',
      "<yes|no>",
      "yes|no",
      "yes",
      "Output in xml format:",
      "all output is in XML",
      urlVgCore::xmlOutput,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::XML_COMMENT,
      this->objectName(),
      "xml-user-comment",
      '\0',
      "<str>",
      "",
      "",
      "Insert verbatim in xml output",
      "copy <str> verbatim to XML output",
      urlVgCore::xmlComment,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALGRIND::DEMANGLE,
      this->objectName(),
      "demangle",
      '\0',
      "<yes|no>",
      "yes|no",
      "yes",
      "Automatically demangle C++ names",
      "automatically demangle C++ names?",
      urlVgCore::autoDemangle,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::NUM_CALLERS,
      this->objectName(),
      "num-callers",
      '\0',
      "<1..50>",
      "1|50",
      "12",
      "Number of stack trace callers:",
      "show <num> callers in stack traces",
      urlVgCore::numCallers,
      VkOPT::ARG_UINT,
      VkOPT::WDG_SPINBOX
   );
   
   options.addOpt(
      VALGRIND::ERROR_LIMIT,
      this->objectName(),
      "error-limit",
      '\0',
      "<yes|no>",
      "yes|no",
      "yes",
      "Limit the number of errors shown",
      "Stop showing new errors if too many?",
      urlVgCore::errorLimit,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::SHOW_BELOW,
      this->objectName(),
      "show-below-main",
      '\0',
      "<yes|no>",
      "yes|no",
      "no",
      "Continue stack traces below main()",
      "continue stack traces below main()",
      urlVgCore::stackTraces,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   
   // ------------------------------------------------------------
   /* list of selected suppression files */
   options.addOpt(
      VALGRIND::SUPPS_SEL,
      this->objectName(),
      "suppressions",
      '\0',
      "<file1,...>",
      "",
      "",
      "Error-suppression file(s):",
      "suppress errors described in suppressions file(s)",
      urlValkyrie::suppsTab,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LISTBOX
   );
   
   
   // ------------------------------------------------------------
   // ...
   options.addOpt(
      VALGRIND::GEN_SUPP,
      this->objectName(),
      "gen-suppressions",
      '\0',
      "<no|all>",
      "no|all",
      "all",     // Vg default is 'no', but we (generally) want this on.
      "Print suppressions for errors",
      "print suppressions for errors?",
      urlVgCore::genSuppressions,
      VkOPT::ARG_STRING,
      VkOPT::WDG_COMBO
   );
   
   options.addOpt(
      VALGRIND::DB_ATTACH,
      this->objectName(),
      "db-attach",
      '\0',
      "<yes|no>",
      "yes|no",
      "no",
      "Start debugger on error detection",
      "start debugger when errors detected?",
      urlVgCore::startDebugger,
      VkOPT::ARG_BOOL,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALGRIND::DB_COMMAND,
      this->objectName(),
      "db-command",
      '\0',
      "<command>",
      "",
      "/usr/bin/gdb -nw %f %p",
      "Debugger:",
      "command to start debugger",
      urlVgCore::whichDebugger,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALGRIND::INPUT_FD,
      this->objectName(),
      "input-fd",
      '\0',
      "<0..1024>",
      "0|1023",
      "0",
      "Input file descriptor:",
      
      "File descriptor for (db) input (0=stdin, 1=stdout, 2=stderr)",
      urlVgCore::inputFd,
      VkOPT::ARG_UINT,
      VkOPT::WDG_SPINBOX
   );
   
   options.addOpt(
      VALGRIND::MAX_SFRAME,
      this->objectName(),
      "max-stackframe",
      '\0',
      "<number>",
      "0|2000000",
      "2000000",
      "Stack switch on SP changes at:",
      "assume stack switch for stack pointer changes larger than <number> bytes",
      urlVgCore::maxSFrames,
      VkOPT::ARG_UINT,
      VkOPT::WDG_SPINBOX
   );
}



/*!
  Check \a argval for the option given by \a optid, updating if necessary.
*/
int Valgrind::checkOptArg( int optid, QString& argval )
{
   vk_assert( optid >= 0 && optid < VALGRIND::NUM_OPTS );
   
   VkOption* opt = getOption( optid );
   int errval = PARSED_OK;

   QChar sep = VkCfg::sepChar();
   
   switch ( (VALGRIND::vgOptId)optid ) {
   
   case VALGRIND::TOOL:
      // Note: gui option disabled, so only reaches here from cmdline
      errval = PERROR_BADOPT;
      vkPrintErr( "Option disabled '--%s'", qPrintable( opt->longFlag ) );
      vkPrintErr( " - Valkyrie currently only supports Memcheck." );
      break;
      
   case VALGRIND::SIM_HINTS:
   case VALGRIND::RUN_LIBC:
   case VALGRIND::NUM_CALLERS:
   case VALGRIND::DEMANGLE:
   case VALGRIND::SHOW_BELOW:
   case VALGRIND::MAX_SFRAME:
   case VALGRIND::SMC_CHECK:
   case VALGRIND::GEN_SUPP:
      opt->isValidArg( &errval, argval );
      break;

   case VALGRIND::VERBOSITY:
   case VALGRIND::TRACK_FDS:
   case VALGRIND::TIME_STAMP:
   case VALGRIND::EM_WARNS:
   case VALGRIND::ERROR_LIMIT:
   case VALGRIND::DB_COMMAND:
   case VALGRIND::DB_ATTACH:
      // Note: gui option disabled, so only reaches here from cmdline
      errval = PERROR_BADOPT;
      vkPrintErr( "Option disabled '--%s'", qPrintable( opt->longFlag ) );
      vkPrintErr( " - Valgrind presets these options for XML output." );
      vkPrintErr( " - See valgrind/docs/internals/xml_output.txt." );
      break;
      
   case VALGRIND::XML_COMMENT:
      // don't wan't xml in comment: escape '<','&',etc
      argval = escapeEntities( argval );
      break;
      
   case VALGRIND::SUPPS_SEL: {
      QStringList files = argval.split( sep, QString::SkipEmptyParts );

      QStringList::iterator it = files.begin();
      for ( ; it != files.end(); ++it ) {
         // check file ok and has at least R permissions
         *it = fileCheck( &errval, *it, true );
         if ( errval != PARSED_OK ) {
            return errval;
         }
         // TODO: ? check valid suppression files
      }

      // check for duplicates
      if ( files.removeDuplicates() > 0 ) {
         return PERROR_DUPLICATE;
      }

      argval = files.join( sep );
   }
   break;
   
   case VALGRIND::TRACE_CH: {
#if 0
      // TODO
      if ( opt->isValidArg( &errval, argval ) ) {
         if ( argval == "yes" ) {
            if ( vkCfgProj->rdBool( "db-attach", "valgrind" ) ) {
               errval = PERROR_DB_CONFLICT;
            }
         }
      }
      
#else
      /* Disabled for now - can't deal with the multiple xml files this generates */
      /* Note: Also disabled in ValgrindOptionsPage() */
      errval = PERROR_BADOPT;
      //         vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      //         vkPrintErr(" - Valkyrie can't yet handle the multiple xml files this generates.");
#endif
   }
   break;
   
   case VALGRIND::SILENT_CH: {
      /* Disabled for now - output between fork and exec is confusing for the XML output */
      /* Note: Also disabled in ValgrindOptionsPage() */
      errval = PERROR_BADOPT;
      //         vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      //         vkPrintErr(" - Necessary, to get clean XML output from Valgrind.");
   }
   break;
   
   case VALGRIND::XML_OUTPUT:
      /* Note: gui option disabled, so only reaches here from cmdline */
      errval = PERROR_BADOPT;
      //      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      //      vkPrintErr(" - Valkyrie always requires xml output from Valgrind.");
      break;
      
      
#if 0 // TODO: Fix Valgrind to allow gdb attaching with XML output
   case VALGRIND::DB_COMMAND: {   /* gdb -nw %f %p */
      int pos = argval.find( ' ' );
      QString tmp = argval.left( pos );
      argval = binaryCheck( &errval, tmp );
      argval += tmp.right( tmp.length() - pos + 1 );
      // vkPrint("db_command: %s", argval.latin1() );
   }
   break;
   
   /* check for conflict with --trace-children */
   case VALGRIND::DB_ATTACH:
   
      if ( opt->isValidArg( &errval, argval ) ) {
         if ( argval == "yes" ) {
            if ( vkCfgProj->rdBool( "trace-children", "valgrind" ) ) {
               errval = PERROR_DB_CONFLICT;
            }
         }
      }
      
      break;
#endif
      
   case VALGRIND::KERN_VAR:
      break;
      
   // Logging options:
   // All tool objects use a logging option to generate/read Vg output.
   // All logging options are therefore ignored.
   case VALGRIND::LOG_FILE:
   case VALGRIND::LOG_FD:
   case VALGRIND::LOG_SOCKET:
      // Note: gui options disabled, so only reaches here from cmdline
      errval = PERROR_BADOPT;
      vkPrintErr( "Option disabled '--%s'", qPrintable( opt->longFlag ) );
      vkPrintErr( " - Valkyrie sets its own logging options to gather data from Valgrind." );
      break;
      
   // Not yet implemented
   case VALGRIND::INPUT_FD:
      // Note: gui option disabled, so only reaches here from cmdline
      errval = PERROR_BADOPT;
      //      vkPrintErr("Option disabled '--%s'", opt->m_longFlag.latin1());
      //      vkPrintErr(" - Not yet implemented.");
      break;
      
   default:
      vk_assert_never_reached();
   }
   
   return errval;
}


/*!
  Return list of non-default flags, including those for tool_obj.

  Note: Valkyrie hijacks any log-to-file flags; these are not passed to
  valgrind, but are used after parsing has finished to save to.
*/
QStringList Valgrind::getVgFlags( ToolObject* tool_obj )
{
   QStringList modFlags;
   
   foreach( VkOption * opt, options.getOptionHash() ) {
      QString defVal = opt->dfltValue.toString();
      QString cfgVal = vkCfgProj->value( opt->configKey() ).toString();
      
      switch (( VALGRIND::vgOptId )opt->optid ) {
      
         // we never want these included
      case VALGRIND::TOOL:            /* tool set by valkyrie */
         // ignore these opts
         break;
         
         // only error-reporting tools have suppressions
      case VALGRIND::SUPPS_SEL: {
         if ( tool_obj->objectName() == "memcheck" ) {
            // we need '--suppressions=' before each and every filename
            QString optEntry = vkCfgProj->value( opt->configKey() ).toString();
            QStringList files = optEntry.split( ",", QString::SkipEmptyParts );
            
            for ( int i = 0; i < files.count(); i++ ) {
               modFlags << "--" + opt->longFlag + "=" + files[i];
            }
         }
         else {
            // ignore opt
         }
      } break;
      
      
      // for memcheck we always need xml=yes
      case VALGRIND::XML_OUTPUT:
      
         if ( tool_obj->objectName() == "memcheck" ) {
//TODO: where do we want this?  is also added in memcheck_object...
//            modFlags << "--" + opt->longFlag + "=yes";
         }
         else {
            if ( defVal != cfgVal ) {
               modFlags << "--" + opt->longFlag + "=" + cfgVal;
            }
         }
         
         break;
         
         // for memcheck we need this enabled to keep the xml clean
      case VALGRIND::SILENT_CH:
      
         if ( tool_obj->objectName() == "memcheck" ) {
            modFlags << "--" + opt->longFlag + "=yes";
         }
         else {
            if ( defVal != cfgVal ) {
               modFlags << "--" + opt->longFlag + "=" + cfgVal;
            }
         }
         
         break;
         
         // memcheck presets/ignores these options for xml output
         //  - ignore these opts
         //  - see valgrind/docs/internals/xml_output.txt
         // Note: gui options not disabled: other tools use these options
      case VALGRIND::VERBOSITY:
      case VALGRIND::TRACK_FDS:
      case VALGRIND::TIME_STAMP:
      case VALGRIND::EM_WARNS:
      case VALGRIND::ERROR_LIMIT:
      case VALGRIND::DB_ATTACH:
      case VALGRIND::DB_COMMAND:
      
         if ( tool_obj->objectName() == "memcheck" ) {
            // ignore these opts
         }
         else {
            if ( defVal != cfgVal ) {
               modFlags << "--" + opt->longFlag + "=" + cfgVal;
            }
         }
         
         break;

         // suppressions
      case VALGRIND::GEN_SUPP:
         // always add, irrespective of cfgVal / dfltVal
         modFlags << "--" + opt->longFlag + "=" + cfgVal;
         break;
         
         // all tools use an internal logging option,
         // so logging options should not be used
      case VALGRIND::LOG_FILE:
      case VALGRIND::LOG_FD:
      case VALGRIND::LOG_SOCKET:
         // ignore these opts
         break;
         
         // default: add --option=value
      default:
      
         if ( defVal != cfgVal ) {
            modFlags << "--" + opt->longFlag + "=" + cfgVal;
         }
         
         break;
      }
   }
   
   // Collect non-default tool flags:
   modFlags += tool_obj->getVgFlags();
   
   return modFlags;
}


/*!
  Register tools
*/
void Valgrind::initToolObjects()
{
   toolObjList.append( new Memcheck() );
   toolObjList.append( new Helgrind() );
   
   // TODO: I need another lifetime!
   //   toolObjList.append( new Cachegrind() );
   //   toolObjList.append( new Massif    () );
}




/*!
  ToolObject access
  - Returns a list of all ptrs to ToolObjects
*/
ToolObjList Valgrind::getToolObjList()
{
   return toolObjList;
}

#if 0
/* Returns a ToolObject's objectId based on its name */
int Valgrind::getToolObjId( const QString& name )
{
   return toolObj( name )->objId();
}
#endif

/* Returns a ToolObject based on its objectId */
ToolObject* Valgrind::getToolObj( VGTOOL::ToolID tid )
{
   //   vkPrint("Valgrind::toolObj( int tid=%d )", tid);
   vk_assert( tid > VGTOOL::ID_NULL );
   
   ToolObject* tool = NULL;
   
   for ( int i = 0; i < toolObjList.size(); i++ ) {
      tool = toolObjList.at( i );
      
      if ( tool->getToolId() == tid ) {
         break;
      }
   }
   
   vk_assert( tool != NULL );
   vk_assert( tool->getToolId() == tid );
   return tool;
}

#if 0
/* Returns a ToolObject based on its name */
ToolObject* Valgrind::getToolObj( const QString& name )
{
   ToolObject* tool = NULL;
   
   for ( tool = toolObjList.first(); tool; tool = toolObjList.next() ) {
      if ( tool->name() == name ) {
         break;
      }
   }
   
   vk_assert( tool != NULL );
   vk_assert( tool->name() == name );
   return tool;
}
getToolId
#endif



VkOptionsPage* Valgrind::createVkOptionsPage()
{
   return ( VkOptionsPage* )new ValgrindOptionsPage( this );
}
