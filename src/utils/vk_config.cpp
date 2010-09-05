/****************************************************************************
** Implementation of VkConfig
**  - Configuration
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
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

#include "vk_config.h"
#include "utils/vk_utils.h"

#include <QDir>

#if 0
#include "config.h"
#include "vk_utils.h"       /* VK_DEBUG */
#include "vk_messages.h"

#include <unistd.h>         /* close, access */
#include <qdatetime.h>
#include <qfile.h>
#include <qstylefactory.h>  /* vkStyle() */
#endif

#include <pwd.h>                    // getpwuid

#include <iostream>

using namespace std;



/*!
   class VkConfig ------------------------------------------------------
   Inherits QSettings.
   Creates a 'working' config, which is temporarily saved to disc (tmp_cfg, deleted upon exit)
   If a 'project' is created or opened, the config settings are transparently saved to that project file.
   Config settings are read in in order from 3 sources, each overriding the last:
    1. Global configuration file (static location per user)
    2. Project configuration file
    3. Command line arguments
   If no project file is created / opened, any configuration changes made (via either the command line arguments or
   the GUI) will be used for the 'working' config, but will be discarded upon application exit.

   Creating a new project saves the current 'working' config to that new project.
   Opening a project, with no previous project being open, will discard the last 'working' config.
*/
VkConfig::VkConfig( QString& tmp_cfg )
   : QSettings( tmp_cfg, QSettings::IniFormat )
{
   // Setup Const Config
   vkRcDir     = QDir::homePath() + "/" + VK_CFG_GLBL_DIR;
   vkSepChar   = ',';
   vkName      = PACKAGE;
   vkVersion   = PACKAGE_VERSION;
   vkCopyright = VK_COPYRIGHT;
   vkAuthor    = VK_AUTHOR;
   vkEmail     = PACKAGE_BUGREPORT;
   //   vgCopyright = VG_COPYRIGHT;
   vkDocPath   = VK_DOC_PATH;
   
   // By default no project.
   vkCfgProjectFilename = QString::null;
}


VkConfig::~VkConfig()
{
   this->sync();
   
   bool ok = QFile::remove( this->fileName() );
   
   if ( !ok ) {
      cerr << "VkConfig::~VkConfig(): Failed to remove temporary file ("
           << this->fileName().toLatin1().constData() << ")" << endl;
   }
}


/*!
   Create yourself: kazam!
*/
void VkConfig::createConfig( Valkyrie* vk /*in*/,
                             VkConfig** new_vkConfig /*out*/ )
{
   *new_vkConfig = NULL;
   
   // ------------------------------------------------------------
   // setup file
   QString cfgTmpDir = VkConfig::vkTmpDir();
   QDir dir;
   
   if ( ! dir.exists( cfgTmpDir ) ) {
      if ( ! dir.mkpath( cfgTmpDir ) ) {
         VK_DEBUG( "Failed to create tmpdir: '%s'.", qPrintable( cfgTmpDir ) );
         return;
      }
   }
   
   QString cfgTmpPath = cfgTmpDir + "/" + VK_CFG_TEMP;
   QString working_cfgfile = vk_mkstemp( cfgTmpPath, VK_CFG_EXT );
   
   if ( working_cfgfile.isNull() ) {
      VK_DEBUG( "failed to create tempfile for working cfg: '%s'",
                qPrintable( working_cfgfile ) );
      return;
   }
   
   // ------------------------------------------------------------
   // Create new config:
   *new_vkConfig = new VkConfig( working_cfgfile );
   
   // 1) Check Global config file exists: if not, create one from compiled defailts.
   if ( ! QFile::exists( vkCfgGlblFilename() ) ) {
      ( *new_vkConfig )->writeConfigDefaults( vk );
   }
   
   // 2) Read global config file into RUNTIME settings
   // check at least owner read/write perms ok
   QFile::Permissions perms = QFile::permissions( VkConfig::vkCfgGlblFilename() );
   
   if (( perms & QFile::WriteOwner ) != QFile::WriteOwner ||
       ( perms & QFile::ReadOwner ) != QFile::ReadOwner ) {
      //TODO
      cerr << "TODO: vkError permissions on vkCfgGlblFilename" << endl;
      
   }
   else {
      // first clear all current settings, then read in global
      ( *new_vkConfig )->clear();
      ( *new_vkConfig )->readFromGlblConfigFile( vk );
   }
   
   // 3) If given a Project file on the command line, read and update 'working' config
   // Done by parseCmdArgs() -> vkObj::updateConfig(), called after us.
   
   // 4) Command line options update 'working' config
   // Done by parseCmdArgs(), called after us.
   
   /*
           vkFatal( 0, "Initialising Config",
                  "<p>Initialisation of Config failed.<br/>"
                  "Please check existence/permissions of the "
                  "config dir '%s', and its files/sub-directories.</p>",
                  m_rcPath.latin1() );
   */
}


