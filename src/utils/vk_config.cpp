/****************************************************************************
** Implementation of valkyrie config classes
**  - based on persistent (file-based) application settings
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

#include "vk_config.h"
#include "objects/valkyrie_object.h"
#include "options/vk_option.h"
#include "utils/vk_defines.h"  // build-generated file.
#include "utils/vk_utils.h"

#include <pwd.h>     // getpwuid

#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QPoint>
#include <QSize>
#include <QStringList>



/***************************************************************************/
/*!
  Initialise static data: Basic configuration setup
*/
const unsigned int VkCfg::_projCfgVersion = 1;   // @@@ increment if project config keys change @@@
const unsigned int VkCfg::_glblCfgVersion = 1;   // @@@ increment if  global config keys change @@@

const QString VkCfg::_email       = "info@open-works.net"; // bug-reports
const QString VkCfg::_copyright   = "Valkyrie is Copyright (C) 2003-2010 by OpenWorks GbR";
const QString VkCfg::_vgCopyright = "Valgrind is Copyright (C) 2000-2010, and GNU GPL'd, by Julian Seward et al.";
const QString VkCfg::_vgVersion   = "3.6.0";               // supports this major Vg release
const QString VkCfg::_name        = VK_NAME;               // application name
const QString VkCfg::_version     = VK_VERSION;            // release version
const QString VkCfg::_package     = VK_PACKAGE;            // package name
const QString VkCfg::_fileType    = "cfg";                 // config file extension
const QString VkCfg::_tmpDir      = "/tmp/valkyrie_%usr/"; // tmp dir (logs, tmp cfg)
const QString VkCfg::_tmpCfgName  = "temp_proj";           // tmp cfg fname
const QString VkCfg::_cfgDir      = "%home/.valkyrie/";    // config dir (per user)
const QString VkCfg::_prjDfltName = "default_proj";        // default project cfg fname
const QString VkCfg::_globalName  = "global";              // global config filename
const QString VkCfg::_suppDir     = "suppressions/";       // suppressions dir
const QString VkCfg::_docDir      = VK_DOC_PATH;           // document dir
const QChar   VkCfg::_sepChar     = ',';                   // separator for lists of strs




/***************************************************************************/
/*!
  Project config version
*/
unsigned int VkCfg::projCfgVersion() { return _projCfgVersion; }

/*!
  Global config version
*/
unsigned int VkCfg::glblCfgVersion() { return _glblCfgVersion; }

/*!
  Valkyrie email for bug reports
*/
const QString& VkCfg::email()        { return _email; }

/*!
  Valkyrie copyright string
*/
const QString& VkCfg::copyright()    { return _copyright; }

/*!
  _Valgrind_ copyright string
*/
const QString& VkCfg::vgCopyright()  { return _vgCopyright; }

/*!
  Valgrind version supported
*/
const QString& VkCfg::vgVersion()    { return _vgVersion; }

/*!
  Valkyrie application name
*/
const QString& VkCfg::appName()      { return _name; }

/*!
  Valkyrie application release version
*/
const QString& VkCfg::appVersion()   { return _version; }

/*!
  Valkyrie application package name
*/
const QString& VkCfg::appPackage()   { return _package; }

/*!
  filetype extension for config files.
*/
const QString& VkCfg::filetype()     { return _fileType; }

/*!
  tmp dir for general use (logs, tmp cfg, ...)
*/
const QString& VkCfg::tmpDir()
{
   static QString res = QString::null;
   if ( res.isNull() ) {
      pid_t me = getuid();
      struct passwd* pw = getpwuid( me );
      vk_assert( pw );
      res = QString( _tmpDir ).replace( "%usr", pw->pw_name );
      vk_assert( !res.isNull() );
   }
   return res;
}

