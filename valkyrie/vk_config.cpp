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
#include <qfiledialog.h>


VkConfig::~VkConfig()
{}


/* test if config holds data difft to that held on disk */
bool VkConfig::isDirty()
{ return m_dirty; }


/* write config entries to disk
   called by user from menu 'Options::Save as Default' ---------------------- */
void VkConfig::sync()
{
   if ( !m_dirty ) return;

   /* read config file from disk, and fill the temporary structure 
      with entries from the file */
	bool ok;
	EntryMap rcMap = parseFile( &ok );
	if (!ok) {
		fprintf(stderr, "Error: sync(): Failed to parse configuration file\n");
		// TODO
		return;
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
   ok = writeConfig( rcMap );
	if (!ok) {
		fprintf(stderr, "Error: sync(): Failed to write configuration file\n");
		// TODO
		return;
	}

	m_dirty = false;
}


/* class VkConfig ------------------------------------------------------ */
VkConfig::VkConfig( Valkyrie* vk, bool *ok ) : QObject( 0, "vkConfig" )
{
   m_valkyrie = vk;

   m_EntryMap.clear();
   m_sep      = ',';         /* separator for lists of strings */
   m_dirty    = false;

   QString packagePath = VK_INSTALL_PREFIX;
   m_vkdocPath    = packagePath + VK_DOC_PATH;
   m_vgdocPath    = packagePath + VG_DOC_PATH;

   m_vk_name      = PACKAGE;
   m_vk_Name      = PACKAGE_NAME;
   m_vk_version   = PACKAGE_VERSION;
   m_vk_copyright = VK_COPYRIGHT;
   m_vk_author    = VK_AUTHOR;
   m_vk_email     = PACKAGE_BUGREPORT;
   m_vg_copyright = VG_COPYRIGHT;

   m_rcPath.sprintf( "%s/.%s", QDir::homeDirPath().latin1(), vkname() );
   m_rcFileName.sprintf( "%s/%src", m_rcPath.latin1(), vkname() );
   m_dbasePath = m_rcPath + VK_DBASE_DIR;
   m_logsPath  = m_rcPath + VK_LOGS_DIR;
   m_suppPath  = m_rcPath + VK_SUPPS_DIR;

   if ( !checkDirs() ) {     /* we always do this on startup */
      *ok = false;
      return;
   }
   m_rcPath    += "/";


   int num_tries = 0;
 retry:
   if ( num_tries > 1 )
      return;

   RetVal rval = checkAccess();
   switch ( rval ) {

   case Okay:
		m_EntryMap = parseFile( ok );
      break;

   case CreateRcFile:
      vkInfo( 0, "Configuration",
              "<p>The configuration file '%s' does not exist, "
              "and %s cannot run without this file.<br>"
              "Creating it now ...</p>", 
              m_rcFileName.latin1(), vkName() );
      writeConfigDefaults();
      num_tries++;
      goto retry;      /* try again */
      break;

   case NoPerms:
      vkFatal( 0, "Configuration",
               "<p>You do not have read/write permissions set"
               "on the directory %s</p>", m_rcPath.latin1() );
      break;

   case BadFilename:
   case NoDir:
   case Fail:
      vkFatal( 0, "Config Creation Failed",
               "<p>Initialisation of Config failed.</p>" );
      break;
   }
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

QString VkConfig::vgdocDir()  { return m_vgdocPath; }
/* ~/.valkyrie/ */
QString VkConfig::rcDir()     { return m_rcPath;    }
/* ~/.valkyrie/dbase/ */
QString VkConfig::dbaseDir()  { return m_dbasePath; }
/* ~/.valkyrie/logs/ */
QString VkConfig::logsDir()   { return m_logsPath;  }
/* ~/.valkyrie/suppressions/ */
QString VkConfig::suppDir()   { return m_suppPath;  }


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


bool VkConfig::rdBool( const QString &pKey, const QString &pGroup )
{
   QString aValue = rdEntry( pKey, pGroup );

   if ( aValue == "true" || aValue == "on"   || 
        aValue == "yes"  || aValue == "1"    ||
        aValue == "T" )
      return true;

   return false;
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
	QString dt = QDateTime::currentDateTime().toString( "_yyyy.MM.dd_hh.mm.ss");
	QString bak = m_rcFileName + dt + ".bak";
	QDir d;
	if ( !d.exists(m_rcFileName) ) {
		// TODO: Err
		return;
	}
	d.rename( m_rcFileName, bak );
	fprintf(stderr, "Backed up old config file to: %s\n", bak.latin1());
}

EntryMap VkConfig::parseFile( bool *ok )
{
   EntryMap rcMap, newMap;
   QFile rFile( m_rcFileName );
   if ( !rFile.open( IO_ReadOnly ) ) {
		/* Error: Failed to open file. */
		vkFatal( 0, "Parse Config File",
               "<p>Failed to open the file %s for reading.<br>"
               "%s cannot run without this file.</p>", 
               m_rcFileName.latin1(), vkName() );
		*ok = false;
      return EntryMap();
   }
	*ok = true;

	/* beam me up, scotty */
	QTextStream stream( &rFile );
	rcMap = parseConfigToMap( stream );
	rFile.close();


	/* Check for correct rc version number
	   - if mismatch, print warning, update entries */
	QString vk_ver_rc = rcMap[ EntryKey( "valkyrie", "version" ) ].mValue;
	QString vk_ver    = vkVersion();

	if (vk_ver_rc != vk_ver) {  /* Version mismatch - fix rc file */
		fprintf(stderr, "Warning: Configuration file version mismatch:\n");
		fprintf(stderr, "Configuration file: %s\n", vk_ver_rc.latin1());
		fprintf(stderr, "Valkyrie:    %s\n", vkVersion());

		/* Create new default config, bringing over any old values */
		fprintf(stderr, "Updating configuration file...\n\n");
		QString new_config = mkConfigDefaults();
		QTextStream strm( &new_config, IO_ReadOnly );
      newMap = parseConfigToMap( strm );

		/* loop over entries in old rc:
         if (entry exists in new map) copy value from old */
		EntryMapIterator aIt;
		for ( aIt = rcMap.begin(); aIt != rcMap.end(); ++aIt) {
			const EntryKey  &rcKey = aIt.key();
			if ( newMap.find(rcKey) != newMap.end() ) {
				newMap[ rcKey ] = aIt.data();
			}
		}
		/* but keep the new version! */
		newMap[ EntryKey( "valkyrie", "version" ) ].mValue = vkVersion();

		/* write out new config */
		*ok = writeConfig( newMap, true );
		if (!*ok) {
			fprintf(stderr, "Error: parseFile(): Failed to write new configuration file\n");
			// TODO
		}

		/* and return new map */
		return newMap;
	}
	/* return map */
	return rcMap;
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
		// TODO
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


VkConfig::RetVal VkConfig::checkAccess() const
{
   /* 0. first things first .... */
   if ( m_rcFileName.isEmpty() ) {
      VK_DEBUG( "checkAccess( %s )\n"
                "m_rcFileName is empty", VK_STRLOC );
      return BadFilename;
   }

   /* 1. check the /rc/ directory actually exists */
   if ( 0 != access( m_rcPath, F_OK ) ) {
      VK_DEBUG("checkAccess( %s )\n" 
               "The directory '%s' does not exist", 
               VK_STRLOC, m_rcPath.latin1() );
      return NoDir;
   }

   /* 2. ... and that the user has read/write permissions set.  
      can we allow the write?  we can, if the program does not run
      SUID.  but if it runs SUID, we must check if the user would be
      allowed to write if it wasn't SUID. */
   if ( 0 != access( m_rcPath, R_OK & W_OK ) ) {
      return NoPerms;
   }

   /* 3. check the rcfile actually exists. If not, create it now */
   if ( 0 != access( m_rcFileName, F_OK ) ) {
      return CreateRcFile;
   }

   /* 4. if it already exists, can we read / write it? */
   if ( 0 != access( m_rcFileName, R_OK & W_OK ) ) {
      vkInfo( 0, "Configuration",
              "<p>The file %s seems to be corrupted, and "
              "valkyrie cannot run without this file.</p>"
              "<p>Re-creating it now ... .. </p>", 
              m_rcFileName.latin1() );
      return CreateRcFile;
   }

   return Okay;
}


/* Returns a ptr to be tool currently set in [valgrind:tool] */
ToolObject* VkConfig::mainToolObj()
{
   QString name = rdEntry("tool", "valgrind");
   ToolObjList tools = m_valkyrie->valgrind()->toolObjList();
   for ( ToolObject* tool=tools.first(); tool; tool=tools.next() ) {
      if ( tool->name() == name )
         return tool;
   }
   vk_assert_never_reached();
   return NULL;
}

/* returns the name of the current tool in [valgrind:tool] */
QString VkConfig::mainToolObjName()
{ return rdEntry("tool", "valgrind"); }

/* returns the tool id of [valgrind:tool] */
int VkConfig::mainToolObjId()
{ return mainToolObj()->objId(); }



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

QString VkConfig::mkConfigDefaults( void )
{
	QString str;
	QTextStream ts( &str, IO_WriteOnly );

	QString header = mkConfigHeader();

   char * window_colors = "[Colors]\n\
background=214,205,187\n\
base=255,255,255\n\
dkgray=128,128,128\n\
editColor=254,222,190\n\
highlight=147,40,40\n\
nullColor=239,227,211\n\
text=0,0,0\n\n";

   char * mainwin_size_pos = "[MainWin]\n\
height=600\n\
width=550\n\
x-pos=400\n\
y-pos=0\n\n";

   char * dbase = "[Database]\n\
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
   VkObjectList vkObjecList = m_valkyrie->vkObjList();
   for ( vkobj = vkObjecList.first(); vkobj; vkobj = vkObjecList.next() ) {
		ts << vkobj->configEntries();
	}
	return str;
}


/* Create the default configuration file.  -----------------------------
   The first time valkyrie is started, vkConfig looks to see if this
   file is present in the user's home dir.  If not, it writes the
   relevant data to ~/.PACKAGE/PACKAGErc */
void VkConfig::writeConfigDefaults()
{
	QString default_config = mkConfigDefaults();
	QTextStream strm( &default_config, IO_ReadOnly );
	EntryMap rcMap = parseConfigToMap( strm );
	
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
		fprintf(stderr, "Error: writeConfigDefaults(): Failed to write configuration file\n");
		// TODO: Err
		return;
	}

   //newConfigFile = true;
}


/* ~/.PACKAGE is a sine qua non ---------------------------------------- 
   checks to see if ~/valkyrie/ and its required sub-dirs are
   all present and correct.  If not, tries to create them.
   ~/valkyrie/ 
   - valkyrierc
   - dbase/
   - logs/
   - suppressions/ 
*/
bool VkConfig::checkDirs()
{
   QString msg = "";
   bool success = true;

   QDir vk_dir( m_rcPath );

   enum State { 
      CHECK_DIR=0, CHECK_SUB_DIRS, MK_TOP_DIR, MK_DB_DIR, 
      MK_LOG_DIR,  MK_SUPP_DIR, DONE, GIVE_UP };
   State state = CHECK_DIR;

   bool not_done = true;
   while ( not_done ) {

      switch ( state ) {

         /* normal startup checks ----------------------------------------- */
      case CHECK_DIR:        /* does ~/.PACKAGE/ exist ? */
         state = vk_dir.exists() ? CHECK_SUB_DIRS : MK_TOP_DIR;
         break;

      case CHECK_SUB_DIRS: { /* check sub-dirs */
         const QFileInfoList * files = vk_dir.entryInfoList();
         QFileInfoListIterator it( *files );
         QFileInfo * fi;
         while ( ( fi=it.current() ) != 0 ) {
            ++it;
            if ( fi->fileName() == "." || fi->fileName() == ".." ) ;
            else if ( fi->isFile() && fi->isReadable() &&
                      fi->fileName() == "valkyrierc" ) ;
            else if ( fi->isDir() && fi->isReadable() ) {
               if ( fi->fileName() == "dbase" ) ;
               else if ( fi->fileName() == "logs" ) ;
               else if ( fi->fileName() == "suppressions" ) ;
               else { /* problem */
                  state = GIVE_UP;
                  break;
               }
            }
         }
         state = (state == GIVE_UP) ? state : DONE;
      } break;

      /* first time ever startup --------------------------------------- */
      case MK_TOP_DIR:       /* create '~/PACKAGE' */
         state = vk_dir.mkdir( m_rcPath ) ? MK_DB_DIR : GIVE_UP;
         break;

      case MK_DB_DIR:        /* create sub-dir '/dbase' */
         state = vk_dir.mkdir( m_dbasePath ) ? MK_LOG_DIR : GIVE_UP;
         break;

      case MK_LOG_DIR:       /* create sub-dir '/logs' */
         state = vk_dir.mkdir( m_logsPath ) ? MK_SUPP_DIR : GIVE_UP;
         break;

         /* last case statement MUST set 'state = DONE || GIVE_UP' */
      case MK_SUPP_DIR:      /* create sub-dir '/suppressions' */
         state = vk_dir.mkdir( m_suppPath ) ? DONE : GIVE_UP;
         if ( state == DONE )
            break;

      case GIVE_UP:
         not_done = false;
         success = false;
         msg.sprintf( "<p>There is a problem with '%s'.<br>"
                      "Either some files or sub-directories do not exist, "
                      "or the permissions are not set correctly."
                      "<p>Please check and retry.</p>", m_rcPath.latin1() );
         break;

      case DONE:
         not_done = false;
         break;

      }  /* end switch ( state ) */
   }    /* end while (1) */

   if ( !msg.isEmpty() ) {
      vkError( 0, "Directory Error", msg.latin1() );
   }

   return success;
}
