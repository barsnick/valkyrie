/* --------------------------------------------------------------------- 
 * Implementation of VkConfig                              vk_config.cpp
 * Configuration file parser
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_config.h"
#include "config.h"
#include "vk_utils.h"       /* VK_DEBUG */
#include "vk_messages.h"

#include <unistd.h>         /* close, access */
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qstylefactory.h>  /* vkStyle() */


VkConfig::~VkConfig()
{}


/* test if config holds data difft to that held on disk */
bool VkConfig::isDirty()
{ return m_dirty; }


/* write config entries to disk
   called by user from menu 'Options::Save as Default' ---------------------- */
bool VkConfig::sync( Valkyrie* vk )
{
   if ( !m_dirty )
      return true;

   /* read config file from disk, and fill the temporary structure 
      with entries from the file */
	EntryMap rcMap;
   if ( !parseFile( vk, rcMap )) {
      VK_DEBUG( "failed to parse config file" );
      return false;
   }

   /* augment this structure with the dirty entries from the
      config object */
   EntryMapIterator aIt;
   for ( aIt = m_EntryMap.begin(); aIt != m_EntryMap.end(); ++aIt) {
      const EntryData &dirtyEntry = aIt.data();
      if ( !dirtyEntry.mDirty ) 
         continue;
      /* put dirty entries from the config object into the temporary map.
         if the entry exists, replaces the data
         if is a new entry (i.e old rc file), insert new key:data */
      rcMap.insert( aIt.key(), dirtyEntry );
   }

   /* write out updated config */
   if ( !writeConfig( rcMap ) ) {
      VK_DEBUG( "failed to write updated config file" );
      return false;
   }

   m_dirty = false;
   return true;
}


/* class VkConfig ------------------------------------------------------ */
VkConfig::VkConfig() : QObject( 0, "vkConfig" )
{
   m_EntryMap.clear();
   m_sep      = ',';         /* separator for lists of strings */
   m_dirty    = false;

   QString packagePath = VK_INSTALL_PREFIX;
   m_vkdocPath    = packagePath + VK_DOC_PATH;

   m_vk_name      = PACKAGE;
   m_vk_Name      = PACKAGE_NAME;
   m_vk_version   = PACKAGE_VERSION;
   m_vk_copyright = VK_COPYRIGHT;
   m_vk_author    = VK_AUTHOR;
   m_vk_email     = PACKAGE_BUGREPORT;
   m_vg_copyright = VG_COPYRIGHT;

   /* set full rc paths (see config.h) */
   m_rcPath     = QDir::homeDirPath() + "/." + vkname() + "/";
   m_rcFileName = m_rcPath + vkname() + "rc";
   m_dbasePath  = m_rcPath + VK_DBASE_DIR;
   m_suppPath   = m_rcPath + VK_SUPPS_DIR;

   m_defaultAppFont = QApplication::font();
}


/* try to create and/or parse the config file */
bool VkConfig::initCfg( Valkyrie* vk )
{
   if ( !checkRCTree( vk ) ||
        !parseFile( vk, m_EntryMap ) ) {
      vkFatal( 0, "Initialising Config",
               "<p>Initialisation of Config failed.<br/>"
               "Please check existence/permissions of the "
               "config dir '%s', and its files/sub-directories.</p>",
               m_rcPath.latin1() );
      return false;
   }
   return true;
}


/* misc. make-life-easier stuff ---------------------------------------- */

/* these fns return vars initialised from the #defines set in config.h */
const char* VkConfig::vkname()      { return m_vk_name.data();      }
const char* VkConfig::vkName()      { return m_vk_Name.data();      }
const char* VkConfig::vkVersion()   { return m_vk_version.data();   }
const char* VkConfig::vkCopyright() { return m_vk_copyright.data(); }
const char* VkConfig::vkAuthor()    { return m_vk_author.data();    }
const char* VkConfig::vkEmail()     { return m_vk_email.data();     }
const char* VkConfig::vgCopyright() { return m_vg_copyright.data(); }

