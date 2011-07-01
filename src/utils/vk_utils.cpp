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

#include "utils/vk_utils.h"
#include "options/vk_option.h"
#include "utils/vk_config.h"        // vkname()

#include <cstdlib>                  // exit, mkstemp, free/malloc, etc

#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegExp>
#include <QString>

/*
#include <stdlib.h>                 // mkstemp()
#include <stdarg.h>                 // va_start, va_end
#include <sys/types.h>              // getpid
#include <unistd.h>                 // getpid
#include <pwd.h>                    // getpwuid

#include <qapplication.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>
*/


/* prints various info msgs to stdout --------------------------------- */
void vkPrint( const char* msg, ... )
{
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stdout, "===%s:%d=== ", qPrintable( VkCfg::appName() ), ( int )getpid() );
   vfprintf( stdout, msg, ap );
   va_end( ap );
   fprintf( stdout, "\n" );
   fflush( stdout );
}


/* prints error msg -------------------------------------------------- */
void vkPrintErr( const char* msg, ... )
{
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stderr, "===%s:%d=== ", qPrintable( VkCfg::appName() ), ( int )getpid() );
   vfprintf( stderr, msg, ap );
   va_end( ap );
   fprintf( stderr, "\n" );
   fflush( stderr );
}


/* prints debug msg -------------------------------------------------- */
void vkDebug( const char* msg, ... )
{
#ifdef DEBUG_ON
   va_list ap;
   va_start( ap, msg );
   va_end( ap );
   fprintf( stderr, "===%s:%d=== ", qPrintable( VkCfg::appName() ), ( int )getpid() );
   vfprintf( stderr, msg, ap );
   va_end( ap );
   fprintf( stderr, "\n" );
#endif
}

/* prints an "Assertion failed" message and exits ---------------------- */
__attribute__(( noreturn ) )
void vk_assert_fail( const char* expr, const char* file,
                     unsigned int line, const char* fn )
{
   vkPrintErr( "Assertion failed '%s':", expr );
   vkPrintErr( "   at %s#%u:%s\n", file, line, fn );
   exit( 1 );
}

/* prints a message asking user to email a bug report,
 * and then exits. ----------------------------------------------------- */
__attribute__(( noreturn ) )
void vk_assert_never_reached_fail( const char* file,
                                   unsigned int line,
                                   const char* fn )
{
   vkPrintErr( "Assertion 'never reached' failed," );
   vkPrintErr( "   at %s#%u:%s", file, line, fn );
   vkPrintErr( "%s version: %s", qPrintable( VkCfg::appName() ), qPrintable( VkCfg::appVersion() ) );
   vkPrintErr( "Built with QT version:   %s", QT_VERSION_STR );
   vkPrintErr( "Running with QT version: %s", qVersion() );
   vkPrintErr( "Hopefully, you should never see this message." );
   vkPrintErr( "If you are, then Something Really Bad just happened." );
   vkPrintErr( "Please report this bug to: %s", qPrintable( VkCfg::email() ) );
   vkPrintErr( "In the bug report, please send the the above text," );
   vkPrintErr( "along with the output of `uname -a`." );
   vkPrintErr( "Thanks.\n" );
   exit( 1 );
}


/* Create a unique filename, with an optional extension ---------------- */
QString vk_mkstemp( QString filepath, QString ext/*=QString::null*/ )
{
   // create tempfiles with datetime, so can sort easily if they stay around
   
   QString datetime = QDateTime::currentDateTime().toString( "_yyyy.MM.dd_hh:mm:ss" );
   QString unique = filepath + datetime;
   
   if ( !ext.isNull() ) {
      unique +=  "." + ext;
   }
   
   if ( QFile::exists( unique ) ) {
      /* fall back on mkstemp */
      char* tmpname = vk_str_malloc( unique.length() + 10 );
      sprintf( tmpname, "%s.XXXXXX", qPrintable( unique ) );
      int fd = mkstemp( tmpname );
      
      if ( fd == -1 ) {
         /* something went wrong */
         VK_DEBUG( "failed to create unique filename from '%s'.",
                   qPrintable( filepath ) );
         return QString::null;
      }
      
      unique = QString( tmpname );
      tmpname = vk_str_free( tmpname );
   }
   
   return unique;
}


/* Version check -------------------------------------------------------
   Given version string of "major.minor.patch" (e.g. 3.3.0),
   hex version = (major << 16) + (minor << 8) + patch
*/
int strVersion2hex( QString ver_str )
{
   QRegExp rxver( ".*(\\d{1,2})\\.(\\d{1,2})\\.(\\d{1,2}).*" );
   
   if ( rxver.indexIn( ver_str ) == -1 ) {
      return -1;
   }
   
   int major = rxver.cap( 1 ).toInt();
   int minor = rxver.cap( 2 ).toInt();
   int patch = rxver.cap( 3 ).toInt();
   return ( major << 16 ) + ( minor << 8 ) + patch;
}



