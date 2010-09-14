/****************************************************************************
** Option implementation
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

#include "options/vk_option.h"
#include "utils/vk_config.h"
#include "utils/vk_utils.h"


/***************************************************************************/
/*!
   To add a new valgrind tool:
   1) Create the subclass in its own files in the src/objects/ directory.
      see the Example below w.r.t. addOpt(...)
   2) In Valgrind::initToolObjects() [valgrind_object.cpp],
      add 'm_toolObjList.append( new tool( objId++ ) )'
      this registers the tool with valkyrie.
   3) Create a new options page for the Options dialog, and reimplement
      VgObject::createOptionsPage() to create this when needed.
   4) Create the ToolView subclass in its own files, in the src/tool_view dir
*/





/*!
 Map error int to error string
*/
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
   case PERROR_BADFILENAME:
      return "Invalid filename";
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
   case PERROR_DUPLICATE:
      return "Duplicate entries";
   default:
      return "unknown error";
   }
}



/***************************************************************************/
/*!
    Constructs a VkOptionHash
*/
VkOptionHash::VkOptionHash()
{
}


/*!
    Destroys this widget, and frees any allocated resources.
*/
VkOptionHash::~VkOptionHash()
{
   Iter_OptionHash it = optionHash.begin();
   
   while ( it != optionHash.end() ) {
      delete it.value();
      it = optionHash.erase( it );  // erase doesn't mess up the iterator
   }
}


/*!
    Adds option to the hash.
    The \a long_flag is used as the hash key.

   Argument explanation:
   QString cfg_group    // e.g. "memcheck" - i.e. this option belongs to memcheck only
   QString long_flag    // e.g. "leak-check" - the long form of the option name
   QChar   short_flag   // e.g. '\0' - the short form option name, '\0' means none
   QString flag_desc    // e.g. "<no|summary|full>" - used to print to commandline
   QString poss_vals    // e.g. "no|summary|full" - fills the option widget
   QString default_val  // e.g. "summary" - the default value for this option
   QString shelp        // e.g. "Search for memory leaks at exit" - gui help text
   QString lhelp        // e.g. "search for memory leaks at exit?" - cmdline help text
   QString url          // e.g. "manual.html#leak-check"
   VkOPT::ArgType   arg_type // e.g. VkOPT::ARG_BOOL - how to parse/test the value
   VkOPT::WidgetType w_type  // e.g. VkOPT::WDG_CHECK - what kind of widget to use
*/
void VkOptionHash::addOpt(
   int optid,
   QString configGrp, QString longFlag, QChar   shortFlag,
   QString flagDesc,  QString possVals, QVariant dfltValue,
   QString shortHelp, QString longHelp, QString urlAddress,
   VkOPT::ArgType argType, VkOPT::WidgType widgType )
{
   VkOption* opt = new VkOption(
      optid,
      configGrp, longFlag, shortFlag,
      flagDesc,  possVals, dfltValue,
      shortHelp, longHelp, urlAddress,
      argType,   widgType );
      
   optionHash.insert( optid, opt );
}

/*!
    Returns option from the hash, using \a key as the hash key.
    The \a key is the long_flag of the option - a unique identifier.
    Returns 0 if not found (that would be a bug!)
*/
VkOption* VkOptionHash::getOption( int optid )
{
   if ( !optionHash.contains( optid ) ) {
      VK_DEBUG( "Warning: optid (%d) not found in options.", optid );
      return NULL;
   }
   return optionHash.value( optid );
}