/*!
  path to temporary (working, volatile) config file.
*/
const QString& VkCfg::tmpCfgPath()
{
   static QString res = QString::null;
   if ( res.isNull() ) {
      QString cfgTmpPath = tmpDir() + _tmpCfgName;
      res = vk_mkstemp( cfgTmpPath, filetype() );
   }
   if ( res.isNull() ) {
      VK_DEBUG( "Failed to create tempfile for working cfg: '%s'",
                qPrintable( res ) );
   }
   return res;
}

/*!
  path to valkyrie application config directory
*/
const QString& VkCfg::cfgDir()
{
   static QString res = QString::null;
   if ( res.isNull() ) {
      res = QString( _cfgDir ).replace( "%home", QDir::homePath() );
   }
   return res;
}

/*!
  path to default project config file
   - this is the default project config used if no other is specified by the user.
*/
const QString& VkCfg::projDfltPath()
{
   static QString res = QString::null;
   if ( res.isNull() ) {
      res = cfgDir() + _prjDfltName + "." + filetype();
   }
   return res;
}

/*!
  path to global config file
*/
const QString& VkCfg::globalPath()
{
   static QString res = QString::null;
   if ( res.isNull() ) {
      res = cfgDir() + _globalName + "." + filetype();
   }
   return res;
}

/*!
  path to suppressions directory
*/
const QString& VkCfg::suppsDir()
{
   static QString res = QString::null;
   if ( res.isNull() ) {
      res = cfgDir() + _suppDir;
   }
   return res;
}

/*!
  dir to installed documents
*/
const QString& VkCfg::docDir()
{
   return _docDir;
}

/*!
  seperator char, for general use.
*/
const QChar& VkCfg::sepChar()  { return _sepChar; }


/*!
  Utility function: Check config files:
   - If missing: ignore it (for now): we can't create it.
   - Check permissions.
*/
bool VkCfg::checkConfigFile( const QString& cfgFile )
{
   QFileInfo finfo( cfgFile );
   if ( !finfo.exists() ) {
      // don't create missing files: done elsewhere.
      VK_DEBUG( "No existing config file '%s' => factory default will be generated.",
                qPrintable( cfgFile ) );
      return true;
   }

   // check exists & permissions.
   int errval = PARSED_OK;
   ( void ) fileCheck( &errval, cfgFile, true, true );
   if ( errval != PARSED_OK ) {
      VK_DEBUG( "Config file error: %s: '%s'", parseErrString( errval ), qPrintable( cfgFile ) );
      return false;
   }

   return true;
}


/*!
  Utility function: Check config dirs:
   - If missing: create it.
   - Check permissions.
*/
bool VkCfg::checkConfigDir( const QString& cfgDir )
{
   QFileInfo finfo( cfgDir );
   if ( !finfo.exists() ) {
      VK_DEBUG( "No existing config dir '%s' => creating.", qPrintable( cfgDir ) );
      QDir dir;
      if ( !dir.mkpath( cfgDir ) ) {
         VK_DEBUG( "Failed to create config dir '%s'.", qPrintable( cfgDir ) );
         return false;
      }
   }

   // check exists & permissions.
   int errval = PARSED_OK;
   ( void ) dirCheck( &errval, cfgDir, true, true, true );
   if ( errval != PARSED_OK ) {
      VK_DEBUG( "Config dir error: %s: '%s'", parseErrString( errval ), qPrintable( cfgDir ) );
      return false;
   }

   return true;
}










/***************************************************************************/
/*!
   Global (non project-specific) settings.
   Inherits VkCfg: a file-based persistent config class
   Since it's global (per user), we don't need to worry about changing
   filenames (as with project-configs), so it's nice and simple.
*/
VkCfgGlbl::VkCfgGlbl( const QString cfg_fname )
   : QSettings( cfg_fname, QSettings::IniFormat )
{}

/*!
   Destructor: nothing to destroy.
*/
VkCfgGlbl::~VkCfgGlbl()
{
   sync();
}


