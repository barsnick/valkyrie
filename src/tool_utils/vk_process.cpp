/* ---------------------------------------------------------------------- 
 * Implementation of class VKProcess                         vk_process.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 * ---------------------------------------------------------------------
 * This file is a re-implementation of QProcess:
 * ** ($Id: qt/qprocess_unix.cpp   3.3.4   edited Dec 23 11:57 $)
 * ** Created : 20000905
 * **
 * ** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
 * **
 * ** This file may be distributed and/or modified under the terms of the
 * ** GNU General Public License version 2 as published by the Free Software
 * ** Foundation and appearing in the file LICENSE.GPL included in the
 * ** packaging of this file.
 */

#include <qplatformdefs.h>

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED.
#if defined(connect)
#undef connect
#endif

#include "vk_process.h"
#include "vk_utils.h"

#include <qapplication.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qcleanuphandler.h>
#include <qregexp.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>


#ifdef __MIPSEL__
# ifndef SOCK_DGRAM
#  define SOCK_DGRAM 1
# endif
# ifndef SOCK_STREAM
#  define SOCK_STREAM 2
# endif
#endif

#ifdef Q_C_CALLBACKS
extern "C" {
#endif // Q_C_CALLBACKS

  QT_SIGNAL_RETTYPE qt_C_sigchldHnd(QT_SIGNAL_ARGS);

#ifdef Q_C_CALLBACKS
}
#endif // Q_C_CALLBACKS



/* class VKMembuf
   - Reimplementation of QMembuf, from qt/kernel/qinternal_p.h
*/

/* This class implements an efficient buffering of data that is often
   used by asynchronous IO classes like QSocket, QHttp and QProcess. */
VKMembuf::VKMembuf() : _size(0), _index(0)
{
  buf = new QPtrList<QByteArray>;
  buf->setAutoDelete( true );
}

VKMembuf::~VKMembuf()
{ delete buf; }

/* This function consumes nbytes bytes of data from the buffer and
   copies it into sink. If sink is a 0 pointer the data goes into the
   nirvana. */
bool VKMembuf::consumeBytes( Q_ULONG nbytes, char *sink )
{
  if ( nbytes <= 0 || nbytes > _size )
    return false;
  _size -= nbytes;
  for ( ;; ) {
    QByteArray *a = buf->first();
    if ( _index + nbytes >= a->size() ) {
      // Here we skip the whole byte array and get the next later
      int len = a->size() - _index;
      if ( sink ) {
        memcpy( sink, a->data()+_index, len );
        sink += len;
      }
      nbytes -= len;
      buf->remove();
      _index = 0;
      if ( nbytes == 0 )
        break;
    } else {
      // Here we skip only a part of the first byte array
      if ( sink )
        memcpy( sink, a->data()+_index, nbytes );
      _index += nbytes;
      break;
    }
  }
  return true;
}

/* Scans for any occurrence of '\n' in the buffer. If store is not 0
   the text up to the first '\n' (or terminating 0) is written to
   store, and a terminating 0 is appended to store if necessary.
   Returns true if a '\n' was found; otherwise returns false. */
bool VKMembuf::scanNewline( QByteArray *store )
{
  if ( _size == 0 )
    return false;
  int i = 0; // index into 'store'
  QByteArray *a = 0;
  char *p;
  int n;
  for ( ;; ) {
    if ( !a ) {
      a = buf->first();
      if ( !a || a->size() == 0 )
        return false;
      p = a->data() + _index;
      n = a->size() - _index;
    } else {
      a = buf->next();
      if ( !a || a->size() == 0 )
        return false;
      p = a->data();
      n = a->size();
    }
    if ( store ) {
      while ( n-- > 0 ) {
        *(store->data()+i) = *p;
        if ( ++i == (int)store->size() )
          store->resize( (store->size() < 256) ? (1024) : (store->size()*4) );
        switch ( *p ) {
        case '\0':
          store->resize( i );
          return false;
        case '\n':
          *(store->data()+i) = '\0';
          store->resize( i );
          return true;
        }
        p++;
      }
    } else {
      while ( n-- > 0 ) {
        switch ( *p++ ) {
        case '\0':
          return false;
        case '\n':
          return true;
        }
      }
    }
  }
}

int VKMembuf::ungetch( int ch )
{
  if ( buf->isEmpty() || _index==0 ) {
    // we need a new QByteArray
    QByteArray *ba = new QByteArray( 1 );
    buf->insert( 0, ba );
    _size++;
    ba->at( 0 ) = ch;
  } else {
    // we can reuse a place in the buffer
    QByteArray *ba = buf->first();
    _index--;
    _size++;
    ba->at( _index ) = ch;
  }
  return ch;
}





/* class VKProc -------------------------------------------------------------- */

VKProc::VKProc( pid_t p, VKProcess *proc/*=0*/ )
  : pid(p), process(proc)
{
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProc: Constructor for pid %d and VKProcess %p", pid, process );
#endif
  socketFDin = 0;
  socketFDout = 0;
  socketStdin = 0;
  socketStdout = 0;
  socketStderr = 0;
}

VKProc::~VKProc()
{
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProc: Destructor for pid %d and VKProcess %p", pid, process );
#endif
  if ( process ) {
    if ( process->d->notifierFDin )
      process->d->notifierFDin->setEnabled( false );
    if ( process->d->notifierFDout )
      process->d->notifierFDout->setEnabled( false );
    if ( process->d->notifierStdin )
      process->d->notifierStdin->setEnabled( false );
    if ( process->d->notifierStdout )
      process->d->notifierStdout->setEnabled( false );
    if ( process->d->notifierStderr )
      process->d->notifierStderr->setEnabled( false );
    process->d->proc = 0;
  }
  if ( socketFDin )
    ::close( socketFDin );
  if ( socketFDout )
    ::close( socketFDout );
  if ( socketStdin )
    ::close( socketStdin );
  if ( socketStdout )
    ::close( socketStdout );
  if ( socketStderr )
    ::close( socketStderr );
}





/*----------------------------------------------------------------------------
 * Helper functions
 ----------------------------------------------------------------------------- */

static void vkprocess_cleanup()
{
  delete VKProcessPrivate::procManager;
  VKProcessPrivate::procManager = 0;
}

#ifdef Q_OS_QNX6
#define BAILOUT close(tmpSocket);close(socketFD[1]);return -1;
int qnx6SocketPairReplacement (int socketFD[2]) 
{
  int tmpSocket;
  tmpSocket = socket (AF_INET, SOCK_STREAM, 0);
  if (tmpSocket == -1)
    return -1;
  socketFD[1] = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD[1] == -1) { BAILOUT };

  sockaddr_in ipAddr;
  memset(&ipAddr, 0, sizeof(ipAddr));
  ipAddr.sin_family = AF_INET;
  ipAddr.sin_addr.s_addr = INADDR_ANY;

  int socketOptions = 1;
  setsockopt(tmpSocket, SOL_SOCKET, SO_REUSEADDR, &socketOptions, sizeof(int));

  bool found = false;
  for (int socketIP = 2000; (socketIP < 2500) && !(found); socketIP++) {
    ipAddr.sin_port = htons(socketIP);
    if (bind(tmpSocket, (struct sockaddr *)&ipAddr, sizeof(ipAddr)))
      found = true;
  }

  if (listen(tmpSocket, 5)) { BAILOUT };

  // Select non-blocking mode
  int originalFlags = fcntl(socketFD[1], F_GETFL, 0);
  fcntl(socketFD[1], F_SETFL, originalFlags | O_NONBLOCK);

  // Request connection
  if (connect(socketFD[1], (struct sockaddr*)&ipAddr, sizeof(ipAddr)))
    if (errno != EINPROGRESS) { BAILOUT };

  // Accept connection
  socketFD[0] = accept(tmpSocket, (struct sockaddr *)NULL, (size_t *)NULL);
  if (socketFD[0] == -1) { BAILOUT };

  // We're done
  close(tmpSocket);

  // Restore original flags , ie return to blocking
  fcntl(socketFD[1], F_SETFL, originalFlags);
  return 0;
}
#undef BAILOUT
#endif


/* sigchld handler callback */
QT_SIGNAL_RETTYPE qt_C_sigchldHnd( QT_SIGNAL_ARGS )
{
  if ( VKProcessPrivate::procManager == 0 )
    return;
  if ( VKProcessPrivate::procManager->sigchldFd[0] == 0 )
    return;

  char a = 1;
  ::write( VKProcessPrivate::procManager->sigchldFd[0], &a, sizeof(a) );
}




/* class VKProcessManager ---------------------------------------------------- */

