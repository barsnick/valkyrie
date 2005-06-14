/* ---------------------------------------------------------------------
 * Declarations of url strings                               html_urls.h
 * Placed in one file in an endeavour to minimise what is probably going
 * to be a maintenance nightmare.
 * --------------------------------------------------------------------- 
 */

#ifndef __VK_HELP_URLS_H
#define __VK_HELP_URLS_H

/*
extern const char* ;
extern const char* ;
extern const char* ;
extern const char* ;
extern const char* ;
extern const char* ;
*/


/* placeholder so we can grep to find missing links */
extern const char* urlNone;


/* MainWindow ---------------------------------------------------------- */
namespace urlValkyrie { 
	extern const char* FlagsButton;
	extern const char* Dummy;
}


/* Valgrind core ------------------------------------------------------- */
namespace urlVgCore { 
	extern const char* Alignment;
}

/* Massif -------------------------------------------------------------- */
namespace urlMassif { 
	extern const char* Heap;
	extern const char* HeapAdmin;
	extern const char* Stacks;
	extern const char* Depth;
	extern const char* AllocFn;
	extern const char* Format;
};


#endif
