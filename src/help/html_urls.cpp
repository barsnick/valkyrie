/* ---------------------------------------------------------------------
 * Declarations of url strings                             html_urls.cpp
 * Placed in one file in an endeavour to minimise what is probably going
 * to be a maintenance nightmare.
 * --------------------------------------------------------------------- 
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


/* Massif -------------------------------------------------------------- */
namespace urlMassif { 
	const char* Heap      = "ms-manual.options.html#heap";
	const char* HeapAdmin = "ms-manual.options.html#heap-admin";
	const char* Stacks    = "ms-manual.options.html#stacks";
	const char* Depth     = "ms-manual.options.html#depth";
	const char* AllocFn   = "ms-manual.options.html#alloc-fn";
	const char* Format    = "ms-manual.options.html#format";
}