VKProcessManager::VKProcessManager() : sn(0)
{
  procList = new QPtrList<VKProc>;
  procList->setAutoDelete( true );

  /* The SIGCHLD handler writes to a socket to tell the manager that
     something happened. This is done to get the processing in sync
     with the event reporting. */
#ifndef Q_OS_QNX6
  if ( ::socketpair( AF_UNIX, SOCK_STREAM, 0, sigchldFd ) ) {
#else
  if ( qnx6SocketPairReplacement (sigchldFd) ) {
#endif
    sigchldFd[0] = 0;
    sigchldFd[1] = 0;
  } else {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcessManager: install socket notifier (%d)", sigchldFd[1] );
#endif
    sn = new QSocketNotifier( sigchldFd[1],
                              QSocketNotifier::Read, this );
    connect( sn, SIGNAL(activated(int)),
             this, SLOT(sigchldHnd(int)) );
    sn->setEnabled( true );
  }

  // install a SIGCHLD handler and ignore SIGPIPE
  struct sigaction act;

#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager: install a SIGCHLD handler" );
#endif
  act.sa_handler = qt_C_sigchldHnd;
  sigemptyset( &(act.sa_mask) );
  sigaddset( &(act.sa_mask), SIGCHLD );
  act.sa_flags = SA_NOCLDSTOP;
#if defined(SA_RESTART)
  act.sa_flags |= SA_RESTART;
#endif
  if ( sigaction( SIGCHLD, &act, &oldactChld ) != 0 )
    qWarning( "Error installing SIGCHLD handler" );
  
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager: install a SIGPIPE handler (SIG_IGN)" );
#endif
  act.sa_handler = QT_SIGNAL_IGNORE;
  sigemptyset( &(act.sa_mask) );
  sigaddset( &(act.sa_mask), SIGPIPE );
  act.sa_flags = 0;
  if ( sigaction( SIGPIPE, &act, &oldactPipe ) != 0 )
    qWarning( "Error installing SIGPIPE handler" );
}
  
VKProcessManager::~VKProcessManager()
{
  delete procList;

  if ( sigchldFd[0] != 0 )
    ::close( sigchldFd[0] );
  if ( sigchldFd[1] != 0 )
    ::close( sigchldFd[1] );

  // restore SIGCHLD handler
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager: restore old sigchild handler" );
#endif
  if ( sigaction( SIGCHLD, &oldactChld, 0 ) != 0 )
    qWarning( "Error restoring SIGCHLD handler" );

#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager: restore old sigpipe handler" );
#endif
  if ( sigaction( SIGPIPE, &oldactPipe, 0 ) != 0 )
    qWarning( "Error restoring SIGPIPE handler" );
}

void VKProcessManager::append( VKProc *p )
{
  procList->append( p );
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager: append process (procList.count(): %d)", 
          procList->count() );
#endif
}

void VKProcessManager::remove( VKProc *p )
{
  procList->remove( p );
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager: remove process (procList.count(): %d)", 
          procList->count() );
#endif
  cleanup();
}

void VKProcessManager::cleanup()
{
  if ( procList->count() == 0 ) {
    QTimer::singleShot( 0, this, SLOT(removeMe()) );
  }
}

void VKProcessManager::removeMe()
{
  if ( procList->count() == 0 ) {
    qRemovePostRoutine(vkprocess_cleanup);
    VKProcessPrivate::procManager = 0;
    delete this;
  }
}

void VKProcessManager::sigchldHnd( int fd )
{
  /* Disable the socket notifier to make sure that this function is
     not called recursively -- this can happen, if you enter the event
     loop in the slot connected to the processExited() signal (e.g. by
     showing a modal dialog) and there are more than one process which
     exited in the meantime. */
  if ( sn ) {
    if ( !sn->isEnabled() )
      return;
    sn->setEnabled( false );
  }

  char tmp;
  ::read( fd, &tmp, sizeof(tmp) );
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessManager::sigchldHnd()" );
#endif
  VKProc *proc;
  VKProcess *process;
  bool removeProc;
  proc = procList->first();
  while ( proc != 0 ) {
    removeProc = false;
    process = proc->process;
    if ( process != 0 ) {
      if ( !process->isRunning() ) {
#if defined(VK_PROCESS_DEBUG)
        qDebug( "VKProcessManager::sigchldHnd() (PID: %d): process exited (VKProcess available)", proc->pid );
#endif
        /* Apparently, there is not consistency among different
           operating systems on how to use FIONREAD.

           FreeBSD, Linux and Solaris all expect the 3rd argument to
           ioctl() to be an int, which is normally 32-bit even on
           64-bit machines.

           IRIX, on the other hand, expects a size_t, which is 64-bit
           on 64-bit machines.

           So, the solution is to use size_t initialized to zero to
           make sure all bits are set to zero, preventing underflow
           with the FreeBSD/Linux/Solaris ioctls. */
        size_t nbytes = 0;
        // read pending data
        if ( proc->socketFDout && ::ioctl(proc->socketFDout, FIONREAD, 
                                          (char*)&nbytes)==0 && nbytes>0 ) {
#if defined(VK_PROCESS_DEBUG)
          qDebug( "VKProcessManager::sigchldHnd() (PID: %d): reading %d bytes of pending data on FDout(%d)", proc->pid, nbytes, process->getFDout() );
#endif
          process->socketRead( proc->socketFDout );
        }
        
        nbytes = 0;
        if ( proc->socketStdout && ::ioctl(proc->socketStdout, FIONREAD, 
                                           (char*)&nbytes)==0 && nbytes>0 ) {
#if defined(VK_PROCESS_DEBUG)
          qDebug( "VKProcessManager::sigchldHnd() (PID: %d): reading %d bytes of pending data on stdout", proc->pid, nbytes );
#endif
          process->socketRead( proc->socketStdout );
        }

        nbytes = 0;
        if ( proc->socketStderr && ::ioctl(proc->socketStderr, FIONREAD, 
                                           (char*)&nbytes)==0 && nbytes>0 ) {
#if defined(VK_PROCESS_DEBUG)
          qDebug( "VKProcessManager::sigchldHnd() (PID: %d): reading %d bytes of pending data on stderr", proc->pid, nbytes );
#endif
          process->socketRead( proc->socketStderr );
        }

        /* close filedescriptors if open, and disable the socket notifiers */
        if ( proc->socketFDout ) {
          ::close( proc->socketFDout );
          proc->socketFDout = 0;
          if (process->d->notifierFDout)
            process->d->notifierFDout->setEnabled(false);
        }
        if ( proc->socketStdout ) {
          ::close( proc->socketStdout );
          proc->socketStdout = 0;
          if (process->d->notifierStdout)
            process->d->notifierStdout->setEnabled(false);
        }
        if ( proc->socketStderr ) {
          ::close( proc->socketStderr );
          proc->socketStderr = 0;
          if (process->d->notifierStderr)
            process->d->notifierStderr->setEnabled(false);
        }

        if ( process->notifyOnExit )
          emit process->processExited();

        removeProc = true;
      }
    } else {
      int status;
      if ( ::waitpid( proc->pid, &status, WNOHANG ) == proc->pid ) {
#if defined(VK_PROCESS_DEBUG)
        qDebug( "VKProcessManager::sigchldHnd() (PID: %d): process exited (VKProcess not available)", proc->pid );
#endif
        removeProc = true;
      }
    }
    if ( removeProc ) {
      VKProc *oldproc = proc;
      proc = procList->next();
      remove( oldproc );
    } else {
      proc = procList->next();
    }
  }
  if ( sn )
    sn->setEnabled( true );
}




/* class VKProcessPrivate ---------------------------------------------------- */
VKProcessManager *VKProcessPrivate::procManager = 0;

VKProcessPrivate::VKProcessPrivate()
{
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessPrivate: Constructor" );
#endif
  fdinBufRead = 0;
  stdinBufRead = 0;

  notifierFDin = 0;
  notifierFDout = 0;
  notifierStdin = 0;
  notifierStdout = 0;
  notifierStderr = 0;

  exitValuesCalculated = false;
  socketReadCalled = false;

  proc = 0;
}

VKProcessPrivate::~VKProcessPrivate()
{
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcessPrivate: Destructor" );
#endif

  if ( proc != 0 ) {
    if ( proc->socketFDin != 0 ) {
      ::close( proc->socketFDin );
      proc->socketFDin = 0;
    }
    if ( proc->socketStdin != 0 ) {
      ::close( proc->socketStdin );
      proc->socketStdin = 0;
    }
    proc->process = 0;
  }

  while ( !fdinBuf.isEmpty() ) {
    delete fdinBuf.dequeue();
  }
  while ( !stdinBuf.isEmpty() ) {
    delete stdinBuf.dequeue();
  }
  if (notifierFDin) {
    delete notifierFDin;
    notifierFDin = 0;
  }
  if (notifierStdin) {
    delete notifierStdin;
    notifierStdin = 0;
  }
  if (notifierFDout) {
    delete notifierFDout;
    notifierFDout = 0;
  }
  if (notifierStdout) {
    delete notifierStdout;
    notifierStdout = 0;
  }
  if (notifierStderr) {
    delete notifierStderr;
    notifierStderr = 0;
  }
}


/* Closes all open sockets in the child process that are not needed by
   the child process. Otherwise one child may have an open socket on
   standard input, etc. of another child. */
