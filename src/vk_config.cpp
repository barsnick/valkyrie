/*---------------------------------------------------------------------- 
  Configuration file Parser                                vk_config.cpp
  ----------------------------------------------------------------------
*/

#include "vk_config.h"
#include "vk_include.h"
#include "vk_utils.h"       /* VK_DEBUG */
#include "vk_msgbox.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>         /* close, access */

#include <qdatetime.h>
#include <qdir.h>
#include <qfiledialog.h>


VkConfig::~VkConfig()
{ 
  sync();
  vkObjectList.setAutoDelete( true );
  vkObjectList.clear();
  vkObjectList.setAutoDelete( false );
}


/* set mDirty to false in order to stop config from writing any
   entries-set-so-far to disk.  called by main() after
   parseOptions() returns false. --------------------------------------- */
void VkConfig::dontSync() 
{ mDirty = false; }


/* write-sync is only necessary if there are dirty entries ------------- */
void VkConfig::sync()
{
  if ( mDirty ) {
    writebackConfig();
    mDirty = false;
  }
}


/* class VkConfig ------------------------------------------------------ */
VkConfig::VkConfig( bool *ok ) : QObject( 0, "vkConfig" )
{
  mEntryMap.clear();
  sep       = ';';         /* separator for lists of strings */
  mDirty    = false;
	newConfigFile = false;   /* set to true in mkConfigFile() */

  mPackagePath = installPath();
  vkdocPath    = mPackagePath + DOCS_PATH;
	vgdocPath    = vkdocPath + "manual/";
  imgPath      = mPackagePath + ICONS_PATH;

  rcPath.sprintf( "%s/.%s-%s", 
                  QDir::homeDirPath().ascii(), vkname(), vkVersion() );
  rcFileName.sprintf( "%s/%src", rcPath.ascii(), vkname() );

  dbasePath = rcPath + DBASE_DIR;
  logsPath  = rcPath + LOGS_DIR;
  suppPath  = rcPath + SUPPS_DIR;

  if ( !checkDirs() ) {	   /* we always do this on startup */
    *ok = false;
    return;
  }
  vkdocPath += "/";
  imgPath   += "/";
  
  rcPath    += "/";
  dbasePath += "/";
  logsPath  += "/";
  suppPath  += "/";

  /* The various valgrind, valkyrie, and tool objects ------------------
     Initialise these first, so that if ~/.PACKAGE/PACKAGErc does not
     exist, vkConfig can write the various options to the config file. */
  initVkObjects();

  int num_tries = 0;
 retry:
  if ( num_tries > 1 )
    return;

  RetVal rval = checkAccess();
  switch ( rval ) {

  case Okay:
    *ok = true;
    if ( parseFile() == Fail ) {
      *ok = false;
    } break;

  case BadRcVersion:
    vkInfo( 0, "Configuration",
            "The configuration file '%s' version is invalid.<br> "
            "Removing and re-creating it now ... ... ", 
            rcFileName.latin1() );
    mkConfigFile( true );
    num_tries++;
    goto retry;      /* try again */

  case CreateRcFile:
    vkInfo( 0, "Configuration",
            "The configuration file '%s' does not exist, "
            "and %s cannot run without this file.<br>"
            "Creating it now ... ... ", 
            rcFileName.latin1(), vkName() );
    mkConfigFile();
    num_tries++;
    goto retry;      /* try again */

  case NoPerms:
    vkFatal( 0, "Configuration" 
             "You do not have read/write permissions set"
             "on the directory %s", rcPath.latin1() );
    break;

  case BadFilename:
  case NoDir:
  case BadRcFile:
  case Fail:
    vkFatal( 0, "Config Creation Failed",
             "Initialisation of Config failed" );
    break;
  }

  /* check valgrind/suppn/etc. paths are set correctly ----------------- 
		 we only do this the very first time valkyrie is started up */
  //RM:if ( !checkPaths() ) {
  //  *ok = false;
  //}
	//if ( newConfigFile ) {
	//	*ok = checkPaths();
	//}

}


