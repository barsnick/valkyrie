/* ---------------------------------------------------------------------
 * Implementation of class AsyncProcess                async_process.cpp
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
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#include <errno.h>                 /* Fedora Core needs this */
#include <fcntl.h>                 /* open(), O_RDONLY, O_WRONLY */
#include <signal.h>
#include <stdlib.h>                /* getenv            */
#include <sys/types.h>             /* waitpid()         */
#include <sys/wait.h>              /* waitpid()         */
#include <unistd.h>                /* dup2(), _exit(),  */

#include "async_process.h"
#include "vk_utils.h"


AsyncProcess::AsyncProcess( QStringList arglist, AsyncFlags flags )
{ 
   searchPath        =  (flags & SEARCH_PATH) != 0;
   closeDescriptors  = !(flags & LEAVE_FDS_OPEN);
   intermediateChild = !(flags & DONT_REAP_CHILD);

   args = arglist;
}


bool AsyncProcess::spawn( QStringList arglist, AsyncFlags flags )
{
   if ( arglist.isEmpty() )
      return false;

   int pid = -1;
   int child_err_report_pipe[2] = { -1, -1 };
   int child_pid_report_pipe[2] = { -1, -1 };
   int status;
  
   AsyncProcess* aProc = new AsyncProcess( arglist, flags );

   if ( !aProc->makePipe( child_err_report_pipe ) )
      goto fail;
   if ( aProc->midChild() && !aProc->makePipe( child_pid_report_pipe ) )
      goto cleanup_and_fail;

   pid = fork();
   if ( pid < 0 ) {   
      VK_DEBUG("Fork failed due to lack of memory");
      goto cleanup_and_fail;
   } else if ( pid == 0 ) {
      /* immediate child: this may or may not be the child that actually
         execs the new process.  be sure we crash if the parent exits
         and we write to the err_report_pipe */
      signal( SIGPIPE, SIG_DFL );

      /* close the parent's end of the pipes; not needed in the
         closeDescriptors case, though */
      aProc->closeAndInvalidate( &child_err_report_pipe[0] );
      aProc->closeAndInvalidate( &child_pid_report_pipe[0] );
   
      if ( aProc->midChild() ) {
         /* we need to fork an intermediate child that launches the final
            child.  the purpose of the intermediate child is to exit, so we
            can waitpid() it immediately.  then the grandchild will not
            become a zombie.  */
         int grandchild_pid = fork();
         if ( grandchild_pid < 0 ) {
            /* report -1 as child PID */
            aProc->writeAll( child_pid_report_pipe[1], 
                             &grandchild_pid, sizeof( grandchild_pid ) );
            aProc->writeErrAndExit( child_err_report_pipe[1], 
                                    CHILD_FORK_FAILED );
         } else if ( grandchild_pid == 0 ) {
            aProc->doExec( child_err_report_pipe[1] );
         } else {
            aProc->writeAll( child_pid_report_pipe[1], 
                             &grandchild_pid, sizeof( grandchild_pid) );
            aProc->closeAndInvalidate( &child_pid_report_pipe[1] );
            _exit(0);
         }
      } else {
         /* just run the child */
         aProc->doExec( child_err_report_pipe[1] );
      }
   } else {
      /* parent */
      int buf[2];
      int n_ints = 0; 

      /* close the uncared-about ends of the pipes */
      aProc->closeAndInvalidate( &child_err_report_pipe[1] );
      aProc->closeAndInvalidate( &child_pid_report_pipe[1] );

      /* if we had an intermediate child, reap it */
      if ( aProc->midChild() ) {
      wait_again:
         if ( waitpid( pid, &status, 0 ) < 0 ) {
            if ( errno == EINTR ) {
               goto wait_again;
            } else if ( errno == ECHILD ) {
               ; /* do nothing, child already reaped */
            } else {
               vk_assert_never_reached();
            }
         }
      }

      if ( !aProc->readInts( child_err_report_pipe[0], buf, 2, &n_ints ) )
         goto cleanup_and_fail;
  
      if ( n_ints >= 2 ) {  /* error from the child. */

         switch ( buf[0] ) {
         case CHILD_EXEC_FAILED:
            VK_DEBUG("Failed to execute child process '%s'", 
                     arglist[0].latin1() );
            break;
         case CHILD_DUP2_FAILED:
            VK_DEBUG("Failed to redirect output or input of child process");
            break;
         case CHILD_FORK_FAILED:
            VK_DEBUG("Failed to fork child process");
            break;
         default:
            VK_DEBUG("Unknown error executing child process '%s'",
                     arglist[0].latin1() );
            break;
         }

         goto cleanup_and_fail;
      }

      /* get child pid from intermediate child pipe. */
      if ( aProc->midChild() ) {
         n_ints = 0;
 
         if ( !aProc->readInts( child_pid_report_pipe[0], buf, 1, &n_ints ) )
            goto cleanup_and_fail;

         if ( n_ints < 1 ) {
            VK_DEBUG("Failed to read enough data from child pid pipe");
            goto cleanup_and_fail;
         } else { 
            pid = buf[0];      /* we have the child pid */
         }
      }
   
      /* success against all odds! return the information */
      aProc->closeAndInvalidate( &child_err_report_pipe[0] );
      aProc->closeAndInvalidate( &child_pid_report_pipe[0] );
      delete aProc;
      aProc = 0;
      return true;
   }

 cleanup_and_fail:
   /* there was an error from the child: reap the child to avoid it
      being a zombie.  */
   if ( pid > 0 ) {
   wait_failed:
      if ( waitpid( pid, NULL, 0 ) < 0 ) {
         if ( errno == EINTR ) {
            goto wait_failed;
         } else if ( errno == ECHILD ) {
            ; /* do nothing, child already reaped */
         } else {
            VK_DEBUG("waitpid() should not fail in fork_exec_with_pipes");
         }
      }
   }

   aProc->closeAndInvalidate( &child_err_report_pipe[0] );
   aProc->closeAndInvalidate( &child_err_report_pipe[1] );
   aProc->closeAndInvalidate( &child_pid_report_pipe[0] );
   aProc->closeAndInvalidate( &child_pid_report_pipe[1] );

 fail:
   delete aProc;
   aProc = 0;

   return false;
}


