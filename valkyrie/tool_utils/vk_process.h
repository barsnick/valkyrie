/* ---------------------------------------------------------------------- 
 * Definition of class VKProcess                            vk_process.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2005-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 * ---------------------------------------------------------------------
 * This file is a re-implementation of QProcess:
 * ** $Id: qt/qprocess.h   3.3.4   edited May 27 2003 $
 * ** Created : 20000905
 * **
 * ** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
 * **
 * ** This file may be distributed and/or modified under the terms of the
 * ** GNU General Public License version 2 as published by the Free Software
 * ** Foundation and appearing in the file COPYING included in the
 * ** packaging of this file.
 */

#ifndef VK_PROCESS_H
#define VK_PROCESS_H

#include <qobject.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qptrqueue.h>
#include <qsocketnotifier.h>

#include <signal.h>
#include <stdlib.h>            // ssize_t


class VKMembuf
{
public:
   VKMembuf();
   ~VKMembuf();

   void append( QByteArray *ba );
   void clear();

   bool consumeBytes( Q_ULONG nbytes, char *sink );
   QByteArray readAll();
   bool scanNewline( QByteArray *store );
   bool canReadLine() const;

   int ungetch( int ch );

   QIODevice::Offset size() const;

private:
   QPtrList<QByteArray> *buf;
   QIODevice::Offset _size;
   QIODevice::Offset _index;
};

inline void VKMembuf::append( QByteArray *ba )
{ buf->append( ba ); _size += ba->size(); }

inline void VKMembuf::clear()
{ buf->clear(); _size=0; _index=0; }

inline QByteArray VKMembuf::readAll()
{ QByteArray ba(_size); consumeBytes(_size,ba.data()); return ba; }

inline bool VKMembuf::canReadLine() const
{ return ((VKMembuf*)this)->scanNewline( 0 ); }

inline QIODevice::Offset VKMembuf::size() const
{ return _size; }




/* class VKProc --------------------------------------------------------------
   The class VKProcess does not necessarily map exactly to the running
   child processes: if the process is finished, the VKProcess class
   may still be there; furthermore a user can use VKProcess to start
   more than one process.
   The helper-class VKProc has the semantics that one instance of this
   class maps directly to a running child process. */
class VKProcess;

class VKProc
{
public:
   VKProc( pid_t p, VKProcess *proc=0 );
   ~VKProc();

   pid_t pid;
   int socketFDin;
   int socketFDout;
   int socketStdin;
   int socketStdout;
   int socketStderr;
   VKProcess *process;
};



/* class VKProcessManager ---------------------------------------------------- */
class VKProcessManager : public QObject
{
   Q_OBJECT
public:
   VKProcessManager();
   ~VKProcessManager();

   void append( VKProc *p );
   void remove( VKProc *p );

   void cleanup();

   public slots:
      void removeMe();
   void sigchldHnd( int );

public:
   struct sigaction oldactChld;
   struct sigaction oldactPipe;
   QPtrList<VKProc> *procList;
   int sigchldFd[2];

private:
   QSocketNotifier *sn;
};




/* class VKProcess ----------------------------------------------------------- */
class VKProcessPrivate
{
public:
   VKProcessPrivate();
   ~VKProcessPrivate();

   void closeOpenSocketsForChild();
   void newProc( pid_t pid, VKProcess *process );

   VKMembuf bufFDout;
   VKMembuf bufStdout;
   VKMembuf bufStderr;

   QPtrQueue<QByteArray> fdinBuf;
   QPtrQueue<QByteArray> stdinBuf;

   QSocketNotifier *notifierFDin;
   QSocketNotifier *notifierFDout;
   QSocketNotifier *notifierStdin;
   QSocketNotifier *notifierStdout;
   QSocketNotifier *notifierStderr;

   ssize_t fdinBufRead;
   ssize_t stdinBufRead;
   VKProc *proc;

   bool exitValuesCalculated;
   bool socketReadCalled;

   static VKProcessManager *procManager;
};


