/****************************************************************************
** Definition of Popt functions
** This is a seriously hacked version of the popt libraries.
** No credit to me, all thanks and many apologies to the Red Hat team
** --------------------------------------------------------------------------
** popt is Copyright (C) 1998 Red Hat Software and distributed under
** an X11-style license, which is in turn compatible the GNU GPL v.2.
** See the file COPYING for the full license details.
*/

#ifndef __VK_POPT_H
#define __VK_POPT_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "options/vk_option.h"


#define OPTION_DEPTH  10

// options can't follow args
#define PCONTEXT_POSIXMEHARDER (1 << 2)

/*!
   vkPoptOption
   This may be one of:
    - TABLE_END: none set of: shortFlag && longFlag && arg
    - table:     arg = array of options, optGrp = table title
    - option:    arg = NULL, remaining vars filled
*/
typedef struct _vkPoptOption {
   struct _vkPoptOption* arg;       // table holds ptr to
   char*                 optGrp;    // opt group name: "valgrind" etc.
   int                   optId;     // eg. VIEW-LOG
   VkOPT::ArgType        argType;   // option type: ARG_***
   char                  shortFlag; // '\0' || 'h'
   char*                 longFlag;  // NULL || --help
   char*                 helptxt;   // help text
   char*                 helpdesc;  // eg. <file>
} vkPoptOption;

#define TABLE_END { NULL, NULL, -1, VkOPT::NOT_POPT, '\0', NULL, NULL, NULL }
vkPoptOption nullOpt();


typedef struct vkPoptContext_s* vkPoptContext;

#ifdef __cplusplus
extern "C" {
#endif

   /* initialize popt context.
      - argc    no. of arguments
      - argv    argument array
      - options address of popt option table
      - returns initialized popt context */
   vkPoptContext vkPoptGetContext( int argc, const char** argv,
                                   const vkPoptOption* options );
                                   
   /* get next option opt_ret
      returns 0 on success, PERROR_* on error */
   int vkPoptGetNextOpt( vkPoptContext con, char* arg_val,
                         const vkPoptOption** opt/*OUT*/,
                         bool& done_vk_flags/*OUT*/ );
                         
   /* return current option's argument,
      or NULL if no more options are available */
   const char* vkPoptGetArg( vkPoptContext con );
   
   /* return remaining argument array, terminated with NULL */
   const char** vkPoptGetArgs( vkPoptContext con );
   
   /* peek at current option's argument */
   const char* vkPoptPeekArg( vkPoptContext con );
   
   /* return the offending option which caused the most recent error */
   const char* vkPoptBadOption( vkPoptContext con );
   
   /* destroy context. return NULL always */
   vkPoptContext vkPoptFreeContext( vkPoptContext con );
   
   /* print detailed description of options.
      fp == ouput file handle */
   void vkPoptPrintHelp( vkPoptContext con, FILE* fp,
                         const char* tableName );
                         
#ifdef  __cplusplus
}
#endif

/*--------------------------------------------------*/
/* wrapper to free(3), hides const compilation noise,
   permit NULL, return NULL always */
static inline void* _free( const void* p )
{
   if ( p != NULL ) {
      free(( void* )p );
   }
   
   return NULL;
}


struct optionStackEntry {
   int argc;
   const char** argv;
   int next;
   const char* nextArg;
   const char* nextCharArg;
};


struct vkPoptContext_s {
   struct optionStackEntry optionStack[OPTION_DEPTH];
   struct optionStackEntry* os;
   const char** leftovers;
   int numLeftovers;
   int nextLeftover;
   const vkPoptOption* options;
   int restLeftover;
   int flags;
   const char** finalArgv;
   int finalArgvCount;
   int finalArgvAlloced;
};


#endif