/*!
   Create yourself: kazam!
   If no existing global config file, generate one from compiled (factory) defaults
   Else use existing file.
*/
bool VkCfgGlbl::createConfig()
{
   // ------------------------------------------------------------
   if ( vkCfgGlbl != NULL ) {
      vkPrintErr( "Program error: Attempted to create VkCfgGlbl singleton more than once." );
      vk_assert_never_reached();
   }

   // ------------------------------------------------------------
   // Check basic cfg dirs/files are ok.
   if ( ! VkCfg::checkConfigDir(  VkCfg::cfgDir()     ) ) { return false; }
   if ( ! VkCfg::checkConfigFile( VkCfg::globalPath() ) ) { return false; }
   // ok, we have (most of) a clean basic setup from here...


   // If global config file !exists, reset to compile-time defaults
   bool cfg_exists = QFile::exists( VkCfg::globalPath() );

   // Create new config:
   // - auto 'loads' if already exists.
   vkCfgGlbl = new VkCfgGlbl( VkCfg::globalPath() );

   if ( !cfg_exists ) {
      vkCfgGlbl->writeConfigDefaults();
   }
   else { // exists
      if ( !checkVersionOk( vkCfgGlbl->value( "config_glbl_version" ).toUInt() ) ) {
         // TODO: can do better: move out of the way, and create new.
         //  - get a life.
         delete vkCfgGlbl;
         vkCfgGlbl = NULL;
         VK_DEBUG( "Failed loading global config: bad version" );
         return false;
      }
   }
   vkCfgGlbl->sync();

   return true;
}


/*!
  Interface function to QSettings::value( key, defaultValue )
   - give an error if 'key' doesn't already exist
*/
QVariant VkCfgGlbl::value( const QString& key, const QVariant& defaultValue ) const
{
   if ( ! this->contains( key ) ) {
      VK_DEBUG( "Key not in global config: '%s'", qPrintable( key ) );
      return QVariant();
   }
   return QSettings::value( key, defaultValue );
}






/*!
  Check config version for compatibility / upgrading.
*/
bool VkCfgGlbl::checkVersionOk( unsigned int new_version )
{
   if ( new_version != VkCfg::glblCfgVersion() ) {
      VK_DEBUG( "Incompsettingsatible project config: expecting ver:%u, found ver:%u",
                VkCfg::glblCfgVersion(), new_version );
      return false;
   }
   return true;
}


/*!
  Reset this config to use the compiled (factory) defaults.
*/
void VkCfgGlbl::writeConfigDefaults()
{
   clear();
   vk_assert( allKeys().count() == 0 );

   setValue( "config_glbl_version",    VkCfg::glblCfgVersion() );
   setValue( "mainwindow_size",        QSize( 600, 600 ) );
   setValue( "mainwindow_pos",         QPoint( 400, 0 ) );
   setValue( "handbook_history",       QString() );
   setValue( "handbook_bookmarks",     QString() );
   setValue( "handbook_max_history",   20 );
   setValue( "handbook_max_bookmarks", 20 );
   setValue( "colour_background",      QColor( 214, 205, 187 ) );
   setValue( "colour_base",            QColor( 255, 255, 255 ) );
   setValue( "colour_dkgray",          QColor( 128, 128, 128 ) );
   setValue( "colour_edit",            QColor( 254, 222, 190 ) );
   setValue( "colour_highlight",       QColor( 147,  40,  40 ) );
   setValue( "colour_null",            QColor( 239, 227, 211 ) );
   setValue( "colour_text",            QColor(   0,   0,   0 ) );
   setValue( "recent_projects",        QString() );

   sync();
}





