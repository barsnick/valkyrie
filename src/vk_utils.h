/* ---------------------------------------------------------------------
 * Various utility functions                                  vk_utils.h
 * ---------------------------------------------------------------------
 */

#ifndef __VK_UTILS_H
#define __VK_UTILS_H

#include <qstring.h>


#define DEBUG_ON 1

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

#define vk_assert(expr) ((void) ((expr) ? 0 : \
  (vk_assert_fail (VK_STRING(expr), \
   __FILE__, __LINE__, __PRETTY_FUNCTION__), 0)))


extern void vk_assert_never_reached_fail( const char* file,
                                          unsigned int line, 
                                          const char* fn )
  __attribute__ ((__noreturn__));

#define vk_assert_never_reached() ((void) ( \
  (vk_assert_never_reached_fail(            \
   __FILE__, __LINE__, __PRETTY_FUNCTION__), 0)))


/* print debugging msg with file+line info ----------------------------- */
extern void vk_print( const char* file, const char* fn,
											unsigned int line, const char* prefix, 
											const char* msg, ... );
#define VK_DEBUG(msg, args...) {            \
  vk_print( __FILE__, __PRETTY_FUNCTION__,  \
  __LINE__, "DEBUG", msg, ## args);       \
}


/* print user info message --------------------------------------------- */
extern void vkPrint( const char *, ... )
     __attribute__ ((format (printf, 1, 2)));

/* Create a unique filename -------------------------------------------- */
QString vk_mkstemp( QString fname, QString path );

/* 3.0.5 --> 0x030005 -------------------------------------------------- */
int str2hex( QString ver_str );

/* Command-line args parsing ------------------------------------------- 
	 implemented in /src/options/parse_cmd_args.cpp */
extern int parseCmdArgs( int argc, char** argv );



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