/* these fns return values held in private vars */
QString VkConfig::vkdocDir()  { return m_vkdocPath; }
QString VkConfig::rcDir()     { return m_rcPath;    }
QString VkConfig::dbaseDir()  { return m_dbasePath; }
QString VkConfig::suppDir()   { return m_suppPath;  }

/* valkyrie's default palette */
QStyle* VkConfig::vkStyle()
{
   return QStyleFactory::create( "windows" ); 
}

/* valkyrie's default palette */
QPalette VkConfig::vkPalette()
{
   QColor bg = rdColor( "background" );
   QColor base = rdColor( "base" );
   QColor text = rdColor( "text" );
   QColor dkgray = rdColor( "dkgray" );
   QColor hilite = rdColor( "highlight" );
   if ( !bg.isValid()   || !base.isValid()   ||
        !text.isValid() || !dkgray.isValid() || !hilite.isValid() )
      return qApp->palette();

   /* 3 colour groups: active, inactive, disabled */
   QPalette pal( bg, bg );
   /* bg colour for text entry widgets */
   pal.setColor( QPalette::Active,   QColorGroup::Base, base );
   pal.setColor( QPalette::Inactive, QColorGroup::Base, base );
   pal.setColor( QPalette::Disabled, QColorGroup::Base, base );
   /* general bg colour */
   pal.setColor( QPalette::Active,   QColorGroup::Background, bg );
   pal.setColor( QPalette::Inactive, QColorGroup::Background, bg );
   pal.setColor( QPalette::Disabled, QColorGroup::Background, bg );
   /* same as bg */
   pal.setColor( QPalette::Active,   QColorGroup::Button, bg );
   pal.setColor( QPalette::Inactive, QColorGroup::Button, bg );
   pal.setColor( QPalette::Disabled, QColorGroup::Button, bg );
   /* general fg colour - same as Text */
   pal.setColor( QPalette::Active,   QColorGroup::Foreground, text );
   pal.setColor( QPalette::Inactive, QColorGroup::Foreground, text );
   pal.setColor( QPalette::Disabled, QColorGroup::Foreground, dkgray );
   /* same as fg */
   pal.setColor( QPalette::Active,   QColorGroup::Text, text );
   pal.setColor( QPalette::Inactive, QColorGroup::Text, text );
   pal.setColor( QPalette::Disabled, QColorGroup::Text, dkgray );
   /* same as text and fg */
   pal.setColor( QPalette::Active,   QColorGroup::ButtonText, text );
   pal.setColor( QPalette::Inactive, QColorGroup::ButtonText, text );
   pal.setColor( QPalette::Disabled, QColorGroup::ButtonText, dkgray );
   /* highlight */
   pal.setColor( QPalette::Active,   QColorGroup::Highlight, hilite );
   pal.setColor( QPalette::Inactive, QColorGroup::Highlight, hilite );
   pal.setColor( QPalette::Disabled, QColorGroup::Highlight, hilite );
   /* contrast with highlight */
   pal.setColor( QPalette::Active,
                 QColorGroup::HighlightedText, base );
   pal.setColor( QPalette::Inactive,
                 QColorGroup::HighlightedText, base );
   pal.setColor( QPalette::Disabled,
                 QColorGroup::HighlightedText, base );
   return pal;
}


QFont VkConfig::defaultAppFont()
{
   return m_defaultAppFont;
}


/* read functions ------------------------------------------------------ */
QString VkConfig::rdEntry( const QString &pKey, 
                           const QString &pGroup )
{
   QString aValue = QString::null;
   EntryKey entryKey( pGroup, pKey );

   EntryMapIterator aIt;
   aIt = m_EntryMap.find( entryKey );
   if ( aIt != m_EntryMap.end() ) {
      aValue = aIt.data().mValue;
   } else {
      /* end() points to the element which is one past the
         last element in the map; ergo, the key wasn't found. */
      VK_DEBUG( "VkConfig::rdEntry(): key not found.\n"
                "\tFile  = %s\n"
                "\tGroup = %s\n"
                "\tKey   = %s\n", 
                m_rcFileName.latin1(), pGroup.latin1(), pKey.latin1() );
   }

   return aValue;
}