/* misc. make-life-easier stuff -------------------------------------- */

QString VkConfig::vkdocDir()     //  /valkyrie-inst-dir/docs/
{ return vkdocPath; }

// FIXME: this should be read from valkyrie's opts
// /usr/share/doc/packages/valgrind/
QString VkConfig::vgdocDir()
{ 
	return vgdocPath; 
	/*FIXME: return rdEntry( "vg-doc-dir", "valkyrie" );*/ 
}

// /usr/share/doc/packages/valgrind/
//RM: QString VkConfig::vgManual( QString url )
//{ 
//	return vgdocPath + url; 
	/*FIXME: return rdEntry("vg-doc-dir", "valkyrie") + url;*/ 
//}

QString VkConfig::imgDir()     //  /valkyrie-inst-dir/icons/
{ return imgPath; }

QString VkConfig::rcDir()      // ~/.valkyrie-X.X.X/
{ return rcPath;  }

QString VkConfig::dbaseDir()   // ~/.valkyrie-X.X.X/dbase/
{ return dbasePath; }

QString VkConfig::logsDir()    // ~/.valkyrie-X.X.X/logs/
{ return logsPath; }

QString VkConfig::suppDir()    // ~/.valkyrie-X.X.X/suppressions/
{ return suppPath;  }

VkObjectList VkConfig::vkObjList()
{ return vkObjectList; }

QPixmap VkConfig::pixmap( QString pix )
{
  QString fname = imgPath + pix;
  QPixmap pm( fname );
  return pm;
}


/* read fns ------------------------------------------------------------ */
QString VkConfig::rdEntry( const QString &pKey, 
                           const QString &pGroup )
{
  QString aValue = QString::null;
  EntryKey entryKey( pGroup, pKey );

  EntryMapIterator aIt;
  aIt = mEntryMap.find( entryKey );
  if ( aIt != mEntryMap.end() ) {
    aValue = aIt.data().mValue;
  } else {
    /* debug: end() points to the element which is one past the
       last element in the map; ergo, the key wasn't found. */
    VK_DEBUG( "VkConfig::rdEntry(): key not found.\n"
             "\tFile  = %s\n"
             "\tGroup = %s\n"
             "\tKey   = %s\n", 
             rcFileName.latin1(), pGroup.latin1(), pKey.latin1() );
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


/* Guaranteed to return a valid font.  If an invalid font is
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
  else { // backward compatibility with older formats
    // find first part (font family)
    int nIndex = aValue.find( ',' );
    if ( nIndex == -1 ) {
      return QFont();
    }
    aRetFont.setFamily( aValue.left( nIndex ) );
    
    // find second part (point size)
    int nOldIndex = nIndex;
    nIndex = aValue.find( ',', nOldIndex+1 );
    if ( nIndex == -1 ) {
      return QFont();
    }
    aRetFont.setPointSize( aValue.mid( nOldIndex+1,
                           nIndex-nOldIndex-1 ).toInt() );

    // find third part (style hint)
    nOldIndex = nIndex;
    nIndex = aValue.find( ',', nOldIndex+1 );

    if ( nIndex == -1 )
      return QFont();
    aRetFont.setStyleHint( (QFont::StyleHint)aValue.mid( nOldIndex+1, 
                                     nIndex-nOldIndex-1 ).toUInt() );
    
    // find fourth part (char set)
    nOldIndex = nIndex;
    nIndex = aValue.find( ',', nOldIndex+1 );

    if ( nIndex == -1 )
      return QFont();
    QString chStr;     /* never used ... */
    chStr = aValue.mid( nOldIndex+1,
                        nIndex-nOldIndex-1 );

    // find fifth part (weight)
    nOldIndex = nIndex;
    nIndex = aValue.find( ',', nOldIndex+1 );
    if ( nIndex == -1 )
      return QFont();
    aRetFont.setWeight( aValue.mid( nOldIndex+1,
                                    nIndex-nOldIndex-1 ).toUInt() );

    // find sixth part (font bits)
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

    // find first part (red)
    int nIndex = aValue.find( ',' );
    if ( nIndex == -1 )
      return QColor();
    nRed = aValue.left( nIndex ).toInt( &ok );

    // find second part (green)
    int nOldIndex = nIndex;
    nIndex = aValue.find( ',', nOldIndex+1 );
    if ( nIndex == -1 )
      return QColor();
    nGreen = aValue.mid( nOldIndex+1, nIndex-nOldIndex-1 ).toInt( &ok );

    // find third part (blue)
    nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &ok );

    aRetColor.setRgb( nRed, nGreen, nBlue );
  }

  return aRetColor;
}