/*!
  VkConfig::createNewProject()
  Sets project filename to given (new) project config file.
  Writes the current config to the given new project config file.
  */
void VkConfig::createNewProject( QString& dir, QString& proj_name )
{
   createNewProject( dir + "/" + proj_name + "." + VK_CFG_EXT );
}


/*!
  VkConfig::createNewProject()
  Sets project filename to given (new) project config file.
  Writes the current config to the given new project config file.
  */
void VkConfig::createNewProject( QString proj_filename )
{
   // ensure any existing project settings are saved first.
   vkConfig->sync();
   
   vkCfgProjectFilename = proj_filename;
   this->saveToProjConfigFile();
}


/*!
  VkConfig::openProject()
  Sets project filename to given (existing) project config file.
  Reads in config from the given (existing) project config file.
  */
void VkConfig::openProject( Valkyrie* vk, QString& proj_filename )
{
   // ensure any existing project settings are saved first.
   vkConfig->sync();
   
   vkCfgProjectFilename = proj_filename;
   this->readFromProjConfigFile( vk );
}

/*!
  sync()
  Save 'working' config changes disc
  If we're we're working within a project, save to the project file too.
*/
void VkConfig::sync()
{
   //   std::cerr << "VkConfig::sync()" << std::endl;
   
   // save 'working' config
   QSettings::sync();
   
   //   save to vkCfgProjectName, if exists
   if ( !vkCfgProjectFilename.isEmpty() &&  !vkCfgProjectFilename.isNull() ) {
      std::cerr << "VkConfig::sync(): in proj => save cfg to proj cfg file ("
                << qPrintable( vkCfgProjectFilename ) << ")" << std::endl;
      saveToProjConfigFile();
   }
}


void VkConfig::readFromProjConfigFile( Valkyrie* vk )
{
   readFromConfigFile( vk, vkCfgProjectFilename );
}

void VkConfig::readFromGlblConfigFile( Valkyrie* vk )
{
   readFromConfigFile( vk, vkCfgGlblFilename() );
}

void VkConfig::saveToProjConfigFile()
{
   saveToConfigFile( vkCfgProjectFilename );
}

void VkConfig::saveToGlblConfigFile()
{
   saveToConfigFile( vkCfgGlblFilename() );
}

/*!
  VkConfig::readFromConfigFile()
  Reads in config from the given (existing) project config file.
  Can't change QSettings::filename, so creates a copy with new filename.
  */
void VkConfig::readFromConfigFile( Valkyrie* vk, QString cfg_filename )
{
   QSettings project_settings( cfg_filename, QSettings::IniFormat );
   QStringList list_keys = project_settings.allKeys();
   QStringList::Iterator it = list_keys.begin();
   this->clear();
   foreach( QString key, list_keys ) {
      VkOption* opt = vk->findOption( key );
      
      if ( !opt ) {
         cerr << "VkConfig::readFromConfigFile(): VkOption not found for key '"
              << qPrintable( key ) << "'" << endl;
      }
      else {
         opt->updateConfig( project_settings.value( key ) );
      }
   }
}


/*!
  VkConfig::saveToConfigFile()
  Reads in config from the given (existing) project config file.
  Can't change QSettings::filename, so creates a copy with new filename.
  */
void VkConfig::saveToConfigFile( QString cfg_filename )
{
   // make sure everything's up to date.
   QSettings::sync();
   
   QSettings project_settings( cfg_filename, QSettings::IniFormat );
   QStringList list_keys = this->allKeys();
   QStringList::Iterator it = list_keys.begin();
   
   while ( it != list_keys.end() ) {
      project_settings.setValue( *it, this->value( *it ) );
      ++it;
   }
}




/*!
  Translate boolean strings into booleans
  returns true if successfully parsed a 'true' string.
  returns false otherwise
  Use argment 'ok' to detect failed 'false' parsing.
*/
bool VkConfig::strToBool( QString str, bool* ok )
{
   if ( ok ) *ok = true;

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
   
   if ( ok ) *ok = false;
   return false;
}