int VkConfig::rdInt( const QString &pKey, const QString &pGroup )
{
   QString aValue = rdEntry( pKey, pGroup );
   if ( aValue.isNull() )
      return -1;

   bool ok;
   int aInt = aValue.toInt( &ok );
   return( ok ? aInt : -1 );
}


bool VkConfig::strToBool( QString str )
{
   if ( str == "true" || str == "on"   || 
        str == "yes"  || str == "1"    ||
        str == "T" )
      return true;

   return false;
}


bool VkConfig::rdBool( const QString &pKey, const QString &pGroup )
{
   return strToBool( rdEntry( pKey, pGroup ) );
}


/* guaranteed to return a valid font.  if an invalid font is
   found, the application's default font is used. */
QFont VkConfig::rdFont( const QString &pKey, 
                        const QString &pGroup/*=QString::null*/ )
{
   QFont aRetFont;

   QString pgroup = pGroup.isNull() ? "Fonts" : pGroup;
   QString aValue = rdEntry( pKey, pgroup/*"Fonts"*/ );
   if ( aValue.isNull() )
      return QFont();

   if ( aValue.contains( ',' ) > 5 ) {    // new format
      if ( !aRetFont.fromString( aValue ) )
         return QFont();
   }
   else { /* backward compatibility with older formats */
      /* find first part (font family) */
      int nIndex = aValue.find( ',' );
      if ( nIndex == -1 ) {
         return QFont();
      }
      aRetFont.setFamily( aValue.left( nIndex ) );
    
      /* find second part (point size) */
      int nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      if ( nIndex == -1 ) {
         return QFont();
      }
      aRetFont.setPointSize( aValue.mid( nOldIndex+1,
                                         nIndex-nOldIndex-1 ).toInt() );

      /* find third part (style hint) */
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if ( nIndex == -1 )
         return QFont();
      aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1, 
                                                           nIndex-nOldIndex-1 ).toUInt() );
    
      /* find fourth part (char set) */
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if ( nIndex == -1 )
         return QFont();
      QString chStr;     /* never used ... */
      chStr = aValue.mid( nOldIndex+1,
                          nIndex-nOldIndex-1 );

      /* find fifth part (weight) */
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      if ( nIndex == -1 )
         return QFont();
      aRetFont.setWeight( aValue.mid( nOldIndex+1,
                                      nIndex-nOldIndex-1 ).toUInt() );

      /* find sixth part (font bits) */
      uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();
      aRetFont.setItalic( nFontBits & 0x01 );
      aRetFont.setUnderline( nFontBits & 0x02 );
      aRetFont.setStrikeOut( nFontBits & 0x04 );
      aRetFont.setFixedPitch( nFontBits & 0x08 );
      aRetFont.setRawMode( nFontBits & 0x20 );
   }

   return aRetFont;
}


QColor VkConfig::rdColor( const QString &pKey )
{
   bool ok;
   QColor aRetColor;

   QString aValue = rdEntry( pKey, "Colors" );
   if ( !aValue.isEmpty() ) {
      int nRed = 0, nGreen = 0, nBlue = 0;

      /* find first part (red) */
      int nIndex = aValue.find( ',' );
      if ( nIndex == -1 )
         return QColor();
      nRed = aValue.left( nIndex ).toInt( &ok );

      /* find second part (green) */
      int nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      if ( nIndex == -1 )
         return QColor();
      nGreen = aValue.mid( nOldIndex+1, nIndex-nOldIndex-1 ).toInt( &ok );

      /* find third part (blue) */
      nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &ok );

      aRetColor.setRgb( nRed, nGreen, nBlue );
   }

   return aRetColor;
}