/***************************************************************************/
/*!
   Project-specific settings.
   Inherits VkCfg: a file-based persistent config class

   User-Interface:
    1) Start Vk with no project specified
       - Default-project settings used (underwater, a temp project is used, removed on exit)
    2) Start Vk with project
       - Opens project of given name, old settings are no longer used.
    3) Start Vk with command-line options
       - Config updated with given values, saved to given (else temp) project.
    4) File::New Project
       - Creates new project, with given name, based on default-project settings,
         old settings no longer used
    5) File::Open Project
       - As "Start Vk with project": Opens project of given name, old settings are no longer used.
    6) File::SaveAs
       - Saves current config to given name: settings are not changed.
    7) OptsDialog::Save_As_Project_Default
       - copy current config to default config file, defining a new default config for future use.

   In all cases, giving a (new) filename to a project means that:
    - Subsequent config updates are saved to that new config file.
    - The old config file (if any) is closed and no longer used.

   Note: This class is a useful means of holding the 'current' project settings.
       The settings themselves are further defined and wrapped in the VkOption classes.
*/
VkCfgProj::VkCfgProj( Valkyrie* valkyrie, const QString& cfg_fname )
   : vk( valkyrie)
{
   currentCfg = new QSettings( cfg_fname, QSettings::IniFormat );
}

/*!
  Destructor - clean up before we go.
*/
VkCfgProj::~VkCfgProj()
{
   sync();
   cleanTempDir();

   if ( currentCfg ) {
      delete currentCfg;
      currentCfg = NULL;
   }
}


/*!
   Create yourself: kazam!
   If no existing default project cfg file, generate one from compiled defaults.
   Else use existing file.

   On first time use, sets up dirs, creates default files, and checks
   existing setup for correctness & version
*/
bool VkCfgProj::createConfig( Valkyrie* valkyrie )
{
   if ( vkCfgProj != NULL ) {
      vkPrintErr( "Program error: Attempted to create VkCfgProj singleton more than once." );
      vk_assert_never_reached();
   }

   // ------------------------------------------------------------
   // Check if basic setup is present and correct
   //  - Tell the user if there was a problem with a previous config dir tree.
   //  - Missing files can be recreated from default.
   //  - Bad permissions problems are left to the user
   if ( QFile::exists( VkCfg::cfgDir() ) ) {
      // existing setup: check it out.

      // old valkyrie cfg files still around?
      QFile old_vkrc( VkCfg::cfgDir() + "/valkyrierc" );
      if ( old_vkrc.exists() ) {
         VK_DEBUG("Old valkyrie config detected: moving it out of the way." );
         QString archive_path( VkCfg::cfgDir() + "/old" );
         QDir dir;
         bool arch_ok = dir.mkpath( archive_path );
         if ( arch_ok ) {
            arch_ok = dir.rename( old_vkrc.fileName(), archive_path + "/valkyrierc" );
            arch_ok = arch_ok &&
                      dir.rename( VkCfg::cfgDir() + "/dbase", archive_path + "/dbase" );
         }

#if 0 // TODO ?
         vkInfo( 0, "Checking Config Setup",
                 "<p>Old configuration files/dirs.<br/>"
                 "Replacing old config (%s)</p>",
                 arch_ok ? "backed up: see " + archive_path
                         : "failed to create backup dir - old config remains." );
#endif
      }
   }
   // else no tree: clean install


   // ------------------------------------------------------------
   // run through paths, checking exists/perms & creating if necessary
   if ( ! VkCfg::checkConfigDir(  VkCfg::cfgDir()       ) ) { return false; }
   if ( ! VkCfg::checkConfigDir(  VkCfg::suppsDir()     ) ) { return false; }
   if ( ! VkCfg::checkConfigDir(  VkCfg::tmpDir()       ) ) { return false; }
   if ( ! VkCfg::checkConfigFile( VkCfg::projDfltPath() ) ) { return false; }
   // we have (most of) a clean basic setup from here...


   // ------------------------------------------------------------
   // clean up tmp dir, in case vk didn't close down and clean up nicely
   cleanTempDir();

   // ------------------------------------------------------------
   // setup config filename for temporary use.
   QString tmp_cfgfile = VkCfg::tmpCfgPath();
   if ( tmp_cfgfile.isNull() ) {
      VK_DEBUG( "Failed to create temporary project file" );
      return false;
   }

   // ------------------------------------------------------------
   // Create new (clean) project config
   //  - QSettings may write to the file immediately, so can't do this earlier
   vkCfgProj = new VkCfgProj( valkyrie, tmp_cfgfile );

   // ------------------------------------------------------------
   // Check default config file exists: if not, create one from compiled defailts.
   if ( ! QFile::exists( VkCfg::projDfltPath() ) ) {
      // This writes the project default file, and updates the current settings.
      vkCfgProj->writeConfigDefaults();
      // all set.
   }
   else { // else default config file exists - perms have been tested already.
      // Read in the default config file
      if ( ! vkCfgProj->loadDefaultConfig() ) {
         // TODO: can do better: move out of the way, and create new.
         //  - get a life.
         delete vkCfgProj;
         vkCfgProj = NULL;
         VK_DEBUG( "Failed to load default project config." );
         return false;
      }
   }
   vkCfgProj->sync();

   // ------------------------------------------------------------
   // Update the config if project file is given on the command-line.
   //  - Done by parseCmdArgs() -> vkObj::updateConfig(), called after us.

   // ------------------------------------------------------------
   // Command line options update this config.
   //  - Done by parseCmdArgs(), called after us.

   return true;
}


