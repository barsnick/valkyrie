/* --------------------------------------------------------------------- 
 * Parse command-line options                         parse_cmd_args.cpp
 * Called from main()
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_objects.h"
#include "valkyrie_object.h"  /* for Valkyrie::enums */

#include "vk_utils.h"
#include "vk_popt.h"
#include "vk_popt_option.h"  /* namespace OPTION */
#include "vk_config.h"

/* return values: error < 0, ok = 0(PARSED_OK), show-help-and-exit = 1 */
#define SHOW_HELP_EXIT 1


void showHelp( vkPoptContext con, char key, Valkyrie* vk )
{
   switch ( key ) {
   case 'v':
      printf("%s-%s\n", vkConfig->vkName(), vkConfig->vkVersion() );
      break;

   case 'h':
      vkPoptPrintHelp( con, stdout, vk->title().latin1() );
      printf( "\n%s is copyright %s %s\n"
              "and licensed under the GNU General Public License, version 2.\n"
              "Bug reports, feedback, praise, abuse, etc, to <%s>\n\n",
              vkConfig->vkName(), vkConfig->vkCopyright(), 
              vkConfig->vkAuthor(), vkConfig->vkEmail() );
      break;

   case 'V':
      vkPoptPrintHelp( con, stdout, NULL );
      printf("\n%s is copyright %s %s\n", 
             vkConfig->vkName(), 
             vkConfig->vkCopyright(), vkConfig->vkAuthor() );
      printf("Valgrind is copyright %s\n\n", vkConfig->vgCopyright() );
      break;

   default:
      vk_assert_never_reached();
   }
}


void parseError( vkPoptContext con, const int err )
{
   /* don't print anything; sender is dealing with msgs */
   if ( err != PERROR_DEFAULT ) {
      fprintf( stderr, "Parse error [%s] : %s\n", 
               vkPoptBadOption(con), parseErrString( err )  );
      fprintf( stderr, 
               "Try 'valkyrie --help' for more information.\n" );
   }
}           


/* determine the total no. of option structs required by counting the
   no. of entries in the optList.  Note: num_options = optList+1
   because we need a NULL option entry to terminate each option array. */
vkPoptOption* getObjOptions( /*IN */VkObject* obj,
                             /*OUT*/vkPoptOption* vkOpts )
{
   int idx = 0;
   Option *opt;
   OptionList optList;
   vk_assert( obj != NULL );
   vk_assert( vkOpts != NULL );

   optList = obj->optList();
   for ( opt = optList.first(); opt; opt = optList.next() ) {
      vk_assert( opt != NULL );

      /* only add me if i'm a popt option */
      if ( opt->m_argType != VkOPTION::NOT_POPT ) {
         vkPoptOption* vkopt = &vkOpts[idx++];
         vkopt->optKey    = opt->m_key;
         vkopt->argType   = opt->m_argType;
         vkopt->shortFlag = opt->m_shortFlag.latin1();
         vkopt->longFlag  = opt->m_longFlag.latin1();
         vkopt->arg       = 0;
         vkopt->helptxt   = opt->m_longHelp.latin1();
         vkopt->helpdesc  = opt->m_flagDescrip.latin1();
         /* to later call obj->checkOptArg() */
         vkopt->objectId  = obj->objId();
      }
   }
   /* null entry terminator */
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

   for ( VkObject* obj = objList.first(); obj; obj = objList.next() ) {
      /* allocate mem for this object's options */
      nopts  = obj->optList().count();
      nbytes = sizeof(vkPoptOption) * (nopts + 1/*null end*/);
      vkOpts = (vkPoptOption*)malloc( nbytes );

      vkPoptOption* tblOpt = &allOpts[idx++];
      *tblOpt = nullOpt();                             /* init null struct */
      tblOpt->arg     = getObjOptions( obj, vkOpts );  /* get this object's options */
      tblOpt->helptxt = obj->title().latin1();         /* for lookup later */
   }
   /* null entry terminator */
   allOpts[idx] = nullOpt();
}


void freeOptions( vkPoptOption* allOpts, int num_objs )
{
   for ( int idx=0; idx<num_objs; idx++) {
      /* free allocated optTable.arg */
      if (allOpts[idx].arg != NULL) {
         free( allOpts[idx].arg );
         allOpts[idx].arg = NULL;
      }
   }
}



int parseCmdArgs( int argc, char** argv, Valkyrie* vk )
{
   int rc;                 // check fn return value / err value
   char argVal[512];       // store argument values for checking

   /* fetch all object options */
   VkObjectList objList = vk->vkObjList();
   int num_objs = objList.count();
   vkPoptOption allOpts[num_objs+1/*null end*/];
   getAllOptions( objList, allOpts );

   /* context for parsing cmd-line opts */
   vkPoptContext optCon =
      vkPoptGetContext( argc, (const char**)argv, allOpts ); 

   /* process the options */ 
   const vkPoptOption* opt = NULL;
   while ( (rc = vkPoptGetNextOpt( optCon, argVal, &opt )) == PARSED_OK ) {
      /* rc == 1 for the first non-valkyrie/valgrind option */

      vk_assert(opt);

      char vk_arg = opt->shortFlag;
      //    printf("vk_arg: '%c'\n", vk_arg);
      if ( vk_arg == 'h' || vk_arg == 'v' || vk_arg == 'V' ) {
         showHelp( optCon, vk_arg, vk );
         rc = SHOW_HELP_EXIT;
         goto done;
      }
    
      VkObject* obj = vk->vkObject( opt->objectId );
      vk_assert( obj != NULL );
      rc = obj->checkOptArg( opt->optKey, argVal );

      if ( rc != PARSED_OK ) {
         parseError( optCon, rc );
         goto done;
      }

   }   /* end while ... */

   /* rc == 1 for the first non-valkyrie/valgrind option */
   if (rc == 1)
      rc = PARSED_OK; 

   /* an error occurred during option processing */ 
   if ( rc != PARSED_OK ) {
      parseError( optCon, rc );
      goto done;
   }

   /* get the leftovers: should only be 'myprog --myflags'.  check we
      really do have a valid prog-to-debug here.  if yes, then all
      flags that follow it on the cmd line are assumed to belong to it. */
   if ( vkPoptPeekArg(optCon) != NULL ) {
      rc = vk->checkOptArg( Valkyrie::BINARY, vkPoptGetArg(optCon) );
      if ( rc != PARSED_OK ) {
         parseError( optCon, rc );
         goto done;
      }

      /* get client flags, if any */
      const char **args = vkPoptGetArgs( optCon );
      int i = 0;
      QStringList aList;
      while ( args && args[i] != NULL ) {
         aList << args[i];
         i++;
      }
      QString flags = aList.join( " " );
      if (!flags.isNull()) {
         rc = vk->checkOptArg( Valkyrie::BIN_FLAGS, flags.latin1() );
      } else {
         rc = vk->checkOptArg( Valkyrie::BIN_FLAGS, "" );
      }
      if ( rc != PARSED_OK ) {
         parseError( optCon, rc );
         goto done;
      }
   }

 done:
   /* Cleanup */
   freeOptions( allOpts, num_objs );
   vkPoptFreeContext( optCon ); 
   return rc;
}
