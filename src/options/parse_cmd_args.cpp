#include "vk_objects.h"
#include "vk_include.h"
#include "vk_utils.h"
#include "vk_popt.h"
#include "vk_config.h"

/* return values: error = -1, show-help-and-exit = 0, ok = 1 */
#define PARSE_ERROR   -1
#define SHOW_HELP_EXIT 0
#define PARSED_OKAY    1


int showHelp( vkPoptContext con, int key )
{
  switch ( key ) {

  case 'v':
    printf("%s-%s\n", Vk_Name, VK_VERSION );
    break;

  case 'h':
    vkPoptPrintHelp( con, stdout, "Valkyrie options:" );
    printf( "\n%s is copyright %s %s\n"
						"and licensed under the GNU General Public License, version 2.\n"
						"Bug reports, feedback, praise, abuse, etc, to <%s>\n\n",
						Vk_Name, VK_COPYRIGHT, VK_AUTHOR, VK_EMAIL );
    break;

  case 'V':
    vkPoptPrintHelp( con, stdout, NULL );
    printf("\n%s is copyright %s %s\n", 
					 Vk_Name, VK_COPYRIGHT, VK_AUTHOR );
    printf("Valgrind is copyright %s\n\n", VG_COPYRIGHT );
    break;

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
  char rc;                // used for argument parsing
  char argVal[512];       // store argument values for checking

  VkObjectList objList = vkConfig->vkObjList();
	int num_objs = objList.count();
  vkPoptOption allOptions[num_objs+1];

	int i = 0;
	QString name_str;
  VkObject* obj;
	for ( obj = objList.first(); obj; obj = objList.next() ) {

		name_str.sprintf( "%s options:", obj->title().ascii() ); 
		allOptions[i].argType   = ARG_INC_TABLE;
		allOptions[i].shortFlag = '\0';
		allOptions[i].longFlag  = NULL;
		allOptions[i].arg       = obj->poptOpts();
	  allOptions[i].val       = 0;
		allOptions[i].helptxt   = vk_strdup( name_str.ascii() );
		allOptions[i].helpdesc  = NULL;

		i++;
	}

	/* null entry terminator */
	allOptions[i].argType   = 0;
	allOptions[i].shortFlag = '\0';
	allOptions[i].longFlag  = NULL;
	allOptions[i].arg       = 0;
	allOptions[i].val       = 0;
	allOptions[i].helptxt   = NULL;
	allOptions[i].helpdesc  = NULL;

	/* context for parsing cmd-line opts */
  vkPoptContext optCon = vkPoptGetContext( argc, (const char**)argv, 
																					 allOptions ); 

  /* process the options */ 
  while ( (rc = vkPoptGetNextOpt( optCon, argVal ) ) >= 0 ) {

		if ( rc == 'h' || rc == 'v' || rc == 'V' ) {
			return showHelp( optCon, rc );
		} else {
			errVal = VkObject::checkArg( rc, argVal );
		}

    if ( errVal != PARSED_OK ) {
      return parseError( optCon, errVal );
    }

  }   /* end while ... */

  /* an error occurred during option processing */ 
  if ( rc < -1 ) {
    return parseError( optCon, rc );
  }

  /* Get the leftovers: should only be 'myprog --myflags'.  Check we
		 really do have the right prog-to-debug here.  If yes, then all
		 flags that follow it on the cmd line are assumed to belong to it. */
  if ( vkPoptPeekArg(optCon) != NULL ) {
		obj    = vkConfig->vkObject( "valkyrie" );
    errVal = obj->checkOptArg( Valkyrie::BINARY, vkPoptGetArg(optCon) );
    if ( errVal != 0 )
      return parseError( optCon, errVal );
  }

  /* peek to see if any flags specified */
  if ( vkPoptPeekArg(optCon) != NULL ) {
    const char **args = vkPoptGetArgs( optCon );
    int i = 0;
    QStringList aList;
    while ( args && args[i] != NULL ) {
      aList << args[i];
      i++;
    }
    QString flags = aList.join( " " );
		obj    = vkConfig->vkObject( "valkyrie" );
    errVal = obj->checkOptArg( Valkyrie::BIN_FLAGS, flags.latin1() );
    if ( errVal != 0 )
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