/*!
  Interface function to QSettings::value( key, defaultValue )
   - give an error if 'key' doesn't already exist
*/
QVariant VkCfgProj::value( const QString& key, const QVariant& defaultValue ) const
{
   if ( ! currentCfg->contains( key ) ) {
      VK_DEBUG( "Key not in project config: '%s'", qPrintable( key ) );
      return QVariant();
   }
   return currentCfg->value( key, defaultValue );
}

/*!
  Interface function to QSettings::setValue( key, value )
*/
void VkCfgProj::setValue ( const QString& key, const QVariant& value )
{
   currentCfg->setValue( key, value );
}

/*!
  Interface function to QSettings::contains( key, value )
*/
bool VkCfgProj::contains( const QString& key ) const
{
   return currentCfg->contains( key );
}

/*!
  Interface function to QSettings::sync()
*/
void VkCfgProj::sync()
{
   currentCfg->sync();
}

/*!
  Interface function to QSettings::clear()
*/
void VkCfgProj::clear()
{
   currentCfg->clear();
}


/*!
  Check config version for compatibility / upgrading.
*/
bool VkCfgProj::checkVersionOk( unsigned int new_version )
{
   if ( new_version != VkCfg::projCfgVersion() ) {
      VK_DEBUG( "Incompatible project config: expecting ver:%u, found ver:%u",
                VkCfg::projCfgVersion(), new_version );
      return false;
   }
   return true;
}


/*!
  Use a new project config.

  Usually in order to change the underlying file, as QSettings doesn't
  like doing this.
*/
void VkCfgProj::replaceConfig( QSettings* new_cfg )
{
   vk_assert( new_cfg );
   
   VK_DEBUG( "Replacing current config with: %s", qPrintable( new_cfg->fileName() ) );

   // replace existing QSettings
   QSettings* old_cfg = currentCfg;
   currentCfg = new_cfg;
   delete old_cfg;
   old_cfg = NULL;

   // TODO: could do the following instead, but doesn't affect existing QSetting instances,
   // so would have to refresh ptr anyway... unless created new QSettings everywhere, which Qt recommends...
   ///vkCfgProj->setPath( ini, user, proj_filename );

   //TODO: need to signal anyone to let them know the changes?
}


/*!
  Create a new project config file, based on the default config file,
  using 'proj_filename' to save/sync it to.

  If file exists -> (try to) overwrite it. Else create a new file.
*/
void VkCfgProj::createNewProject( const QString& proj_filename )
{
   // make sure everything's up to date.
   currentCfg->sync();

   // check if exists: if so, is writable?
   QFileInfo fi( proj_filename );
   if ( fi.exists() ) {
      VK_DEBUG( "Creating new project: File exists: overwriting: '%s'", qPrintable( proj_filename ) );
      if ( !fi.isWritable() ) {
         VK_DEBUG( "Can't overwrite file: '%s'", qPrintable( proj_filename ) );
         return;
      }
   }

   // create new config & clear it.
   QSettings* new_cfg = new QSettings( proj_filename, QSettings::IniFormat );
   new_cfg->clear();   // just in case there was an existing file.

   replaceConfig( new_cfg );

   // update config from default project config file
   if ( ! loadDefaultConfig() ) {
      VK_DEBUG( "Failed to create new project: Bad default project config. "
                "Try restarting Valkyrie to regenerate the default project config." );
      // TODO: something reasonable.
   }
}