/* write functions ----------------------------------------------------- 
   called from
    - parseCmdArgs()
    - OptionWidget::saveEdit()/cancelEdit()
    - MainWindow::~MainWindow()
 */
void VkConfig::wrEntry( const QString &pValue,
                        const QString &pKey, const QString &pGroup )
{
   /* debug: check we aren't unwittingly inserting a new value. */
   EntryKey entryKey( pGroup, pKey );
   if ( !m_EntryMap.contains( entryKey ) ) {
      VK_DEBUG( "Config::wrEntry(): key not found.\n"
                "\tFile  = %s\n"
                "\tGroup = %s\n"
                "\tKey   = %s\n"
                "\tValue = %s\n", 
                m_rcFileName.latin1(), pGroup.latin1(),
                pKey.latin1(), pValue.latin1() );
   }

   m_dirty = true;
   /* the vkConfig object is dirty now - no going back. */
   /* TODO?
      currently no option for getting back to non-dirty state,
      for e.g. nicer dis/enable of menu item Options::Save_as_Default
      dirty = (current == initial vals (held in struct or reread from disk))
      do we really care? */

   /* set new value */
   EntryData entryData( pValue, true );
   /* rewrite the new value */
   insertData( entryKey, entryData );
}


/* special version of wrEntry: adds values to the existing entry,
   rather than replacing it */
void VkConfig::addEntry( const QString &pValue, 
                         const QString &pKey, const QString &pGroup )
{
   /* get hold of the current value(s) */
   QString curr_values = rdEntry( pKey, pGroup );

   /* concat curr_values with new value */
   if ( !curr_values.isEmpty() )
      curr_values += m_sep;
   curr_values += pValue;

   wrEntry( curr_values, pKey, pGroup );
}


void VkConfig::wrInt( const int pValue, const QString &pKey, 
                      const QString &pGroup )
{ wrEntry( QString::number( pValue ), pKey, pGroup ); }


void VkConfig::wrBool( const bool &pValue, const QString &pKey, 
                       const QString &pGroup )
{
   QString aValue = ( pValue == true ) ? "true" : "false";
   wrEntry( aValue, pKey, pGroup );
}


void VkConfig::wrFont( const QFont &pFont, const QString &pKey )
{ wrEntry( pFont.toString(), pKey, "Fonts" ); }


void VkConfig::wrColor( const QColor &pColor, const QString &pKey )
{
   QString aValue = "";
   if ( pColor.isValid() )
      aValue.sprintf( "%d,%d,%d", 
                      pColor.red(), pColor.green(), pColor.blue() );

   wrEntry( aValue, pKey, "Colors" );
}


/* private functions --------------------------------------------------- */
void VkConfig::insertData( const EntryKey &ekey, 
                           const EntryData &edata )
{
   EntryData &entry = m_EntryMap[ekey];
   entry = edata;
}


void VkConfig::backupConfigFile()
{
   QString bakfile = vk_mkstemp( m_rcFileName, "bak" );
   if (bakfile.isNull()) {
      /* rather unlikely... */
      VK_DEBUG( "failed to backup rc file" );
      return;
   }
	if ( !QFile::exists(m_rcFileName) ) {
      /* not much we can do about that... */
      VK_DEBUG( "no rc file to backup!" );
		return;
	}
	if ( !QDir().rename( m_rcFileName, bakfile ) ) {
      /* oh well... */
      VK_DEBUG( "rename failed: couldn't backup rc file" );
      return;
   }
   vkInfo( 0, "Writing Config File",
           "<p>Backed up previous config file to '%s'.</p>", 
           bakfile.latin1() );
}

