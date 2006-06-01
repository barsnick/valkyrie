/* --------------------------------------------------------------------- 
 * Implementation of class Option                          vk_option.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_option.h"
#include "vk_utils.h"       /* vk_assert(), free(), malloc() */

#include <ctype.h>
#include <math.h>
#include <malloc.h>

#include <qdir.h>


/* these enums are defined in vk_option.h --------------------------- */
const char* parseErrString( const int error )
{
   switch ( error ) {
   case PERROR_NODASH:
      return "Short options do not require a '='";
   case PERROR_NOARG:
      return "Missing argument";
   case PERROR_BADARG:
      return "Invalid argument";
   case PERROR_BADOPT:
      return "Unknown/Disabled option";
   case PERROR_BADOPERATION:
      return "Mutually exclusive logical operations requested";
   case PERROR_NULLARG:
      return "opt->arg should not be NULL";
   case PERROR_BADQUOTE:
      return "Error in parameter quoting";
   case PERROR_BADNUMBER:
      return "Invalid numeric value";
   case PERROR_OUTOFRANGE:
      return "Numeric value outside valid range";
   case PERROR_OVERFLOW:
      return "Number too large or too small";
   case PERROR_MALLOC:
      return "Memory allocation failed";
   case PERROR_BADDIR:
      return "Invalid directory";
   case PERROR_BADFILE:
      return "Invalid file";
   case PERROR_BADNUMFILES:
      return "Invalid number of files ( max == 10 )";
   case PERROR_BADFILERD:
      return "You do not have read permission";
   case PERROR_BADFILEWR:
      return "You do not have write permission";
   case PERROR_BADEXEC:
      return "You do not have executable permission";
   case PERROR_DB_CONFLICT:
      return "Conflicting argument\n"
         "--db-attach=yes conflicts with --trace-children=yes\n"
         "Please choose one or the other, but not both.";
   case PERROR_DB_OUTPUT:
      return "using --db-attach=yes only makes sense "
         "when sending output to stderr.";
   case PERROR_POWER_OF_TWO:
      return "number is not a power of two";
   case PERROR_BADVERSION:
      return "Bad program version";
   default:
      return "unknown error";
   }
}


/* helper functions ---------------------------------------------------- */

QString checkFile( int* err_val, const QString file_name )
{
   *err_val = PARSED_OK;
   QString absPath = QString::null;

   if (file_name.isEmpty()) {
      *err_val = PERROR_BADFILE;
      return file_name;
   }

   if ( QFile::exists(file_name) ) {
      absPath = QFileInfo( file_name ).absFilePath();
   } else if ( file_name.find('/') == -1 ) {
      /* else (if no '/' in file_name)
           try to find in $PATH environment variable */
      char* pEnv = getenv( "PATH" );
      QStringList paths( QStringList::split(QChar(':'), pEnv) );
      for ( QStringList::Iterator p = paths.begin(); p != paths.end(); ++p ) {
         QDir dir( *p );
         QString candidate = dir.absPath() + "/" + file_name;
         if ( QFile::exists(candidate) ) {
            absPath = candidate;
            break;
         }
      }
   }

   if ( ! QFile::exists(absPath) ) {
      *err_val = PERROR_BADFILE;
   }
   return absPath;
}


/* transforms relative paths to absolute ones; checks the file exists,
   and that the user has the specified permissions */
QString fileCheck( int* err_val, const QString fpath, 
                   bool rd_perms, bool wr_perms )
{
   QString absPath = checkFile( err_val, fpath );
  
   QFileInfo fi( absPath );
   if ( fi.exists() ) {

      /* check this is really a file */
      if ( !fi.isFile() ) {
         *err_val = PERROR_BADFILE;
         goto bye;
      }

      /* check user has executable permission */
      if ( rd_perms ) {
         if ( !fi.isReadable() ) {
            *err_val = PERROR_BADFILERD;
         }
      }

      /* check user has write permission */
      if ( wr_perms ) {
         if ( !fi.isWritable() ) {
            *err_val = PERROR_BADFILEWR;
         }
      }

   }

 bye:
   return absPath;
}


/* check the file exists; try to get the absolute path; if found,
   check that the user has executable permissions */
QString binaryCheck( int* err_val, const QString exe_name )
{
   QString absPath = checkFile( err_val, exe_name );

   QFileInfo fi( absPath );
   if ( fi.exists() ) {
      if ( !fi.isExecutable() ) {
         *err_val = PERROR_BADEXEC;
      }
   }

   return absPath;
}



QString dirCheck( int* err_val, const QString dir,
                  bool rd_perms, bool wr_perms )
{
   QString absPath = checkFile( err_val, dir );

   QFileInfo fi( absPath );
   if ( fi.exists() ) {

      /* check it's really a directory */
      if ( !fi.isDir() ) {
         *err_val = PERROR_BADDIR;
         goto bye;
      }

      /* check user has executable permission */
      if ( rd_perms ) {
         if ( !fi.isReadable() ) {
            *err_val = PERROR_BADFILERD;
         }
      }

      /* check user has write permission */
      if ( wr_perms ) {
         if ( !fi.isWritable() ) {
            *err_val = PERROR_BADFILEWR;
         }
      }

   }

 bye:
   return absPath;
}



