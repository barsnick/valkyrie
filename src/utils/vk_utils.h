/****************************************************************************
** Various utility functions
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __VK_UTILS_H
#define __VK_UTILS_H

#include <stdio.h>          // printf and friends
#include <iostream>

#include <QString>
#include <QFileDialog>

using namespace std;


#define DEBUG_ON 1
#define VK_CHECK_NULL

#if defined(VK_CHECK_NULL)
#  define VK_CHECK_PTR(p) do { \
      if ((p)==0) vkPrintErr("Out of memory: %s#%d", __FILE__, __LINE__); } while(0)
#else
#  define VK_CHECK_PTR(p)
#endif


#define VK_STRING(__str)  #__str

#if 0
#define VK_STRINGIFY_ARG(contents) #contents
#define vkStringify(macro_or_string) VK_STRINGIFY_ARG(macro_or_string)
#define VK_STRLOC  __FILE__ ":" vkStringify(__LINE__)

#define oink(n) printf("oink %d\n", n )
#define oynk    printf("oynk %s:%d\n", __FILE__, __LINE__)
#endif


/* vk_assert can never be turned off :) ------------------------------- */
extern void vk_assert_fail( const char* expr, const char* file,
                            unsigned int line, const char* fn )
__attribute__(( __noreturn__ ) );

#define vk_assert(expr)                                       \
   ( (void)( (expr) ? 0 :                                     \
      ( vk_assert_fail( VK_STRING(expr),                      \
           __FILE__, __LINE__, __PRETTY_FUNCTION__ ), 0 ) ) )


extern void vk_assert_never_reached_fail( const char* file,
                                          unsigned int line,
                                          const char* fn )
__attribute__(( __noreturn__ ) );

#define vk_assert_never_reached()                                    \
   ( (void) ( ( vk_assert_never_reached_fail(                        \
                   __FILE__, __LINE__, __PRETTY_FUNCTION__ ), 0) ) )


#if DEBUG_ON
/* print debugging msg with file+line info to stderr ------------------- */
// Not using (fmt, args...), (fmt, ##args), as QtCreator code checker doesn't like it.
// TODO: reintroduce when we start using a proper IDE
#  define VK_DEBUG(...) {                                                        \
      vkPrintErr("DEBUG: %s#%d: %s:", __FILE__, __LINE__, __PRETTY_FUNCTION__ ); \
      vkPrintErr(__VA_ARGS__);                                                   \
      vkPrintErr(" ");                                                           \
   }
#else
#  define VK_DEBUG(msg, args...) /*NOTHING*/
#endif


/* print user info message --------------------------------------------- */
void vkPrint( const char*, ... )
__attribute__(( format( printf, 1, 2 ) ) );

/* print error message ------------------------------------------------- */
void vkPrintErr( const char*, ... )
__attribute__(( format( printf, 1, 2 ) ) );

/* print debug message ------------------------------------------------- */
void vkDebug( const char*, ... )
__attribute__(( format( printf, 1, 2 ) ) );


/* create a unique filename -------------------------------------------- */
QString vk_mkstemp( QString filepath, QString ext = QString() );

/* "valgrind 3.0.5" --> 0x030005 --------------------------------------- */
int strVersion2hex( QString ver_str );

/* escape html entities
 * current list: '<', '>', '&' ----------------------------------------- */
QString escapeEntities( const QString& str );

/* swap '\n' for <br> */
QString str2html( QString str );

/* malloc and free fns ------------------------------------------------- */
void* vk_free( const void* p );

char* vk_str_free( const char* ptr );

void* vk_malloc( unsigned long n_bytes );

char* vk_str_malloc( int sz );

#define vk_new( type, num ) \
   ((type *)vk_malloc(((unsigned int) sizeof(type)) * ((unsigned int)(num))))

bool vk_strcmp( const char* str1, const char* str2 );

char* vk_strdup( const char* str );





// ============================================================
// helper functions
bool strToBool( QString str, bool* ok = NULL );

QString fileCheck( int* err_val, const QString fpath,
                   bool check_read=false, bool check_write=false,
                   bool check_exe=false );

QString dirCheck( int* err_val, const QString fpath,
                  bool check_read=false, bool check_write=false,
                  bool check_exe=false );


// ============================================================
// file/dir dialogs,
QString vkDlgGetFile( QWidget* parent,
                      const QString& start_dir = "./",
                      const QString& cfg_key_path = QString(),
                      QFileDialog::AcceptMode mode = QFileDialog::AcceptOpen );

QString vkDlgCfgGetFile( QWidget* parent,
                         const QString& cfg_key_path = QString(),
                         QFileDialog::AcceptMode mode = QFileDialog::AcceptOpen );

QString vkDlgGetDir( QWidget* parent,
                     const QString& start_dir = "./" );




#endif