bool VkConfig::parseFile( Valkyrie* vk, /*OUT*/EntryMap& dstMap )
{
   QFile rFile( m_rcFileName );
   if ( !rFile.open( IO_ReadOnly ) ) {
		/* Error: Failed to open file. */
		vkError( 0, "Parsing Config File",
               "<p>Failed to open config file '%s' for reading.<br>"
               "%s cannot run without this file.</p>", 
               m_rcFileName.latin1(), vkName() );
      return false;
   }

	/* beam me up, scotty */
	QTextStream stream( &rFile );
	EntryMap fileMap = parseConfigToMap( stream );
	rFile.close();

   QString defaultConfig = mkConfigDefaults( vk );
   QTextStream strm( &defaultConfig, IO_ReadOnly );
   EntryMap defaultMap = parseConfigToMap( strm );

	/* Check for correct rc version number
	   - if mismatch, print warning, update entries */
	QString vk_ver_rc = fileMap[ EntryKey( "valkyrie", "version" ) ].mValue;
	QString vk_ver    = vkVersion();

	if (vk_ver_rc != vk_ver) {  /* Version mismatch - fix rc file */
      vkInfo( 0, "Parsing Config File",
              "<p>Valkyrie version mismatch<br/>"
              "Config file: %s<br/>"
              "Valkyrie:    %s<br/>"
              "<br/>"
              "Updating configuration file...</p>",
              vk_ver_rc.latin1(), vkVersion() );

      /* dstMap = (defaultMap updated with fileMap values)*/
      return updateCfgFile( defaultMap, fileMap, dstMap );

   } else {

      /* check our default config has the same entry keys as the config file */
      /* entries are sorted, so we can easily test them */
      bool fileCfgOk=false;
      if (fileMap.count() == defaultMap.count()) {
         fileCfgOk = true;
         /* loop over entries in config file: same entries should exist in default cfg */
         EntryMapIterator aIt;
         for ( aIt = fileMap.begin(); aIt != fileMap.end(); ++aIt) {
            const EntryKey  &rcKey = aIt.key();
            if ( defaultMap.find(rcKey) == defaultMap.end() ) {
               /* missing key */
               fileCfgOk = false;
               break;
            }
         }
      }
      if (!fileCfgOk) {
         vkInfo( 0, "Parsing Config File",
                 "<p>Detected bad/missing config entry keys.<br/>"
                 "Attempting to fix...</p>" );
         
         /* dstMap = (defaultMap updated with fileMap values)*/
         return updateCfgFile( defaultMap, fileMap, dstMap );
      }
   }

	/* fileMap is fine: use that */
   dstMap = fileMap;
   return true;
}


bool VkConfig::updateCfgFile( EntryMap &newMap, EntryMap &fileMap,
                              /*OUT*/EntryMap& dstMap )
{
   /* loop over entries in old rc:
      if (entry exists in new map) copy value from old */
   EntryMapIterator aIt;
   for ( aIt = fileMap.begin(); aIt != fileMap.end(); ++aIt) {
      const EntryKey  &rcKey = aIt.key();
      if ( newMap.find(rcKey) != newMap.end() ) {
         newMap[ rcKey ] = aIt.data();
      }
   }
   /* but keep the new version! */
   newMap[ EntryKey( "valkyrie", "version" ) ].mValue = vkVersion();
   
   /* ... and keep the new path to vk_logmerge */
   newMap[ EntryKey( "valkyrie", "merge-exec" ) ].mValue = BIN_LOGMERGE;

   /* write out new config */
   if ( !writeConfig( newMap, true ) ) {
      VK_DEBUG( "failed to write config file" );
      return false;
   }
   
   /* and return new map */
   dstMap = newMap;
   return true;
}


