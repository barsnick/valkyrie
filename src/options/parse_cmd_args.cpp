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
#include "valkyrie_object.h"  /* for obj->checkOptArg(...); */

#include "vk_utils.h"
#include "vk_popt.h"
#include "vk_config.h"

/* return values: error = -1, show-help-and-exit = 0, ok = 1 */
#define PARSE_ERROR   -1
#define SHOW_HELP_EXIT 0
#define PARSED_OKAY    1


int showHelp( vkPoptContext con, char key )
{
  switch ( key ) {

  case 'v':
    printf("%s-%s\n", vkConfig->vkName(), vkConfig->vkVersion() );
    break;

  case 'h':
    vkPoptPrintHelp( con, stdout, "Valkyrie options:" );
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

  vkPoptFreeContext( con ); 
  return SHOW_HELP_EXIT;
}


int parseError( vkPoptContext con, const int err )
{
  /* don't print anything; sender is dealing with msgs */
  if ( err != PERROR_DEFAULT ) {
    fprintf( stderr, "Parse error [%s] : %s\n", 
             vkPoptBadOption(con), parseErrString( err )  );
    fprintf( stderr, 
             "Try 'valkyrie --help' for more information.\n" );
  }
  vkPoptFreeContext( con ); 
  return PARSE_ERROR;
}           


int parseCmdArgs( int argc, char** argv )
{
  int errVal;             // check fn return value
  int rc;                 // used for argument parsing
  char argVal[512];       // store argument values for checking

  VkObjectList objList = vkConfig->vkObjList();
  int num_objs = objList.count();
  vkPoptOption allOptions[num_objs+1];

  int i = 0;
  QString name_str;
  VkObject* obj;
  for ( obj = objList.first(); obj; obj = objList.next() ) {

    name_str.sprintf( "%s options:", obj->title().latin1() ); 
    allOptions[i].optKey    = -1;
    allOptions[i].argType   = ARG_INC_TABLE;
    allOptions[i].shortFlag = '\0';
    allOptions[i].longFlag  = NULL;
    allOptions[i].arg       = obj->poptOpts();
    allOptions[i].helptxt   = vk_strdup( name_str.latin1() );
    allOptions[i].helpdesc  = NULL;

    i++;
  }

  /* null entry terminator */
  allOptions[i].optKey    = -1;
  allOptions[i].argType   = 0;
  allOptions[i].shortFlag = '\0';
  allOptions[i].longFlag  = NULL;
  allOptions[i].arg       = 0;
  allOptions[i].helptxt   = NULL;
  allOptions[i].helpdesc  = NULL;

  /* context for parsing cmd-line opts */
  vkPoptContext optCon = vkPoptGetContext( argc, (const char**)argv, 
                                           allOptions ); 

  /* process the options */ 
  const vkPoptOption* opt = NULL;
  while ( (rc = vkPoptGetNextOpt( optCon, argVal, &opt ) ) == 1 ) {
    vk_assert(opt);

    char vk_arg = opt->shortFlag;
    //    printf("vk_arg: '%c'\n", vk_arg);
    if ( vk_arg == 'h' || vk_arg == 'v' || vk_arg == 'V' )
      return showHelp( optCon, vk_arg );
    
    obj = vkConfig->vkObject( opt->objectId );
    vk_assert( obj != NULL );
    errVal = obj->checkOptArg( opt->optKey, argVal );

    if ( errVal != PARSED_OK ) {
      return parseError( optCon, errVal );
    }

  }   /* end while ... */

  /* an error occurred during option processing */ 
  if ( rc < -1 ) {
    return parseError( optCon, rc );
  }

  /* get the leftovers: should only be 'myprog --myflags'.  check we
     really do have a valid prog-to-debug here.  if yes, then all
     flags that follow it on the cmd line are assumed to belong to it. */
  if ( vkPoptPeekArg(optCon) != NULL ) {
    obj    = vkConfig->vkObject( "valkyrie" );
    errVal = obj->checkOptArg( Valkyrie::BINARY, vkPoptGetArg(optCon) );
    if ( errVal != PARSED_OK )
      return parseError( optCon, errVal );

    /* get client flags, if any */
    const char **args = vkPoptGetArgs( optCon );
    int i = 0;
    QStringList aList;
    while ( args && args[i] != NULL ) {
      aList << args[i];
      i++;
    }
    QString flags = aList.join( " " );
    obj    = vkConfig->vkObject( "valkyrie" );
    if (!flags.isNull()) {
      errVal = obj->checkOptArg( Valkyrie::BIN_FLAGS, flags.latin1() );
    } else {
      errVal = obj->checkOptArg( Valkyrie::BIN_FLAGS, "" );
    }
    if ( errVal != PARSED_OK )
      return parseError( optCon, errVal );
  }

  i = 0;
  for ( obj = objList.first(); obj; obj = objList.next() ) {
    vkPoptOption * args = (vkPoptOption*)allOptions[i].arg;
    obj->freePoptOpts( args );
    i++;
  }

  vkPoptFreeContext( optCon ); 

  return PARSED_OKAY;
}
