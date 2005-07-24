/* ---------------------------------------------------------------------
 * Declarations of url strings                               html_urls.h
 * Placed in one file in an endeavour to minimise 
 * what is probably going to be a maintenance nightmare.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_HELP_URLS_H
#define __VK_HELP_URLS_H


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


/* Memcheck ------------------------------------------------------------ */
namespace urlMemcheck { 
  extern const char* Partial;
  extern const char* Freelist;
  extern const char* Leakcheck;
  extern const char* Leakres;
  extern const char* Showreach;
  extern const char* gcc296;
  extern const char* Strlen;
}


/* Cachegrind ---------------------------------------------------------- */
namespace urlCachegrind { 
  extern const char* Cacheopts;
  extern const char* Pid;
  extern const char* Sort;
  extern const char* Show;
  extern const char* Threshold;
  extern const char* Auto;
  extern const char* Context;
  extern const char* Include;
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
