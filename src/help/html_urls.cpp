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
   const char* menuBar      = "mainwindow.html#menu_bar";
   const char* fileMenu     = "mainwindow.html#file_menu";
   const char* toolMenu     = "mainwindow.html#tools_menu";
   const char* optionsMenu  = "mainwindow.html#options_menu";
   const char* runButton    = "mainwindow.html#run_button";
   const char* stopButton   = "mainwindow.html#stop_button";
   const char* helpMenu     = "mainwindow.html#help_menu";
   const char* statusMsg    = "mainwindow.status_msg.html";
   const char* flagsWidget  = "mainwindow.flags_widget.html";

   /* valkyrie's options page */
   const char* optsPage     = "options_dialog.html#valkyrie";
   const char* toolTips     = "options_dialog.html#tool_tips";
   const char* toolLabels   = "options_dialog.html#tool_label";
   const char* userFont     = "options_dialog.html#user_font";
   const char* palette      = "options_dialog.html#palette";
   const char* srcLines     = "options_dialog.html#src_lines";
   const char* srcEditor    = "options_dialog.html#src_editor";
   const char* binary       = "options_dialog.html#binary";
   const char* binFlags     = "options_dialog.html#bin_flags";
   const char* vgDir        = "options_dialog.html#valgrind_dir";
   const char* suppDir      = "options_dialog.html#supps_dir";

   /* valgrind`s options page: tab Suppressions */
   const char* coreTab  = "options_valgrind.html#core_tab";
   const char* errorTab = "options_valgrind.html#error_tab";
   const char* suppsTab = "options_valgrind.html#supps_tab";

   /* MemcheckView toolbuttons */
   const char* openAllButton = "";
   const char* openOneButton = "";
   const char* srcPathButton = "";
   const char* loadLogButton = "";
   const char* mrgLogButton  = "";
   const char* saveLogButton = "";
   const char* suppEdButton  = "";
}


/* Valgrind core ------------------------------------------------------- */
namespace urlVgCore { 
   /* valgrind's options page: tab Core */
   const char* mainTool      = "manual-core.html#tool_name";
   const char* verbosity     = "manual-core.html#verbosity";
   const char* traceChild    = "manual-core.html#trace_children";
   const char* trackFds      = "manual-core.html#track_fds";
   const char* timeStamp     = "manual-core.html#time_stamp";
   const char* xmlOutput     = "manual-core.html#xml_output";
   const char* xmlComment    = "manual-core.html#xml_user_comment";
   const char* freeGlibc     = "manual-core.html#free_glibc";
   const char* showEmWarns   = "manual-core.html#show_emwarns";
   const char* smcSupport    = "manual-core.html#smc_support";
   const char* simHints      = "manual-core.html#simulation_hints";
   const char* kernelVariant = "manual-core.html#kernel_variant";

   /* valgrind's options page: tab Error Reporting */
   const char* logToFd         = "manual-core.html#log2fd";
   const char* logToFilePid    = "manual-core.html#log2file_pid";
   const char* logToFile       = "manual-core.html#log2file";
   const char* logFileQual     = "manual-core.html#log2file_qualifier";
   const char* logToSocket     = "manual-core.html#log2socket";
   const char* autoDemangle    = "manual-core.html#auto_demangle";
   const char* numCallers      = "manual-core.html#num_callers";
   const char* errorLimit      = "manual-core.html#error_limit";
   const char* stackTraces     = "manual-core.html#stack_traces";
   const char* genSuppressions = "manual-core.html#gen_supps";
   const char* startDebugger   = "manual-core.html#attach_debugger";
   const char* whichDebugger   = "manual-core.html#which_debugger";
   const char* inputFd         = "manual-core.html#input_fd";
   const char* maxSFrames      = "manual-core.html#max_frames";

   /* only used by Memcheck and Massif */
   const char* Alignment       = "manual-core.html#alignment";
}


/* Memcheck ------------------------------------------------------------ */
namespace urlMemcheck { 
   const char* Leakcheck = "mc-manual.html#leakcheck";
   const char* Showreach = "mc-manual.html#showreach";
   const char* Leakres   = "mc-manual.html#leakres";
   const char* Freelist  = "mc-manual.html#freelist";
   const char* gcc296    = "mc-manual.html#gcc296";
   const char* Partial   = "mc-manual.html#partial";
}


/* Cachegrind ---------------------------------------------------------- */
namespace urlCachegrind { 
   const char* Cacheopts = "cg-manual.html#cg-manual.cgopts";
   const char* Pid       = "cg-manual.html#pid";
   const char* Sort      = "cg-manual.html#sort";
   const char* Show      = "cg-manual.html#show";
   const char* Threshold = "cg-manual.html#threshold";
   const char* Auto      = "cg-manual.html#auto";
   const char* Context   = "cg-manual.html#context";
   const char* Include   = "cg-manual.html#include";
}


/* Massif -------------------------------------------------------------- */
namespace urlMassif { 
   const char* Heap      = "ms-manual.html#heap";
   const char* HeapAdmin = "ms-manual.html#heap-admin";
   const char* Stacks    = "ms-manual.html#stacks";
   const char* Depth     = "ms-manual.html#depth";
   const char* AllocFn   = "ms-manual.html#alloc-fn";
   const char* Format    = "ms-manual.html#format";
}

