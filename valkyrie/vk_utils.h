/* ---------------------------------------------------------------------
 * Various utility functions                                  vk_utils.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_UTILS_H
#define __VK_UTILS_H

#include <stdio.h>          // printf and friends

#include <qstring.h>

#define DEBUG_ON 1
#define VK_CHECK_NULL

#if defined(VK_CHECK_NULL)
#  define VK_CHECK_PTR(p) do { \
   if ((p)==0) vkPrintErr("Out of memory: %s#%d", __FILE__, __LINE__); } while(0)
#else
#  define VK_CHECK_PTR(p)
#endif

#define VK_STRINGIFY_ARG(contents) #contents
#define vkStringify(macro_or_string) VK_STRINGIFY_ARG(macro_or_string)
#define VK_STRLOC  __FILE__ ":" vkStringify(__LINE__)
#define VK_STRING(__str)  #__str

#define oink(n) printf("oink %d\n", n )
#define oynk    printf("oynk %s:%d\n", __FILE__, __LINE__)


/* vk_assert can never be turned off :) ------------------------------- */
extern void vk_assert_fail( const char* expr, const char* file, 
                            unsigned int line, const char* fn )
     __attribute__ ((__noreturn__));

#define vk_assert(expr) ((void) ((expr) ? 0 :      \
  (vk_assert_fail (VK_STRING(expr),                \
   __FILE__, __LINE__, __PRETTY_FUNCTION__), 0)))


extern void vk_assert_never_reached_fail( const char* file,
                                          unsigned int line, 
                                          const char* fn )
     __attribute__ ((__noreturn__));

#define vk_assert_never_reached() ((void) (        \
  (vk_assert_never_reached_fail(                   \
   __FILE__, __LINE__, __PRETTY_FUNCTION__), 0)))


#if DEBUG_ON
/* print debugging msg with file+line info to stderr ------------------- */
#  define VK_DEBUG(msg, args...) {                             \
   vkPrintErr("DEBUG: %s#%d:%s:",                              \
               __FILE__, __LINE__, __PRETTY_FUNCTION__ );      \
   vkPrintErr(msg, ## args );                                  \
   vkPrintErr(" ");                                            \
}
#else
#  define VK_DEBUG(msg, args...) /*NOTHING*/
#endif


/* print user info message --------------------------------------------- */
extern void vkPrint( const char *, ... )
     __attribute__ ((format (printf, 1, 2)));

/* print error message ------------------------------------------------- */
extern void vkPrintErr( const char *, ... )
     __attribute__ ((format (printf, 1, 2)));


/* kludge to keep vglog happy ------------------------------------------ */
extern void vklmPrint( int, const char*, ... )
   __attribute__((format (printf, 2, 3)));
extern void vklmPrintErr( const char*, ... )
   __attribute__((format (printf, 1, 2)));


/* create a unique filename -------------------------------------------- */
QString vk_mkstemp( QString filepath, QString ext=QString::null );

/* 3.0.5 --> 0x030005 -------------------------------------------------- */
int str2hex( QString ver_str );

/* escape html entities
 * current list: '<', '>', '&' ----------------------------------------- */
QString escapeEntities( const QString& str );

/* malloc and free fns ------------------------------------------------- */
void * vk_free( const void* p );

char * vk_str_free( const char* ptr );

void * vk_malloc( unsigned long n_bytes );

char* vk_str_malloc( int sz );

#define vk_new( type, num ) \
  ((type *)vk_malloc(((unsigned int) sizeof(type)) * ((unsigned int)(num))))

bool vk_strcmp( const char* str1, const char* str2 );

char* vk_strdup( const char* str );


#endif