/* class Option -------------------------------------------------------- */
Option::Option( int opt_key, VkOPTION::ArgType arg_type, VkOPTION::WidgetType w_type, 
                QString cfg_group, QChar short_flag,  QString long_flag, 
                QString flag_desc, QString poss_vals, QString default_val, 
                QString shelp,     QString lhelp,     QString url ) 
{
   m_key          = opt_key;
   m_argType      = arg_type;
   m_widgType     = w_type;
   m_configGroup  = cfg_group;
   m_shortFlag    = short_flag;
   m_longFlag     = long_flag;
   m_flagDescrip  = flag_desc;
   m_possValues   = QStringList::split( "|", poss_vals );
   m_defaultValue = default_val;
   m_shortHelp    = shelp;
   m_longHelp     = lhelp;
   m_urlHelp      = url;

   /* augment m_longHelp if m_defaultValue not empty */
   if ( m_argType != VkOPTION::NOT_POPT &&
        !m_defaultValue.isEmpty() )
      m_longHelp += " [" + m_defaultValue + "]";

   /* Assert is valid option ---------------------------------------- */
   if (0) { print(); }

   /* we always expect the following fields */
   vk_assert( !m_configGroup.isEmpty() );
   vk_assert( !m_longFlag.isEmpty() );
   vk_assert( !m_longHelp.isEmpty() || !m_shortHelp.isEmpty() );

   /* if ARG_NONE, we don't expect any argument related stuff */
   if (m_argType == VkOPTION::ARG_NONE) {
      vk_assert( m_flagDescrip.isEmpty() );
      vk_assert( m_possValues.isEmpty() );
   }

   /* NOT_POPT options: not for command-line processing */
   if (m_argType == VkOPTION::NOT_POPT) {
      /* vk_popt option parsing relies on at least these being non-empty: */
      vk_assert( !m_shortFlag.isNull() || !m_longFlag.isEmpty() );
      /* only short help wanted: used for gui widget text */
      vk_assert( !m_shortHelp.isEmpty() );
      vk_assert( m_longHelp.isEmpty() );
   }

   /* ARG_PWR2 options */
   if (m_argType == VkOPTION::ARG_PWR2) {
      /* min|max */
      vk_assert( m_possValues.count() == 2 );
      /* m_defaultValue, m_possValues must all be powers of 2 */
      vk_assert( isPowerOfTwo( m_defaultValue  ) );
      vk_assert( isPowerOfTwo( m_possValues[0] ) );
      vk_assert( isPowerOfTwo( m_possValues[1] ) );
      /* min <= default <= max */
      bool ok;
      unsigned long dflt = m_defaultValue.toULong(&ok);
      vk_assert(ok);
      unsigned long min  = m_possValues[0].toULong(&ok);
      vk_assert(ok);
      unsigned long max  = m_possValues[1].toULong(&ok);
      vk_assert(ok);
      vk_assert( min <= dflt );
      vk_assert( dflt <= max );
   }

   /* ARG_UINT options */
   if (m_argType == VkOPTION::ARG_UINT) {
      /* except don't test cachegrind's horrible cache options */
      bool dontTest = ( m_configGroup == "cachegrind" &&
                        ( m_longFlag == "I1" |
                          m_longFlag == "D1" |
                          m_longFlag == "L2" ) );
      if (!dontTest) {
         /* min|max */
         vk_assert( m_possValues.count() == 2 );
         /* min <= default <= max */
         bool ok;
         unsigned long dflt = m_defaultValue.toULong(&ok);
         vk_assert(ok);
         unsigned long min  = m_possValues[0].toULong(&ok);
         vk_assert(ok);
         unsigned long max  = m_possValues[1].toULong(&ok);
         vk_assert(ok);
         vk_assert( min <= dflt );
         vk_assert( dflt <= max );
      }
   }

   /* ARG_BOOL options */
   if (m_argType == VkOPTION::ARG_BOOL) {
      /* true|false (in various guises) */
      vk_assert( m_possValues.count() == 2 );
      /* accepted bool forms: */
      QString t = m_possValues[0];
      QString f = m_possValues[1];
      vk_assert( t == "true"  || t == "on"  || t == "yes" || t == "1" || t == "T" );
      vk_assert( f == "false" || f == "off" || f == "no"  || f == "0" || f == "F" );
   }

   /* OptWidget relies on bool values being in order <true|false> */
   if ( m_widgType==VkOPTION::WDG_CHECK ||
        m_widgType==VkOPTION::WDG_RADIO ) {

      vk_assert( m_possValues.count() == 2 );

      QString yes = m_possValues[0];
      QString no  = m_possValues[1];

      vk_assert( yes=="1" || yes=="on" || yes=="yes" || yes=="true" );
      vk_assert(  no=="0" || no=="off" ||  no=="no"  || no=="false" );
   }
}


