/* --------------------------------------------------------------------- 
 * Definition of class Valgrind                        valgrind_object.h
 * Valgrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VALGRIND_OBJECT_H
#define __VALGRIND_OBJECT_H


#include "vk_objects.h"
#include "valgrind_options_page.h"


/* class Valgrind ------------------------------------------------------ */
class Valgrind : public VkObject
{
public:
  Valgrind();
  ~Valgrind();

  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum vgOpts {
    TOOL,
    /* common options relevant to all tools */
    VERBOSITY,
    XML_OUTPUT,   /* this may apply to more tools later */
    TRACE_CH,   TRACK_FDS,  TIME_STAMP,
    /* uncommon options relevant to all tools */
    RUN_LIBC,  WEIRD, PTR_CHECK, ELAN_HACKS, EM_WARNS,
    /* options relevant to error-reporting tools */
    LOG_FD,   LOG_PID,     LOG_FILE,    LOG_SOCKET,
    DEMANGLE, NUM_CALLERS, ERROR_LIMIT, SHOW_BELOW,
    SUPPS_SEL,  /* the currently selected suppression(s) */
    SUPPS_ALL,  /* list of all supp. files ever found, inc. paths */
    SUPPS_DEF,  /* as above, but never gets changed -> defaults */
    GEN_SUPP, DB_ATTACH, DB_COMMAND, INPUT_FD, MAX_SFRAME,
    LAST_CMD_OPT = MAX_SFRAME
  };

  OptionsPage* createOptionsPage( OptionsWindow* parent ) {
    return (OptionsPage*)new ValgrindOptionsPage( parent, this );
  }
};


#endif
