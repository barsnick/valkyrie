/* ---------------------------------------------------------------------
 * main(): vk_logmerge program entry point                   vklm_main.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VKLM_MAIN_H
#define __VKLM_MAIN_H

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#define VKLM_DEBUG_ON 0


/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

/* print message to stderr */
extern void vklmPrint( const char *, ... )
     __attribute__((format (printf, 1, 2)));


#if VKLM_DEBUG_ON
/* print debugging msg to stderr */
#define VKLM_DEBUG(msg, args...) {     \
  fprintf(stderr, "VKLM_DEBUG: ");   \
  fprintf(stderr, msg, ## args );      \
  fprintf(stderr, "\n" );              \
}
#define VKLM_DEBUG_FN(msg, args...) {                           \
  fprintf(stderr, "\nVKLM_DEBUG: %s: ", __PRETTY_FUNCTION__ );  \
  fprintf(stderr, msg, ## args );                               \
  fprintf(stderr, "\n" );                                       \
}
#else
#define VKLM_DEBUG(msg, args...)    /*NOTHING*/
#define VKLM_DEBUG_FN(msg, args...) /*NOTHING*/
#endif


#endif // #ifndef __VKLM_MAIN_H
