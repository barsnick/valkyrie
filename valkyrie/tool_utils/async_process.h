/* ---------------------------------------------------------------------
 * Definition of class AsyncProcess                      async_process.h
 *
 * Contains various bits and pieces ripped off from all over the
 * place.  No credit to me, all credit to the various authors.
 *
 * Process Launching
 * Executes a child program asynchronously (main program won't block
 * waiting for the child to exit).
 *
 * The first string in arglist (ie. arglist[0]) is the name of the
 * program to execute. By default, the name of the program must be a
 * full path; the PATH shell variable will only be searched if you
 * pass the SEARCH_PATH flag.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_ASYNC_PROCESS_H
#define __VK_ASYNC_PROCESS_H


#include <qstringlist.h>


class AsyncProcess
{
public:
   enum AsyncFlags { 
      LEAVE_FDS_OPEN  = 1 << 0, 
      DONT_REAP_CHILD = 1 << 1,
      SEARCH_PATH     = 1 << 2 
   };

   AsyncProcess( QStringList arglist, AsyncFlags flags );
   static bool spawn( QStringList arglist, AsyncFlags flags );

   int  closeAndInvalidate( int* fd );

   bool makePipe( int p[2] );
   bool midChild() { return intermediateChild; }
   bool writeAll( int fd, const void* vbuf, unsigned int to_write );

   void doExec( int child_err_report_fd );

private:
   int  execute();
   int  saneDup2( int fd1, int fd2 );

   bool readInts( int fd, int* buf, int n_ints_in_buf, int* n_ints_read );

   void writeErrAndExit( int fd, int msg );
   void setCloexec( int fd );
   void scriptExecute( const char* file, char** argv );

   char* vkStrchrnul( const char* str, char c );

private:
   enum { CHILD_EXEC_FAILED, CHILD_DUP2_FAILED, CHILD_FORK_FAILED };

   bool searchPath;
   bool closeDescriptors;
   bool intermediateChild;

   QStringList args;
};


#endif