bool Option::isValidArg( int* err_val, QString argval  )
{
   *err_val = PARSED_OK;
   switch ( m_argType ) {

      /* eg. m_possValues == { 0, 4 }, ie. min=0, max=4 */
   case VkOPTION::ARG_UINT: {
      vk_assert( m_possValues.count() == 2 );
      /* is this a number? */
      bool ok;
      unsigned int val = argval.toUInt(&ok);
      if (!ok)
         *err_val = PERROR_BADNUMBER;
      else { /* looking good ... */
         unsigned int min = m_possValues[0].toUInt(&ok);
         vk_assert(ok);
         unsigned int max = m_possValues[1].toUInt(&ok);
         vk_assert(ok);
         if ( val < min || val > max ) {
            *err_val = PERROR_OUTOFRANGE;
         }
      }
   } break;

   /* eg. m_possValues == { 2, 16 }, ie. min=2, max=16 */
   case VkOPTION::ARG_PWR2: {
      vk_assert( m_possValues.count() == 2 );
      /* is this a number? */
      bool ok;
      unsigned long val = argval.toULong(&ok);
      if (!ok)
         *err_val = PERROR_BADNUMBER;
      else { /* looking good ... */
         /* is this a power of 2? */
         if ( isPowerOfTwo( argval, err_val ) ) { /* looking better ... */
            unsigned long min = m_possValues[0].toULong();
            unsigned long max = m_possValues[1].toULong();
            if ( val < min || val > max ) {
               *err_val = PERROR_OUTOFRANGE;
            }
         }
      }
   } break;

   /* m_possValues == { memcheck, addrcheck, ... } */
   case VkOPTION::ARG_STRING: {
      if ( m_possValues.findIndex( argval ) == -1 )
         *err_val = PERROR_BADARG;
   } break;

   /* m_possValues == { yes|true, no|false } or whatever */
   case VkOPTION::ARG_BOOL: {
      vk_assert( m_possValues.count() == 2 );
      if ( argval != m_possValues[0] && 
           argval != m_possValues[1] ) {
         *err_val = PERROR_BADARG;
      }
   } break;

   /* this option is only ever called from within an options page via
      radio buttons etc., so the values can never be typed in. ergo,
      don't bother to check. */
   case VkOPTION::NOT_POPT:
      break;

      /* this should never happen: only relevant to popt short help
         options. */
   case VkOPTION::ARG_NONE:
      vk_assert_never_reached();
      break;
      
   }

   return ( *err_val == PARSED_OK );
}


// static fn
bool Option::isPowerOfTwo( QString argval, int *err_val/*=0*/  )
{
   bool ok;
   unsigned long val = argval.toULong(&ok);
   
   if ( !ok ) {
      if (err_val)
         *err_val = PERROR_BADNUMBER;
      return false;
   }

   switch ( val ) {
   case 1:       case 2:       case 4:       case 8:
   case 16:      case 32:      case 64:      case 128:
   case 256:     case 512:     case 1024:    case 2048:
   case 4096:    case 8192:    case 16384:   case 32768:
   case 65536:   case 131072:  case 262144:  case 524288:
   case 1048576: case 2097152: case 4194304: case 8388608:
      break;
   default:
      if (err_val)
         *err_val = PERROR_POWER_OF_TWO;
      return false;
   }

   if (err_val)
      *err_val = PARSED_OK;
   return true;
}


void Option::setPossibleValues( QStringList vals ) 
{
   m_possValues.clear();
   for ( uint i=0; i<vals.count(); i++ )
      m_possValues << vals[i];
}


/* for debugging */
void Option::print()
{
   vkPrint( "m_key          = %d", m_key );
   vkPrint( "m_widgType     = %d", m_widgType );
   vkPrint( "m_argType      = %d", m_argType );
   vkPrint( "m_shortFlag    = %c", m_shortFlag.latin1() );
   vkPrint( "m_longFlag     = %s", m_longFlag.latin1() );
   vkPrint( "m_flagDescrip  = %s", m_flagDescrip.latin1() );
   vkPrint( "m_shortHelp    = %s", m_shortHelp.latin1() );
   vkPrint( "m_longHelp     = %s", m_longHelp.latin1() );
   vkPrint( "m_urlHelp      = %s", m_urlHelp.latin1() );
   vkPrint( "m_defaultValue = %s", m_defaultValue.latin1() );
   vkPrint( "m_configGroup  = %s", m_configGroup.latin1() );
   vkPrint( "m_possValues   = |%s|", m_possValues.join("|").latin1() );
   vkPrint( " " );
}