void VKProcessPrivate::closeOpenSocketsForChild()
{
  if ( procManager != 0 ) {
    if ( procManager->sigchldFd[0] != 0 )
      ::close( procManager->sigchldFd[0] );
    if ( procManager->sigchldFd[1] != 0 )
      ::close( procManager->sigchldFd[1] );
    
    // close also the sockets from other VKProcess instances
    for ( VKProc *p=procManager->procList->first(); p!=0; 
          p=procManager->procList->next() ) {
      ::close( p->socketFDin );
      ::close( p->socketStdin );
      ::close( p->socketFDout );
      ::close( p->socketStdout );
      ::close( p->socketStderr );
    }
  }
}


void VKProcessPrivate::newProc( pid_t pid, VKProcess *process )
{
  proc = new VKProc( pid, process );
  if ( procManager == 0 ) {
    procManager = new VKProcessManager;
    qAddPostRoutine(vkprocess_cleanup);
  }
  // the VKProcessManager takes care of deleting the VKProc instances
  procManager->append( proc );
}






/* class VKProcess -----------------------------------------------------------
   Constructs a VKProcess object. The parent and name parameters are
   passed to the QObject constructor. */
VKProcess::VKProcess( QObject* parent, const char* name )
  : QObject( parent, name ), 
    ioRedirection( false ), 
    notifyOnExit( false ),
    wroteToFDinConnected( false ), 
    wroteToStdinConnected( false ),
    readFDoutCalled( false ), 
    readStdoutCalled( false ), 
    readStderrCalled( false ),
    comms( Stdin|Stdout|Stderr ),
    filedesc_in( 0 ), 
    filedesc_out( 1 ),
    disabledStdin( false ), 
    disabledStdout( false ), 
    disabledStderr( false )
{ init(); }


/* Constructs a VKProcess with arg0 as the command to be executed. The
   parent and name parameters are passed to the QObject constructor.
   The process is not started. You must call start() or launch() to
   start the process. */
VKProcess::VKProcess( const QString& arg0, QObject *parent, const char *name )
  : QObject( parent, name ), ioRedirection( false ), notifyOnExit( false ),
    wroteToFDinConnected( false ), wroteToStdinConnected( false ),
    readFDoutCalled( false ), readStdoutCalled( false ), readStderrCalled( false ),
    comms( Stdin|Stdout|Stderr ),
    filedesc_in( 0 ), filedesc_out( 1 ),
    disabledStdin( false ), disabledStdout( false ), disabledStderr( false )
{
  init();
  addArgument( arg0 );
}


/* Constructs a VKProcess with args as the arguments of the
   process. The first element in the list is the command to be
   executed. The other elements in the list are the arguments to this
   command. The parent and name parameters are passed to the QObject
   constructor.

   The process is not started. You must call start() or launch() to
   start the process. */
VKProcess::VKProcess( const QStringList& args, QObject *parent, const char *name )
  : QObject( parent, name ), ioRedirection( false ), notifyOnExit( false ),
    wroteToFDinConnected( false ), wroteToStdinConnected( false ),
    readFDoutCalled( false ), readStdoutCalled( false ), readStderrCalled( false ),
    comms( Stdin|Stdout|Stderr ),
    filedesc_in( 0 ), filedesc_out( 1 ),
    disabledStdin( false ), disabledStdout( false ), disabledStderr( false )
{
  init();
  setArguments( args );
}


/* This private class does basic initialization. */
void VKProcess::init()
{
  d = new VKProcessPrivate();
  exitStat = 0;
  exitNormal = false;
}


/* This private class resets the process variables, etc. so that it
  can be used for another process to start. */
void VKProcess::reset()
{
  delete d;
  d = new VKProcessPrivate();
  exitStat = 0;
  exitNormal = false;
  d->bufFDout.clear();
  d->bufStdout.clear();
  d->bufStderr.clear();
}


/* Returns the list of arguments that are set for the process.
   Arguments can be specified with the constructor or with the
   functions setArguments() and addArgument(). */
QStringList VKProcess::arguments() const
{
  return _arguments;
}


/* Clears the list of arguments that are set for the process. */
void VKProcess::clearArguments()
{
  _arguments.clear();
}


/* Sets args as the arguments for the process. The first element in
   the list is the command to be executed. The other elements in the
   list are the arguments to the command. Any previous arguments are
   deleted.

   VKProcess does not perform argument substitutions; for example, if
   you specify "*" or "$DISPLAY", these values are passed to the
   process literally. If you want to have the same behavior as the
   shell provides, you must do the substitutions yourself;
   i.e. instead of specifying a "*" you must specify the list of all
   the filenames in the current directory, and instead of "$DISPLAY"
   you must specify the value of the environment variable DISPLAY. */
void VKProcess::setArguments( const QStringList& args )
{
  _arguments = args;
}


/* Adds arg to the end of the list of arguments.
   The first element in the list of arguments is the command to be
   executed; the following elements are the command's arguments. */
void VKProcess::addArgument( const QString& arg )
{
  _arguments.append( arg );
}


/* Returns the working directory that was set with
   setWorkingDirectory(), or the current directory if none has been
   explicitly set. */
QDir VKProcess::workingDirectory() const
{
  return workingDir;
}


/* Sets dir as the working directory for processes. This does not
   affect running processes; only processes that are started
   afterwards are affected.
   Setting the working directory is especially useful for processes
   that try to access files with relative paths. */
void VKProcess::setWorkingDirectory( const QDir& dir )
{
  workingDir = dir;
}


/* Returns the communication required with the process, i.e. some
   combination of the Communication flags. */
int VKProcess::communication() const
{
  return comms;
}


/* If FDout/in overlaps with Stdout/err/in, FDout/in take precedence */
void VKProcess::reprioritiseComms()
{
  /* If FDin enabled, take care of (dis/re)enabling stdin */
  if (comms & FDin) {
    if (comms & Stdin) {   /* Stdin enabled: disable it? */
      if (filedesc_in == STDIN_FILENO) {
        comms &= ~Stdin;
        disabledStdin = true;
      }
    } else {               /* Stdin not enabled: re-enable it? */
      if (disabledStdin && filedesc_in != STDIN_FILENO) {
        comms |= Stdin;
        disabledStdin = false;
      }
    }
  }

  /* If FDout enabled, take care of (dis/re)enabling stdout/stderr */
  if (comms & FDout) {
    if (comms & Stdout) {  /* Stdout enabled: disable it? */
      if (filedesc_out == STDOUT_FILENO) {
        comms &= ~Stdout;
        disabledStdout = true;
      }
    } else {               /* Stdout not enabled: re-enable it? */
      if (disabledStdout && filedesc_out != STDOUT_FILENO) {
        comms |= Stdout;
        disabledStdout = false;
      }
    }
    if (comms & Stderr) {  /* Stderr enabled: disable it? */
      if (filedesc_out == STDERR_FILENO) {
        comms &= ~Stderr;
        disabledStderr = true;
      }
    } else {               /* Stderr not enabled: re-enable it? */
      if (disabledStderr && filedesc_out != STDERR_FILENO) {
        comms |= Stderr;
        disabledStderr = false;
      }
    }
  }
}


/* Sets commFlags as the communication required with the process.
   commFlags is a bitwise OR of the flags defined by the Communication
   enum.  The default is { Stdin | Stdout | Stderr }.

   NOTE: FDout|in take priority over Stdout/err/in, so the latter are
   NOT enabled if there's an overlap of fd's.  However, their
   disabling is recorded, and subsequent calls to setCommunication()
   and setFDin/out() will reflect this.  See reprioritiseComms().
   Returns actual communications set. */
int VKProcess::setCommunication( int commFlags )
{
  comms = commFlags;

  /* Reset disabling of std fd's */
  disabledStdin  = false;
  disabledStdout = false;
  disabledStderr = false;

  /* disable std fd's if overlap FDout/in */
  reprioritiseComms();

  return comms;
}


/* Sets input file descriptor
   The default is 0
   If fd == stdin, disable Stdin: FDin takes precedence
   
   NOTE: FDin takes priority over Stdin, so the latter may be DISABLED
   if there's an overlap of fd's.  See reprioritiseComms(). */
void VKProcess::setFDin( int fd )
{
  vk_assert(fd >= 0 && fd < 1024);  // <0..1023>
  filedesc_in = fd;

  /* disable/enable std fd's wrt overlapping FDout/in */
  reprioritiseComms();
}


/* Sets output file descriptor
   The default is 1
   If fd == stdout|stderr, disable Stdout|Stdin: FDout takes precedence

   NOTE: FDout takes priority over Stdout/err, so one of the latter
   may be DISABLED if there's an overlap of fd's.  See
   reprioritiseComms(). */
void VKProcess::setFDout( int fd )
{
  vk_assert(fd > 0 && fd < 1024);  // <1..1023>
  filedesc_out = fd;

  /* disable/enable std fd's wrt overlapping FDout/in */
  reprioritiseComms();
}


