/* --------------------------------------------------------------------- 
 * Definition of class Valgrind                        valgrind_object.h
 * Valgrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VALGRIND_OBJECT_H
#define __VALGRIND_OBJECT_H


#include "vk_objects.h"
#include "valgrind_options_page.h"
#include "tool_object.h"


/* class Valgrind ------------------------------------------------------ */
class Valgrind : public VkObject
{
public:
  Valgrind();
  ~Valgrind();

  /* returns a list of non-default flags to pass to valgrind */
  QStringList modifiedFlags( const ToolObject* tool_obj );

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum vgOpts {
    TOOL,           // --tool
    /* common options relevant to all tools */
    VERBOSITY,      // --verbosity
    TRACE_CH,       // --trace-children
    TRACK_FDS,      // --track-fds
    TIME_STAMP,     // --time-stamp
    LOG_FD,         // --log-fd
    LOG_PID,        // --log-file
    LOG_FILE,       // --log-file-exactly
    LOG_QUAL,       // --log-file-qualifier
    LOG_SOCKET,     // --log-socket
    /* uncommon options relevant to all tools */
    RUN_LIBC,       // --run-libc-freeres
    SIM_HINTS,      // --sim-hints
    KERN_VAR,       // --kernel-variant
    EM_WARNS,       // --show-emwarns
    SMC_CHECK,      // --smc-check
    /* options relevant to error-reporting tools */
    XML_OUTPUT,     // --xml
    XML_COMMENT,    // -- xml-user-comment
    DEMANGLE,       // --demangle
    NUM_CALLERS,    // --num-callers
    ERROR_LIMIT,    // --error-limit
    SHOW_BELOW,     // --show-below-main
    /* suppressions hackery */
    SUPPS_SEL,      /* the currently selected suppression(s) */
    SUPPS_ALL,      /* list of all supp. files ever found, inc. paths */
    SUPPS_DEF,      // --suppressions 
    GEN_SUPP,       // --gen-suppressions
    DB_ATTACH,      // --db-attach
    DB_COMMAND,     // --db-command
    INPUT_FD,       // --input-fd
    MAX_SFRAME,     // --max-stackframe
    LAST_CMD_OPT = MAX_SFRAME
  };

  OptionsPage* createOptionsPage( OptionsWindow* parent ) {
    return (OptionsPage*)new ValgrindOptionsPage( parent, this );
  }
};


#endif