/***************************************************************************/
/*!
   class VkOption
*/
VkOption::VkOption(
   int _optid,
   QString _configGrp, QString _longFlag, QChar    _shortFlag,
   QString _flagDescr, QString _possVals, QVariant _dfltValue,
   QString _shortHelp, QString _longHelp, QString  _urlAddress,
   VkOPT::ArgType _argType, VkOPT::WidgType _widgType )
{
   optid      = _optid;
   configGrp  = _configGrp;
   longFlag   = _longFlag;
   shortFlag  = _shortFlag;
   flagDescr  = _flagDescr;
   possValues = _possVals.split( '|', QString::SkipEmptyParts );
   dfltValue  = _dfltValue;
   shortHelp  = _shortHelp;
   longHelp   = _longHelp;
   urlAddress = _urlAddress;
   argType    = _argType;
   widgType   = _widgType;
   
   // augment longHelp if dfltValue not empty
   if ( argType != VkOPT::NOT_POPT &&
        !dfltValue.toString().isEmpty() ) {
      longHelp += " [" + dfltValue.toString() + "]";
   }
   
   // Assert is valid option ----------------------------------------
   if ( 0 ) {
      print();
   }
   
   // we always expect the following fields
   vk_assert( !configGrp.isEmpty() );
   vk_assert( !longFlag.isEmpty() );

   // always at least short or long help
   vk_assert( !longHelp.isEmpty() || !shortHelp.isEmpty() );
   
   // if ARG_NONE, we don't expect any argument related stuff
   if ( argType == VkOPT::ARG_NONE ) {
      vk_assert( flagDescr.isEmpty() );
      vk_assert( possValues.isEmpty() );
   }
   
   // NOT_POPT options: not for command-line processing
   if ( argType == VkOPT::NOT_POPT ) {

      // vk_popt option parsing relies on at least these being non-empty:
      vk_assert( !shortFlag.isNull() || !longFlag.isEmpty() );

      // gui-only options
      if ( widgType != VkOPT::WDG_NONE ) {
         // only short help wanted: used for gui widget text
         vk_assert( !shortHelp.isEmpty() );
         vk_assert( longHelp.isEmpty() );
      }
      else {
         // NOT_POPT && WDG_NONE => shouldn't exist!
         vk_assert_never_reached();
      }
   }
   
   // ARG_PWR2 options
   if ( argType == VkOPT::ARG_PWR2 ) {
      // min|max
      vk_assert( possValues.count() == 2 );
      // dfltValue, possValues must all be powers of 2
      vk_assert( isPowerOfTwo( dfltValue.toString() ) );
      vk_assert( isPowerOfTwo( possValues[0] ) );
      vk_assert( isPowerOfTwo( possValues[1] ) );
      // min <= default <= max
      bool ok;
      unsigned long dflt = dfltValue.toString().toULong( &ok );
      vk_assert( ok );
      unsigned long min  = possValues[0].toULong( &ok );
      vk_assert( ok );
      unsigned long max  = possValues[1].toULong( &ok );
      vk_assert( ok );
      vk_assert( min <= dflt );
      vk_assert( dflt <= max );
   }
   
   // ARG_UINT options
   if ( argType == VkOPT::ARG_UINT ) {
      // except don't test cachegrind's horrible cache options
      bool dontTest = ( configGrp == "cachegrind" &&
                        ( longFlag == "I1" ||
                          longFlag == "D1" ||
                          longFlag == "L2" ) );
                          
      if ( dontTest ) {
         VK_DEBUG( "Warning: Not performing UInt value test for option: %s",
                   qPrintable( longFlag ) );
      } else {
         // min|max
         vk_assert( possValues.count() == 2 );
         // min <= default <= max
         bool ok;
         unsigned long dflt = dfltValue.toString().toULong( &ok );
         vk_assert( ok );
         unsigned long min  = possValues[0].toULong( &ok );
         vk_assert( ok );
         unsigned long max  = possValues[1].toULong( &ok );
         vk_assert( ok );
         vk_assert( min <= dflt );
         vk_assert( dflt <= max );
      }
   }
   
   // ARG_BOOL options
   if ( argType == VkOPT::ARG_BOOL ) {
      // true|false (in various guises)
      vk_assert( possValues.count() == 2 );
      // accepted bool forms:
      QString t = possValues[0];
      QString f = possValues[1];
      vk_assert( t == "true"  || t == "on"  || t == "yes" || t == "1" || t == "T" );
      vk_assert( f == "false" || f == "off" || f == "no"  || f == "0" || f == "F" );
   }
   
   // OptWidget relies on bool values being in order <true|false>
   if ( widgType == VkOPT::WDG_CHECK ||
        widgType == VkOPT::WDG_RADIO ) {
        
      vk_assert( possValues.count() == 2 );
      
      QString yes = possValues[0];
      QString no  = possValues[1];
      
      vk_assert( yes == "1" || yes == "on" || yes == "yes" || yes == "true" );
      vk_assert(  no == "0" || no == "off" ||  no == "no"  || no == "false" );
   }
}


/*!
   configuration settings (QSettings) key for persistent storage:
*/
QString VkOption::configKey() {
   return configGrp + "/" + longFlag;
}



