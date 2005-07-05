/* --------------------------------------------------------------------- 
 * Definition of VkConfig                                    vk_config.h
 * Configuration file parser
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_CONFIG_H
#define __VK_CONFIG_H

#include <qfile.h>
#include <qmap.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qcstring.h>
#include <qstring.h>

#include "vk_objects.h"


struct EntryData
{
  EntryData( const QString &value, bool dirty ) 
    : mValue(value), mDirty(dirty) { }
  EntryData() : mValue(QString::null), mDirty(false) { }
  QString mValue;     /* the actual value we want */
  bool    mDirty;     /* must the entry be written to disk? */
};

struct EntryKey
{
  EntryKey( const QString &group=QString::null, 
            const QString &key=QString::null )
    : mGroup(group), mKey(key), cKey( key.data() ) { }
  QString mGroup;     /* group to which this EntryKey belongs */
  QString mKey;       /* key of the entry in question         */
  const char *cKey;   /* testing equality with operator <     */
};

/* compares two EntryKeys (needed for QMap) */
inline bool operator <( const EntryKey &k1, const EntryKey &k2 )
{
  int result = qstrcmp( k1.mGroup.data(), k2.mGroup.data() );
  if ( result != 0 )
    return ( result < 0 );     

  if ( !k1.cKey && k2.cKey )
    return true;

  result = 0;
  if ( k1.cKey && k2.cKey )
    result = strcmp( k1.cKey, k2.cKey );
  if ( result != 0 )
    return result < 0;

  return false;
}

typedef QMap<EntryKey, EntryData> EntryMap;
typedef QMap<EntryKey, EntryData>::Iterator EntryMapIterator;



typedef QPtrList<VkObject> VkObjectList;

class VkConfig : public QObject
{
public:
  VkConfig( bool *ok );
  ~VkConfig();

  void dontSync();  /* don't write back to disk on exit */

  /* these fns return the values set in vk_include.h ------------------- */
  const char* vkname();
  const char* vkName();
  const char* vkVersion();
  const char* vkCopyright();
  const char* vkAuthor();
  const char* vkEmail();
  const char* vgCopyright();

  /* these fns return values held in private vars ---------------------- */
  QString vgdocDir();
  QString vkdocDir();
  QString imgDir();
  QString rcDir();
  QString logsDir();
  QString dbaseDir();
  QString suppDir();
  QPixmap pixmap( QString pixfile );
  QChar sepChar() { return sep; }

  /* Returns a ptr to the tool currently set in [valgrind:tool] */
  ToolObject* currentTool();
  /* Returns the name of the current tool in [valgrind:tool] */
  QString currentToolName();
  /* Returns the tool id of [valgrind:tool] */
  VkObject::ObjectId currentToolId();

  /* Returns a list of all VkObjects, irrespective of whether they are
     tools or otherwise */
  VkObjectList vkObjList();
  /* returns a vkObject based on its name */
  VkObject* vkObject( const QString& obj_name );
  /* returns a vkObject based on its ObjectId */
  VkObject* vkObject( int tvid, bool tools_only=false );
  /* returns a vkObject tool, based on its ObjectId */
  ToolObject* vkToolObj( int tvid );

  /* read fns ---------------------------------------------------------- */
  QString rdEntry( const QString &pKey, const QString &pGroup );
  int     rdInt  ( const QString &pKey, const QString &pGroup );
  bool    rdBool ( const QString &pKey, const QString &pGroup );
  QFont   rdFont ( const QString &pKey, const QString &pGroup=QString::null );
  QColor  rdColor( const QString &pKey );

  /* write fns --------------------------------------------------------- */
  void wrEntry( const QString &pValue, 
                const QString &pKey,   const QString &pGroup );
  void wrInt  ( const int     pValue,    
                const QString &pKey, const QString &pGroup );
  void wrBool ( const bool    &bValue,    
                const QString &pKey,   const QString &pGroup );
  void wrFont ( const QFont   &rFont,  const QString &pKey );
  void wrColor( const QColor  &pColor, const QString &pKey );
  /* special version of wrEntry: adds values to the existing entry,
     rather than replacing */
  void addEntry( const QString &pValue, 
                 const QString &pKey,   const QString &pGroup );

private:
  enum RetVal { Okay=1, BadFilename, NoDir, NoPerms,
                BadRcFile, BadRcVersion, CreateRcFile, Fail };
  void sync();

  bool checkDirs();
  bool checkPaths();
  void mkConfigFile( bool rm=false );

  void writebackConfig();
  void parseConfigFile( QFile &rFile, EntryMap *writeBackMap=NULL );
  void insertData( const EntryKey &key, const EntryData &data );
  RetVal parseFile();
  RetVal checkAccess() const;

  /* creates the various VkObjects and initialises their options,
     ready for cmd-line parsing (if any). */
  bool initVkObjects();

private:
  QChar sep;
  bool mDirty;
  bool newConfigFile;

  QCString vk_name;
  QCString vk_Name;
  QCString vk_version;
  QCString vk_copyright;
  QCString vk_author;
  QCString vk_email;
  QCString vg_copyright;

  QString mPackagePath;     /* valkyrie's install dir */
  QString vkdocPath;        /* path to valkyrie docs dir */
  QString vgdocPath;        /* path to valgrind docs dir */
  QString imgPath;          /* path to images */

  QString rcPath;           /* path to ~/valkyrie=X.X.X dir */
  QString rcFileName;       /* where valkyrierc lives */
  QString logsPath;         /* path to log dir */
  QString dbasePath;        /* path to dbase dir */
  /* path to user's suppression files dir.
     default is ~/.valkyrie-X.X.X/suppressions */
  QString suppPath;

  EntryMap mEntryMap;       /* the config dict */

  /* list of the various objects */
  VkObjectList vkObjectList;
};



/* Globally available object ------------------------------------------- */
extern VkConfig* vkConfig;

#endif