/* write fns ----------------------------------------------------------- 
   the Config object is dirty now */
void VkConfig::wrEntry( const QString &pValue,
                        const QString &pKey, const QString &pGroup )
{
  /* debug: check we aren't unwittingly inserting a new value. */
  EntryKey entryKey( pGroup, pKey );
  if ( !mEntryMap.contains( entryKey ) ) {
    VK_DEBUG( "Config::wrEntry(): key not found.\n"
             "\tFile  = %s\n"
             "\tGroup = %s\n"
             "\tKey   = %s\n"
             "\tValue = %s\n", 
             rcFileName.latin1(), pGroup.latin1(),
             pKey.latin1(), pValue.latin1() );
  }

  mDirty = true;
  // set new value
  EntryData entryData( pValue, true );
  // rewrite the new value
  insertData( entryKey, entryData );
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
  EntryData &entry = mEntryMap[ekey];
  entry = edata;
}


VkConfig::RetVal VkConfig::parseFile()
{
  QFile rFile( rcFileName );
  if ( !rFile.open( IO_ReadOnly ) ) {
    vkFatal( 0, "Parse Config File" 
            "Failed to open the file %s for reading.<br>"
             "%s cannot run without this file.", 
             rcFileName.latin1(), vkName() );
    return Fail;
  } else {
    /* beam me up, scotty */
    parseConfigFile( rFile );
    rFile.close();
  }
  return Okay;
}


void VkConfig::parseConfigFile( QFile &rFile, EntryMap *writeBackMap )
{
  if ( !rFile.isOpen() ) {
    VK_DEBUG( "parseConfigFile( %s )\n"
             "rFile '%s' is not open for parsing", 
             VK_STRLOC, rcFileName.latin1() );
    return;
  }

  QString line;
  QString aGroup;

  QTextStream stream( &rFile );
  stream.setEncoding( QTextStream::UnicodeUTF8 );
  while ( !stream.atEnd() ) {

    line = stream.readLine();
    if ( line.isEmpty() ) {       // empty line... skip it
      continue;
    }
    if ( line[0] == QChar('#') ) { // comment line... skip it
      continue;
    }
    
    int len = line.length();
    if ( line[0] == QChar('[') && line[len-1] == QChar(']') ) {
      // found a group
      line.setLength( len-1 );    // chop off the ']'
      aGroup = line.remove(0, 1); // ditto re the '['
    }
    else {
      // key --> value pair
      int pos = line.find('=');
      QString key   = line.left( pos );
      QString value = line.right( len-pos-1 );

      EntryKey entryKey( aGroup, key );
      EntryData entryData( value, false );

      // insert into the temporary scratchpad map
      if ( writeBackMap )
        writeBackMap->insert( entryKey, entryData );
      else  // directly insert value into config object
        insertData( entryKey, entryData );

    }
  }
}