int VKProcess::getFDin()
{ return filedesc_in; }


int VKProcess::getFDout()
{ return filedesc_out; }


/* Returns true if the process has exited normally; otherwise returns
   false. This implies that this function returns false if the process
   is still running. */
bool VKProcess::normalExit() const
{
  // isRunning() has the side effect that it determines the exit status!
  if ( isRunning() )
    return false;
  else
    return exitNormal;
}


/* Returns the exit status of the process or 0 if the process is still
   running. This function returns immediately and does not wait until
   the process is finished.
   If normalExit() is false (e.g. if the program was killed or
   crashed), this function returns 0, so you should check the return
   value of normalExit() before relying on this value. */
int VKProcess::exitStatus() const
{
  // isRunning() has the side effect that it determines the exit status!
  if ( isRunning() )
    return 0;
  else
    return exitStat;
}


/* Reads the data that the process has written to FDout.  When new
   data is written to standard output, the class emits the signal
   readyReadFDout().

   If there is no data to read, this function returns a QByteArray of
   size 0: it does not wait until there is something to read. */
QByteArray VKProcess::readFDout()
{
  if ( readFDoutCalled ) {
    return QByteArray();
  }
  readFDoutCalled = true;
  VKMembuf *buf = membufFDout();
  readFDoutCalled = false;

  return buf->readAll();
}


/* Reads the data that the process has written to standard output.
   When new data is written to standard output, the class emits the
   signal readyReadStdout().

   If there is no data to read, this function returns a QByteArray of
   size 0: it does not wait until there is something to read. */
QByteArray VKProcess::readStdout()
{
  if ( readStdoutCalled ) {
    return QByteArray();
  }
  readStdoutCalled = true;
  VKMembuf *buf = membufStdout();
  readStdoutCalled = false;

  return buf->readAll();
}


/* Reads the data that the process has written to standard error.
   When new data is written to standard error, the class emits the
   signal readyReadStderr().

   If there is no data to read, this function returns a QByteArray of
   size 0: it does not wait until there is something to read. */
QByteArray VKProcess::readStderr()
{
  if ( readStderrCalled ) {
    return QByteArray();
  }
  readStderrCalled = true;
  VKMembuf *buf = membufStderr();
  readStderrCalled = false;

  return buf->readAll();
}


/* Reads a line of text from FDout, excluding any trailing newline or
   carriage return characters, and returns it. Returns QString::null
   if canReadLineFDout() returns false.

   By default, the text is interpreted to be in Latin-1 encoding. If
   you need other codecs, you can set a different codec with
   QTextCodec::setCodecForCStrings(). */
QString VKProcess::readLineFDout()
{
  QByteArray a( 256 );
  VKMembuf *buf = membufFDout();
  if ( !buf->scanNewline( &a ) ) {
    if ( !canReadLineFDout() )
      return QString::null;

    if ( !buf->scanNewline( &a ) )
      return QString( buf->readAll() );
  }

  uint size = a.size();
  buf->consumeBytes( size, 0 );

  // get rid of terminating \n or \r\n
  if ( size>0 && a.at( size - 1 ) == '\n' ) {
    if ( size>1 && a.at( size - 2 ) == '\r' )
      a.at( size - 2 ) = '\0';
    else
      a.at( size - 1 ) = '\0';
  }
  return QString( a );
}


/* Reads a line of text from standard output, excluding any trailing
   newline or carriage return characters, and returns it. Returns
   QString::null if canReadLineStdout() returns false.

   By default, the text is interpreted to be in Latin-1 encoding. If
   you need other codecs, you can set a different codec with
   QTextCodec::setCodecForCStrings(). */
QString VKProcess::readLineStdout()
{
  QByteArray a( 256 );
  VKMembuf *buf = membufStdout();
  if ( !buf->scanNewline( &a ) ) {
    if ( !canReadLineStdout() )
      return QString::null;
    
    if ( !buf->scanNewline( &a ) )
      return QString( buf->readAll() );
  }

  uint size = a.size();
  buf->consumeBytes( size, 0 );

  // get rid of terminating \n or \r\n
  if ( size>0 && a.at( size - 1 ) == '\n' ) {
    if ( size>1 && a.at( size - 2 ) == '\r' )
      a.at( size - 2 ) = '\0';
    else
      a.at( size - 1 ) = '\0';
  }
  return QString( a );
}


/* Reads a line of text from standard error, excluding any trailing
   newline or carriage return characters and returns it. Returns
   QString::null if canReadLineStderr() returns false.

   By default, the text is interpreted to be in Latin-1 encoding. If
   you need other codecs, you can set a different codec with
   QTextCodec::setCodecForCStrings(). */
QString VKProcess::readLineStderr()
{
  QByteArray a( 256 );
  VKMembuf *buf = membufStderr();
  if ( !buf->scanNewline( &a ) ) {
    if ( !canReadLineStderr() )
      return QString::null;

    if ( !buf->scanNewline( &a ) )
      return QString( buf->readAll() );
  }

  uint size = a.size();
  buf->consumeBytes( size, 0 );

  // get rid of terminating \n or \r\n
  if ( size>0 && a.at( size - 1 ) == '\n' ) {
    if ( size>1 && a.at( size - 2 ) == '\r' )
      a.at( size - 2 ) = '\0';
    else
      a.at( size - 1 ) = '\0';
  }
  return QString( a );
}


/* void VKProcess::launchFinished()

  This signal is emitted when the process was started with launch().
  If the start was successful, this signal is emitted after all the
  data has been written to standard input. If the start failed, then
  this signal is emitted immediately.

  This signal is especially useful if you want to know when you can
  safely delete the VKProcess object when you are not interested in
  reading from standard output or standard error.
*/


/* Runs the process and writes the data buf to the process's standard
   input. If all the data is written to standard input, standard input
   is closed. The command is searched for in the path for executable
   programs; you can also use an absolute path in the command itself.

   If env is null, then the process is started with the same
   environment as the starting process. If env is non-null, then the
   values in the string list are interpreted as environment setttings
   of the form {key=value} and the process is started with these
   environment settings. For convenience, there is a small exception
   to this rule under Unix: if env does not contain any settings for
   the environment variable LD_LIBRARY_PATH, then this variable is
   inherited from the starting process.

   Returns true if the process could be started; otherwise returns
   false.

   Note that you should not use the slots writeToStdin() and
   closeStdin() on processes started with launch(), since the result
   is not well-defined. If you need these slots, use start() instead.

   The process may or may not read the buf data sent to its standard
   input.

   You can call this function even when a process that was started
   with this instance is still running. Be aware that if you do this
   the standard input of the process that was launched first will be
   closed, with any pending data being deleted, and the process will
   be left to run out of your control. Similarly, if the process could
   not be started the standard input will be closed and the pending
   data deleted. (On operating systems that have zombie processes, Qt
   will also wait() on the old process.)

   The object emits the signal launchFinished() when this function
   call is finished. If the start was successful, this signal is
   emitted after all the data has been written to standard input. If
   the start failed, then this signal is emitted immediately. */
bool VKProcess::launch( const QByteArray& buf, QStringList *env )
{
  if ( start( env ) ) {
    if ( !buf.isEmpty() ) {
      connect( this, SIGNAL(wroteToStdin()),
               this, SLOT(closeStdinLaunch()) );
      writeToStdin( buf );
    } else {
      closeStdin();
      emit launchFinished();
    }
    return true;
  } else {
    emit launchFinished();
    return false;
  }
}


/* The data buf is written to standard input with writeToStdin() using
   the QString::local8Bit() representation of the strings. */
bool VKProcess::launch( const QString& buf, QStringList *env )
{
  if ( start( env ) ) {
    if ( !buf.isEmpty() ) {
      connect( this, SIGNAL(wroteToStdin()),
               this, SLOT(closeStdinLaunch()) );
      writeToStdin( buf );
    } else {
      closeStdin();
      emit launchFinished();
    }
    return true;
  } else {
    emit launchFinished();
    return false;
  }
}


/* Private slot - used by the launch() functions to close standard input. */
void VKProcess::closeStdinLaunch()
{
  disconnect( this, SIGNAL(wroteToStdin()),
              this, SLOT(closeStdinLaunch()) );
  closeStdin();
  emit launchFinished();
}


/* void VKProcess::readyReadStdout()

   This signal is emitted when the process has written data to
   standard output. You can read the data with readStdout().

   Note that this signal is only emitted when there is new data and
   not when there is old, but unread data. In the slot connected to
   this signal, you should always read everything that is available at
   that moment to make sure that you don't lose any data. 
*/


/* void VKProcess::readyReadStderr()

   This signal is emitted when the process has written data to
   standard error. You can read the data with readStderr().

   Note that this signal is only emitted when there is new data and
   not when there is old, but unread data. In the slot connected to
   this signal, you should always read everything that is available at
   that moment to make sure that you don't lose any data. 
*/


/* void VKProcess::processExited()
   This signal is emitted when the process has exited.
*/