#if 0
bool VkConfig::rdBool( const QString& pKey, const QString& pGroup )
{
   return strToBool( rdEntry( pKey, pGroup ) );
}



QColor VkConfig::rdColor( const QString& pKey )
{
   bool ok;
   QColor aRetColor;
   
   QString aValue = rdEntry( pKey, "Colors" );
   
   if ( !aValue.isEmpty() ) {
      int nRed = 0, nGreen = 0, nBlue = 0;
      
      /* find first part (red) */
      int nIndex = aValue.find( ',' );
      
      if ( nIndex == -1 ) {
         return QColor();
      }
      
      nRed = aValue.left( nIndex ).toInt( &ok );
      
      /* find second part (green) */
      int nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex + 1 );
      
      if ( nIndex == -1 ) {
         return QColor();
      }
      
      nGreen = aValue.mid( nOldIndex + 1, nIndex - nOldIndex - 1 ).toInt( &ok );
      
      /* find third part (blue) */
      nBlue = aValue.right( aValue.length() - nIndex - 1 ).toInt( &ok );
      
      aRetColor.setRgb( nRed, nGreen, nBlue );
   }
   
   return aRetColor;
}




/* special version of wrEntry: adds values to the existing entry,
   rather than replacing it */
void VkConfig::addEntry( const QString& pValue,
                         const QString& pKey, const QString& pGroup )
{
   /* get hold of the current value(s) */
   QString curr_values = rdEntry( pKey, pGroup );
   
   /* concat curr_values with new value */
   if ( !curr_values.isEmpty() ) {
      curr_values += m_sep;
   }
   
   curr_values += pValue;
   
   wrEntry( curr_values, pKey, pGroup );
}


void VkConfig::wrBool( const bool& pValue, const QString& pKey,
                       const QString& pGroup )
{
   QString aValue = ( pValue == true ) ? "true" : "false";
   wrEntry( aValue, pKey, pGroup );
}


void VkConfig::wrColor( const QColor& pColor, const QString& pKey )
{
   QString aValue = "";
   
   if ( pColor.isValid() )
      aValue.sprintf( "%d,%d,%d",
                      pColor.red(), pColor.green(), pColor.blue() );
                      
   wrEntry( aValue, pKey, "Colors" );
}


/* private functions --------------------------------------------------- */
bool VkConfig::updateFromOldCfgFile( EntryMap& newMap, EntryMap& fileMap,
                                     /*OUT*/EntryMap& dstMap )
{
   /* loop over entries in old rc:
      if (entry exists in new map) copy value from old */
   EntryMapIterator aIt;
   
   for ( aIt = fileMap.begin(); aIt != fileMap.end(); ++aIt ) {
      const EntryKey&  rcKey = aIt.key();
      
      if ( newMap.find( rcKey ) != newMap.end() ) {
         newMap[ rcKey ] = aIt.data();
      }
   }
   
   /* but keep the new version! */
   newMap[ EntryKey( "valkyrie", "version" )].mValue = vkVersion();
   
   /* ... and set the binaries to configured values: */
   newMap[ EntryKey( "valkyrie", "merge-exec" )].mValue = BIN_LOGMERGE;
   newMap[ EntryKey( "valkyrie", "vg-exec" )].mValue = BIN_VALGRIND;
   
   /* write out new config */
   if ( !writeConfig( newMap, true ) ) {
      VK_DEBUG( "failed to write config file" );
      return false;
   }
   
   /* and return new map */
   dstMap = newMap;
   return true;
}

#endif



/* Create the default global configuration file.  -----------------------------
   The first time valkyrie is started, vkConfig looks to see if this
   file is present in the user's home dir.  If not, it writes the
   relevant data to the (user) global configuration file */
void VkConfig::writeConfigDefaults( Valkyrie* vk )
{
   // Call each object and have it set its config defaults.
   VkObjectList vkObjectList = vk->vkObjList();
   
   for ( int i = 0; i < vkObjectList.size(); ++i ) {
      VkObject* obj = vkObjectList.at( i );
      obj->setConfigDefaults();
   }
   
   // Working config is now the installation default.
   // Save to GLBL config file
   saveToGlblConfigFile();
}

#if 0

