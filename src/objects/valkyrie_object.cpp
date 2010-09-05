/****************************************************************************
** Valkyrie implementation
**  - Valkyrie-specific: options / flags / functionality
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

#include "help/help_urls.h"
#include "objects/tool_object.h"
#include "objects/valkyrie_object.h"
#include "options/valkyrie_options_page.h"   // createVkOptionsPage()
#include "utils/vk_config.h"
#include "utils/vk_utils.h"

#include <QFile>
#include <QFileInfo>
#include <QPoint>
#include <QStringList>


// TODO: put this in a define in the build scripts,or something.
// Minimum version of Valgrind required
const char* pchVersionVgMin = "3.6.0";



/***************************************************************************/
/*!
    Constructs a Valkyrie object
*/
Valkyrie::Valkyrie()
   : VkObject( "valkyrie" )
{
   setupOptions();
   
   // init valgrind
   m_valgrind = new Valgrind();
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
Valkyrie:: ~Valkyrie()
{
   if ( m_valgrind != 0 ) {
      delete m_valgrind;
      m_valgrind = 0;
   }
   
}


/*!
    Setup the options for this object.
*/
void Valkyrie::setupOptions()
{
   options.addOpt(
      VALKYRIE::HELP,
      this->objectName(),
      "help",
      'h',
      "",
      "",
      "",
      "",
      "show this message",
      urlNone,
      VkOPT::ARG_NONE,
      VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::VGHELP,
      this->objectName(),
      "valgrind-opts",
      'V',
      "",
      "",
      "",
      "",
      "show this message, plus valgrind options",
      urlNone,
      VkOPT::ARG_NONE,
      VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::OPT_VERSION,
      this->objectName(),
      "version",
      'v',
      "",
      "",
      "",
      "",
      "show version",
      urlNone,
      VkOPT::ARG_NONE,
      VkOPT::WDG_NONE
   );

   options.addOpt(
      VALKYRIE::TOOLTIP,
      this->objectName(),
      "show-tooltips",
      '\0',
      "",
      "true|false",
      "true",
      "Show tooltips",
      "",
      urlValkyrie::toolTips,
      VkOPT::NOT_POPT,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALKYRIE::PALETTE,
      this->objectName(),
      "use-vk-palette",
      '\0',
      "",
      "true|false",
      "true",
      "Use valkyrie's palette",
      "",
      urlValkyrie::palette,
      VkOPT::NOT_POPT,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALKYRIE::ICONTXT,
      this->objectName(),
      "show-butt-text",
      '\0',
      "",
      "true|false",
      "false",
      "Show toolbar text labels",
      "",
      urlValkyrie::toolLabels,
      VkOPT::NOT_POPT,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALKYRIE::FNT_GEN_SYS,
      this->objectName(),
      "font-gen-sys",
      '\0',
      "",
      "true|false",
      "true",
      "General Font - use system default",
      "",
      urlValkyrie::userFontGen,
      VkOPT::NOT_POPT,
      VkOPT::WDG_CHECK
   );
   
   options.addOpt(
      VALKYRIE::FNT_GEN_USR,
      this->objectName(),
      "font-gen-user",
      '\0',
      "",
      "",
      "Luxi Sans,10,-1,5,50,0,0,0,0,0",
      "General Font:",
      "",
      urlValkyrie::userFontGen,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALKYRIE::FNT_TOOL_USR,
      this->objectName(),
      "font-tool-user",
      '\0',
      "",
      "",
      "Misc Fixed,11,-1,5,50,0,0,0,0,0",
      "Tool Font:",
      "",
      urlValkyrie::userFontTool,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALKYRIE::SRC_EDITOR,
      this->objectName(),
      "src-editor",
      '\0',
      "",
      "",
      VK_BIN_EDITOR,
      "Src Editor:",
      "",
      urlValkyrie::srcEditor,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALKYRIE::SRC_LINES,
      this->objectName(),
      "src-lines",
      '\0',
      "",
      "1|10",
      "2",
      "Src Editor: Extra lines shown above/below target line:",
      "",
      urlValkyrie::srcLines,
      VkOPT::NOT_POPT,
      VkOPT::WDG_SPINBOX
   );
   
   options.addOpt(
      VALKYRIE::BROWSER,
      this->objectName(),
      "browser",
      '\0',
      "",
      "",
      "",
      "Browser:",
      "",
      urlValkyrie::browser,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   QString projfile = QString("<project.") + VK_CFG_EXT + ">";
   options.addOpt(
      VALKYRIE::PROJ_FILE,
      this->objectName(),
      "project-file",
      'f',
      projfile,
      "",
      "",
      "",
      "use " + projfile + " for project settings",
      urlValkyrie::projectFile,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LEDIT
   );
   
   
   options.addOpt(
      VALKYRIE::WORKING_DIR,
      this->objectName(),
      "working-dir",
      '\0',
      "<dir>",
      "",
      "./",
      "Working Dir:",
      "dir under which to run valgrind",
      urlValkyrie::workingDir,
      VkOPT::ARG_STRING,
      VkOPT::WDG_LEDIT
   );

   // path to valgrind executable (maybe found by configure)
   options.addOpt(
      VALKYRIE::VG_EXEC,
      this->objectName(),
      "vg-exec",
      '\0',
      "",
      "",
      VK_BIN_VALGRIND,
      "Valgrind:",
      "",
      urlValkyrie::vgDir,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALKYRIE::BINARY,
      this->objectName(),
      "binary",
      '\0',
      "",
      "",
      "",
      "Binary:",
      "",
      urlValkyrie::binary,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALKYRIE::BIN_FLAGS,
      this->objectName(),
      "binary-flags",
      '\0',
      "",
      "",
      "",
      "Binary flags:",
      "",
      urlValkyrie::binFlags,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );
   
   options.addOpt(
      VALKYRIE::VIEW_LOG,
      this->objectName(),
      "view-log",
      'v',
      "<file>",
      "",
      "",
      "",
      "parse and view a valgrind logfile",
      urlNone,
      VkOPT::ARG_STRING,
      VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::DFLT_LOGDIR,
      this->objectName(),
      "default-logdir",
      '\0',
      "",
      "",
      VkConfig::vkTmpDir(),
      "Tmp Log Dir:",
      "",
      urlValkyrie::logDir,
      VkOPT::NOT_POPT,
      VkOPT::WDG_LEDIT
   );


   
   // Internal configuration
   options.addOpt(
      VALKYRIE::MW_SIZE, this->objectName(), "mainwindow_size",
      '\0', "", "", QSize( 600, 600 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::MW_POS, this->objectName(), "mainwindow_pos",
      '\0', "", "", QPoint( 400, 0 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::HB_HIST, this->objectName(), "handbook_history",
      '\0', "", "", "", "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::HB_BKMK, this->objectName(), "handbook_bookmarks",
      '\0', "", "", "", "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::HB_MXHIST, this->objectName(), "handbook_max_history",
      '\0', "", "", "20", "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::HB_MXBKMK, this->objectName(), "handbook_max_bookmarks",
      '\0', "", "", "20", "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_BKGD, this->objectName(), "colour_background",
      '\0', "", "", QColor( 214, 205, 187 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_BASE, this->objectName(), "colour_base",
      '\0', "", "", QColor( 255, 255, 255 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_DKGR, this->objectName(), "colour_dkgray",
      '\0', "", "", QColor( 128, 128, 128 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_EDIT, this->objectName(), "colour_edit",
      '\0', "", "", QColor( 254, 222, 190 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_HILT, this->objectName(), "colour_highlight",
      '\0', "", "", QColor( 147, 40, 40 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_NULL, this->objectName(), "colour_null",
      '\0', "", "", QColor( 239, 227, 211 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
   
   options.addOpt(
      VALKYRIE::COL_TEXT, this->objectName(), "colour_text",
      '\0', "", "", QColor( 0,  0,  0 ), "", "",
      urlNone, VkOPT::NOT_POPT, VkOPT::WDG_NONE
   );
}


void Valkyrie::setConfigDefaults()
{
   foreach( VkOption * opt, options.getOptionHash() ) {
      // Don't create config entries for these options:
      //  - They don't hold persistent data, and have no associated option widget
      if ( opt->optid == VALKYRIE::HELP ) {
         continue;
      }
      
      if ( opt->optid == VALKYRIE::VGHELP ) {
         continue;
      }
      
      opt->updateConfig( opt->dfltValue );
   }
}


/*!
   Check \a argval for the option given by \a optGrp and \a optid.
   The \a argval may be updated.
*/
int Valkyrie::checkOptArg( QString optGrp, int optid, QString& argval )
{
   VkObjectList objList = this->vkObjList();
   
   for ( int i = 0; i < objList.size(); ++i ) {
      VkObject* obj = objList.at( i );
      
      // find the option owner, call checkOptArg for that owner.
      if ( obj->objectName() == optGrp ) {
         return obj->checkOptArg( optid, argval );
      }
   }
   
   return PERROR_BADOPT;
}



/*!
  Update config
  General access function, when specific object not known
*/
void Valkyrie::updateConfig( QString optGrp, int optid, QString& argval )
{
   VkObjectList objList = this->vkObjList();
   
   for ( int i = 0; i < objList.size(); ++i ) {
      VkObject* obj = objList.at( i );
      
      // find the option owner, call checkOptArg for that owner.
      if ( obj->objectName() == optGrp ) {
         return obj->updateConfig( optid, argval );
      }
   }
}



/*!
  Update config - special cases
*/
void Valkyrie::updateConfig( int optid, QString& argval )
{
   // Load config settings from project file
   //  - load these first before updating the rest
   if ( optid == VALKYRIE::PROJ_FILE ) {
      QString proj_filename = argval;
      
      if ( !QFile::exists( proj_filename ) ) {
         vkConfig->createNewProject( proj_filename );
      }
      else {
         vkConfig->openProject( this, proj_filename );
      }
   }
   
   VkObject::updateConfig( optid, argval );
}



/*!
  Check \a argval for the option given by \a optid, updating if necessary.
*/
int Valkyrie::checkOptArg( int optid, QString& argval )
{
   vk_assert( optid >= 0 && optid < VALKYRIE::NUM_OPTS );
   
   VkOption* opt = getOption( optid );
   int errval = PARSED_OK;
   
   switch ( optid ) {

   // the following options are _only_ set via the gui, and are either
   //  a) limited to a set of available values, or
   //  b) have already been checked, so no need to re-check them.
   case VALKYRIE::TOOLTIP:
   case VALKYRIE::PALETTE:
   case VALKYRIE::ICONTXT:
   case VALKYRIE::FNT_GEN_SYS:
   case VALKYRIE::FNT_GEN_USR:
   case VALKYRIE::FNT_TOOL_USR:
   case VALKYRIE::SRC_LINES: {
         vk_assert( opt->argType == VkOPT::NOT_POPT );
         return errval;
      } break;
      
   // dir options: check for RWX permissions
   case VALKYRIE::DFLT_LOGDIR:
   case VALKYRIE::WORKING_DIR: {
         ( void ) dirCheck( &errval, argval, true, true, true );
      } break;
   
   // browser executable
   case VALKYRIE::BROWSER: {
         if ( argval.isEmpty() ) {
            // empty is fine.
         } else {
            // check for at least X permissions.
            QString path = argval.split( " " ).first();
            QString file = fileCheck( &errval, path, false, false, true );
            argval.replace( path, file );
            argval = argval.simplified();
         }
      } break;

   // editor executable
   case VALKYRIE::SRC_EDITOR: {
         if ( argval.isEmpty() ) {
            // empty is fine.
         } else {
            // check for at least X permissions.
            QString path = argval.split( " " ).first();
            QString file = fileCheck( &errval, path, false, false, true );
            argval.replace( path, file );
            argval = argval.simplified();

            if ( !argval.contains( "%n" ) ) {
               // for a few well-known editors, add the go-to-line
               // editor flag, plus our replacement string (%n)
               QFileInfo fi( argval );
               QString fname = fi.fileName();
               if ( fname.contains( QRegExp( "^(emacs|gedit|gvim|nano|nedit)[\\W]*" ) ) ) {
                  argval += " +%n";
               }
            }
         }
      } break;

   // valkyrie project file
   case VALKYRIE::PROJ_FILE: {
         // check file exists and has RW perms
         argval = fileCheck( &errval, argval, true, true );
         if ( errval != PARSED_OK ) {
            return errval;
         }

         // check filename format: ".*\.VK_CFG_EXT$"
         if ( !argval.contains( QRegExp( QString(".*\\.") + VK_CFG_EXT + "$" ) ) ) {
            return PERROR_BADFILENAME;
         }

      } break;
   
   case VALKYRIE::VG_EXEC: {
         // see if we have a valgrind exec with at least X permissions:
         argval = fileCheck( &errval, argval, false, false, true );
         if ( errval != PARSED_OK ) {
            return errval;
         }

         // check the version
         QString cmd_qstr = argval + " --version 2>&1";
         const char* cmd = cmd_qstr.toLatin1().constData();
         FILE* fp = popen( cmd, "r" );
         if ( !fp ) {
            pclose( fp );
            return PERROR_BADFILE;
         }

         char line[50];
         if ( fgets( line, sizeof( line ), fp ) == NULL ) {
            return PERROR_BADFILE;
         }

         if ( pclose( fp ) == -1 ) {
            return PERROR_BADFILE;
         }

         // we have a string from the executable: is it what we hope?
         QString vg_version = QString( line ).simplified();

         if ( !vg_version.startsWith( "valgrind" ) ) {
            return PERROR_BADFILE;
         }

         // compare with minimum req'd version
         int versionVg     = strVersion2hex( vg_version );
         int versionVgReqd = strVersion2hex( pchVersionVgMin );

         if ( versionVg == -1 || versionVgReqd == -1 ) {
            return PERROR_BADFILE;
         }

         if ( versionVg <  versionVgReqd ) {
            return PERROR_BADVERSION;
         }

         // if we get here, we're fine.
      } break;

   case VALKYRIE::VIEW_LOG:
      if ( !argval.isEmpty() ) {
         // see if we have a logfile with at least R permissions:
         argval = fileCheck( &errval, argval, true );
      } break;

   case VALKYRIE::BINARY:
      if ( !argval.isEmpty() ) {
         // see if we have a binary with at least X permissions:
         argval = fileCheck( &errval, argval, false, false, true );
      } break;

   case VALKYRIE::BIN_FLAGS:
      // Can't (easily) test this.
      break;
      
      // ignore these opts
   case VALKYRIE::HELP:
   case VALKYRIE::VGHELP:
   case VALKYRIE::OPT_VERSION:
      break;
      
   default:
      vk_assert_never_reached();
   }
   
   return errval;
}


/*!
  Find the option owner, find option within that owner.
  Called from VkConfig::readFromConfigFile(...)
  Warning: slow!
*/
VkOption* Valkyrie::findOption( QString& optKey )
{
   QStringList parts = optKey.split( "/", QString::SkipEmptyParts );
   vk_assert( parts.count() == 2 );
   QString optGrp = parts.at( 0 );
   QString optFlag = parts.at( 1 );
   
   VkObjectList objList = this->vkObjList();
   
   for ( int i = 0; i < objList.size(); ++i ) {
      VkObject* obj = objList.at( i );
      
      if ( obj->objectName() == optGrp ) {
         foreach( VkOption * opt, obj->getOptions() ) {
            if ( opt->longFlag == optFlag ) {
               return opt;
            }
         }
      }
   }
   
   return NULL;
}


#if 0
/*!
  Find the option owner, find option within that owner.
*/
VkOption* Valkyrie::findOption( QString& optGrp, int optid )
{
   VkObjectList objList = this->vkObjList();
   
   for ( int i = 0; i < objList.size(); ++i ) {
      VkObject* obj = objList.at( i );
      
      if ( obj->objectName() == optGrp ) {
         return obj->getOption( optid );
      }
   }
   
   return NULL;
}
#endif



/*!
   Return list of all VkObjects
*/
const VkObjectList Valkyrie::vkObjList()
{
   VkObjectList vkObjList;
   
   vkObjList.append( this );
   vkObjList.append( valgrind() );
   foreach( ToolObject * tool, valgrind()->getToolObjList() )
   vkObjList.append( tool );
   
   return vkObjList;
}


VkOptionsPage* Valkyrie::createVkOptionsPage()
{
   return ( VkOptionsPage* )new ValkyrieOptionsPage( this );
}


/*!
  called from MainWin when user clicks stopButton
*/
void Valkyrie::stopTool( VGTOOL::ToolID tId )
{
   vk_assert( tId != VGTOOL::ID_NULL );
   ToolObject* tool = valgrind()->getToolObj( tId );
   vk_assert( tool != 0 );
   
   tool->stop();
   vk_assert( !tool->isRunning() );
}


/*!
  find out if tool is running or not.
*/
bool Valkyrie::queryToolDone( VGTOOL::ToolID tId )
{
   vk_assert( tId != VGTOOL::ID_NULL );
   ToolObject* tool = valgrind()->getToolObj( tId );
   vk_assert( tool != 0 );

   return tool->queryDone();
}


/*!
  Run the tool with given process id.
*/
bool Valkyrie::runTool( VGTOOL::ToolID tId, int procId )
{
   cerr << "Valkyrie::runTool( "  << tId  << ", " << procId << ")" << endl;
   
   vk_assert( tId != VGTOOL::ID_NULL );
   ToolObject* activeTool = valgrind()->getToolObj( tId );
   vk_assert( activeTool != 0 );
   
   QStringList vg_flags = getVgFlags( tId );


   // update the flags with the necessary options: xml etc.
   QString log_basename = activeTool->objectName() + "_log";
   QString logfile = vk_mkstemp( QString( VkConfig::vkTmpDir() ) + log_basename, "xml" );
   vk_assert( !logfile.isEmpty() );

//TODO: rm
// Vg < 3.6:
//   vg_flags.insert( ++( vg_flags.begin() ), ( "--log-file=" + logfile ) );

   vg_flags.insert( ++( vg_flags.begin() ), ( "--xml-file=" + logfile ) );
   vg_flags.insert( ++( vg_flags.begin() ), "--xml=yes" );
   
   return activeTool->start( procId, vg_flags, logfile );
}


/*!
  Returns valgrind flags for given tool
*/
QStringList Valkyrie::getVgFlags( VGTOOL::ToolID tId )
{
   vk_assert( tId != VGTOOL::ID_NULL );
   ToolObject* tool = valgrind()->getToolObj( tId );
   vk_assert( tool != 0 );
   
   // if we don't find it in config, let's hope it's in $PATH
   VkOption* opt = options.getOption( VALKYRIE::VG_EXEC );
   QString vg_exec = vkConfig->value( opt->configKey(), "valgrind" ).toString();
   
   QStringList vg_flags;
   vg_flags << vg_exec;                          // path/to/valgrind
   vg_flags << "--tool=" + tool->objectName();   // active tool (!= valgrind()->TOOL)
   vg_flags += valgrind()->getVgFlags( tool );   // valgrind (+ tool) opts
   vg_flags += getTargetFlags();                 // valkyrie opts
   
   return vg_flags;
}


/*!
  Returns valgrind flags for given tool
*/
QStringList Valkyrie::getTargetFlags()
{
   QStringList modFlags;
   VkOption* opt  = options.getOption( VALKYRIE::BINARY );
   QString cfgVal = vkConfig->value( opt->configKey() ).toString();
   
   // only add binary & bin_flags if binary present
   if ( !cfgVal.isEmpty() ) {
      modFlags << cfgVal;
      
      // add any target binary flags
      opt    = options.getOption( VALKYRIE::BIN_FLAGS );
      cfgVal = vkConfig->value( opt->configKey() ).toString();
      modFlags += cfgVal.split( " ", QString::SkipEmptyParts );
   }
   
   return modFlags;
}
