/****************************************************************************
** Parse command-line options implementation
**  - Called from main() to parse both valkyrie and valgrind args
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

#include "objects/vk_objects.h"
#include "objects/valkyrie_object.h"      // for VALKYRIE::enums

#include "utils/vk_utils.h"
#include "options/vk_popt.h"
#include "options/vk_option.h"    // namespace VkOPT
#include "utils/vk_config.h"


void showHelp( vkPoptContext con, char key, Valkyrie* vk )
{
   QString VkName = VkCfg::appName();
   VkName.replace( 0, 1, VkName[0].toUpper() );
   
   switch ( key ) {
   case 'v':
      printf( "%s\n", qPrintable( VkCfg::appPackage() ) );
      break;

   case 'h':
      vkPoptPrintHelp( con, stdout, vk->objectName().toLatin1().constData() );
      printf( "\n%s\n"
              "and licensed under the GNU General Public License, version 2.\n"
              "Bug reports, feedback, praise, abuse, etc, to <%s>\n",
              qPrintable( VkCfg::copyright() ),
              qPrintable( VkCfg::email() ) );
      printf( "\nThis Valkyrie release (v%s) supports Valgrind v%s\n\n",
              qPrintable( VkCfg::appVersion() ),
              qPrintable( VkCfg::vgVersion() ) );
      break;
      
   case 'V':
      vkPoptPrintHelp( con, stdout, NULL );
      printf( "%s\n%s\n",
              qPrintable( VkCfg::copyright() ),
              qPrintable( VkCfg::vgCopyright() ) );
      printf( "\nThis Valkyrie release (v%s) supports Valgrind v%s\n\n",
              qPrintable( VkCfg::appVersion() ),
              qPrintable( VkCfg::vgVersion() ) );
      break;
      
   default:
      vk_assert_never_reached();
   }
}


void parseError( vkPoptContext con, const int err )
{
   // don't print anything; sender is dealing with msgs
   if ( err != PERROR_DEFAULT ) {
      fprintf( stderr, "%s: Parse error [%s] : %s\n",
               qPrintable( VkCfg::appName() ),
               vkPoptBadOption( con ),
               parseErrString( err ) );
      fprintf( stderr, "%s: Use --help for more information.\n",
               qPrintable( VkCfg::appName() ) );
   }
}


/*!
  getObjOptions()
  Determine the total no. of option structs required by counting the
  no. of entries in the optList.  Note: num_options = optList+1
  because we need a NULL option entry to terminate each option array.
*/
vkPoptOption* getObjOptions( /*IN */VkObject* obj,
                                    /*OUT*/vkPoptOption* vkOpts )
{
   vk_assert( obj != NULL );
   vk_assert( vkOpts != NULL );
   
   int idx = 0;
   OptionHash opthash = obj->getOptions();
   
   for ( Iter_OptionHash it = opthash.begin(); it != opthash.end(); ++it ) {
      VkOption* opt = it.value();
      vk_assert( opt != NULL );
      
      // ignore non-popt options
      if ( opt->argType == VkOPT::NOT_POPT ) {
         continue;
      }
      
      vkPoptOption* vkopt = &vkOpts[idx++];
      vkopt->arg       = NULL;
      vkopt->optId     = opt->optid;
      vkopt->argType   = opt->argType;
      vkopt->shortFlag = opt->shortFlag.toLatin1();
      
      vkopt->optGrp    = ( char* )malloc( opt->configGrp.length() + 1 );
      strncpy( vkopt->optGrp, opt->configGrp.toLatin1().data(),
               opt->configGrp.length() + 1 );
      
      vkopt->longFlag  = ( char* )malloc( opt->longFlag.length() + 1 );
      strncpy( vkopt->longFlag, opt->longFlag.toLatin1().data(),
               opt->longFlag.length() + 1 );
      
      vkopt->helptxt   = ( char* )malloc( opt->longHelp.length() + 1 );
      strncpy( vkopt->helptxt, opt->longHelp.toLatin1().data(),
               opt->longHelp.length() + 1 );
      
      vkopt->helpdesc  = ( char* )malloc( opt->flagDescr.length() + 1 );
      strncpy( vkopt->helpdesc, opt->flagDescr.toLatin1().data(),
               opt->flagDescr.length() + 1 );
   }
   
   // null entry terminator
   vkOpts[idx] = nullOpt();
   return vkOpts;
}


