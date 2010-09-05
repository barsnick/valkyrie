/****************************************************************************
** Definition of VkConfig
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

#ifndef __VK_CONFIG_H
#define __VK_CONFIG_H

#if 0
#include <qfile.h>
#include <qmap.h>
#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>
#include <qfont.h>

#include "tool_object.h"
#endif

#include "vk_defines.h"
#include "objects/valkyrie_object.h"

#include <QSettings>
#include <QVariant>


// ============================================================
// Globally available object
class VkConfig;
extern VkConfig* vkConfig;


// ============================================================
#define VK_TMP_DIR "/tmp/valkyrie_"   // temp vk directory. Completed by vkTmpDir()

#define VK_CFG_TEMP "valkyrie_temp"   // temp cfg fname (for the 'working' config)
#define VK_CFG_GLBL_DIR ".valkyrie"   // global (user) cfg/data directory
#define VK_CFG_GLBL "valkyrie_global" // global (user) cfg fname
#define VK_CFG_EXT  "conf"            // valkyrie config file extension

#define PACKAGE "valkyrie"
#define PACKAGE_VERSION "2.0.0-SVN"
#define PACKAGE_STRING "Valkyrie 1.4.0"
#define PACKAGE_BUGREPORT "info@open-works.co.uk"

#define VK_INSTALL_PREFIX "./" //"$VK_INSTALL_PREFIX"
//#define VK_DOC_PATH "/doc/"
#define VK_DBASE_DIR "dbase/"
#define VK_BIN_VALGRIND "valgrind"
#define VK_BIN_EDITOR "vi"
//#define VK_COPYRIGHT "(c) 2003-2010"
//#define VK_AUTHOR "OpenWorks LLP"
//#define VG_COPYRIGHT "(c) 2000-2010 and GNU GPL'd by Julian Seward et al."





// ============================================================
class VkConfig : public QSettings
{
public:
   VkConfig( QString& tmp_cfg );
   ~VkConfig();
   
   static void createConfig( Valkyrie* vk, VkConfig** cfg );
   
   void createNewProject( QString& dir, QString& proj_name );
   void createNewProject( QString proj_filename );
   void openProject( Valkyrie* vk, QString& proj_filename );
   
   void sync();
   void readFromProjConfigFile( Valkyrie* vk );
   void readFromGlblConfigFile( Valkyrie* vk );
   void saveToProjConfigFile();
   void saveToGlblConfigFile();
   
   // Override QSettings::value(), to print error if doesn't exist
   QVariant value( const QString& key, const QVariant& defaultValue = QVariant() ) const;
   
private:
   void readFromConfigFile( Valkyrie* vk, QString cfg_filename );
   void saveToConfigFile( QString cfg_filename );
   
public:
   // util functions
   static bool strToBool( QString str, bool* ok=0 );
   static QString vkCfgGlblFilename();
   static QString vkTmpDir();
   
#if 0
   QColor  rdColor( const QString& pKey );
   void wrColor( const QColor&  pColor, const QString& pKey );
   /* special version of wrEntry: adds values to the existing entry, rather than replacing */
   void addEntry( const QString& pValue, const QString& pKey, const QString& pGroup );
#endif
   
   
private:
   //   bool checkRCEntry( QString path, Valkyrie* vk );
   //   bool checkRCTree( Valkyrie* vk );
   //   void mkConfigDefaults( Valkyrie* vk );
   void writeConfigDefaults( Valkyrie* vk );
   //   bool updateCfgFile( EntryMap &newMap, EntryMap &rcMap, /*OUT*/EntryMap &dstMap );
   
public:
   // Const Config
   QString vkRcDir;
   QChar   vkSepChar;          // separator for lists of strings
   QString vkName;
   QString vkVersion;
   QString vkCopyright;
   QString vkAuthor;
   QString vkEmail;
   QString vkDocPath;          // path to valkyrie docs dir
   //   QString vgCopyright;
   
#if 0    // valkyrie cfg options -> PUT IN VK OBJECT!
   QString vkCfgTempPath;         // path to run-specific config dir
   QString vkCfgTempFilePath;     // filename for run-specific config
   QString vkCfgProjectPath;      // path to project-specific config dir
   QString vkCfgProjectFilePath;  // filename for project-specific config: Empty if none.
   QString vkCfgGlobalPath;       // path to global config dir
   QString vkCfgGlobalFilePath;   // filename for global config
#endif
   QString vkCfgProjectFilename;  // hold active project filename (may be empty)
   
private:
   QString vkCfgTmpDir;
};

#endif