/* void VKProcess::wroteToStdin()

   This signal is emitted if the data sent to standard input (via
   writeToStdin()) was actually written to the process. This does not
   imply that the process really read the data, since this class only
   detects when it was able to write the data to the operating
   system. But it is now safe to close standard input without losing
   pending data. 
*/


/* The string buf is handled as text using the QString::local8Bit()
   representation. */
void VKProcess::writeToFDin( const QString& buf )
{
  QByteArray tmp = buf.local8Bit();
  tmp.resize( buf.length() );
  writeToFDin( tmp );
}


/* The string buf is handled as text using the QString::local8Bit()
   representation. */
void VKProcess::writeToStdin( const QString& buf )
{
  QByteArray tmp = buf.local8Bit();
  tmp.resize( buf.length() );
  writeToStdin( tmp );
}


/* Under Windows the implementation is not so nice: it is not that
   easy to detect when one of the signals should be emitted; therefore
   there are some timers that query the information.  To keep it a
   little efficient, use the timers only when they are needed.  They
   are needed, if you are interested in the signals. So use
   connectNotify() and disconnectNotify() to keep track of your
   interest. */
void VKProcess::connectNotify( const char * signal )
{
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcess::connectNotify(): signal %s has been connected", signal );
#endif
  if ( !ioRedirection )
    if ( qstrcmp( signal, SIGNAL(readyReadFDout())  )==0 ||
         qstrcmp( signal, SIGNAL(readyReadStdout()) )==0 ||
         qstrcmp( signal, SIGNAL(readyReadStderr()) )==0
         ) {
#if defined(VK_PROCESS_DEBUG)
      qDebug( "VKProcess::connectNotify(): set ioRedirection to true" );
#endif
      setIoRedirection( true );
      return;
    }
  if ( !notifyOnExit && qstrcmp( signal, SIGNAL(processExited()) )==0 ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::connectNotify(): set notifyOnExit to true" );
#endif
    setNotifyOnExit( true );
    return;
  }
  if ( !wroteToFDinConnected  && qstrcmp( signal, SIGNAL(wroteToFDin())  )==0 ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::connectNotify(): set wroteToFDinConnected to true" );
#endif
    setWroteFDinConnected( true );
    return;
  }
  if ( !wroteToStdinConnected && qstrcmp( signal, SIGNAL(wroteToStdin()) )==0 ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::connectNotify(): set wroteToStdinConnected to true" );
#endif
    setWroteStdinConnected( true );
    return;
  }
}


void VKProcess::disconnectNotify( const char * )
{
  if ( ioRedirection &&
       receivers( SIGNAL(readyReadFDout())  ) ==0 &&
       receivers( SIGNAL(readyReadStdout()) ) ==0 &&
       receivers( SIGNAL(readyReadStderr()) ) ==0
       ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::disconnectNotify(): set ioRedirection to false" );
#endif
    setIoRedirection( false );
  }
  if ( notifyOnExit && receivers( SIGNAL(processExited()) ) == 0 ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::disconnectNotify(): set notifyOnExit to false" );
#endif
    setNotifyOnExit( false );
  }
  if ( wroteToFDinConnected && receivers( SIGNAL(wroteToFDin()) ) == 0 ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::disconnectNotify(): set wroteToFDinConnected to false" );
#endif
    setWroteFDinConnected( false );
  }
  if ( wroteToStdinConnected && receivers( SIGNAL(wroteToStdin()) ) == 0 ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::disconnectNotify(): set wroteToStdinConnected to false" );
#endif
    setWroteStdinConnected( false );
  }
}


VKMembuf* VKProcess::membufFDout()
{
  if ( d->proc && d->proc->socketFDout ) {
    /* Apparently, there is not consistency among different operating
       systems on how to use FIONREAD.
       FreeBSD, Linux and Solaris all expect the 3rd argument to
       ioctl() to be an int, which is normally 32-bit even on 64-bit
       machines.
       IRIX, on the other hand, expects a size_t, which is 64-bit on
       64-bit machines.
       So, the solution is to use size_t initialized to zero to make
       sure all bits are set to zero, preventing underflow with the
       FreeBSD/Linux/Solaris ioctls. */
    size_t nbytes = 0;
    if ( ::ioctl(d->proc->socketFDout, FIONREAD, (char*)&nbytes)==0 && nbytes>0 )
      socketRead( d->proc->socketFDout );
  }
  return &d->bufFDout;
}


VKMembuf* VKProcess::membufStdout()
{
  if ( d->proc && d->proc->socketStdout ) {
    /* Apparently, there is not consistency among different operating
       systems on how to use FIONREAD.
       FreeBSD, Linux and Solaris all expect the 3rd argument to
       ioctl() to be an int, which is normally 32-bit even on 64-bit
       machines.
       IRIX, on the other hand, expects a size_t, which is 64-bit on
       64-bit machines.
       So, the solution is to use size_t initialized to zero to make
       sure all bits are set to zero, preventing underflow with the
       FreeBSD/Linux/Solaris ioctls. */
    size_t nbytes = 0;
    if ( ::ioctl(d->proc->socketStdout, FIONREAD, (char*)&nbytes)==0 && nbytes>0 )
      socketRead( d->proc->socketStdout );
  }
  return &d->bufStdout;
}


VKMembuf* VKProcess::membufStderr()
{
  if ( d->proc && d->proc->socketStderr ) {
    /* Apparently, there is not consistency among different operating
       systems on how to use FIONREAD.
       FreeBSD, Linux and Solaris all expect the 3rd argument to
       ioctl() to be an int, which is normally 32-bit even on 64-bit
       machines.
       IRIX, on the other hand, expects a size_t, which is 64-bit on
       64-bit machines.
       So, the solution is to use size_t initialized to zero to make
       sure all bits are set to zero, preventing underflow with the
       FreeBSD/Linux/Solaris ioctls. */
    size_t nbytes = 0;
    if ( ::ioctl(d->proc->socketStderr, FIONREAD, (char*)&nbytes)==0 && nbytes>0 )
      socketRead( d->proc->socketStderr );
  }
  return &d->bufStderr;
}


/* Destroys the instance.
   If the process is running, it is <b>not</b> terminated! The
   standard input, standard output and standard error of the process
   are closed.
   You can connect the destroyed() signal to the kill() slot, if you
   want the process to be terminated automatically when the instance
   is destroyed. */
VKProcess::~VKProcess()
{
  delete d;
}


/* Tries to run a process for the command and arguments that were
   specified with setArguments(), addArgument() or that were specified
   in the constructor. The command is searched for in the path for
   executable programs; you can also use an absolute path in the
   command itself.

   If env is null, then the process is started with the same
   environment as the starting process. If env is non-null, then the
   values in the stringlist are interpreted as environment setttings
   of the form {key=value} and the process is started in these
   environment settings. For convenience, there is a small exception
   to this rule: under Unix, if env does not contain any settings for
   the environment variable LD_LIBRARY_PATH, then this variable is
   inherited from the starting process; under Windows the same applies
   for the environment variable PATH.

   Returns true if the process could be started; otherwise returns
   false.

   You can write data to the process's standard input with
   writeToStdin(). You can close standard input with closeStdin() and
   you can terminate the process with tryTerminate(), or with kill().

   You can call this function even if you've used this instance to
   create a another process which is still running. In such cases,
   VKProcess closes the old process's standard input and deletes
   pending data, i.e., you lose all control over the old process, but
   the old process is not terminated. This applies also if the process
   could not be started. (On operating systems that have zombie
   processes, Qt will also wait() on the old process.) */