/*!
  Opens the given project config file: 'proj_filename', henceforth
  using this to save/sync config changes to.

  Entry assumption: file exists && readable
*/
void VkCfgProj::openProject( const QString& proj_filename )
{
   // make sure everything's up to date.
   currentCfg->sync();

   // check exists && readable
   QFileInfo fi( proj_filename );
   if ( !fi.exists() || !fi.isReadable() ) {
      VK_DEBUG( "Can't open project: File doesn't exist, or is not readable: '%s'", qPrintable( proj_filename ) );
      return;
   }

   // open new config
   QSettings* new_cfg = new QSettings( proj_filename, QSettings::IniFormat );

   if ( ! checkValidConfig( new_cfg ) ) {
      vkPrintErr( "New project file bad/incomplete. Keeping existing config." );
      delete new_cfg;
      return;
   }

   // use new config
   replaceConfig( new_cfg );
}


/*!
  Saves the current config to the given 'proj_filename', henceforth
  using this to save/sync config changes to.

  If file exists -> (try to) overwrite it. Else create a new file.
*/
void VkCfgProj::saveProjectAs( const QString proj_filename, bool replace/*=true*/ )
{
   // make sure everything's up to date.
   currentCfg->sync();

   // check if exists: if so, is writable?
   QFileInfo fi( proj_filename );
   if ( fi.exists() ) {
      VK_DEBUG( "Creating new project: File exists: overwriting: '%s'", qPrintable( proj_filename ) );
      if ( ! fi.isWritable() ) {
         VK_DEBUG( "Can't overwrite file: '%s'", qPrintable( proj_filename ) );
         return;
      }
   }

   // create new config & clear it.
   QSettings* new_cfg = new QSettings( proj_filename, QSettings::IniFormat );
   new_cfg->clear(); // just in case there was an existing file.

   // copy current config to new config
   // - don't need to go via VkOption, as no settings are changed.
   QStringList list_keys = currentCfg->allKeys();
   foreach( QString key, list_keys ) {
      new_cfg->setValue( key, currentCfg->value( key ) );
   }
   new_cfg->sync();

   // check all went well.
   if ( checkValidConfig( new_cfg ) ) {
      if ( replace ) {
         replaceConfig( new_cfg );
      }
      else {
         delete new_cfg;
      }
   }
   else {
      vkPrintErr( "Failed to copy config to new project file." );
      delete new_cfg;
   }
}


/*!
  Saves the current settings to the default config file, for use
  by all subsequent projects (by this user).

  Note: This does _not_ change the current config file being used
  to save/sync settings.
*/
void VkCfgProj::saveToDefaultCfg()
{
   saveProjectAs( VkCfg::projDfltPath(), false );
}