void VkConfig::writebackConfig()
{
  EntryMap tmpMap;

  /* Read config file from disk, and fill the temporary structure 
     with entries from the file */
  QFile rcFile( rcFileName );
  if ( !rcFile.open(IO_ReadOnly) ) {
    VK_DEBUG( "writebackConfig( %s )\n"
             "failed to open rcfile: %s", 
             VK_STRLOC, rcFileName.latin1() );
  } else {
    parseConfigFile( rcFile, &tmpMap );
    rcFile.close();
  }

  /* augment this structure with the dirty entries from the
     config object */
  EntryMapIterator aIt;
  for ( aIt = mEntryMap.begin(); aIt != mEntryMap.end(); ++aIt) {
    const EntryData &dirtyEntry = aIt.data();
    if ( !dirtyEntry.mDirty ) 
      continue;
    /* put dirty entries from the config object into the
       temporary map, possibly replacing an existing entry */
    tmpMap.replace( aIt.key(), dirtyEntry );
  }

  /* The temporary map should now be full of ALL entries.
     Write it out to disk. */
  FILE *pStream = 0;
  int fd = open( rcFileName, O_WRONLY | O_TRUNC);
  if ( fd < 0 )
    return;
  pStream = fdopen( fd, "w");
  if ( !pStream ) {
    close(fd);
    return;
  }

  bool firstEntry = true;
  QString currGroup;
  for ( aIt = tmpMap.begin(); aIt != tmpMap.end(); ++aIt) {

    const EntryKey  &currKey   = aIt.key();
    const EntryData &currEntry = aIt.data();

    // new group
    if ( currGroup != currKey.mGroup ) {

      if ( firstEntry ) {
        QString hdr = QString("# %1 %2 Configuration File\n")
                             .arg(vkName()).arg(vkVersion());

        fprintf( pStream, hdr.latin1() );

        QDateTime dt = QDateTime::currentDateTime();
        hdr = "# " + dt.toString( "MMMM d hh:mm yyyy" ) + "\n";
        fprintf( pStream, hdr.latin1() );
      }

      currGroup = currKey.mGroup;
      fprintf( pStream, "\n[%s]\n", currGroup.latin1() );
    }
    firstEntry = false;

    // group data
    fprintf( pStream, "%s=%s\n",  
             currKey.mKey.latin1(), currEntry.mValue.latin1() );
  }

  fclose( pStream );
}


VkConfig::RetVal VkConfig::checkAccess() const
{
  int ok;

  // 0. first things first ....
  if ( rcFileName.isEmpty() ) {
    VK_DEBUG( "checkAccess( %s )\n"
             "rcFileName is empty", VK_STRLOC );
    return BadFilename;
  }

  // 1. check the /rc/ directory actually exists
  ok = access( rcPath, F_OK );
  if ( ok != 0 ) {
    VK_DEBUG("checkAccess( %s )\n" 
            "The directory '%s' does not exist", 
            VK_STRLOC, rcPath.latin1() );
    return NoDir;
  }

  /* 2. ... and that the user has read/write permissions set
     Can we allow the write? We can, if the program does not run
     SUID. But if it runs SUID, we must check if the user would
     be allowed to write if it wasn't SUID. */
  ok = access( rcPath, R_OK & W_OK );
  if ( ok != 0 ) {
    return NoPerms;
  }

  // 3. check the rcfile actually exists. If not, create it now.
  ok = access( rcFileName, F_OK );
  if ( ok != 0 ) {
    return CreateRcFile;
  }

  // 4. if it already exists, can we read / write it?
  ok = access( rcFileName, R_OK & W_OK );
  if ( ok != 0 ) {
    ok = vkQuery( 0, 2, "Configuration", 
                  "The file %s seems to be corrupted, and\n"
                  "%s cannot run without this file."
                  "Shall I recreate it now?", 
                  rcFileName.latin1(), vkName() );
    if ( ok == MsgBox::vkNo )
      return BadRcFile;
    else
      return CreateRcFile;
  }

  // 5. and finally, check the version no. for compatibility
  QFile rcFile( rcFileName );
  if ( rcFile.open( IO_ReadOnly ) ) {
    QTextStream ts( &rcFile );
    QString line = ts.readLine();
    rcFile.close();
    /* get the version as a string */
    int i = 0;
    while ( !line[i].isDigit() ) i++;
    line = line.right( line.length() - i );
    i = 0;
    while ( line[i].isDigit() || line[i] == '.' )  i++;
    line = line.left( i );
    if ( line != vkVersion() )
      return BadRcVersion;
  }

  return Okay;
}