bool VKProcess::start( QStringList *env )
{
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcess::start()" );
#endif
  reset();

  int sFDin[2];
  int sStdin[2];
  int sFDout[2];
  int sStdout[2];
  int sStderr[2];

  // open sockets for piping
#ifndef Q_OS_QNX6
  if ( (comms & Stdin) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sStdin ) == -1 ) {
#else
  if ( (comms & Stdin) && qnx6SocketPairReplacement(sStdin) == -1 ) {
#endif
    return false;
  }

#ifndef Q_OS_QNX6
  if ( (comms & Stderr) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sStderr ) == -1 ) {
#else
  if ( (comms & Stderr) && qnx6SocketPairReplacement(sStderr) == -1 ) {
#endif
    if ( comms & Stdin ) {
      ::close( sStdin[0] );
      ::close( sStdin[1] );
    }
    return false;
  }

#ifndef Q_OS_QNX6
  if ( (comms & Stdout) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sStdout ) == -1 ) {
#else
  if ( (comms & Stdout) && qnx6SocketPairReplacement(sStdout) == -1 ) {
#endif
    if ( comms & Stdin ) {
      ::close( sStdin[0] );
      ::close( sStdin[1] );
    }
    if ( comms & Stderr ) {
      ::close( sStderr[0] );
      ::close( sStderr[1] );
    }
    return false;
  }

#ifndef Q_OS_QNX6
  if ( (comms & FDin) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sFDin ) == -1 ) {
#else
  if ( (comms & FDin) && qnx6SocketPairReplacement(sFDin) == -1 ) {
#endif
    if ( comms & Stdin ) {
      ::close( sStdin[0] );
      ::close( sStdin[1] );
    }
    if ( comms & Stderr ) {
      ::close( sStderr[0] );
      ::close( sStderr[1] );
    }
    if ( comms & Stdout ) {
      ::close( sStdout[0] );
      ::close( sStdout[1] );
    }
    return false;
  }

#ifndef Q_OS_QNX6
  if ( (comms & FDout) && ::socketpair( AF_UNIX, SOCK_STREAM, 0, sFDout ) == -1 ) {
#else
  if ( (comms & FDout) && qnx6SocketPairReplacement(sFDout) == -1 ) {
#endif
    if ( comms & Stdin ) {
      ::close( sStdin[0] );
      ::close( sStdin[1] );
    }
    if ( comms & Stderr ) {
      ::close( sStderr[0] );
      ::close( sStderr[1] );
    }
    if ( comms & Stdout ) {
      ::close( sStdout[0] );
      ::close( sStdout[1] );
    }
    if ( comms & FDin ) {
      ::close( sFDin[0] );
      ::close( sFDin[1] );
    }
    return false;
  }
  
  /* the following pipe is only used to determine if the process could
     be started */
  int fd[2];
  if ( pipe( fd ) < 0 ) {
    // non critical error, go on
    fd[0] = 0;
    fd[1] = 0;
  }

  // construct the arguments for exec
  QCString *arglistQ = new QCString[ _arguments.count() + 1 ];
  const char** arglist = new const char*[ _arguments.count() + 1 ];
  int i = 0;
  for ( QStringList::Iterator it = _arguments.begin(); 
        it != _arguments.end(); ++it ) {
    arglistQ[i] = (*it).local8Bit();
    arglist[i] = arglistQ[i];
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::start(): arg %d = %s", i, arglist[i] );
#endif
    i++;
  }
#ifdef Q_OS_MACX
  if(i) {
    QCString arg_bundle = arglistQ[0];
    QFileInfo fi(arg_bundle);
    if(fi.exists() && fi.isDir() && arg_bundle.right(4) == ".app") {
      QCString exe = arg_bundle;
      int lslash = exe.findRev('/');
      if(lslash != -1)
        exe = exe.mid(lslash+1);
      exe = QCString(arg_bundle + "/Contents/MacOS/" + exe);
      exe = exe.left(exe.length() - 4); //chop off the .app
      if(QFile::exists(exe)) {
        arglistQ[0] = exe;
        arglist[0] = arglistQ[0];
      }
    }
  }
#endif
  arglist[i] = 0;

  /* Must make sure signal handlers are installed before exec'ing in
     case the process exits quickly. */
  if ( d->procManager == 0 ) {
    d->procManager = new VKProcessManager;
    qAddPostRoutine(vkprocess_cleanup);
  }

  // fork and exec
  QApplication::flushX();
  pid_t pid = fork();
  if ( pid == 0 ) {
    // child
    d->closeOpenSocketsForChild();
    if ( comms & FDin ) {
      ::close( sFDin[1] );
      ::dup2( sFDin[0], filedesc_in );
    }
    if ( comms & FDout ) {
      ::close( sFDout[0] );
      ::dup2( sFDout[1], filedesc_out );
    }
    if ( comms & Stdin ) {
      ::close( sStdin[1] );
      ::dup2( sStdin[0], STDIN_FILENO );
    }
    if ( comms & Stdout ) {
      ::close( sStdout[0] );
      ::dup2( sStdout[1], STDOUT_FILENO );
    }
    if ( comms & Stderr ) {
      ::close( sStderr[0] );
      ::dup2( sStderr[1], STDERR_FILENO );
    }
    if ( comms & DupStderr ) {
      ::dup2( STDOUT_FILENO, STDERR_FILENO );
    }
    ::chdir( workingDir.absPath().latin1() );
    if ( fd[0] )
      ::close( fd[0] );
    if ( fd[1] )
      ::fcntl( fd[1], F_SETFD, FD_CLOEXEC ); // close on exec shows sucess
    
    if ( env == 0 ) { // inherit environment and start process
      QString command = _arguments[0];
#if defined(Q_OS_MACX) //look in a bundle
      const QString mac_bundle_suffix = ".app/Contents/MacOS/";
      if(!QFile::exists(command) && QFile::exists(command + mac_bundle_suffix)) {
        QString exec = command;
        int lslash = command.findRev('/');
        if(lslash != -1)
          exec = command.mid(lslash+1);
        QFileInfo fileInfo( command + mac_bundle_suffix + exec );
        if ( fileInfo.isExecutable() )
          command = fileInfo.absFilePath().local8Bit();
      }
#endif
#ifndef Q_OS_QNX4
      ::execvp( command, (char*const*)arglist ); // ### cast not nice
#else
      ::execvp( command, (char const*const*)arglist ); // ### cast not nice
#endif
    } else { // start process with environment settins as specified in env
      // construct the environment for exec
      int numEntries = env->count();
#if defined(Q_OS_MACX)
      QString ld_library_path("DYLD_LIBRARY_PATH");
#else
      QString ld_library_path("LD_LIBRARY_PATH");
#endif
      bool setLibraryPath =
        env->grep( QRegExp( "^" + ld_library_path + "=" ) ).empty() &&
        getenv( ld_library_path ) != 0;
      if ( setLibraryPath )
        numEntries++;
      QCString *envlistQ = new QCString[ numEntries + 1 ];
      const char** envlist = new const char*[ numEntries + 1 ];
      int i = 0;
      if ( setLibraryPath ) {
        envlistQ[i] = QString( ld_library_path + "=%1" ).arg( getenv( ld_library_path ) ).local8Bit();
        envlist[i] = envlistQ[i];
        i++;
      }
      for ( QStringList::Iterator it = env->begin(); it != env->end(); ++it ) {
        envlistQ[i] = (*it).local8Bit();
        envlist[i] = envlistQ[i];
        i++;
      }
      envlist[i] = 0;
      
      // look for the executable in the search path
      if ( _arguments.count()>0 && getenv("PATH")!=0 ) {
        QString command = _arguments[0];
        if ( !command.contains( '/' ) ) {
          QStringList pathList = QStringList::split( ':', getenv( "PATH" ) );
          for (QStringList::Iterator it = pathList.begin(); 
               it != pathList.end(); ++it ) {
            QString dir = *it;
#if defined(Q_OS_MACX) //look in a bundle
            if ( !QFile::exists(dir + "/" + command) && 
                 QFile::exists(dir + "/" + command + ".app") )
              dir += "/" + command + ".app/Contents/MacOS";
#endif
            QFileInfo fileInfo( dir, command );
            if ( fileInfo.isExecutable() ) {
#if defined(Q_OS_MACX)
              arglistQ[0] = fileInfo.absFilePath().local8Bit();
#else
              arglistQ[0] = fileInfo.filePath().local8Bit();
#endif
              arglist[0] = arglistQ[0];
              break;
            }
          }
        }
      }
#if defined(Q_OS_MACX)
      if ( ! QFile::exists( arglist[0] ) ) {
        QString command = arglist[0];
        const QString mac_bundle_suffix = ".app/Contents/MacOS/";
        if ( QFile::exists(command + mac_bundle_suffix ) ) {
          QString exec = command;
          int lslash = command.findRev('/');
          if(lslash != -1)
            exec = command.mid(lslash+1);
          QFileInfo fileInfo( command + mac_bundle_suffix + exec );
          if ( fileInfo.isExecutable() ) {
            arglistQ[0] = fileInfo.absFilePath().local8Bit();
            arglist[0] = arglistQ[0];
          }
        }
      }
#endif
#ifndef Q_OS_QNX4
      ::execve( arglist[0], (char*const*)arglist, (char*const*)envlist ); // ### casts not nice
#else
      ::execve( arglist[0], (char const*const*)arglist,(char const*const*)envlist ); // ### casts not nice
#endif
    }
    if ( fd[1] ) {
      char buf = 0;
      ::write( fd[1], &buf, 1 );
      ::close( fd[1] );
    }
    ::_exit( -1 );
  } else if ( pid == -1 ) {
    // error forking
    goto error;
  }

  // test if exec was successful
  if ( fd[1] )
    ::close( fd[1] );
  if ( fd[0] ) {
    char buf;
    for ( ;; ) {
      int n = ::read( fd[0], &buf, 1 );
      if ( n==1 ) {
        // socket was not closed => error
        if ( ::waitpid( pid, 0, WNOHANG ) != pid ) {
          /* The wait did not succeed yet, so try again when we get
             the sigchild (to avoid zombies). */
          d->newProc( pid, 0 );
        }
        d->proc = 0;
        goto error;
      } else if ( n==-1 ) {
        if ( errno==EAGAIN || errno==EINTR )
          // try it again
          continue;
      }
      break;
    }
    ::close( fd[0] );
  }
  
  d->newProc( pid, this );
  
  if ( comms & FDin ) {
    ::close( sFDin[0] );
    d->proc->socketFDin = sFDin[1];

    // Select non-blocking mode
    int originalFlags = fcntl(d->proc->socketFDin, F_GETFL, 0);
    fcntl(d->proc->socketFDin, F_SETFL, originalFlags | O_NONBLOCK);

    d->notifierFDin = new QSocketNotifier( sFDin[1], QSocketNotifier::Write );
    connect( d->notifierFDin, SIGNAL(activated(int)),
             this, SLOT(socketWrite(int)) );
    // setup notifiers for the sockets
    if ( !d->fdinBuf.isEmpty() ) {
      d->notifierFDin->setEnabled( true );
    }
  }
  if ( comms & Stdin ) {
    ::close( sStdin[0] );
    d->proc->socketStdin = sStdin[1];

    // Select non-blocking mode
    int originalFlags = fcntl(d->proc->socketStdin, F_GETFL, 0);
    fcntl(d->proc->socketStdin, F_SETFL, originalFlags | O_NONBLOCK);
    
    d->notifierStdin = new QSocketNotifier( sStdin[1], QSocketNotifier::Write );
    connect( d->notifierStdin, SIGNAL(activated(int)),
             this, SLOT(socketWrite(int)) );
    // setup notifiers for the sockets
    if ( !d->stdinBuf.isEmpty() ) {
      d->notifierStdin->setEnabled( true );
    }
  }
  if ( comms & FDout ) {
    ::close( sFDout[1] );
    d->proc->socketFDout = sFDout[0];
    d->notifierFDout = new QSocketNotifier( sFDout[0], QSocketNotifier::Read );
    connect( d->notifierFDout, SIGNAL(activated(int)),
             this, SLOT(socketRead(int)) );
    if ( ioRedirection )
      d->notifierFDout->setEnabled( true );
  }
  if ( comms & Stdout ) {
    ::close( sStdout[1] );
    d->proc->socketStdout = sStdout[0];
    d->notifierStdout = new QSocketNotifier( sStdout[0], QSocketNotifier::Read );
    connect( d->notifierStdout, SIGNAL(activated(int)),
             this, SLOT(socketRead(int)) );
    if ( ioRedirection )
      d->notifierStdout->setEnabled( true );
  }
  if ( comms & Stderr ) {
    ::close( sStderr[1] );
    d->proc->socketStderr = sStderr[0];
    d->notifierStderr = new QSocketNotifier( sStderr[0], QSocketNotifier::Read );
    connect( d->notifierStderr, SIGNAL(activated(int)),
             this, SLOT(socketRead(int)) );
    if ( ioRedirection )
      d->notifierStderr->setEnabled( true );
  }

  // cleanup and return
  delete[] arglistQ;
  delete[] arglist;
  return true;

error:
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcess::start(): error starting process" );
#endif
  if ( d->procManager )
    d->procManager->cleanup();
  if ( comms & FDin ) {
    ::close( sFDin[1] );
    ::close( sFDin[0] );
  }
  if ( comms & Stdin ) {
    ::close( sStdin[1] );
    ::close( sStdin[0] );
  }
  if ( comms & FDout ) {
    ::close( sFDout[0] );
    ::close( sFDout[1] );
  }
  if ( comms & Stdout ) {
    ::close( sStdout[0] );
    ::close( sStdout[1] );
  }
  if ( comms & Stderr ) {
    ::close( sStderr[0] );
    ::close( sStderr[1] );
  }
  ::close( fd[0] );
  ::close( fd[1] );
  delete[] arglistQ;
  delete[] arglist;
  return false;
}


/* Asks the process to terminate. Processes can ignore this if they
   wish. If you want to be certain that the process really terminates,
   you can use kill() instead.

   The slot returns immediately: it does not wait until the process
   has finished. When the process terminates, the processExited()
   signal is emitted. */
void VKProcess::tryTerminate() const
{
  if ( d->proc != 0 )
    ::kill( d->proc->pid, SIGTERM );
}

/* Terminates the process. This is not a safe way to end a process
   since the process will not be able to do any cleanup.
   tryTerminate() is safer, but processes can ignore a
   tryTerminate(). */
void VKProcess::kill() const
{
  if ( d->proc != 0 )
    ::kill( d->proc->pid, SIGKILL );
}

/* Returns true if the process is running; otherwise returns false. */
bool VKProcess::isRunning() const
{
  if ( d->exitValuesCalculated ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::isRunning(): false (already computed)" );
#endif
    return false;
  }
  if ( d->proc == 0 )
    return false;
  int status;
  if ( ::waitpid( d->proc->pid, &status, WNOHANG ) == d->proc->pid ) {
    // compute the exit values
    VKProcess *that = (VKProcess*)this; // mutable
    that->exitNormal = WIFEXITED( status ) != 0;
    if ( exitNormal ) {
      that->exitStat = (char)WEXITSTATUS( status );
    }
    d->exitValuesCalculated = true;

    /* On heavy processing, the socket notifier for the sigchild might
       not have found time to fire yet. */
    if ( d->procManager && d->procManager->sigchldFd[1] < FD_SETSIZE ) {
      fd_set fds;
      struct timeval tv;
      FD_ZERO( &fds );
      FD_SET( d->procManager->sigchldFd[1], &fds );
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      if ( ::select( d->procManager->sigchldFd[1]+1, &fds, 0, 0, &tv ) > 0 )
        d->procManager->sigchldHnd( d->procManager->sigchldFd[1] );
    }

#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::isRunning() (PID: %d): false", d->proc->pid );
#endif
    return false;
  }
#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcess::isRunning() (PID: %d): true", d->proc->pid );
#endif
  return true;
}


/* Returns true if it's possible to read an entire line of text from
   FDout at this time; otherwise returns false. */
bool VKProcess::canReadLineFDout() const
{
  if ( !d->proc || !d->proc->socketFDout )
    return d->bufFDout.size() != 0;

  VKProcess *that = (VKProcess*)this;
  return that->membufFDout()->scanNewline( 0 );
}


/* Returns true if it's possible to read an entire line of text from
   standard output at this time; otherwise returns false. */
bool VKProcess::canReadLineStdout() const
{
  if ( !d->proc || !d->proc->socketStdout )
    return d->bufStdout.size() != 0;

  VKProcess *that = (VKProcess*)this;
  return that->membufStdout()->scanNewline( 0 );
}


/* Returns true if it's possible to read an entire line of text from
   standard error at this time; otherwise returns false. */
bool VKProcess::canReadLineStderr() const
{
  if ( !d->proc || !d->proc->socketStderr )
    return d->bufStderr.size() != 0;

  VKProcess *that = (VKProcess*)this;
  return that->membufStderr()->scanNewline( 0 );
}


/* Writes the data buf to the process's standard input. The process
   may or may not read this data.

   This function always returns immediately. The data you pass to
   writeToStdin() is copied into an internal memory buffer in
   VKProcess, and when control goes back to the event loop, VKProcess
   will starting transferring data from this buffer to the running
   process.   Sometimes the data will be transferred in several
   payloads, depending on how much data is read at a time by the
   process itself. When VKProcess has transferred all the data from
   its memory buffer to the running process, it emits wroteToStdin().
    
   Note that some operating systems use a buffer to transfer the
   data. As a result, wroteToStdin() may be emitted before the running
   process has actually read all the data. */
void VKProcess::writeToFDin( const QByteArray& buf )
{
#if defined(VK_PROCESS_DEBUG)
  // qDebug( "VKProcess::writeToFDin(): write to fdin (%d)", d->socketFDin );
#endif
  d->fdinBuf.enqueue( new QByteArray(buf) );
  if ( d->notifierFDin != 0 )
    d->notifierFDin->setEnabled( true );
}

void VKProcess::writeToStdin( const QByteArray& buf )
{
#if defined(VK_PROCESS_DEBUG)
  // qDebug( "VKProcess::writeToStdin(): write to stdin (%d)", d->socketStdin );
#endif
  d->stdinBuf.enqueue( new QByteArray(buf) );
  if ( d->notifierStdin != 0 )
    d->notifierStdin->setEnabled( true );
}


/* Closes the process's FDin.

   This function also deletes any pending data that has not been
   written to FDin. */
void VKProcess::closeFDin()
{
  if ( d->proc == 0 )
    return;
  if ( d->proc->socketFDin !=0 ) {
    while ( !d->fdinBuf.isEmpty() ) {
      delete d->fdinBuf.dequeue();
    }
    delete d->notifierFDin;
    d->notifierFDin = 0;
    if ( ::close( d->proc->socketFDin ) != 0 ) {
      qWarning( "Could not close fdin of child process" );
    }
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::closeFDin(): fdin (%d) closed", d->proc->socketFDin );
#endif
    d->proc->socketFDin = 0;
  }
}


/* Closes the process's standard input.

   This function also deletes any pending data that has not been
   written to standard input. */
void VKProcess::closeStdin()
{
  if ( d->proc == 0 )
    return;
  if ( d->proc->socketStdin !=0 ) {
    while ( !d->stdinBuf.isEmpty() ) {
      delete d->stdinBuf.dequeue();
    }
    delete d->notifierStdin;
    d->notifierStdin = 0;
    if ( ::close( d->proc->socketStdin ) != 0 ) {
      qWarning( "Could not close stdin of child process" );
    }
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::closeStdin(): stdin (%d) closed", d->proc->socketStdin );
#endif
    d->proc->socketStdin = 0;
  }
}


/* This private slot is called when the process has outputted data to
   either standard output or standard error.
*/
void VKProcess::socketRead( int fd )
{
  if ( d->socketReadCalled ) {
    /* the slots that are connected to the readyRead...() signals
       might trigger a recursive call of socketRead(). Avoid this
       since you get a blocking read otherwise. */
    return;
  }

#if defined(VK_PROCESS_DEBUG)
  qDebug( "VKProcess::socketRead(): %d", fd );
#endif
  if ( fd == 0 )
    return;
  if ( !d->proc )
    return;
  VKMembuf *buffer = 0;
  int n;
  if ( fd == d->proc->socketFDout ) {
    buffer = &d->bufFDout;
  } else if ( fd == d->proc->socketStdout ) {
    buffer = &d->bufStdout;
  } else if ( fd == d->proc->socketStderr ) {
    buffer = &d->bufStderr;
  } else {
    // this case should never happen, but just to be safe
    return;
  }
#if defined(VK_PROCESS_DEBUG)
  uint oldSize = buffer->size();
#endif

  // try to read data first (if it fails, the filedescriptor was closed)
  const int basize = 4096;
  QByteArray *ba = new QByteArray( basize );
  n = ::read( fd, ba->data(), basize );
  if ( n > 0 ) {
    ba->resize( n );
    buffer->append( ba );
    ba = 0;
  } else {
    delete ba;
    ba = 0;
  }
  // eof or error?
  if ( n == 0 || n == -1 ) {
    if ( fd == d->proc->socketFDout ) {
#if defined(VK_PROCESS_DEBUG)
      qDebug( "VKProcess::socketRead(): FDout (%d) closed", fd );
#endif
      d->notifierFDout->setEnabled( false );
      delete d->notifierFDout;
      d->notifierFDout = 0;
      ::close( d->proc->socketFDout );
      d->proc->socketFDout = 0;
      return;
    } else if ( fd == d->proc->socketStdout ) {
#if defined(VK_PROCESS_DEBUG)
      qDebug( "VKProcess::socketRead(): stdout (%d) closed", fd );
#endif
      d->notifierStdout->setEnabled( false );
      delete d->notifierStdout;
      d->notifierStdout = 0;
      ::close( d->proc->socketStdout );
      d->proc->socketStdout = 0;
      return;
    } else if ( fd == d->proc->socketStderr ) {
#if defined(VK_PROCESS_DEBUG)
      qDebug( "VKProcess::socketRead(): stderr (%d) closed", fd );
#endif
      d->notifierStderr->setEnabled( false );
      delete d->notifierStderr;
      d->notifierStderr = 0;
      ::close( d->proc->socketStderr );
      d->proc->socketStderr = 0;
      return;
    }
  }

  if ( fd < FD_SETSIZE ) {
    fd_set fds;
    struct timeval tv;
    FD_ZERO( &fds );
    FD_SET( fd, &fds );
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    while ( ::select( fd+1, &fds, 0, 0, &tv ) > 0 ) {
      // prepare for the next round
      FD_ZERO( &fds );
      FD_SET( fd, &fds );
      // read data
      ba = new QByteArray( basize );
      n = ::read( fd, ba->data(), basize );
      if ( n > 0 ) {
        ba->resize( n );
        buffer->append( ba );
        ba = 0;
      } else {
        delete ba;
        ba = 0;
        break;
      }
    }
  }

  d->socketReadCalled = true;
  if ( fd == d->proc->socketFDout ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::socketRead(): %d bytes read from FDout (%d)",
            (int)(buffer->size()-oldSize), fd );
#endif
    emit readyReadFDout();
  } else if ( fd == d->proc->socketStdout ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::socketRead(): %d bytes read from stdout (%d)",
            (int)(buffer->size()-oldSize), fd );
#endif
    emit readyReadStdout();
  } else if ( fd == d->proc->socketStderr ) {
#if defined(VK_PROCESS_DEBUG)
    qDebug( "VKProcess::socketRead(): %d bytes read from stderr (%d)",
            (int)(buffer->size()-oldSize), fd );
#endif
    emit readyReadStderr();
  }
  d->socketReadCalled = false;
}