/* check rc file or dir exists, is read & writeable */
bool VkConfig::checkRCEntry( QString path, Valkyrie* vk )
{
   QFileInfo fi( path );
   
   if ( !fi.exists() ) {
      VK_DEBUG( "creating config entry '%s'.", path.latin1() );
      
      /* if it's a dir, create it */
      if ( path.endsWith( "/" ) ) {    // TODO: more robust but abstract way to do this?
         QDir dir;
         
         if ( !dir.mkdir( path ) ) {
            VK_DEBUG( "failed creation of new dir '%s'.", path.latin1() );
            return false;
         }
      }
      else {
         /* if it's a file... */
         if ( path == m_rcFileName ) {
            /* create shiny new rc file */
            if ( !writeConfigDefaults( vk ) ) {
               VK_DEBUG( "failed to create default config file '%s'.",
                         path.latin1() );
               return false;
            }
         }
      }
   }
   
   /* still doesn't exist?! */
   if ( !fi.exists() ) {
      VK_DEBUG( "rc entry '%s' doesn't exist.", fi.filePath().latin1() );
      return false;
   }
   
   /* permissions ok? */
   if ( !fi.isReadable() ) {
      VK_DEBUG( "rc entry '%s' not readable.", fi.filePath().latin1() );
      return false;
   }
   
   if ( !fi.isWritable() ) {
      VK_DEBUG( "rc entry '%s' not writable.", fi.filePath().latin1() );
      return false;
   }
   
   if ( fi.isDir() && !fi.isExecutable() ) {
      VK_DEBUG( "rc dir '%s' not executable.", fi.filePath().latin1() );
      return false;
   }
   
   return true;
}


/* ~/.PACKAGE is a sine qua non ----------------------------------------
   checks to see if ~/valkyrie/ and its required files/sub-dirs are
   all present and correct.  If not, tries to create them.
   (see config.h for #defines)
*/
bool VkConfig::checkRCTree( Valkyrie* vk )
{
   QStringList entries;
   entries << m_rcPath << m_rcFileName
           << m_dbasePath << m_suppPath;
           
   /* Note: don't just run through and test/fix: want to tell the user
      if there was a problem with a previous config dir tree.
      Missing files can be recreated from default.
      Bad permissions problems are left to the user */
   
   /* first just find out if anything's missing... */
   bool ok = true;
   QStringList::Iterator it;
   
   for ( it = entries.begin(); it != entries.end(); ++it ) {
      if ( !QFile::exists( *it ) ) {
         VK_DEBUG( "rc entry '%s' doesn't exist.", ( *it ).latin1() );
         ok = false;
         break;
      }
   }
   
   if ( !ok ) { /* rc tree !exists or !well */
      /* this an existing tree?
         if so, tell the user there was a problem */
      if ( QFile::exists( m_rcPath ) ) {
         vkInfo( 0, "Checking Config Setup",
                 "<p>Detected missing configuration files/dirs.<br/>"
                 "Attempting to recreate from defaults...</p>" );
      }
      
      // else clean tree to setup: just do it.
   }
   
   /* run through rc entries, checking existence/permissions and
      creating them if necessary */
   for ( it = entries.begin(); it != entries.end(); ++it ) {
      if ( !checkRCEntry( *it, vk ) ) {
         return false;
      }
   }
   
   /* Further, check for temporary log dir (VK_TMP_DIR + username),
      make if !exists */
   if ( !QFile::exists( VkConfig::vkTmpDir() ) ) {
      if ( !checkRCEntry( VkConfig::vkTmpDir(), vk ) ) {
         return false;
      }
   }
   
   return true;
}
#endif

/*!
  Override QSettings::value() to give an error if 'key' doesn't already exist
*/
QVariant VkConfig::value( const QString& key, const QVariant& defaultValue ) const
{
   if ( ! contains( key ) ) {
      cerr << "TODO: vkError: key not in config: '" << key.toLatin1().constData() << "'" << endl;
   }
   
   return QSettings::value( key, defaultValue );
}



/*!
  Get the log directory associated with this user
  Just do this once, and cache the results.
*/
QString VkConfig::vkTmpDir()
{
   static QString res = QString::null;
   
   if ( res.isNull() ) {
      pid_t me = getuid();
      struct passwd* pw = getpwuid( me );
      vk_assert( pw );
      
      res = QString( VK_TMP_DIR ) + QString( pw->pw_name ) + "/";
      vk_assert( !res.isNull() );
   }
   
   return res;
}





// static util functions
QString VkConfig::vkCfgGlblFilename()
{
   return QDir::homePath() + "/" + VK_CFG_GLBL_DIR
          + "/" + QString( VK_CFG_GLBL ) + "." + QString( VK_CFG_EXT );
}