class VKProcess : public QObject
{
   Q_OBJECT
public:
   VKProcess( QObject *parent=0, const char *name=0 );
   VKProcess( const QString& arg0, QObject *parent=0, const char *name=0 );
   VKProcess( const QStringList& args, QObject *parent=0, const char *name=0 );
   ~VKProcess();

   // set and get the arguments and working directory
   QStringList arguments() const;
   void clearArguments();
   virtual void setArguments( const QStringList& args );
   virtual void addArgument( const QString& arg );
#ifndef QT_NO_DIR
   QDir workingDirectory() const;
   virtual void setWorkingDirectory( const QDir& dir );
#endif

   // set and get the comms wanted
   enum Communication { Stdin=0x01, Stdout=0x02, Stderr=0x04, DupStderr=0x08, FDin=0x10, FDout=0x20 };
   int setCommunication( int c );
   int communication() const;
   void setFDin( int fd );
   void setFDout( int fd );
   int getFDin();
   int getFDout();

   // start the execution
   virtual bool start( QStringList *env=0 );
   virtual bool launch( const QString& buf, QStringList *env=0  );
   virtual bool launch( const QByteArray& buf, QStringList *env=0  );

   // inquire the status
   bool isRunning() const;
   bool normalExit() const;
   int exitStatus() const;

   // reading
   virtual QByteArray readFDout();
   virtual QByteArray readStdout();
   virtual QByteArray readStderr();

   bool canReadLineFDout() const;
   bool canReadLineStdout() const;
   bool canReadLineStderr() const;

   virtual QString readLineFDout();
   virtual QString readLineStdout();
   virtual QString readLineStderr();
  
   // get platform dependent process information
#if defined(Q_OS_WIN32)
   typedef void* PID;
#else
   typedef Q_LONG PID;
#endif
   PID processIdentifier();

   void flushFDin();
   void flushStdin();

signals:
   void readyReadFDout();
   void readyReadStdout();
   void readyReadStderr();
   void processExited();
   void wroteToFDin();
   void wroteToStdin();
   void launchFinished();

public slots:
   // end the execution
   void stop( int msec_kill_timeout=2000 );
   void tryTerminate() const;
   void kill() const;

   // input
   virtual void writeToFDin( const QByteArray& buf );
   virtual void writeToFDin( const QString& buf );
   virtual void closeFDin();
   virtual void writeToStdin( const QByteArray& buf );
   virtual void writeToStdin( const QString& buf );
   virtual void closeStdin();

protected: // ### or private?
   void connectNotify( const char * signal );
   void disconnectNotify( const char * signal );

private:
   void reprioritiseComms();

   void setIoRedirection( bool value );
   void setNotifyOnExit( bool value );
   void setWroteFDinConnected( bool value );
   void setWroteStdinConnected( bool value );

   void init();
   void reset();
#if defined(Q_OS_WIN32)
   uint readStddev( HANDLE dev, char *buf, uint bytes );
#endif
   VKMembuf* membufFDout();
   VKMembuf* membufStdout();
   VKMembuf* membufStderr();

private slots:
   void socketRead( int fd );
   void socketWrite( int fd );
   void timeout();
   void closeStdinLaunch();

private:
   VKProcessPrivate *d;
#ifndef QT_NO_DIR
   QDir workingDir;
#endif
   QStringList _arguments;

   int  exitStat;              // exit status
   bool exitNormal;            // normal exit?
   bool ioRedirection;         // automatically set be (dis)connectNotify
   bool notifyOnExit;          // automatically set be (dis)connectNotify
   bool wroteToFDinConnected;  // automatically set be (dis)connectNotify
   bool wroteToStdinConnected; // automatically set be (dis)connectNotify

   bool readFDoutCalled;
   bool readStdoutCalled;
   bool readStderrCalled;
   int comms;
   int filedesc_in, filedesc_out;

   friend class VKProcessPrivate;
#if defined(Q_OS_UNIX)
   friend class VKProcessManager;
   friend class VKProc;
#endif

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
   VKProcess( const VKProcess & );
   VKProcess &operator=( const VKProcess & );
#endif

   /* Keep track of FDout/in disabling stdout/err/in */
   bool disabledStdin;
   bool disabledStdout;
   bool disabledStderr;
};


#endif // VK_PROCESS_H