EntryMap VkConfig::parseConfigToMap( QTextStream &stream )
{
   EntryMap rcMap;
   QString  line;
   QString  aGroup;

   stream.setEncoding( QTextStream::UnicodeUTF8 );
   while ( !stream.atEnd() ) {

      line = stream.readLine();
      if ( line.isEmpty() ) {         /* empty line... skip it   */
         continue;
      }
      if ( line[0] == QChar('#') ) {  /* comment line... skip it */
         continue;
      }
    
      int len = line.length();
      if ( line[0] == QChar('[') && line[len-1] == QChar(']') ) {
         /* found a group */
         line.setLength( len-1 );      /* chop off the ']' */
         aGroup = line.remove(0, 1);   /* ditto re the '[' */
      }
      else {
         /* found a key --> value pair */
         int pos = line.find('=');
         QString key   = line.left( pos );
         QString value = line.right( len-pos-1 );

         EntryKey entryKey( aGroup, key );
         EntryData entryData( value, false );

			/* insert into the temporary scratchpad map */
			rcMap.insert( entryKey, entryData );
      }
   }
	return rcMap;
}


bool VkConfig::writeConfig( EntryMap rcMap, bool backup/*=false*/ )
{
	if (backup) {
		/* move old config file, if it exists */
		backupConfigFile();
	}

   /* The temporary map should now be full of ALL entries.
      Write it out to disk. */
   QFile outF( m_rcFileName );
   if ( !outF.open( IO_WriteOnly ) ) {
		vkError( 0, "Writing Config File",
               "<p>Failed to open config file '%s' for writing.</p>", 
               m_rcFileName.latin1() );
		return false;
	}
	QTextStream aStream( &outF );
	
	/* Write comment header */
	QString hdr = mkConfigHeader();
	aStream << hdr.latin1();

	/* Write out map entries under relevant group sections */
   QString currGroup;
   EntryMapIterator aIt;
   for ( aIt = rcMap.begin(); aIt != rcMap.end(); ++aIt) {

      const EntryKey  &currKey   = aIt.key();
      const EntryData &currEntry = aIt.data();

      /* new group */
      if ( currGroup != currKey.mGroup ) {
         currGroup = currKey.mGroup;
         aStream << "\n[" << currGroup << "]\n";
      }

      /* group data */
		aStream << currKey.mKey << "=" << currEntry.mValue << "\n";
   }

	outF.close();
	return true;
}


/* Create the default configuration file.  -----------------------------
 */
QString VkConfig::mkConfigHeader( void )
{
   QDateTime dt = QDateTime::currentDateTime();
   QString hdr = QString("# %1 configuration file\n").arg(vkName());
   hdr += "# " + dt.toString( "MMMM d hh:mm yyyy" ) + "\n";
   hdr += "# Warning: This file is auto-generated, edits may be lost!\n";
	return hdr;
}

QString VkConfig::mkConfigDefaults( Valkyrie* vk )
{
	QString str;
	QTextStream ts( &str, IO_WriteOnly );

	QString header = mkConfigHeader();

   const char * window_colors = "[Colors]\n\
background=214,205,187\n\
base=255,255,255\n\
dkgray=128,128,128\n\
editColor=254,222,190\n\
highlight=147,40,40\n\
nullColor=239,227,211\n\
text=0,0,0\n\n";

   const char * mainwin_size_pos = "[MainWin]\n\
height=600\n\
width=550\n\
x-pos=400\n\
y-pos=0\n\n";

   const char * dbase = "[Database]\n\
user=auser\n\
host=localhost\n\
pword=123\n\
dbase=valkyrie\n\
logging=true\n\
logfile=\n\n";

	ts << header << "\n" << window_colors << mainwin_size_pos << dbase;

	/* a new tool might have been added, or other changes made, in
      which case this fn wouldn't contain the correct options=values
      if it were hard-wired in. Better safe than sorry: just get all
      tools that are present to spew their options/flags out to disk. */
   VkObject* vkobj;
   VkObjectList vkObjecList = vk->vkObjList();
   for ( vkobj = vkObjecList.first(); vkobj; vkobj = vkObjecList.next() ) {
		ts << vkobj->configEntries();
	}
	return str;
}


/* Create the default configuration file.  -----------------------------
   The first time valkyrie is started, vkConfig looks to see if this
   file is present in the user's home dir.  If not, it writes the
   relevant data to ~/.PACKAGE/PACKAGErc */