/* escape html entities
   current list: '<', '>', '&' ----------------------------------------- */
QString escapeEntities( const QString& content )
{
   QString ret_str = "";
   
   for ( int i = 0; i < content.length(); i++ ) {
      switch ( content[i].toLatin1() ) {
      case '<':
         ret_str += "&lt;";
         break;
      case '>':
         ret_str += "&gt;";
         break;
      case '&': {
         /* already escaped? */
         if (( content.mid( i + 1, 4 ) == "amp;" ) ||
             ( content.mid( i + 1, 3 ) == "lt;" ) ||
             ( content.mid( i + 1, 3 ) == "gt;" ) ) {
            ret_str += content[i];
         }
         else {
            ret_str += "&amp;";
         }
      }
      break;
      default:
         ret_str += content[i];
         break;
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
void* vk_free( const void* ptr )
{
   if ( ptr != NULL ) {
      free(( void* )ptr );
      ptr = NULL;
   }
   
   return NULL;
}

char* vk_str_free( const char* ptr )
{
   if ( ptr != NULL ) {
      free(( char* )ptr );
   }
   
   return NULL;
}

void* vk_malloc( unsigned long n_bytes )
{
   void* mem;
   mem = malloc( n_bytes );
   
   if ( !mem ) {
      VK_DEBUG( "failed to allocate %lu bytes", n_bytes );
   }
   
   return mem;
}


char* vk_str_malloc( int sz )
{
   char* arr;
   arr = ( char* ) malloc(( size_t )(( sz + 2 ) * sizeof( char ) ) );
   
   if ( !arr ) {
      VK_DEBUG( "malloc failure: virtual memory exhausted" );
      vk_assert_never_reached();
   }
   
   return arr;
}


/* wrapper to strcmp(): returns true || false */
bool vk_strcmp( const char* str1, const char* str2 )
{
   if ( !str1 || !str2 ) {
      VK_DEBUG( "can't call strcmp on null strings:\n"
                "str1 == %s, str2 == %s\n", str1, str2 );
      return false;
   }
   
   if (( strlen( str1 ) == 0 ) || ( strlen( str2 ) == 0 ) ) {
      VK_DEBUG( "one of these two strings is empty:\n"
                "\tstr1: -->%s<--, str2: -->%s<--\n", str1, str2 );
      return false;
   }
   
   return ( strcmp( str1, str2 ) == 0 ) ? true : false;
}


char* vk_strdup( const char* str )
{
   char* new_str;
   unsigned int length;
   
   if ( str ) {
      length = strlen( str ) + 1;
      new_str = vk_str_malloc( length );
      strcpy( new_str, str );
   }
   else {
      new_str = NULL;
   }
   
   return new_str;
}






//***************************************************************************
// helper functions
//***************************************************************************

/*!
  Translate boolean strings into booleans
  returns true if successfully parsed a 'true' string.
  returns false if successfully parsed a 'false' string.
  returns false if parsing failed.
  Use argment 'ok' to detect a parsing failure.
*/
bool strToBool( QString str, bool* ok/*=NULL*/ )
{
   if ( ok )
      *ok = true;

   if ( str == "true" || str == "on"   ||
        str == "yes"  || str == "1"    ||
        str == "T" ) {
      return true;
   }
   if ( str == "false" || str == "off" ||
        str == "no"    || str == "0"   ||
        str == "F" ) {
      return false;
   }

   if ( ok ) {
      *ok = false;
      VK_DEBUG( "Failed to parse string as a boolean." );
   }
   return false;
}


/*!
  Local helper function to find file_name: either directly or via $PATH
  Returns absolute path to file_name.
*/
static QString getFileAbsPath( const QString file_name )
{
   QString absPath = QString::null;

   if ( QFile::exists( file_name ) ) {
      // true for (good) directories, too.
      // file_name exists: get its absolute path.
      absPath = QFileInfo( file_name ).absoluteFilePath();
   }
   else if ( !file_name.contains('/') ) {
      // no '/' in file_name -> try $PATH env variable
      QString env = getenv( "PATH" );
      QStringList paths = env.split( ':' );

      foreach ( QString candidate, paths ) {
         candidate += "/" + file_name;
         if ( QFile::exists( candidate ) ) {
            // found it!  set its absolute path.
            absPath = QFileInfo( candidate ).absoluteFilePath();
            break;
         }
      }
   }

   return absPath;
}


/*!
  Checks file exists and has correct permissions.
  Returns absolute path to file, if it exists: else QString::null
*/
QString fileCheck( int* err_val, const QString fpath,
                   bool check_read/*=false*/,
                   bool check_write/*=false*/,
                   bool check_exe/*=false*/ )
{
   *err_val = PARSED_OK;
   QString absPath = QString::null;
   QFileInfo fi;

   // check exists: if so, return absolute path
   if ( fpath.isEmpty() ) {
      *err_val = PERROR_BADFILE;
      goto bye;
   }

   // try to find fpath: directly or via $PATH
   absPath = getFileAbsPath( fpath );
   if ( absPath.isNull() ) {
      // couldn't find fpath :-(
      *err_val = PERROR_BADFILE;
      goto bye;
   }

   fi.setFile( absPath );

   // check this is really a file
   if ( !fi.isFile() ) {
      *err_val = PERROR_BADFILE;
      goto bye;
   }

   // check for read permissions
   if ( check_read && !fi.isReadable() ) {
      *err_val = PERROR_BADFILERD;
      goto bye;
   }

   // check for write permissions
   if ( check_write && !fi.isWritable() ) {
      *err_val = PERROR_BADFILEWR;
      goto bye;
   }

   // check for executable permissions
   if ( check_exe && !fi.isExecutable() ) {
      *err_val = PERROR_BADEXEC;
      goto bye;
   }

bye:
   return absPath;
}


/*!
  Checks dir exists and has correct permissions.
  Returns absolute path to dir, if it exists: else QString::null
*/
QString dirCheck( int* err_val, const QString dir,
                  bool check_read/*=false*/,
                  bool check_write/*=false*/,
                  bool check_exe/*=false*/ )
{
   *err_val = PARSED_OK;
   QString absPath = QString::null;
   QFileInfo fi( dir );

   // check exists: if so, return absolute path
   if ( dir.isEmpty() || !fi.exists() || !fi.isDir() ) {
      *err_val = PERROR_BADDIR;
      goto bye;
   }

   // dir exists: set its absolute path
   absPath = fi.absolutePath();

   // check for read permissions
   if ( check_read && !fi.isReadable() ) {
      *err_val = PERROR_BADFILERD;
      goto bye;
   }

   // check for write permissions
   if ( check_write && !fi.isWritable() ) {
      *err_val = PERROR_BADFILEWR;
      goto bye;
   }

   // check for executable permissions
   if ( check_exe && !fi.isExecutable() ) {
      *err_val = PERROR_BADEXEC;
      goto bye;
   }

bye:
   return absPath;
}



/*!
  Dialog to choose a file
   - start_path gives the path to first show (default: current dir)
   - cfg_key_path is the (proj/glbl) cfg key for the item's path.
     This is used to find the 'filefilter/<cfg_key_path>' setting.
     If no key is given, default (and only) filter is "All files (*)".
   - mode indicates save / open (changes QFileDialog)
  Returns: chosen file path
*/
QString vkDlgGetFile( QWidget* parent,
                      const QString& start_path/*="./"*/,
                      const QString& cfg_key_path/*=QString()*/,
                      QFileDialog::AcceptMode mode/*=QFileDialog::AcceptOpen*/ )
{
   // defaults
   QString filterlist = "All Files (*)";   // default filter list
   QString filter = "";                    // default filter
   QString cfg_key_filterlist = "";        // glbl key, built from key_path
   QString cfg_key_filter = "";            // glbl key, built from key_path
   QString start_dir = start_path;
   
   if ( start_dir.isEmpty() ) {
      start_dir = "./";
   }
   else {
      // start_dir may be a path-less file, a bad dir, or who knows what.
      // try to resolve: first through given path, then via $PATH env var.
      start_dir = getFileAbsPath( start_dir );
   }

   // setup filters
   if ( !cfg_key_path.isEmpty() ){
      // check the global cfg (derive key from key_path)
      cfg_key_filterlist = cfg_key_path;
      cfg_key_filterlist = "filefilters/" + cfg_key_filterlist.replace("/", "_");
      cfg_key_filter = cfg_key_filterlist + "-default";

      QString cfg_filterlist = vkCfgGlbl->value( cfg_key_filterlist ).toString();
      QString cfg_filter = vkCfgGlbl->value( cfg_key_filter ).toString();

      if ( !cfg_filterlist.isEmpty() ) {
         filterlist = cfg_filterlist;
         if ( cfg_filterlist.contains( cfg_filter ) ) {
            filter = cfg_filter;
         }
      }
   }

   // Setup dialog
   QString caption = "Choose File";
   if ( mode == QFileDialog::AcceptSave )
      caption = "Save As";
   
   QFileDialog dlg( parent, caption, start_dir, filterlist );
   dlg.setFileMode(QFileDialog::AnyFile);
   dlg.setViewMode(QFileDialog::Detail);
   dlg.setAcceptMode( mode );
   dlg.selectFilter( filter );

   // Run dialog - get filename to save to: asks for overwrite confirmation
   QString fname;
   if ( dlg.exec() ) {
      QStringList fileNames = dlg.selectedFiles();
      if ( !fileNames.isEmpty() )
         fname = fileNames.first();
   }
   
   // save chosen filter (if changed) for next time
   if ( !cfg_key_path.isEmpty() ) {
      QString filter_new = dlg.selectedNameFilter();
      
      if ( filter_new != filter ) {
         vkCfgGlbl->setValue( cfg_key_filter, filter_new );
      }
   }

   return fname;
}


/*!
  Dialog to choose a file, using cached start_paths and filters
   - start_path gives the directory to first show (default: current dir)
   - cfg_key_path is the (proj/glbl) cfg key for the item's path.
     This is used to get:
     1) the directory to open the dialog in  (default: currentdir)
     2) the filter settings                  (default: "All files (*)")
   - mode indicates save / open (changes QFileDialog)
  Returns: chosen file path

  Note: don't use from options pages: they save to cache already.
*/
QString vkDlgCfgGetFile( QWidget* parent,
                         const QString& cfg_key_path/*=QString()*/,
                         QFileDialog::AcceptMode mode/*=QFileDialog::AcceptOpen*/ )
{
   // defaults
   QString start_path = "./";               // default dir
   bool pathIsDir = true;
   bool isProjKey = true;                  // is key_path a project key
   
   // setup start directory
   if ( !cfg_key_path.isEmpty() ){
      QFileInfo fi;
      // check first project cfg then glbl cfg for the path key
      if ( vkCfgProj->contains( cfg_key_path ) ) {
         fi = vkCfgProj->value( cfg_key_path ).toString();
      }
      else if ( vkCfgGlbl->contains( cfg_key_path ) ) {
         fi = vkCfgGlbl->value( cfg_key_path ).toString();
         isProjKey = false;
      }
      else {
         vkPrintErr( "Cfg key not found: '%s'. This shouldn't happen!",
                     qPrintable( cfg_key_path ) );
      }

      if ( fi.exists() ) {
         pathIsDir = fi.isDir();
         // looks like a bug in Qt: if dir, absolutePath still takes off the last dir!
         start_path = pathIsDir ? fi.absoluteFilePath() : fi.absolutePath();
      }
      else {
         vkDebug( "Bad path: '%s'", qPrintable( fi.absoluteFilePath() ) );
         // ignore a bad path.
      }
   }
   
   QString fname = vkDlgGetFile( parent, start_path, cfg_key_path, mode );

   // save chosen path to cfg for next time   
   if ( !cfg_key_path.isEmpty() && !fname.isEmpty() ) {
      QString path = fname;
      QFileInfo fi( path );
      path = pathIsDir ? fi.absolutePath() : fi.absoluteFilePath();
      if ( isProjKey )
         vkCfgProj->setValue( cfg_key_path, path );
      else
         vkCfgGlbl->setValue( cfg_key_path, path );
   }
   
   return fname;
}




/*!
  Dialog to choose a directory
   - default start_dir is current directory
  Returns: chosen directory path
*/
QString vkDlgGetDir( QWidget* parent, const QString& start_dir/*="./"*/ )
{
   // Setup dialog
   QFileDialog dlg( parent, "Choose Directory", start_dir );
   dlg.setFileMode( QFileDialog::Directory );
   dlg.setViewMode( QFileDialog::Detail );
   dlg.setAcceptMode( QFileDialog::AcceptOpen );
   dlg.setOption( QFileDialog::ShowDirsOnly );
   
   // Run dialog - get filename to save to: asks for overwrite confirmation
   QString dir_selected;
   if ( dlg.exec() ) {
      QStringList fileNames = dlg.selectedFiles();
      if ( !fileNames.isEmpty() )
         dir_selected = fileNames.first();
   }

   return dir_selected;
}


#if 0 // As-yet unused, untested...
/*!
  Dialog to choose a directory
   - cfg_key_path is the (proj/glbl) cfg key for the start directory
  The cfg is updated if a directory is chosen.
  Returns: chosen directory path
  
  Note: don't use from options pages: they save to cache already.// - careful: don't use in options pages: don't want to save cfg before our turn!
*/
QString vkDirCfgDialog( QWidget* parent,
                        const QString& cfg_key_dir/*=QString()*/ )
{
   // setup start directory
   QString start_dir  = "./";
   if ( !cfg_key_dir.isEmpty() ) {
      QFileInfo fi( vkCfgProj->value( cfg_key_dir ).toString() );
      start_dir = fi.exists() ? fi.absolutePath() : "./";
   }
   
   QString dirname = vkDlgGetDir( parent, start_dir );
   
   // save chosen directory for next time   
   if ( !cfg_key_dir.isEmpty() && !dirname.isEmpty() ) {
      vkCfgProj->setValue( cfg_key_dir, dirname );
   }
   
   return dirname;
}
#endif


