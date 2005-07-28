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


/* Valkyrie ------------------------------------------------------------ */
namespace urlValkyrie { 

  /* Mainwindow */
  const char* fileMenu     = "mainwindow.html#file_menu";
  const char* toolMenu     = "mainwindow.html#tools_menu";
  const char* optionsMenu  = "mainwindow.html#options_menu";
  const char* runButton    = "mainwindow.html#run_button";
  const char* stopButton   = "mainwindow.html#stop_button";
  const char* helpMenu     = "mainwindow.html#help_menu";
  const char* statusMsg    = "mainwindow.html#status_msg";
  const char* tviewButtons = "mainwindow.html#tview_buttons";
  const char* flagsButton  = "mainwindow.html#flags_widget";

  /* valkyrie's options page */
  const char* toolTips     = "options_valkyrie.html#tool_tips";
  const char* toolLabels   = "options_valkyrie.html#tool_label";
  const char* userFont     = "options_valkyrie.html#user_font";
  const char* palette      = "options_valkyrie.html#palette";
  const char* srcLines     = "options_valkyrie.html#src_lines";
  const char* srcEditor    = "options_valkyrie.html#src_editor";
  const char* binary       = "options_valkyrie.html#binary";
  const char* binFlags     = "options_valkyrie.html#bin_flags";
  const char* vgDir        = "options_valkyrie.html#valgrind_dir";
  const char* suppDir      = "options_valkyrie.html#supps_dir";

  /* valgrind`s options page: tab Suppressions */
  const char* coreTab  = "options_valgrind.html#core_tab";
  const char* errorTab = "options_valgrind.html#error_tab";
  const char* suppsTab = "options_valgrind.html#supps_tab";

  /* MemcheckView toolbuttons */
  const char* openAllButton = "";
  const char* openOneButton = "";
  const char* srcPathButton = "";
  const char* openLogButton = "";
  const char* saveLogButton = "";
  const char* suppEdButton  = "";
}


/* Valgrind core ------------------------------------------------------- */
namespace urlVgCore { 
  /* valgrind's options page: tab Core */
  const char* mainTool   = "manual-core.flags.html#tool_name";
  const char* verbosity  = "manual-core.flags.html#verbosity";
  const char* xmlOutput  = "manual-core.flags.html#xml_output";
  const char* xmlComment = "manual-core.flags.html#xml_user_comment";
  const char* traceChild = "manual-core.flags.html#trace_children";
  const char* trackFds   = "manual-core.flags.html#track_fds";
  const char* timeStamp  = "manual-core.flags.html#time_stamp";
  const char* freeGlibc    = "manual-core.flags.html#free_glibc";
  const char* pointerCheck = "manual-core.flags.html#pointer_check";
  const char* showEmWarns  = "manual-core.flags.html#show_emwarns";
  const char* smcSupport   = "manual-core.flags.html#smc_support";
  const char* weirdHacks   = "manual-core.flags.html#weird_hacks";
  /* valgrind's options page: tab Error Reporting */
  const char* genSuppressions = "manual-core.flags.html#gen_supps";
  const char* autoDemangle    = "manual-core.flags.html#auto_demangle";
  const char* errorLimit      = "manual-core.flags.html#error_limit";
  const char* stackTraces     = "manual-core.flags.html#stack_traces";
  const char* numCallers      = "manual-core.flags.html#num_callers";
  const char* maxSFrames      = "manual-core.flags.html#max_frames";
  const char* startDebugger   = "manual-core.flags.html#attach_debugger";
  const char* whichDebugger   = "manual-core.flags.html#which_debugger";
  const char* inputFd         = "manual-core.flags.html#input_fd";
  const char* logToFd         = "manual-core.flags.html#log2fd";
  const char* logToFilePid    = "manual-core.flags.html#log2file_pid";
  const char* logToFile       = "manual-core.flags.html#log2file";
  const char* logToSocket     = "manual-core.flags.html#log2socket";
  const char* logFileQual     = "manual-core.flags.html#log2file_qualifier";
  /* only used by Memcheck and Massif */
  const char* Alignment       = "manual-core.flags.html#alignment";
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