/* This private slot is called when the process tries to read data
   from standard input. */
void VKProcess::socketWrite( int fd )
{
  if (fd == d->proc->socketFDin) {
    while ( fd == d->proc->socketFDin && d->proc->socketFDin != 0 ) {
      if ( d->fdinBuf.isEmpty() ) {
        d->notifierFDin->setEnabled( false );
        return;
      }
      ssize_t ret = ::write( fd,
                             d->fdinBuf.head()->data() + d->fdinBufRead,
                             d->fdinBuf.head()->size() - d->fdinBufRead );
#if defined(VK_PROCESS_DEBUG)
      qDebug( "VKProcess::socketWrite(): wrote %d bytes to fdin (%d)", ret, fd );
#endif
      if ( ret == -1 )
        return;
      d->fdinBufRead += ret;
      if ( d->fdinBufRead == (ssize_t)d->fdinBuf.head()->size() ) {
        d->fdinBufRead = 0;
        delete d->fdinBuf.dequeue();
        if ( wroteToFDinConnected && d->fdinBuf.isEmpty() )
          emit wroteToFDin();
      }
    }
  }
  else if (fd == d->proc->socketStdin) {
    while ( fd == d->proc->socketStdin && d->proc->socketStdin != 0 ) {
      if ( d->stdinBuf.isEmpty() ) {
        d->notifierStdin->setEnabled( false );
        return;
      }
      ssize_t ret = ::write( fd,
                             d->stdinBuf.head()->data() + d->stdinBufRead,
                             d->stdinBuf.head()->size() - d->stdinBufRead );
#if defined(VK_PROCESS_DEBUG)
      qDebug( "VKProcess::socketWrite(): wrote %d bytes to stdin (%d)", ret, fd );
#endif
      if ( ret == -1 )
        return;
      d->stdinBufRead += ret;
      if ( d->stdinBufRead == (ssize_t)d->stdinBuf.head()->size() ) {
        d->stdinBufRead = 0;
        delete d->stdinBuf.dequeue();
        if ( wroteToStdinConnected && d->stdinBuf.isEmpty() )
          emit wroteToStdin();
      }
    }
  }
}


