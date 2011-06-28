/****************************************************************************
** Definition of valkyrie config classes
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

#ifndef __VK_CONFIG_H
#define __VK_CONFIG_H

#include <QChar>
#include <QSettings>
#include <QString>
#include <QVariant>


// ============================================================
// externals
class VkCfgProj;
class VkCfgGlbl;
extern VkCfgProj* vkCfgProj;  // Project config
extern VkCfgGlbl* vkCfgGlbl;  // Global (all non-project) config



// ============================================================
// namespace class only.
class VkCfg
{
public:
   static unsigned int projCfgVersion();
   static unsigned int glblCfgVersion();
   static const QString& email();
   static const QString& copyright();
   static const QString& vgCopyright();
   static const QString& vgVersion();
   static const QString& appName();
   static const QString& appVersion();
   static const QString& appPackage();
   static const QString& tmpDir();
   static const QString& tmpCfgPath();
   static const QString& cfgDir();
   static const QString& projDfltPath();
   static const QString& globalPath();
   static const QString& filetype();
   static const QString& suppsDir();
   static const QString& docDir();
   static const QChar& sepChar();

   // util functions
   static bool checkConfigFile( const QString& cfgFile );
   static bool checkConfigDir( const QString& cfgDir );

private:
   static const unsigned int _projCfgVersion;
   static const unsigned int _glblCfgVersion;
   static const QString _email;
   static const QString _copyright;
   static const QString _vgCopyright;
   static const QString _vgVersion;
   static const QString _name;
   static const QString _version;
   static const QString _package;
   static const QString _fileType;
   static const QString _tmpDir;
   static const QString _tmpCfgName;
   static const QString _cfgDir;
   static const QString _prjDfltName;
   static const QString _globalName;
   static const QString _suppDir;
   static const QString _docDir;
   static const QChar   _sepChar;
};




// ============================================================
// global (non-project) config
class VkCfgGlbl : public QSettings
{
   // Singleton class constrution/destruction
public:
   static bool createConfig();
   ~VkCfgGlbl();

   QVariant value( const QString& key, const QVariant& defaultValue = QVariant() ) const;

   // private cons's
private:
   VkCfgGlbl();
   VkCfgGlbl( const QString cfg_fname );
   VkCfgGlbl( const VkCfgGlbl& );
   VkCfgGlbl& operator= ( const VkCfgGlbl& );

private:
   static bool checkVersionOk( unsigned int new_version );
   void writeConfigDefaults();
};







// ============================================================
// Project config
// has-a QSettings, so we can change the underlying file.
class Valkyrie;
class VkCfgProj
{
   // Singleton class constrution/destruction
public:
   static bool createConfig( Valkyrie* valkyrie );
   ~VkCfgProj();

   // private cons's
private:
   VkCfgProj();
   VkCfgProj( Valkyrie* valkyrie, const QString& cfg_fname );
   VkCfgProj( const VkCfgProj& );
   VkCfgProj& operator= ( const VkCfgProj& );

   // public interface
public:
   QVariant value( const QString& key, const QVariant& defaultValue = QVariant() ) const;
   bool contains ( const QString & key ) const;
   void setValue ( const QString& key, const QVariant& value );
   void sync();
   void clear();
   void createNewProject(const  QString& proj_filename );
   void openProject( const QString& proj_filename );
   void saveProjectAs( const QString proj_filename, bool replace=true );
   void saveToDefaultCfg();

private:
   bool checkValidConfig( QSettings* cfg );
   static bool checkVersionOk( unsigned int new_version );
   static void cleanTempDir();
   void writeConfigDefaults();
   bool loadDefaultConfig();
   void replaceConfig( QSettings* new_cfg );
   
private:
   Valkyrie* vk;
   QSettings* currentCfg;
};


#endif
