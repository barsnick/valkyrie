/* ---------------------------------------------------------------------
 * Declarations of url strings                             html_urls.cpp
 * Placed in one file in an endeavour to minimise 
 * what is probably going to be a maintenance nightmare.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
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

   /* options dialog */
   const char* optsDlg      = "options_dialog.html";

   /* valkyrie's options page */
   const char* optsPage     = "options_dialog.html#valkyrie";
   const char* toolTips     = "options_dialog.html#tool_tips";
   const char* toolLabels   = "options_dialog.html#tool_label";
   const char* browser      = "options_dialog.html#browser";
   const char* logDir       = "options_dialog.html#log_dir";
   const char* workingDir   = "options_dialog.html#working_dir";
   const char* userFontGen  = "options_dialog.html#user_font_general";
   const char* userFontTool = "options_dialog.html#user_font_tool";
   const char* palette      = "options_dialog.html#palette";
   const char* srcLines     = "options_dialog.html#src_lines";
   const char* srcEditor    = "options_dialog.html#src_editor";
   const char* binary       = "options_dialog.html#binary";
   const char* binFlags     = "options_dialog.html#bin_flags";
   const char* vgDir        = "options_dialog.html#valgrind";

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
   const char* mainTool        = "vg-manual-core.html#tool_name";
   const char* verbosity       = "vg-manual-core.html#verbosity";
   const char* traceChild      = "vg-manual-core.html#trace_children";
   const char* trackFds        = "vg-manual-core.html#track_fds";
   const char* timeStamp       = "vg-manual-core.html#time_stamp";
   const char* xmlOutput       = "vg-manual-core.html#xml_output";
   const char* xmlComment      = "vg-manual-core.html#xml_user_comment";
   const char* freeGlibc       = "vg-manual-core.html#free_glibc";
   const char* showEmWarns     = "vg-manual-core.html#show_emwarns";
   const char* smcSupport      = "vg-manual-core.html#smc_support";
   const char* simHints        = "vg-manual-core.html#simulation_hints";
   const char* kernelVariant   = "vg-manual-core.html#kernel_variant";

   /* valgrind's options page: tab Error Reporting */
   const char* logToFd         = "vg-manual-core.html#log2fd";
   const char* logToFilePid    = "vg-manual-core.html#log2file_pid";
   const char* logToFile       = "vg-manual-core.html#log2file";
   const char* logFileQual     = "vg-manual-core.html#log2file_qualifier";
   const char* logToSocket     = "vg-manual-core.html#log2socket";
   const char* autoDemangle    = "vg-manual-core.html#auto_demangle";
   const char* numCallers      = "vg-manual-core.html#num_callers";
   const char* errorLimit      = "vg-manual-core.html#error_limit";
   const char* stackTraces     = "vg-manual-core.html#stack_traces";
   const char* genSuppressions = "vg-manual-core.html#gen_supps";
   const char* startDebugger   = "vg-manual-core.html#attach_debugger";
   const char* whichDebugger   = "vg-manual-core.html#which_debugger";
   const char* inputFd         = "vg-manual-core.html#input_fd";
   const char* maxSFrames      = "vg-manual-core.html#max_frames";

   /* only used by Memcheck and Massif */
   const char* Alignment       = "vg-manual-core.html#alignment";
}


/* Memcheck ------------------------------------------------------------ */
namespace urlMemcheck { 
   const char* optsMC    = "vg-manual-mc.html";
   const char* Leakcheck = "vg-manual-mc.html#leakcheck";
   const char* Showreach = "vg-manual-mc.html#showreach";
   const char* UndefVal  = "vg-manual-mc.html#undefvalerrs";
   const char* Leakres   = "vg-manual-mc.html#leakres";
   const char* Freelist  = "vg-manual-mc.html#freelist";
   const char* gcc296    = "vg-manual-mc.html#gcc296";
   const char* Partial   = "vg-manual-mc.html#partial";
}


/* Cachegrind ---------------------------------------------------------- */
namespace urlCachegrind { 
   const char* optsCG    = "vg-manual-cg.html";
   const char* Cacheopts = "vg-manual-cg.html#cg-manual.cgopts";
   const char* Pid       = "vg-manual-cg.html#pid";
   const char* Sort      = "vg-manual-cg.html#sort";
   const char* Show      = "vg-manual-cg.html#show";
   const char* Threshold = "vg-manual-cg.html#threshold";
   const char* Auto      = "vg-manual-cg.html#auto";
   const char* Context   = "vg-manual-cg.html#context";
   const char* Include   = "vg-manual-cg.html#include";
}


/* Massif -------------------------------------------------------------- */
namespace urlMassif {
   const char* optsMS    = "vg-manual-ms.html";
   const char* Heap      = "vg-manual-ms.html#heap";
   const char* HeapAdmin = "vg-manual-ms.html#heap-admin";
   const char* Stacks    = "vg-manual-ms.html#stacks";
   const char* Depth     = "vg-manual-ms.html#depth";
   const char* AllocFn   = "vg-manual-ms.html#alloc-fn";
   const char* Format    = "vg-manual-ms.html#format";
}