void getAllOptions( /*IN */VkObjectList objList,
                           /*OUT*/vkPoptOption* allOpts )
{
   int nopts, idx = 0;
   size_t nbytes;
   vkPoptOption* vkOpts;
   vk_assert( allOpts != NULL );
   
   for ( int i = 0; i < objList.size(); ++i ) {
      VkObject* obj = objList.at( i );
      
      // allocate mem for this object's options
      nopts  = obj->getOptions().count();
      nbytes = sizeof( vkPoptOption ) * ( nopts + 1/*null end*/ );
      vkOpts = ( vkPoptOption* )malloc( nbytes );
      
      vkPoptOption* tblOpt = &allOpts[idx++];
      *tblOpt = nullOpt();                             // init null struct
      tblOpt->arg     = getObjOptions( obj, vkOpts );  // get this object's options
      // set table name to option group name
      tblOpt->optGrp = ( char* )malloc( obj->objectName().length() + 1 );
      strncpy( tblOpt->optGrp, obj->objectName().toLatin1().data(),
               obj->objectName().length() + 1 );
   }
   
   // null entry terminator
   allOpts[idx] = nullOpt();
}


void freeOptions( vkPoptOption* opt )
{
   for ( ; opt && opt->optGrp; opt++ ) {
      // recurse and free the opt tree.
      if ( opt->arg != NULL ) {
         freeOptions( opt->arg );
         _free( opt->arg );  // Do free non-top level opts
      }
      
      _free( opt->optGrp );
      _free( opt->longFlag );
      _free( opt->helptxt );
      _free( opt->helpdesc );
      // Don't free top level opts: allocated on stack.
   }
}


bool parseCmdArgs( int argc, char** argv, Valkyrie* vk,
                   bool& show_help_and_exit )
{
   int rc = PARSED_OK;         // check fn return value / err value
   bool ret = true;            // return value
   show_help_and_exit = false; // just quit.
   char argVal[512];           // store argument values for checking
   
   // --------------------------------------------------
   // fetch all object options
   VkObjectList objList = vk->vkObjList();
   int num_objs = objList.count();
   vkPoptOption allOpts[num_objs+1/*null end*/];
   getAllOptions( objList, allOpts );
   
   // --------------------------------------------------
   // context for parsing cmd-line opts
   vkPoptContext optCon =
      vkPoptGetContext( argc, ( const char** )argv, allOpts );
      
   // --------------------------------------------------
   // process the options
   while ( true ) {
      const vkPoptOption* opt = NULL;
      bool done_vk_flags = false;
      rc = vkPoptGetNextOpt( optCon, argVal, &opt, done_vk_flags );
      
      if ( rc != PARSED_OK ) {
         // an error occurred during option processing
         parseError( optCon, rc );
         ret = false;
         goto done;
      }
      
      if ( done_vk_flags ) {
         // no more vk/vg flags - continue on.
         break;
      }
      
      vk_assert( opt );
      
      // Just show help and exit?
      char vk_arg = opt->shortFlag;
      
      //    printf("vk_arg: '%c'\n", vk_arg);
      if ( vk_arg == 'h' || vk_arg == 'v' || vk_arg == 'V' ) {
         showHelp( optCon, vk_arg, vk );
         show_help_and_exit = true;
         goto done;
      }
      
      // vkObjects check their own arguments
      QString qs_argval = argVal;
      rc = vk->checkOptArg( opt->optGrp, opt->optId, qs_argval );
      
      if ( rc != PARSED_OK ) {
         parseError( optCon, rc );
         ret = false;
         goto done;
      }
      
      vk->updateConfig( opt->optGrp, opt->optId, qs_argval );
      
   } // end while.
   
   
   // --------------------------------------------------
   // Get the leftovers: should only be 'myprog --myflags'.
   //  - Check we really do have a valid prog-to-debug here.
   //    If yes, then assume all subsequent flags belong to it.
   if ( vkPoptPeekArg( optCon ) != NULL ) {
   
      // First get myprog
      QString qs_argval = vkPoptGetArg( optCon );
      rc = vk->checkOptArg( VALKYRIE::BINARY, qs_argval );
      
      if ( rc != PARSED_OK ) {
         parseError( optCon, rc );
         ret = false;
         goto done;
      }
      
      vk->updateConfig( VALKYRIE::BINARY, qs_argval );
      
      // Get myprog's flags, if any
      const char** args = vkPoptGetArgs( optCon );
      QStringList aList;
      
      for ( int i = 0; args && args[i] != NULL; i++ ) {
         aList << args[i];
      }
      
      qs_argval = aList.isEmpty() ? "" : aList.join( " " );
      rc = vk->checkOptArg( vk->objectName(), VALKYRIE::BIN_FLAGS, qs_argval );
      
      if ( rc != PARSED_OK ) {
         parseError( optCon, rc );
         ret = false;
         goto done;
      }
      
      vk->updateConfig( VALKYRIE::BIN_FLAGS, qs_argval );
   }
   
   
done:
   // --------------------------------------------------
   // Cleanup
   freeOptions( allOpts );
   vkPoptFreeContext( optCon );
   return ret;
}