/* FIXME: this is horrible.
   reads a comma-separated list of suppression files, each of
   which starts with '[+]' or '[-]'.  Returns a comma-separated
   subset of those files which start with '[+]' */
QString VkConfig::selSuppFiles( const QString &pKey, const QString &pGroup )
{
  QString supp_str = rdEntry( pKey, pGroup );
  QStringList supp_list = QStringList::split( ",", supp_str );

  supp_str = "";
  for ( uint i=0; i<supp_list.count(); i++ ) {
    if ( supp_list[i].left(3) == "[+]" ) {
      supp_list[i].remove( 0, 3 );
      supp_str += supp_list[i] + ',';
    }
  }

  /* remove trailing ',' */
  supp_str.remove( supp_str.length()-1, 1 );
  return supp_str;
}


/* Returns a list of strings containing the user-specified options;
   these might be modified on the cmd-line, or via the OptionsWindow.
   Returns eg. valgrind --tool=memcheck, --leak-check=yes, ... */
QStringList VkConfig::modFlags( VkObject* vkobj )
{
  QStringList flags;

  /* make sure we get the right valgrind */
  flags << rdEntry( "vg-exec","valkyrie");

  /* set the tool we are using */
  flags << "--tool=" + vkobj->name();

 QStringList alist;
  /* check if any valgrind core opts have been modified.  
     'modified' means 'set to anything other than default' */
  VkObject* obj;
  for ( obj = vkObjectList.first(); obj; obj = vkObjectList.next() ) {
    if ( obj->name() == "valgrind" ) {
      alist = obj->modifiedFlags();
      for ( uint i=0; i<alist.count(); i++ )
        flags << alist[i];
      break;
    }
  }

  /* get flags which have been specified|modified for this tool _only_ */
  alist.clear();
  alist = vkobj->modifiedFlags();
  for ( uint i=0; i<alist.count(); i++ )
    flags << alist[i];

  /* finally, check for valkyrie-specific flags */
  for ( obj = vkObjectList.first(); obj; obj = vkObjectList.next() ) {
    if ( obj->name() == "valkyrie" ) {
      alist = obj->modifiedFlags();
      for ( uint i=0; i<alist.count(); i++ )
        flags << alist[i];
      break;
    }
  }

  return flags;
}


int VkConfig::defaultToolId()
{
  QString tool_name = rdEntry( "tool", "valgrind" );

  VkObject* obj;
  for ( obj = vkObjectList.first(); obj; obj = vkObjectList.next() ) {
    if ( obj->isTool() && obj->name() == tool_name )
      return obj->id();
  }
  return -1;
}


/* Objects MUST be inserted into this list in the exact order in which
   their id appears in VkObject::ObjectId (/src/options/vk_objects.h):
   VALKYRIE=0, VALGRIND=1, MEMCHECK=2, CACHEGRIND=3, MASSIF=4 
   so that we can index into the list with ObjectId */
bool VkConfig::initVkObjects() 
{ 
  vkObjectList.append( new Valkyrie() );
  vkObjectList.append( new Valgrind() );
  vkObjectList.append( new Memcheck() );
  vkObjectList.append( new Cachegrind() );
  vkObjectList.append( new Massif() );

  return true; 
}


/* returns an object based on its ObjectId */
VkObject* VkConfig::vkObject( int tvid, bool tools_only/*=false*/ )
{
  VkObject* obj = vkObjectList.at( tvid );
  vk_assert( obj != 0 && obj->id() == tvid );
  if ( tools_only ) {
    vk_assert( obj->isTool() );
  }

  return obj;
}