bool AsyncProcess::makePipe( int p[2] )
{
   if ( pipe( p ) < 0 ) {
      VK_DEBUG("Failed to create pipe for communicating with child process");
      return false;
   }

   return true;
}


/* avoids a danger in threaded situations (calling close() on a file
   descriptor twice, and another thread has re-opened it since the
   first close) */
int AsyncProcess::closeAndInvalidate( int* fd )
{
   int ret;

   if ( *fd < 0 ) {
      return -1;
   } else {
      ret = ::close( *fd );
      *fd = -1;
   }

   return ret;
}


bool AsyncProcess::writeAll( int fd, const void* vbuf, 
                             unsigned int to_write )
{
   char* buf = (char*) vbuf;
  
   while ( to_write > 0 ) {
      signed int count = write( fd, buf, to_write );
      if ( count < 0 ) {
         if ( errno != EINTR )
            return false;
      } else {
         to_write -= count;
         buf += count;
      }
   }
  
   return true;
}


void AsyncProcess::writeErrAndExit( int fd, int msg )
{
   int en = errno;
   writeAll( fd, &msg, sizeof(msg) );
   writeAll( fd, &en, sizeof(en) );
   _exit( 1 );
}


void AsyncProcess::doExec( int child_err_report_fd )
{
   /* close all file descriptors but stdin stdout and stderr as soon as
      we exec.  note that this includes child_err_report_fd, which keeps
      the parent from blocking forever on the other end of that pipe.  */
   if ( closeDescriptors ) {
      int open_max = sysconf( _SC_OPEN_MAX );

      for ( int i = 3; i < open_max; i++ )
         setCloexec( i );
   } else {
      /* we need to do child_err_report_fd anyway */
      setCloexec( child_err_report_fd );
   }
  
   /* keep process from blocking on a read of stdin */
   int read_null = ::open ( "/dev/null", O_RDONLY );
   saneDup2( read_null, 0 );
   closeAndInvalidate( &read_null );

   execute();

   /* exec failed */
   writeErrAndExit( child_err_report_fd, CHILD_EXEC_FAILED );
}


bool AsyncProcess::readInts( int fd, int* buf, int n_ints_in_buf, 
                             int* n_ints_read )
{
   unsigned int bytes = 0; 
  
   while ( true ) {
      signed int chunk; 

      if ( bytes >= sizeof(int)*2 )
         break; /* give up, who knows what happened, should'nt be
                   possible */
   again:
      chunk = ::read( fd, ((char*)buf) + bytes, 
                      sizeof(int) * n_ints_in_buf - bytes );
      if ( chunk < 0 && errno == EINTR )
         goto again;
 
      if ( chunk < 0 ) {
         /* some weird shit happened, bail out */
         VK_DEBUG("Failed to read from child pipe");
         return false;
      } else if ( chunk == 0 ) {
         break;    /* EOF */
      } else {    /* chunk > 0 */
         bytes += chunk;
      }
   }

   *n_ints_read = (int)(bytes / sizeof(int));

   return true;
}


