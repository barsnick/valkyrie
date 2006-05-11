/* --------------------------------------------------------------------- 
 * Definition of VkConfig                                    vk_config.h
 * Configuration file parser
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_CONFIG_H
#define __VK_CONFIG_H

#include <qfile.h>
#include <qmap.h>
#include <qobject.h>
#include <qcstring.h>
#include <qstring.h>

#include "tool_object.h"
#include "valkyrie_object.h"


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



class VkConfig : public QObject
{
public:
   VkConfig();
   ~VkConfig();

   bool initCfg( Valkyrie* vk );

   bool isDirty();   /* config holds data difft to that held on disk */
   bool sync( Valkyrie* vk );  /* write config to disk */

   /* these fns return the values set in config.h ------------------- */
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
   QString rcDir();
   QString logsDir();
   QString dbaseDir();
   QString suppDir();
   QChar sepChar() { return m_sep; }

   /* util functions */
   bool strToBool( QString str );

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
   bool    checkRCEntry( QString path, Valkyrie* vk );
   bool    checkRCTree( Valkyrie* vk );
   QString mkConfigHeader( void );
   QString mkConfigDefaults( Valkyrie* vk );
   bool    writeConfigDefaults( Valkyrie* vk );

   bool     writeConfig( EntryMap rcMap, bool backup=false );
   EntryMap parseConfigToMap( QTextStream &stream );
   void     insertData( const EntryKey &key, const EntryData &data );
   void     backupConfigFile();
   bool     parseFile( Valkyrie *vk, /*OUT*/EntryMap &map );
   bool     updateCfgFile( EntryMap &newMap, EntryMap &rcMap,
                           /*OUT*/EntryMap &dstMap );

private:
   QChar m_sep;
   bool  m_dirty;

   QCString m_vk_name;
   QCString m_vk_Name;
   QCString m_vk_version;
   QCString m_vk_copyright;
   QCString m_vk_author;
   QCString m_vk_email;
   QCString m_vg_copyright;

   QString m_vkdocPath;        /* path to valkyrie docs dir */
   QString m_vgdocPath;        /* path to valgrind docs dir */

   QString m_rcPath;           /* path to ~/valkyrie=X.X.X dir */
   QString m_rcFileName;       /* where valkyrierc lives */
   QString m_logsPath;         /* path to log dir */
   QString m_dbasePath;        /* path to dbase dir */
   /* path to user's suppression files dir.
      default is ~/.valkyrie/suppressions */
   QString m_suppPath;

   EntryMap m_EntryMap;       /* the config dict */
};



/* Globally available object ------------------------------------------- */
extern VkConfig* vkConfig;

#endif