/* returns a vkObject based on its name */
VkObject* VkConfig::vkObject( const QString& obj_name )
{
  VkObject* obj;
  for ( obj = vkObjectList.first(); obj; obj = vkObjectList.next() ) {
    if ( obj->name() == obj_name )
      return obj;
  }
  return NULL;
}


/* Create the default configuration file.  -----------------------------
   The first time valkyrie is started, vkConfig looks to see if this
   file is present in the user's home dir.  If not, it writes the
   relevant data to ~/.PACKAGE/PACKAGErc */
void VkConfig::mkConfigFile( bool rm )
{
  /* we might have to remove an old rc file */
  if ( rm ) {
    QFile rcF( rcFileName ); 
    if ( !rcF.remove() ) 
      VK_DEBUG("Failed to delete old version rcfile.");
  }

  QDateTime dt = QDateTime::currentDateTime();

  QString header = QString("# %1 %2 configuration file\n")
                          .arg(vkName()).arg(vkVersion());
  header += "# " + dt.toString( "MMMM d hh:mm yyyy" ) + "\n\n";


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
user=root\n\
host=localhost\n\
pword=Poniarl7\n\
dbase=valkyrie\n\
logging=true\n\
logfile=\n\n";


  QFile outF( rcFileName ); 
  if ( outF.open( IO_WriteOnly ) ) { 
    QTextStream aStream( &outF ); 

    aStream << header << window_colors << mainwin_size_pos << dbase;

    /* a new tool might have been added, or other changes made, in
       which case this fn wouldn't contain the correct options=values
       if it were hard-wired in. Better safe than sorry: just get all
       tools that are present to spew their options/flags out to disk. */
    VkObject* vkobj;
    for ( vkobj = vkObjectList.first(); vkobj; vkobj = vkObjectList.next() ) {
      aStream << vkobj->configEntries();
    }

    outF.close();
  }

	newConfigFile = true;
}


/* ~/.PACKAGE-X.X.X is a sine qua non ---------------------------------- 
   checks to see if ~/valkyrie-X.X.X/ and its required sub-dirs are
   all present and correct.  If not, tries to create them.
  ~/valkyrie-X.X.X/ 
    - valkyrierc
    - dbase/
    - logs/
    - suppressions/ 
*/
bool VkConfig::checkDirs()
{
	// VK_DEBUG("bool VkConfig::checkDirs()");
  QString msg = "";
  bool success = true;

  QDir vk_dir( rcPath );

  enum State { 
    CHECK_DIR=0, CHECK_SUB_DIRS, MK_TOP_DIR, MK_DB_DIR, 
    MK_LOG_DIR,  MK_SUPP_DIR, DONE, GIVE_UP };
  State state = CHECK_DIR;

  bool not_done = true;
  while ( not_done ) {

    switch ( state ) {

      /* normal startup checks - --------------------------------------- */
      case CHECK_DIR:        /* does ~/.PACKAGE-X.X.X/ exist ? */
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
      case MK_TOP_DIR:       /* create '~/PACKAGE-X.X.X' */
        state = vk_dir.mkdir( rcPath ) ? MK_DB_DIR : GIVE_UP;
        break;

      case MK_DB_DIR:        /* create sub-dir '/dbase' */
        state = vk_dir.mkdir( dbasePath ) ? MK_LOG_DIR : GIVE_UP;
        break;

      case MK_LOG_DIR:       /* create sub-dir '/logs' */
        state = vk_dir.mkdir( logsPath ) ? MK_SUPP_DIR : GIVE_UP;
        break;

      /* last case statement MUST set 'state = DONE || GIVE_UP' */
      case MK_SUPP_DIR:      /* create sub-dir '/suppressions' */
        state = vk_dir.mkdir( suppPath ) ? DONE : GIVE_UP;
        if ( state == DONE )
        break;

      case GIVE_UP:
        not_done = false;
        success = false;
        msg.sprintf( "<p>There is a problem with '%s'.<br>"
                     "Either some files or sub-directories do not exist, "
                     "or the permissions are not set correctly."
                     "<p>Please check and retry.</p>", rcPath.ascii() );
        break;

      case DONE:
        not_done = false;
        break;

    }  /* end switch ( state ) */
  }    /* end while (1) */

  if ( !msg.isEmpty() )
    vkError( 0, "Directory Error", msg.ascii() );

  return success;
}