/*!
  Reads in the settings from the default project config file.
   - first clears all current settings, then copies over the default settings.

  Note: The default project config is not (necessarily) the same
     as the factory defaults. The latter are compiled in, the former
     are saved to file and can be updated to affect all projects.
*/
bool VkCfgProj::loadDefaultConfig()
{
   // check at least projDfltPath() is readable
   QFileInfo fi( VkCfg::projDfltPath() );
   vk_assert( fi.isFile() );
   if ( !fi.exists() || !fi.isReadable() ) {
      VK_DEBUG( "Failed to load default project config: file doesn't exist / not readable: '%s'",
                qPrintable( VkCfg::projDfltPath() ) );
      // TODO: if it really doesn't exist, something v. strange is going on, since we already create it on startup...
      return false;
   }

   // open default config
   QSettings* defaultCfg = new QSettings( VkCfg::projDfltPath(), QSettings::IniFormat );

   // test default project config is ok.
   if ( ! checkValidConfig( defaultCfg ) ) {
      vkPrintErr( "Please (re)move the default project file: %s",
                  qPrintable( VkCfg::projDfltPath() ) );
      delete defaultCfg;
      return false;
   }

   // looking good: reset current config, to start with a clean slate.
   currentCfg->clear();

   // iterate over default config, updating current config.
   QStringList default_keys = defaultCfg->allKeys();
   default_keys.removeAll( "config_proj_version" );   // not-a-VkOption
   currentCfg->setValue( "config_proj_version", VkCfg::projCfgVersion() );

   foreach( QString key, default_keys ) {
      VkOption* opt = vk->findOption( key );
      if ( !opt ) {
         vkPrintErr( "Error in VkCfgProj::loadDefaultConfig(): Key not found in VkOption: '%s':'%s'",
                     qPrintable( VkCfg::projDfltPath() ), qPrintable( key ) );
      }
      else {   // Update current config
         // TODO: ick! Can't do it directly, must go via VkOption,
         // as it needs to send a signal to say the setting changed. FIXME! (sometime...)
         opt->updateConfig( defaultCfg->value( key ) );
      }
   }

   // force write to disk, before anything horrible happens!
   currentCfg->sync();

   delete defaultCfg;
   return true;
}


/*!
  Sanity check:
  Iterate over all options and make sure there is a config entry for it in the defaultCfg
*/
bool VkCfgProj::checkValidConfig( QSettings* cfg )
{
   if ( ! checkVersionOk( cfg->value( "config_proj_version" ).toUInt() ) ) {
      return false;
   }

   int nSettings = cfg->allKeys().count();
   int nOpts = 0;
   foreach( VkObject* obj, vk->vkObjList() ) {
      foreach( VkOption* opt, obj->getOptions() ) {
         if ( !opt->isaConfigOpt() ) {  // not all opts are meant to be saved to VkCfgProj
            continue;
         }

         nOpts++;
         if ( !cfg->contains( opt->configKey() ) ) {
            vkPrintErr( "Error in VkCfgProj::checkValidConfig(): Key not found in '%s':'%s'",
                        qPrintable( VkCfg::projDfltPath() ),
                        qPrintable( opt->configKey() ) );
            return false;
         }
      }
   }

   if ( nSettings != (nOpts+1) ) { // +1 for config_version
      vkPrintErr( "Error in VkCfgProj::checkValidConfig(): Bad number of keys/opts: '%u':'%u'",
                  nSettings, nOpts+1 );
      return false;
   }

   return true;
}


/*
  Reset this config to use the compiled (factory) defaults.
*/
void VkCfgProj::writeConfigDefaults()
{
   // should be empty
   vk_assert( currentCfg->allKeys().count() == 0 );

   // exception to vkobject options:
   currentCfg->setValue( "config_proj_version", VkCfg::projCfgVersion() );

   // Call each object and have it set its config defaults.
   // TODO: horrible. currently VkOption needs to signal that an option has changed. FIXME!
   foreach( VkObject* obj, vk->vkObjList() ) {
      obj->resetOptsToFactoryDefault();
   }
   currentCfg->sync();

   // Copy the loaded settings to the default config file
   saveToDefaultCfg();
}


/*!
   Clean up temporary dir: rm all files
*/
void VkCfgProj::cleanTempDir()
{
   QDir dir( VkCfg::tmpDir() );
   if ( dir.exists() ) {
      QFileInfoList entries = dir.entryInfoList( QDir::NoDotAndDotDot | QDir::Files);
      foreach( QFileInfo entryInfo, entries ) {
         QFile file( entryInfo.absoluteFilePath() );
         // do your best: not bothering to deal with errors at this stage of the game.
         (void) file.remove();
      }
   }
}
