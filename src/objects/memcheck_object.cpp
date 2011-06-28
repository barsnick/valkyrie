/****************************************************************************
** Memcheck implementation
**  - Memcheck-specific options / flags / fns
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

#include "help/help_urls.h"
#include "objects/memcheck_object.h"

#include "options/memcheck_options_page.h"
#include "toolview/memcheckview.h"

#include "utils/vk_utils.h"


/*!
  class Memcheck
*/
Memcheck::Memcheck()
   : ToolObject( "memcheck", VGTOOL::ID_MEMCHECK )
{
   setupOptions();
}


Memcheck::~Memcheck()
{
}


/*!
   Setup the options for this object.

   Note: These opts should be kept in exactly the same order as valgrind
   outputs them, as it makes keeping up-to-date a lot easier.
*/
void Memcheck::setupOptions()
{
   // ------------------------------------------------------------
   // leak-check
   options.addOpt(
      MEMCHECK::LEAK_CHECK, this->objectName(), "leak-check",
      '\0',
      "<no|summary|full>", "no|summary|full", "full",
      "Search for memory leaks at exit:",
      "search for memory leaks at exit?",
      urlMemcheck::Leakcheck, VkOPT::ARG_STRING, VkOPT::WDG_COMBO
   );
   
   // ------------------------------------------------------------
   // leak-resolution
   options.addOpt(
      MEMCHECK::LEAK_RES, this->objectName(), "leak-resolution",
      '\0',
      "<low|med|high>", "low|med|high", "low",
      "Degree of backtrace merging:",
      "how much backtrace merging in leak check",
      urlMemcheck::Leakres, VkOPT::ARG_STRING, VkOPT::WDG_COMBO
   );
   
   // ------------------------------------------------------------
   // leak-resolution
   options.addOpt(
      MEMCHECK::SHOW_REACH, this->objectName(), "show-reachable",
      '\0',
      "<yes|no>", "yes|no", "no",
      "Show reachable blocks in leak check",
      "show reachable blocks in leak check?",
      urlMemcheck::Showreach, VkOPT::ARG_BOOL, VkOPT::WDG_CHECK
   );
   
   // ------------------------------------------------------------
   // undef-value-errors
   //options.addOpt(
   //      MEMCHECK::UNDEF_VAL, this->objectName(), "undef-value-errors",
   //      '\0',
   //      "<yes|no>", "yes|no", "yes",
   //      "Check for undefined value errors",
   //      "check for undefined value errors?",
   //      urlMemcheck::UndefVal, VkOPT::ARG_BOOL, VkOPT::WDG_CHECK
   //      );
   
   // ------------------------------------------------------------
   // track-origins
   options.addOpt(
      MEMCHECK::TRACK_ORI, this->objectName(), "track-origins",
      '\0',
      "<yes|no>", "yes|no", "no",
      "Show the origins of uninitialised values",
      "show the origins of uninitialised values?",
      urlMemcheck::TrackOri, VkOPT::ARG_BOOL, VkOPT::WDG_CHECK
   );
   
   // ------------------------------------------------------------
   // partial-loads-ok
   options.addOpt(
      MEMCHECK::PARTIAL, this->objectName(), "partial-loads-ok",
      '\0',
      "<yes|no>", "yes|no", "no",
      "Ignore errors on partially invalid addresses",
      "too hard to explain here; see manual",
      urlMemcheck::Partial, VkOPT::ARG_BOOL, VkOPT::WDG_CHECK
   );
   
   // ------------------------------------------------------------
   // freelist-vol
   options.addOpt(
      MEMCHECK::FREELIST, this->objectName(), "freelist-vol",
      '\0',
      "<number>", "0|1000000000", "10000000",
      "Volume of freed blocks queue:",
      "volume of freed blocks queue",
      urlMemcheck::Freelist, VkOPT::ARG_UINT, VkOPT::WDG_LEDIT
   );
   
   // ------------------------------------------------------------
   // workaround-gcc296-bugs
   options.addOpt(
      MEMCHECK::GCC_296, this->objectName(), "workaround-gcc296-bugs",
      '\0',
      "<yes|no>", "yes|no", "no",
      "Work around gcc-296 bugs",
      "self explanatory",
      urlMemcheck::gcc296, VkOPT::ARG_BOOL, VkOPT::WDG_CHECK
   );
   
   // ------------------------------------------------------------
   // alignment
   options.addOpt(
      MEMCHECK::ALIGNMENT, this->objectName(), "alignment",
      '\0',
      "<number>", "8|1048576", "8",
      "Minimum alignment of allocations",
      "set minimum alignment of allocations",
      urlVgCore::Alignment, VkOPT::ARG_PWR2, VkOPT::WDG_SPINBOX
   );
}



/* check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages -------------------- */
int Memcheck::checkOptArg( int optid, QString& argval )
{
   vk_assert( optid >= 0 && optid < MEMCHECK::NUM_OPTS );
   
   int errval = PARSED_OK;

   VkOption* opt = getOption( optid );
   
   switch ( (MEMCHECK::mcOptId)optid ) {
   case MEMCHECK::PARTIAL:
   case MEMCHECK::FREELIST:
   case MEMCHECK::LEAK_RES:
   case MEMCHECK::SHOW_REACH:
      //case MEMCHECK::UNDEF_VAL:
   case MEMCHECK::TRACK_ORI:
   case MEMCHECK::GCC_296:
   case MEMCHECK::ALIGNMENT:
      opt->isValidArg( &errval, argval );
      break;
      
      // when using xml output from valgrind, this option is preset to
      // 'full' by valgrind, so this option should not be used.
   case MEMCHECK::LEAK_CHECK:
      // Note: gui options disabled, so only reaches here from cmdline
      errval = PERROR_BADOPT;
      vkPrintErr( "Option disabled '--%s'", qPrintable( opt->longFlag ) );
      vkPrintErr( " - Memcheck presets this option to 'full' when generating the required xml output." );
      vkPrintErr( " - See valgrind/docs/internals/xml_output.txt." );
      break;
      
   default:
      vk_assert_never_reached();
   }
   
   return errval;
}


/*!
   Creates this tool's ToolView window
*/
ToolView* Memcheck::createToolView( QWidget* parent )
{
   return (ToolView*) new MemcheckView( parent );
}


/*!
  Creates option page for this tool.
*/
VkOptionsPage* Memcheck::createVkOptionsPage()
{
   return (VkOptionsPage*) new MemcheckOptionsPage( this );
}


/*!
  outputs a message to the status bar.
*/
void Memcheck::statusMsg( QString msg )
{
   emit message( "Memcheck: " + msg );
}