/* Flushes standard input. This is useful if you want to use VKProcess
   in a synchronous manner. */
void VKProcess::flushFDin()
{
  if (d->proc)
    socketWrite(d->proc->socketFDin);
}

void VKProcess::flushStdin()
{
  if (d->proc)
    socketWrite(d->proc->socketStdin);
}


/* Private slot - only used under Windows (but moc does not know about
  #if defined(). */
void VKProcess::timeout()
{ }


/* Private function - used by connectNotify() and disconnectNotify()
   to change the value of ioRedirection (and related behaviour) */
void VKProcess::setIoRedirection( bool value )
{
  ioRedirection = value;
  if ( ioRedirection ) {
    if ( d->notifierFDout )
      d->notifierFDout->setEnabled( true );
    if ( d->notifierStdout )
      d->notifierStdout->setEnabled( true );
    if ( d->notifierStderr )
      d->notifierStderr->setEnabled( true );
  } else {
    if ( d->notifierFDout )
      d->notifierFDout->setEnabled( false );
    if ( d->notifierStdout )
      d->notifierStdout->setEnabled( false );
    if ( d->notifierStderr )
      d->notifierStderr->setEnabled( false );
  }
}


/* This private function is used by connectNotify() and
   disconnectNotify() to change the value of notifyOnExit (and related
   behaviour) */
void VKProcess::setNotifyOnExit( bool value )
{
  notifyOnExit = value;
}


/* This private function is used by connectNotify() and
   disconnectNotify() to change the value of wroteToFDinConnected (and
   related behaviour) */
void VKProcess::setWroteFDinConnected( bool value )
{
  wroteToFDinConnected = value;
}


/* This private function is used by connectNotify() and
   disconnectNotify() to change the value of wroteToStdinConnected 
   (and related behaviour) */
void VKProcess::setWroteStdinConnected( bool value )
{
  wroteToStdinConnected = value;
}


/* enum VKProcess::PID */

/* The return value is the PID of the process, or -1 if no process
   belongs to this object. */
VKProcess::PID VKProcess::processIdentifier()
{
  if ( d->proc == 0 )
    return -1;
  return d->proc->pid;
}

