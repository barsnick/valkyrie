/* --------------------------------------------------------------------- 
 * Implementation of class Option                          vk_option.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_option.h"
#include "vk_popt_option.h"
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
      return "Unknown option";
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
        "--gdb-attach=yes conflicts with --trace-children=yes\n"
        "Please choose one or the other, but not both.";
    case PERROR_DB_OUTPUT:
      return "using --gdb-attach=yes only makes sense "
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

  if ( file_name[0] == '/' ) {
    /* if 'file_name' starts with a '/', it is already an absolute path */
    absPath = file_name;
  } else if ( file_name.find('/') != -1 ) {
    /* if 'file_name' contains one or more slashes, it is a file
       path relative to the current directory.  */
    absPath = QDir::current().absFilePath( file_name );
  } else {
    /* no '/' found, so the file_name path needs to be determined
       using the $PATH environment variable */
    char* pEnv = getenv( "PATH" );
    QStringList paths( QStringList::split(QChar(':'), pEnv) );
    for ( QStringList::Iterator p = paths.begin(); p != paths.end(); ++p ) {
      QDir dir( *p + "/" + file_name );
      QString candidate = dir.absPath();
      if ( QFile::exists(candidate) ) {
        absPath = candidate;
        break;
      }
    }
  }

  absPath = QDir::cleanDirPath( absPath );
  QFileInfo fi( absPath );
  if ( !fi.exists() ) {
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
Option::Option( int opt_key, ArgType arg_type, WidgetType w_type, 
                QString cfg_group, QChar short_flag,  QString long_flag, 
                QString flag_desc, QString poss_vals, QString default_val, 
                QString shelp,     QString lhelp,     const char* url ) 
{
  key          = opt_key;
  argType      = arg_type;
  widgType     = w_type;
  configGroup  = cfg_group;
  shortFlag    = short_flag;
  longFlag     = long_flag;
  flagDescrip  = flag_desc;
  possValues   = QStringList::split( "|", poss_vals );
  defaultValue = default_val;
  shortHelp    = shelp;
  longHelp     = lhelp;
  urlHelp      = url;
  modified     = false;

#if 0
  /* OptWidget relies on bool values being in order <true|false> */
  if ( widgType==OptWidget::CHECK || widgType==OptWidget::RADIO ) {

    vk_assert( possValues.count() == 2 );

    QString yes = possValues[0];
    QString no  = possValues[1];

    vk_assert( yes=="1" || yes=="on" || yes=="yes" || yes=="true" );
    vk_assert(  no=="0" || no=="off" ||  no=="no"  || no=="false" );
  }
#endif
}


bool Option::isValidArg( int* err_val, const char* argval  )
{
  *err_val = PARSED_OK;
  switch ( argType ) {

    /* eg. possValues == { 0, 4 }, ie. min=0, max=4 */
    case ARG_UINT: {
      vk_assert( possValues.count() == 2 );
      /* is this a number? */
      unsigned int val = str2UInt( err_val, argval );
      if ( *err_val == PARSED_OK ) { /* looking good ... */
        unsigned int min = possValues[0].toUInt();
        /* if max == -1, then no upper bound */
        unsigned int max = (possValues[1] == "-1") 
          ? UINT_MAX : possValues[1].toUInt();
        if ( val < min || val > max ) {
          *err_val = PERROR_OUTOFRANGE;
        }
      }
    } break;

    /* possValues == { memcheck, addrcheck, ... } */
    case ARG_STRING: {
      if ( possValues.findIndex( argval ) == -1 )
        *err_val = PERROR_BADARG;
    } break;

    /* possValues == { yes|true, no|false } or whatever */
    case ARG_BOOL: {
      vk_assert( possValues.count() == 2 );
      if ( ( !vk_strcmp( argval, possValues[0] ) ) && 
           ( !vk_strcmp( argval, possValues[1] ) ) ) {
        *err_val = PERROR_BADARG;
      }
    } break;

    /* this option is only ever called from within an options page via
       radio buttons etc., so the values can never be typed in. ergo,
       don't bother to check. */
    case NOT_POPT:
      break;

      /* this should never happen: only relevant to popt short help
         options. */
    case ARG_NONE:
      vk_assert_never_reached();
      break;
      
  }

  return ( *err_val == PARSED_OK );
}


bool Option::isPowerOfTwo( int *err_val, const char *argval )
{
  if ( isValidArg( err_val, argval ) ) {
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
  }

 bye:
  return (*err_val == PARSED_OK);
}


void Option::setPossibleValues( QStringList vals ) 
{
  possValues.clear();
  for ( uint i=0; i<vals.count(); i++ )
    possValues << vals[i];
}


/* for debugging */
void Option::print()
{
  printf( "key          = %d\n", key );
  printf( "modified     = %d\n", modified );
  printf( "widgType     = %d\n", widgType );
  printf( "argType      = %d\n", argType );
  printf( "shortFlag    = %c\n", shortFlag.latin1() );
  printf( "longFlag     = %s\n", longFlag.latin1() );
  printf( "flagDescrip  = %s\n", flagDescrip.latin1() );
  printf( "shortHelp    = %s\n", shortHelp.latin1() );
  printf( "longHelp     = %s\n", longHelp.latin1() );
  printf( "urlHelp      = %s\n", urlHelp.latin1() );
  printf( "defaultValue = %s\n", defaultValue.latin1() );
  printf( "configGroup  = %s\n", configGroup.latin1() );
  printf( "possValues   = |" );
  for ( uint i=0; i<possValues.count(); i++ )
    printf("%s|", possValues[i].latin1() );
  printf("\n");
}


