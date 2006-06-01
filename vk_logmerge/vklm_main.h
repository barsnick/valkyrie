/* ---------------------------------------------------------------------
 * main(): vk_logmerge program entry point                   vklm_main.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VKLM_MAIN_H
#define __VKLM_MAIN_H

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

/* If we're not using GNU C, elide __attribute__ */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

/* print message to stderr */
extern void vklmPrint( int verb, const char *, ... )
     __attribute__((format (printf, 2, 3)));

extern void vklmPrintErr( const char *, ... )
     __attribute__((format (printf, 1, 2)));

extern int vklm_verbosity;

#endif // #ifndef __VKLM_MAIN_H
