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


/* class Valgrind ------------------------------------------------------ */
class Valgrind : public VkObject
{
public:
  Valgrind();
  ~Valgrind();

  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum vgOpts {
    FIRST_CMD_OPT = 16,
    TOOL          = FIRST_CMD_OPT,
    /* common options relevant to all tools */
    VERBOSITY     = 17,
    XML_OUTPUT    = 18,   /* this may apply to more tools later */
    TRACE_CH      = 19,
    TRACK_FDS     = 20,
    TIME_STAMP    = 21,
    /* uncommon options relevant to all tools */
    RUN_LIBC      = 22,
    WEIRD         = 23,
    PTR_CHECK     = 24,
    ELAN_HACKS    = 25,
    EM_WARNS      = 26,
    /* options relevant to error-reporting tools */
    LOG_FD        = 27,
    LOG_PID       = 28,
    LOG_FILE      = 29,
    LOG_SOCKET    = 30,
    DEMANGLE      = 31,
    NUM_CALLERS   = 32,
    ERROR_LIMIT   = 33,
    SHOW_BELOW    = 34,
    SUPPS_SEL     = 35,  /* the currently selected suppression(s) */
    SUPPS_ALL     = 36,  /* list of all supp. files ever found, inc. paths */
    SUPPS_DEF     = 37,  /* as above, but never gets changed -> defaults */
    GEN_SUPP      = 38,
    DB_ATTACH     = 39,
    DB_COMMAND    = 40,
    INPUT_FD      = 41,
    MAX_SFRAME    = 42,
    LAST_CMD_OPT = MAX_SFRAME
  };

};


#endif
