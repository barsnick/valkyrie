/* --------------------------------------------------------------------- 
 * Implementation of class Option                          vk_option.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_option.h"
#include "vk_popt_option.h" /* namespace OPTION */
#include "vk_utils.h"       /* vk_assert(), free(), malloc() */

#include <ctype.h>
#include <math.h>
#include <malloc.h>
#include <limits.h>          /* ULONG_MAX et al */

#include <qdir.h>


/* these enums are defined in vk_popt_option.h ------------------------- */
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
   default:
      return "unknown error";
   }
}


/* helper functions ---------------------------------------------------- */

QString checkFile( int* err_val, const char* fname )
{
   *err_val = PARSED_OK;
   QString file_name = fname;
   QString absPath = QString::null;

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
QString fileCheck( int* err_val, const char* fpath, 
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
QString binaryCheck( int* err_val, const char* exe_name )
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



QString dirCheck( int* err_val, const char* dir,
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


/* [Ip address : port num], eg. --logsocket=192.168.0.1:12345 
   if port-num omitted, default of 1500 used.
   check arg format, should all be ints && dots */
QString formatCheck( int* err_val, const char* argval ) 
{
   *err_val = PARSED_OK;

   QString sockArg( argval );
   /* first check if we have a semi-colon present */
   QString ip, port;
   int scpos = sockArg.find( ':' );
   if ( scpos != -1 ) {  /* we have a semi-colon */
      ip   = sockArg.left( scpos );
      port = sockArg.right( sockArg.length() - scpos-1 );
   } else {
      ip   = sockArg;
      port = "1500";      /* default */
   }

   int dots = 0;
   for ( uint i = 0; i < ip.length(); i++ ) {
      if ( ip[i] == '.' || ip[i].isDigit() ) {
         if ( ip[i] == '.' )
            dots++;
         if ( dots > 3 ) {
            *err_val = PERROR_BADARG;
            break;
         }
      } else {
         *err_val = PERROR_BADARG;
         break;
      }
   }

   if ( *err_val == PARSED_OK ) { /* so far, so good */
      /* only parse this if not the default */
      if ( scpos != -1 ) {
         for ( uint i=0; i<port.length(); i++ ) {
            if ( !port[i].isDigit() ) {
               *err_val = PERROR_BADARG;
               break;
            }
         }
      }
   }

   return ip + ":" + port;
}


unsigned long str2ULong( int *err_val, const char *str )
{
   *err_val = PERROR_BADNUMBER;
   char *p    = (char*)str;
   unsigned long val = 0;
   const unsigned long max_mult = ULONG_MAX / 10;

   if ( !p ) {
      *err_val = PERROR_BADNUMBER;
      goto bye;
   }

   while ( isspace((uchar) *p) )    /* skip leading space */
      p++;
   if ( *p == '+' )
      p++;
   if ( !isdigit((uchar) *p) ) {
      *err_val = PERROR_BADNUMBER;
      goto bye;
   }

   while ( isdigit((uchar) *p) ) {
      if ( val > max_mult || (val == max_mult && (*p-'0') > 5) ) {
         *err_val = PERROR_OVERFLOW;
         goto bye;
      }
      val = 10*val + (*p++ - '0');
   }

   while ( isspace((uchar) *p) )    /* skip trailing space */
      p++;
   if ( *p == '\0' )
      *err_val = PARSED_OK;

 bye:
   return val;
}


unsigned int str2UInt( int *err_val, const char *str )
{
   *err_val = PERROR_BADNUMBER;

   /* word up to 64 bit unsigned */
   unsigned long val = str2ULong( err_val, str );

   return (unsigned int)val;
}



/* class Option -------------------------------------------------------- */
Option::Option( int opt_key, VkOPTION::ArgType arg_type, VkOPTION::WidgetType w_type, 
                QString cfg_group, QChar short_flag,  QString long_flag, 
                QString flag_desc, QString poss_vals, QString default_val, 
                QString shelp,     QString lhelp,     const char* url ) 
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
   m_modified     = false;


   /* Assert is valid option ---------------------------------------- */
   if (0) { print(); printf("\n"); }

   if (m_argType == VkOPTION::NOT_POPT) {
      /* vk_popt option parsing relies on at least these being non-empty: */
      vk_assert( !m_shortFlag.isNull() || !m_longFlag.isEmpty() );
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


bool Option::isValidArg( int* err_val, const char* argval  )
{
   *err_val = PARSED_OK;
   switch ( m_argType ) {

      /* eg. m_possValues == { 0, 4 }, ie. min=0, max=4 */
   case VkOPTION::ARG_UINT: {
      vk_assert( m_possValues.count() == 2 );
      /* is this a number? */
      unsigned int val = str2UInt( err_val, argval );
      if ( *err_val == PARSED_OK ) { /* looking good ... */
         /* TODO: test for -ve values */
         unsigned int min = m_possValues[0].toUInt();
         /* if max == -1, then no upper bound */
         unsigned int max = (m_possValues[1] == "-1") 
            ? UINT_MAX : m_possValues[1].toUInt();
         if ( val < min || val > max ) {
            *err_val = PERROR_OUTOFRANGE;
         }
      }
   } break;

   /* eg. m_possValues == { 2, 16 }, ie. min=2, max=16 */
   case VkOPTION::ARG_PWR2: {
      vk_assert( m_possValues.count() == 2 );
      /* is this a number? */
      unsigned int val = str2UInt( err_val, argval );
      if ( *err_val == PARSED_OK ) { /* looking good ... */
         /* is this a power of 2? */
         isPowerOfTwo( err_val, argval ); 
         if ( *err_val == PARSED_OK ) { /* looking better ... */
            /* TODO: Both min/max should be +ve pwrs of 2 */
            unsigned int min = m_possValues[0].toUInt();
            unsigned int max = m_possValues[1].toUInt();
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
      if ( ( !vk_strcmp( argval, m_possValues[0] ) ) && 
           ( !vk_strcmp( argval, m_possValues[1] ) ) ) {
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
bool Option::isPowerOfTwo( int *err_val, const char *argval )
{
   *err_val = PERROR_BADNUMBER;

   unsigned int val = str2UInt( err_val, argval );
   if ( *err_val != PARSED_OK )
      goto bye;

   switch ( val ) {
   case 4:      case 8:      case 16:    case 32:    case 64:
   case 128:    case 256:    case 512:   case 1024:  case 2048:
   case 4096:   case 8192:   case 16384: case 32768: case 65536:
   case 131072: case 262144: case 1048576:
      break;
   default:
      *err_val = PERROR_POWER_OF_TWO;
      break;
   }

 bye:
   return (*err_val == PARSED_OK);
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
   printf( "m_key          = %d\n", m_key );
   printf( "m_modified     = %d\n", m_modified );
   printf( "m_widgType     = %d\n", m_widgType );
   printf( "m_argType      = %d\n", m_argType );
   printf( "m_shortFlag    = %c\n", m_shortFlag.latin1() );
   printf( "m_longFlag     = %s\n", m_longFlag.latin1() );
   printf( "m_flagDescrip  = %s\n", m_flagDescrip.latin1() );
   printf( "m_shortHelp    = %s\n", m_shortHelp.latin1() );
   printf( "m_longHelp     = %s\n", m_longHelp.latin1() );
   printf( "m_urlHelp      = %s\n", m_urlHelp.latin1() );
   printf( "m_defaultValue = %s\n", m_defaultValue.latin1() );
   printf( "m_configGroup  = %s\n", m_configGroup.latin1() );
   printf( "m_possValues   = |" );
   for ( uint i=0; i<m_possValues.count(); i++ )
      printf("%s|", m_possValues[i].latin1() );
   printf("\n");
}