/*!
   Defines which options have their values stored to disk (via vkCfgProj)
   The following option types are NOT saved:
   ::argType == ARG_NONE  (e.g. --help)
    - They don't take args, so there's nothing to save
   ::widgType == WDG_NONE (e.g. --project-file=<myfile>)
    - These may or may not take an argument, but are only meant to drive
      Valkyrie from the commandline. There nothing to save here either.
*/
bool VkOption::isaConfigOpt()
{
   return ( argType != VkOPT::ARG_NONE &&    // ARG_NONE: takes no args
            widgType != VkOPT::WDG_NONE );   // WDG_NONE: has no UI widget
}

/*!
  Update config with option key and value
   - skip opts not destined for vkCfgProj
*/
void VkOption::updateConfig( QVariant argVal )
{
   if ( isaConfigOpt() ) {
      QString key = configKey();
      if ( !vkCfgProj->contains( key ) || vkCfgProj->value( key ) != argVal ) {
         vkCfgProj->setValue( key, argVal );
         emit valueChanged();
      }
   }
}


/*!
  tests for potential argument update
*/
bool VkOption::isValidArg( int* err_val, QString argval  )
{
   *err_val = PARSED_OK;
   switch ( argType ) {

   // eg. possValues == { 0, 4 }, ie. min=0, max=4
   case VkOPT::ARG_UINT: {
         vk_assert( possValues.count() == 2 );
         // is this a number?
         bool ok;
         unsigned int val = argval.toUInt(&ok);
         if (!ok)
            *err_val = PERROR_BADNUMBER;
         else { // looking good ...
            unsigned int min = possValues[0].toUInt(&ok);
            vk_assert(ok);
            unsigned int max = possValues[1].toUInt(&ok);
            vk_assert(ok);
            if ( val < min || val > max ) {
               *err_val = PERROR_OUTOFRANGE;
            }
         }
      }
      break;

   // eg. possValues == { 2, 16 }, ie. min=2, max=16
   case VkOPT::ARG_PWR2: {
         vk_assert( possValues.count() == 2 );
         // is this a number?
         bool ok;
         unsigned long val = argval.toULong(&ok);
         if (!ok)
            *err_val = PERROR_BADNUMBER;
         else { // looking good ...
            // is this a power of 2?
            if ( isPowerOfTwo( argval, err_val ) ) { // looking better ...
               unsigned long min = possValues[0].toULong();
               unsigned long max = possValues[1].toULong();
               if ( val < min || val > max ) {
                  *err_val = PERROR_OUTOFRANGE;
               }
            }
         }
      } break;

   // possValues == { memcheck, addrcheck, ... }
   case VkOPT::ARG_STRING: {
         if ( ! possValues.contains( argval ) ) {
            *err_val = PERROR_BADARG;
         }
      } break;

   // possValues == { yes|true, no|false } or whatever
   case VkOPT::ARG_BOOL: {
         vk_assert( possValues.count() == 2 );
         if ( argval != possValues[0] &&
              argval != possValues[1] ) {
            *err_val = PERROR_BADARG;
         }
      } break;

   // This option is only ever called from within an options page via
   // radio buttons etc., so the values can never be typed in.
   // Hence, there's nothing to check.
   case VkOPT::NOT_POPT:
         break;

   // this should never happen: only relevant to popt short help options.
   case VkOPT::ARG_NONE:
      vk_assert_never_reached();
      break;

   default:
      vk_assert_never_reached();
      break;
   }

   return ( *err_val == PARSED_OK );
}


// static fn
bool VkOption::isPowerOfTwo( QString argval, int *err_val/*=0*/  )
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


/*!
  for debugging
*/
void VkOption::print()
{
   vkPrint( "optid      = %d", optid );
   vkPrint( "widgType   = %d", widgType );
   vkPrint( "argType    = %d", argType );
   vkPrint( "shortFlag  = %c", shortFlag.toLatin1() );
   vkPrint( "longFlag   = %s", qPrintable( longFlag ) );
   vkPrint( "flagDescr  = %s", qPrintable( flagDescr ) );
   vkPrint( "shortHelp  = %s", qPrintable( shortHelp ) );
   vkPrint( "longHelp   = %s", qPrintable( longHelp ) );
   vkPrint( "urlAddress = %s", qPrintable( urlAddress ) );
   vkPrint( "dfltValue  = %s", qPrintable( dfltValue.toString() ) );
   vkPrint( "configGrp  = %s", qPrintable( configGrp ) );
   vkPrint( "possValues = |%s|", qPrintable( possValues.join( "|" ) ) );
   vkPrint( " " );
}
