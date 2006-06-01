/* ---------------------------------------------------------------------- 
 * Various utility functions                                 vk_utils.cpp
 * ----------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_utils.h"
#include "config.h"                 // PACKAGE_BUGREPORT
#include "vk_config.h"              // vkname()

#include <stdlib.h>                 // mkstemp()
#include <stdarg.h>                 // va_start, va_end
#include <sys/types.h>              // getpid
#include <unistd.h>                 // getpid

#include <qapplication.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>


/* TODO: add user-opt to output valkyrie messages to different fd... a-la valgrind
    - keeps stdout/err clean for client program output
    fdopen?
*/

/* prints various info msgs to stdout --------------------------------- */
void vkPrint( const char* msg, ... )
{
   const char* vkname = vkConfig ? vkConfig->vkname() : "";
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stdout, "===%s:%d=== ", vkname, (int)getpid() ); 
   vfprintf( stdout, msg, ap );
   va_end( ap );
   fprintf( stdout, "\n" );
   fflush(stdout);
}


/* prints error msg -------------------------------------------------- */
void vkPrintErr( const char* msg, ... )
{
   const char* vkname = vkConfig ? vkConfig->vkname() : "";
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stderr, "===%s:%d=== ", vkname, (int)getpid() ); 
   vfprintf( stderr, msg, ap );
   va_end( ap );
   fprintf( stderr, "\n" );
   fflush(stderr);
}


/* Kludge to keep vglog happy
   - Want vk_logmerge to print messages from class VgLog, but keep that
     class within valkyrie...
   - Hmm... perhaps shove that class into vk_logmerge after all...
   TODO: Figure out what we want and implement it!
 */
void vklmPrint( int, const char*, ... ) { /* nada */ }
void vklmPrintErr( const char*, ... )   { /* nada */ }



/* prints an "Assertion failed" message and exits ---------------------- */
__attribute__ ((noreturn))
   void vk_assert_fail( const char* expr, const char* file,
                        unsigned int line, const char* fn )
{ 
   vkPrintErr("Assertion failed '%s':", expr );
   vkPrintErr("   at %s#%u:%s\n", file, line, fn );
   exit(1);
}

/* prints a message asking user to email a bug report, 
 * and then exits. ----------------------------------------------------- */
__attribute__ ((noreturn))
   void vk_assert_never_reached_fail( const char* file,
                                      unsigned int line, 
                                      const char* fn )
{
   vkPrintErr("Assertion 'never reached' failed,");
   vkPrintErr("   at %s#%u:%s", file, line, fn );
   vkPrintErr("%s version: %s", vkConfig->vkName(), PACKAGE_VERSION);
   vkPrintErr("Built with QT version:   %s", QT_VERSION_STR);
   vkPrintErr("Running with QT version: %s", qVersion());
   vkPrintErr("Hopefully, you should never see this message.");
   vkPrintErr("If you are, then Something Really Bad just happened.");
   vkPrintErr("Please report this bug to: %s", PACKAGE_BUGREPORT );
   vkPrintErr("In the bug report, please send the the above text,");
   vkPrintErr("along with the output of `uname -a`.");
   vkPrintErr("Thanks.\n");
   exit(1);
}


/* Create a unique filename, with an optional extension ---------------- */
QString vk_mkstemp( QString filepath, QString ext/*=QString::null*/ )
{
   /* create tempfiles with datetime, so can sort easily if they stay around */

   QString datetime = QDateTime::currentDateTime().toString( "-yyyy.MM.dd-hh.mm.ss");
	QString unique = filepath + datetime;
   if (!ext.isNull()) unique +=  "." + ext;

   if ( QFile::exists(unique) ) {
      /* fall back on mkstemp */
      char* tmpname = vk_str_malloc( unique.length() + 10 );
      sprintf( tmpname, "%s.XXXXXX", unique.latin1() );
      int fd = mkstemp( tmpname );
      if ( fd == -1 ) {
         /* something went wrong */
         VK_DEBUG("failed to create unique filename from '%s'.",
                  filepath.latin1() );
         return QString::null;
      }
      unique = QString( tmpname );
      tmpname = vk_str_free( tmpname );
   }
   return unique;
}


/* Version check ------------------------------------------------------- 
   Given version string of "major.minor.patch" (e.g. 3.0.0),
   hex version = (major << 16) + (minor << 8) + patch
*/
int str2hex( QString ver_str )
{
   QRegExp rxver(".*(\\d{1,2})\\.(\\d{1,2})\\.(\\d{1,2}).*");
   if ( rxver.search( ver_str ) == -1)
      return -1;
   int major = rxver.cap(1).toInt();
   int minor = rxver.cap(2).toInt();
   int patch = rxver.cap(3).toInt();
   return (major << 16) + (minor << 8) + patch;
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
      case '&': {
         /* already escaped? */
         if ((content.mid(i+1,4) == "amp;") ||
             (content.mid(i+1,3) == "lt;" ) ||
             (content.mid(i+1,3) == "gt;" ))
            ret_str += content[i];
         else
            ret_str += "&amp;";    
      } break;
      default:  ret_str += content[i]; break;
      }
   }
  
   return ret_str;
}


/* swap '\n' for <br> */
QString str2html( QString str )
{
   str.replace( '\n', "<br>" );
   return str;
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


