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


/* not every flag/option has context-sensitive help */
const char* urlNone  = "";
/* placeholder so we can grep to find missing links */
const char* urlDummy = "dummy.html#dummy";


/* Valkyrie ------------------------------------------------------------ */
namespace urlValkyrie { 

	/* Mainwindow */
  const char* fileMenu     = "mainwindow.html#file_menu";
  const char* toolMenu     = "mainwindow.html#tool_menu";
  const char* optionsMenu  = "mainwindow.html#options_menu";
  const char* runButton    = "mainwindow.html#run_button";
  const char* stopButton   = "mainwindow.html#stop_button";
  const char* statusMsg    = "mainwindow.html#status_msg";
  const char* tviewButtons = "mainwindow.html#tview_buttons";

	/* valkyrie object herself */
  const char* FlagsButton = "context_help.html#flags_button";
  const char* toolTips    = "context_help.html#tool_tips";
  const char* toolLabels  = "context_help.html#tool_label";
  const char* userFont    = "context_help.html#user_font";
  const char* palette     = "context_help.html#palette";
  const char* srcLines    = "context_help.html#src_lines";
  const char* srcEditor   = "context_help.html#src_editor";
  const char* binary      = "context_help.html#binary";
  const char* binFlags    = "context_help.html#bin_flags";
  const char* vgDir       = "context_help.html#valgrind_dir";
  const char* suppDir     = "context_help.html#supps_dir";
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

