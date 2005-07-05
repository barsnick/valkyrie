/* ---------------------------------------------------------------------- 
 * Various utility functions                                 vk_utils.cpp
 * ----------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_utils.h"
#include "vk_config.h"

#include <stdlib.h>                 // mkstemp()
#include <stdio.h>                  // printf and friends
#include <stdarg.h>                 // va_start, va_end

#include <qfile.h>



/* prints various info msgs to stdout --------------------------------- */
void vkPrint( const char* msg, ... )
{
  va_list ap;
  va_start( ap, msg );
  va_end( ap );
  fprintf( stdout, "\n" ); 
  vfprintf( stdout, msg, ap );
  va_end( ap );
  fprintf( stdout, "\n\n" );
}


/* prints an "Assertion failed" message and exits ---------------------- */
__attribute__ ((noreturn))
void vk_assert_fail( const char* expr, const char* file,
                     unsigned int line, const char* fn )
{ 
  vkPrint("\n%s:%u\n"
          "     %s: Assertion '%s' failed.\n",
          file, line, fn, expr );
  exit(1);
}

/* prints a message asking user to email a bug report, 
 * and then exits. ----------------------------------------------------- */
__attribute__ ((noreturn))
void vk_assert_never_reached_fail( const char* file,
                                   unsigned int line, 
                                   const char* fn )
{
  printf("\nAssertion 'never reached' failed,\n"
         "in %s:%u %s\n", file, line, fn );
  printf("Hopefully, you should never see this message.\n"
         "If you are, then Something Really Bad just happened.\n"
         "Please report this bug to: %s\n", vkConfig->vkEmail() );
  printf("In the bug report, please send the the above text.\n"
         "Thanks.\n\n");
  exit(1);
}


/* prints a debugging message with file+line info to stdout ------------ */
void vk_print( const char* file, const char* fn, unsigned int line, 
               const char* prefix, const char* msg, ... )
{
#if DEBUG_ON
  fprintf( stdout, "\n%s: %s : %s #%d:\n\t", prefix, file, fn, line );
  va_list ap;
  va_start( ap, msg );
  vfprintf( stdout, msg, ap );
  va_end( ap );
  fprintf( stdout, "\n");
#endif
}


/* Create a unique filename, with an optional extension ---------------- */
QString vk_mkstemp( QString fname, QString path, 
                    QString ext/*=QString::null*/ )
{
  int len = fname.length() + path.length() + 10;
  char* filename = vk_str_malloc( len );
  sprintf( filename, "%s%s-XXXXXX", path.latin1(), fname.latin1() );
  int fd = mkstemp( filename );
  if ( fd == -1 ) {
    /* something went wrong */
    VK_DEBUG("vk_mkstemp(): failed to create tmp file '%s'", filename );
    return QString::null;
  }

  QString ret_fname( filename );
  filename = vk_str_free( filename );

  /* add eg. '.xml' to filename */
  if ( !ext.isNull() ) {
    QFileInfo fi( ret_fname );
    QDir dir( fi.dir() );
    if ( dir.rename( fi.fileName(), ret_fname + ext ) )
      ret_fname += ext;
  }
  
  return ret_fname;
}


/* Version check ------------------------------------------------------- 
   Version strings have a numeric value in the form:
   0x mm ii bb (m = major, i = minor, b = bugfix). 
   For example, Valgrind 3.0.5 is 0x 030005 */
int str2hex( QString ver_str )
{
  int dot1 = ver_str.find( '.', 0 );
  int dot2 = ver_str.find( '.', dot1+1 );
  int dot3 = ver_str.find( '.', dot2+1 );

  int major = ver_str.left( dot1 ).toInt();
  int minor = ver_str.mid( dot1+1, dot2 - (dot1+1) ).toInt();
  int plevel;
  if ( dot3 == -1 )
    plevel = ver_str.right( ver_str.length() - (dot2+1) ).toInt();
  else
    plevel = ver_str.mid( dot2+1, dot3 - (dot2+1) ).toInt();

  int hex = (major << 16) + (minor << 8) + plevel;

  return hex;
}



/* escape html entities
   current list: '<', '>', '&' ----------------------------------------- */
QString escapeEntities( const QString& content )
{
  QString ret_str = "";

  for ( unsigned int i=0; i<content.length(); i++ ) {
    switch ( content[i].latin1() ) {
      case '<': ret_str += "&lt;";     break;
      case '>': ret_str += "&gt;";     break;
      case '&': ret_str += "&amp;";    break;
      default:  ret_str += content[i]; break;
    }
  }
  
  return ret_str;
}




/* wrappers for various fns -------------------------------------------- */

/* wrappers to free(3)
   hides const compilation noise, permit NULL, return NULL always. */
void * vk_free( const void* ptr )
{
  if ( ptr != NULL ) {
    free( (void*)ptr );
    ptr = NULL;
  }
  return NULL;
}

char * vk_str_free( const char* ptr )
{
  if ( ptr != NULL ) {
    free( (char*)ptr );
  }
  return NULL;
}

void * vk_malloc( unsigned long n_bytes )
{
  void * mem;
  mem = malloc( n_bytes );
  if ( !mem ) {
    VK_DEBUG( "failed to allocate %lu bytes", n_bytes );
  }
  return mem;
}


char * vk_str_malloc( int sz )
{ 
  char* arr;
  arr = (char*) malloc( (size_t) ((sz + 2)*sizeof(char)) );
  if ( !arr ) {
    VK_DEBUG("malloc failure: virtual memory exhausted");
    vk_assert_never_reached();
  }
  return arr;
}


/* wrapper to strcmp(): returns true || false */
bool vk_strcmp( const char* str1, const char* str2 )
{
  if ( !str1 || !str2 ) {
    VK_DEBUG("can't call strcmp on null strings:\n"
             "str1 == %s, str2 == %s\n", str1, str2 );
    return false;
  }
  if ( (strlen(str1) == 0) || (strlen(str2) == 0) ) {
    VK_DEBUG("one of these two strings is empty:\n"
             "\tstr1: -->%s<--, str2: -->%s<--\n", str1, str2 );
    return false;
  }

  return (strcmp( str1, str2 ) == 0) ? true : false;
}


char* vk_strdup( const char* str )
{
  char* new_str;
  unsigned int length;
  if ( str ) {
    length = strlen( str ) + 1;
    new_str = vk_str_malloc( length );
    strcpy( new_str, str );
  } else {
    new_str = NULL;
  }
  return new_str;
}


