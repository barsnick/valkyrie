/* This is a seriously hacked version of the popt libraries.  
 * No credit to me, all thanks and many apologies to the Red Hat
 * team. 
 */

#ifndef __VK_POPT_H
#define __VK_POPT_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* for struct vkPoptOption and parseErrString() */
#include "vk_popt_option.h" 


#define vmefail() (fprintf(stderr, "virtual memory exhausted.\n"), exit(EXIT_FAILURE), NULL)
#define xstrdup(_str) (strcpy((malloc(strlen(_str)+1) ? : vmefail()), (_str)))


#define OPTION_DEPTH  10

/* Arg type identifiers */
#define ARG_NONE      0  /* no arg */
#define ARG_STRING    1  /* no test performed           */
#define ARG_UINT      2  /* arg tested as unsigned int  */
#define ARG_BOOL      3  /* arg tested as { yes, no }   */
#define ARG_INC_TABLE 4  /* arg points to table         */
#define ARG_MASK      0x0000FFFF

/* options can't follow args */
#define PCONTEXT_POSIXMEHARDER (1 << 2)

#define TABLE_END { 0, '\0', NULL, 0, 0, NULL, NULL }

typedef struct vkPoptContext_s * vkPoptContext;

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize popt context.
 * argc    no. of arguments
 * argv    argument array
 * options address of popt option table
 * returns initialized popt context */
vkPoptContext vkPoptGetContext( int argc, const char ** argv,
                                const vkPoptOption * options );

/* Return value of next option found.
 * return next option val, -1 on last item, PERROR_* on error */
int vkPoptGetNextOpt( vkPoptContext con, char *akey );

/* Return current option's argument.
 * return option argument, NULL if no more options are available */
const char * vkPoptGetArg( vkPoptContext con );

/* Return remaining arguments.
 * return argument array, terminated with NULL */
const char ** vkPoptGetArgs( vkPoptContext con );

/* Peek at current option's argument.
 * return option argument */
const char * vkPoptPeekArg( vkPoptContext con );

/* Return the option which caused the most recent error.
 * return offending option */
const char * vkPoptBadOption( vkPoptContext con );

/* Destroy context.
 * return NULL always */
vkPoptContext vkPoptFreeContext( vkPoptContext con );

/* Print detailed description of options.
 * fp    ouput file handle */
void vkPoptPrintHelp( vkPoptContext con, FILE * fp,
                      const char * tableName );

#ifdef  __cplusplus
}
#endif

/*--------------------------------------------------*/
/* Wrapper to free(3), hides const compilation noise,
   permit NULL, return NULL always */
static inline void * _free( const void * p )
{
  if ( p != NULL )  
    free((void *)p);
  return NULL;
}


struct optionStackEntry { 
  int argc;
  const char ** argv;
  int next;
  const char * nextArg;
  const char * nextCharArg;
};


struct vkPoptContext_s {
  struct optionStackEntry optionStack[OPTION_DEPTH];
  struct optionStackEntry * os;
  const char ** leftovers;
  int numLeftovers;
  int nextLeftover;
  const vkPoptOption * options;
  int restLeftover;
  int flags;
  const char ** finalArgv;
  int finalArgvCount;
  int finalArgvAlloced;
};
/*--------------------------------------------------*/



#endif