/* The default value for vg-exec is "", so the very first time
   valkyrie is started, we need to find out:
   (a) where valgrind lives;
   (b) what version of valgrind we are being asked to use;
	 (c) where valgrind's manual lives;
   (c) where the suppression files live, and
	 (d) what suppression files exist */
// FIXME: this lot should be done by automake 
bool VkConfig::checkPaths()
{
	//VK_DEBUG("checking valgrind paths");
	VK_DEBUG("TODO: find valgrind doc dir\n");

  QFile file;
  QString caption, str;
  QString cmd, tmp_fname, vg_exec_path, vg_version, 
		vg_supp_dir, vg_supp_files, def_supp;

  enum State { 
    DEF_PATH, SYS_PATH, USR_PATH, 
    CHECK_EXEC, CHECK_VERSION, SUPP_FILES, DONE, GIVE_UP };
  State state = DEF_PATH;

  bool success = true;

  /* so we don't get our knickers in a twist */
  bool def_path_checked, sys_path_checked, usr_path_checked;
  def_path_checked = sys_path_checked = usr_path_checked = false;

  /* so we know what to tell the user if all fails */
  bool bad_exec, bad_version, bad_supps;
  bad_exec = bad_version = bad_supps = false;

  bool not_done = true;
  while ( not_done ) {

    switch ( state ) {

      /* path to valgrind executable ----------------------------------- */
      case DEF_PATH: {    /* try the default entry first */
        vg_exec_path = rdEntry( "vg-exec", "valkyrie" );
        def_path_checked = true;
        state = (vg_exec_path.isEmpty()) ? SYS_PATH : CHECK_EXEC;
      } break;

      case SYS_PATH: {    /* try 'which valgrind' next */
        tmp_fname = vk_mkstemp( "vg-path", rcPath );
        cmd.sprintf( "which valgrind > %s", tmp_fname.ascii() );
        system( cmd.ascii() );
        file.setName( tmp_fname );
        if ( file.open( IO_ReadOnly ) ) 
          file.readLine( vg_exec_path, 100 );
        file.remove();    /* close and delete the temporary file */
        vg_exec_path = vg_exec_path.simplifyWhiteSpace();
        sys_path_checked = true;
        state = (vg_exec_path.isEmpty()) ? USR_PATH : CHECK_EXEC;
      } break;

      case USR_PATH: {    /* ask user as a last resort */
        vg_exec_path = QFileDialog::getOpenFileName( "/usr", 
                                        "All Files (*)", 0, 
                                        "f_dlg", "Select Valgrind path" );
        usr_path_checked = true;
        state = (vg_exec_path.isEmpty()) ? GIVE_UP : CHECK_EXEC;
      } break;            /* stop if user clicked cancel */

      case CHECK_EXEC: {
        QFileInfo fi( vg_exec_path );
        if ( fi.isFile() && fi.isExecutable() && fi.baseName()=="valgrind" )
          state = CHECK_VERSION;
        else if (def_path_checked && (!sys_path_checked && !usr_path_checked))
					state = SYS_PATH;
				else if (def_path_checked && sys_path_checked && !usr_path_checked) 
					state = USR_PATH;
				else if (def_path_checked && sys_path_checked && usr_path_checked) { 
					bad_version = true; 
					state = GIVE_UP; 
				}
      } break;

      case CHECK_VERSION: {
        tmp_fname = vk_mkstemp( "vg-version", rcPath );
        cmd.sprintf( "%s --version | sed \"s/valgrind-//g\" > %s", 
                     vg_exec_path.ascii(), tmp_fname.ascii() );
        system( cmd.ascii() );
        file.setName( tmp_fname );
        if ( file.open( IO_ReadOnly ) )
          file.readLine( vg_version, 100 );
        file.remove();    /* close and delete the temporary file */
        vg_version = vg_version.simplifyWhiteSpace();
        /* do some fancy stuff */
        int found = str2hex( vg_version );
        int reqd  = str2hex( "3.0.0" );
        if ( found >= reqd ) state = SUPP_FILES;
        else if (def_path_checked && (!sys_path_checked && !usr_path_checked))
					state = SYS_PATH;
				else if (def_path_checked && sys_path_checked && !usr_path_checked)
					state = USR_PATH;
				else if (def_path_checked && sys_path_checked && usr_path_checked) { 
					bad_version = true; 
					state = GIVE_UP; 
				}
      } break;

      /* path to valgrind's manual ------------------------------------- 
				 default vg-doc-path is /usr/share/doc/packages/valgrind/x.html */

      /* suppressions file path -------------------------------------------- 
         default vg-exec dir is /usr/bin/valgrind
         default supps   dir is /usr/lib/valgrind/x.supp */
      case SUPP_FILES: {
        QDir dir;
				dir.setPath( vg_exec_path );      /* ho hum    */
				dir.cd( "../..", false );         /* eg. /usr/ */
				if ( dir.cd( "lib/valgrind/", false ) ) {
					vg_supp_dir = dir.absPath();
				} else {
					vg_supp_dir = QFileDialog::getExistingDirectory( vg_exec_path, 
                     0, "d_dlg", "Select Valgrind Suppressions Directory" );
				}
        if ( !vg_supp_dir.isEmpty() ) {
          /* see if we have any *.supp files in here */
          dir.setPath( vg_supp_dir );
					/* grab 'em while the going's good */
					QStringList supp_list = dir.entryList( "*.supp", QDir::Files );
					vg_supp_files = "";
					for ( uint i=0; i<supp_list.count(); i++ ) {
						vg_supp_files += dir.absPath() + "/" + supp_list[i] + ",";
						/* the only selected one is the default suppression file */
						if ( supp_list[i] == rdEntry( "suppressions", "memcheck" ) )
							def_supp = dir.absPath() + "/" + supp_list[i];
					}
				}
				state = ( !vg_supp_files.isEmpty() ) ? DONE : GIVE_UP;
        bad_supps = ( state == GIVE_UP ) ? true : false;
      } break;

      case GIVE_UP: {
        not_done = false;
        success  = false;
        if ( bad_exec ) {
          caption = "   Invalid Valgrind Executable   ";
          str.sprintf("<p>The file '%s' doesn't look like "
                      "a valgrind executable.</p>"
                      "<p>Giving up.</p>", vg_exec_path.ascii() );
        }  else if ( bad_version ) {
          caption = "   Incompatible Valgrind Version   ";
          str.sprintf("<p>Valkyrie requires Valgrind >= v3.0.0.<br>" 
                      "Valgrind version found in '%s' is v%s</p>"
                      "<p>Giving up.</p>", vg_exec_path.ascii(), 
                                           vg_version.ascii() );
        } else if ( bad_supps ) {
          caption = "   Invalid Suppressions Directory   ";
          str.sprintf("<p>Failed to find valgrind's suppressions files."
                      "<p>Giving up.</p>");
        }  
      } break;

      case DONE: {
        success  = true;
        not_done = false;
        /* set the default valgrind path on this machine */
        wrEntry( vg_exec_path, "vg-exec", "valkyrie" );
				/* write the list of found .supp files */
				wrEntry( vg_supp_files, "all-supps", "valkyrie" );
				/* and write the default supp. file including path */
				wrEntry( def_supp, "sel-supps", "valkyrie" );
				wrEntry( def_supp, "suppressions", "memcheck" );
      } break;

    }  /* end switch ( state ) */
  }    /* end while ( not_done ) */

  if ( !str.isEmpty() )
    vkError( 0, caption.ascii(), str.ascii() );

  return success;
}


