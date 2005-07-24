/* ---------------------------------------------------------------------
 * Declarations of url strings                             html_urls.cpp
 * Placed in one file in an endeavour to minimise 
 * what is probably going to be a maintenance nightmare.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "html_urls.h"


/* placeholder so we can grep to find missing links */
const char* urlNone = "";


/* MainWindow ---------------------------------------------------------- */
namespace urlValkyrie { 
  const char* FlagsButton = "context_help.html#flags_button";
  const char* Dummy       = "dummy.html#dummy";
}


/* Valgrind core ------------------------------------------------------- */
namespace urlVgCore { 
  const char* Alignment = "manual-core.flags.html#alignment";
}


/* Memcheck ------------------------------------------------------------ */
namespace urlMemcheck { 
  const char* Partial   = "mc-manual.flags.html#partial";
  const char* Freelist  = "mc-manual.flags.html#freelist";
  const char* Leakcheck = "mc-manual.flags.html#leakcheck";
  const char* Leakres   = "mc-manual.flags.html#leakres";
  const char* Showreach = "mc-manual.flags.html#showreach";
  const char* gcc296    = "mc-manual.flags.html#gcc296";
  const char* Strlen    = "mc-manual.flags.html#strlen";
}


/* Cachegrind ---------------------------------------------------------- */
namespace urlCachegrind { 
  const char* Cacheopts = "cg-manual.profile.html#cg-manual.cgopts";
  const char* Pid       = "cg-manual.annopts.html#pid";
  const char* Sort      = "cg-manual.annopts.html#sort";
  const char* Show      = "cg-manual.annopts.html#show";
  const char* Threshold = "cg-manual.annopts.html#threshold";
  const char* Auto      = "cg-manual.annopts.html#auto";
  const char* Context   = "cg-manual.annopts.html#context";
  const char* Include   = "cg-manual.annopts.html#include";
}


/* Massif -------------------------------------------------------------- */
namespace urlMassif { 
  const char* Heap      = "ms-manual.options.html#heap";
  const char* HeapAdmin = "ms-manual.options.html#heap-admin";
  const char* Stacks    = "ms-manual.options.html#stacks";
  const char* Depth     = "ms-manual.options.html#depth";
  const char* AllocFn   = "ms-manual.options.html#alloc-fn";
  const char* Format    = "ms-manual.options.html#format";
}