void AsyncProcess::setCloexec( int fd )
{ fcntl( fd, F_SETFD, FD_CLOEXEC ); }


int AsyncProcess::saneDup2( int fd1, int fd2 )
{
   int ret;
  
 retry:
   ret = dup2( fd1, fd2 );
   if ( ret < 0 && errno == EINTR )
      goto retry;

   return ret;
}


int AsyncProcess::execute()
{
   const char* file = args[0].latin1();
   if ( *file == '\0' ) {
      /* we check the simple case first. */
      errno = ENOENT;
      return -1;
   }

   /* construct the arguments for exec */
   char** argv = new char*[ args.count() + 1 ];
   int i = 0;
   for ( QStringList::Iterator it = args.begin(); 
         it != args.end(); ++it ) {
      argv[i]  = (char*)args[i].latin1();
      i++;
   }
   argv[i] = 0;

   if ( !searchPath || strchr( file, '/' ) != NULL ) {
      ::execv( file, argv );
   
      if ( errno == ENOEXEC ) {
         scriptExecute( file, argv );
      }
   } else {
      bool got_eacces = 0;
      const char *p;
      char *name, *freeme;

      const char* path = getenv( "PATH" );
      if ( path == NULL ) {
         /* there is no 'PATH' in the environment.  the default
            search path in libc is the current directory followed by
            the path 'confstr' returns for '_CS_PATH'.  */

         /* we put '.' last, for security, and don't use the
            unportable confstr();  UNIX98 does not actually specify
            what to search if PATH is unset.  POSIX may, dunno.  */
         path = "/bin:/usr/bin:.";
      }

      size_t len = strlen( file ) + 1;
      size_t pathlen = strlen( path );
      freeme = name = vk_str_malloc( pathlen + len + 1 );

      /* copy the file name at the top, including '\0'  */
      memcpy( name + pathlen + 1, file, len );
      name = name + pathlen;
      /* and add the slash before the filename  */
      *name = '/';

      p = path;
      do {
         char* startp;
         path = p;
         p = vkStrchrnul( path, ':' );

         if ( p == path )
            /* two adjacent colons, or a colon at the beginning or the
               end of `PATH' means to search the current directory.  */
            startp = name + 1;
         else
            startp = (char*)memcpy( name - (p-path), path, p-path );
         /* try to execute this name.  If it works, execv will not
            return.  */
         ::execv( startp, argv );
 
         if ( errno == ENOEXEC ) {
            scriptExecute( startp, argv );
         }

         switch ( errno ) {
         case EACCES:
            /* record that we got a 'Permission denied' error.  If we end
               up finding no executable we can use, we want to diagnose
               that we did find one but were denied access.  */
            got_eacces = true;

            /* FALL THRU */
         case ENOENT:
         case ESTALE:
         case ENOTDIR:
            /* those errors indicate the file is missing or not executable
               by us, in which case we want to just try the next path
               directory.  */
            break;

         default:
            /* some other error means we found an executable file, but
               something went wrong executing it; return the error to our
               caller.  */
            vk_free( freeme );
            return -1;
         }
      } while ( *p++ != '\0' );
   
      /* we tried every element and none of them worked  */
      if ( got_eacces )
         /* at least one failure was due to permissions, so report that
            error.  */
         errno = EACCES;
      vk_free( freeme );
   }

   /* Return the error from the last attempt (probably ENOENT) */
   return -1;
}


/* based on execvp from GNU C Library */
void AsyncProcess::scriptExecute( const char* file, char **argv )
{
   /* count the arguments */
   int argc = 0;
   while ( argv[argc] )
      ++argc;
  
   /* construct an argument list for the shell.  */
   char** new_argv;
   new_argv = vk_new( char*, argc + 2 ); /* /bin/sh and NULL */
   new_argv[0] = (char *) "/bin/sh";
   new_argv[1] = (char *) file;
   while ( argc > 0 ) {
      new_argv[argc + 1] = argv[argc];
      --argc;
   }

   /* execute the shell. */
   ::execv( new_argv[0], new_argv );
 
   vk_free( new_argv );
}


char * AsyncProcess::vkStrchrnul( const char* str, char c )
{
   char* p = (char*) str;
   while ( *p && (*p != c) )
      ++p;

   return p;
}