bool VkConfig::writeConfigDefaults( Valkyrie* vk )
{
   QString default_config = mkConfigDefaults( vk );
   QTextStream strm( &default_config, IO_ReadOnly );
   EntryMap rcMap = parseConfigToMap( strm );
   
   /* Set valkyrie version: used for rc upgrading */
   rcMap[ EntryKey( "valkyrie", "version" )  ].mValue
      = PACKAGE_VERSION;

   /* Set our 'configured' valgrind paths, if we have them */
   {
      rcMap[ EntryKey( "valkyrie", "merge-exec" )  ].mValue
         = BIN_LOGMERGE;

      rcMap[ EntryKey( "valkyrie", "vg-exec" )     ].mValue
         = BIN_VALGRIND;

      rcMap[ EntryKey( "valgrind", "suppressions" )].mValue
         = "";

      rcMap[ EntryKey( "valgrind", "supps-dirs" )  ].mValue
         = suppDir();
   }

   /* write out new config */
   if ( !writeConfig( rcMap, true ) ) {
      VK_DEBUG( "failed to write default config file" );
      return false;
   }
   return true;
}


/* check rc file or dir exists, is read & writeable */
bool VkConfig::checkRCEntry( QString path, Valkyrie* vk)
{
   QFileInfo fi( path );
   if ( !fi.exists() ) {
      VK_DEBUG("creating config entry '%s'.", path.latin1() );
      /* if it's a dir, create it */
      if ( path.endsWith("/") ) {      // TODO: more robust but abstract way to do this?
         QDir dir;
         if (!dir.mkdir( path ) ) {
            VK_DEBUG("failed creation of new dir '%s'.", path.latin1() );
            return false;
         }
      } else {
         /* if it's a file... */
         if (path == m_rcFileName) {
            /* create shiny new rc file */
            if ( !writeConfigDefaults( vk ) ) {
               VK_DEBUG("failed to create default config file '%s'.",
                        path.latin1() );
               return false;
            }
         }
      }
   }

   /* still doesn't exist?! */
   if ( !fi.exists() ) {
      VK_DEBUG("rc entry '%s' doesn't exist.", fi.filePath().latin1() );
      return false;
   }
   /* permissions ok? */
   if ( !fi.isReadable() ) {
      VK_DEBUG("rc entry '%s' not readable.", fi.filePath().latin1() );
      return false;
   }
   if ( !fi.isWritable() ) {
      VK_DEBUG("rc entry '%s' not writable.", fi.filePath().latin1() );
      return false;
   }
   if ( fi.isDir() && !fi.isExecutable() ) {
      VK_DEBUG("rc dir '%s' not executable.", fi.filePath().latin1() );
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
   for ( it = entries.begin(); it != entries.end(); ++it) {
      if ( !QFile::exists(*it) ) {
         VK_DEBUG("rc entry '%s' doesn't exist.", (*it).latin1() );
         ok = false;
         break;
      }
   }

   if (!ok) { /* rc tree !exists or !well */
      /* this an existing tree?
         if so, tell the user there was a problem */
      if ( QFile::exists(m_rcPath) ) {
         vkInfo( 0, "Checking Config Setup",
                 "<p>Detected missing configuration files/dirs.<br/>"
                 "Attempting to recreate from defaults...</p>" );
      }
      // else clean tree to setup: just do it.
   }

   /* run through rc entries, checking existence/permissions and
      creating them if necessary */
   for ( it = entries.begin(); it != entries.end(); ++it) {
      if (!checkRCEntry(*it, vk)) {
         return false;
      }
   }

   /* Further, check for temporary log dir (VK_LOGS_DIR), make if !exists */
   if ( !QFile::exists( VK_LOGS_DIR ) ) {
      if ( !checkRCEntry( VK_LOGS_DIR, vk) ) {
         return false;
      }
   }

   return true;
}
