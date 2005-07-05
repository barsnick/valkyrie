/* --------------------------------------------------------------------- 
 * Implementation of Popt functions                            vk_popt.c
 * This is a seriously hacked version of the popt libraries.
 * No credit to me, all thanks and many apologies to the Red Hat team
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_popt.h"


const char * vkPoptPeekArg( vkPoptContext con )
{
  const char * ret = NULL;
  if ( con && con->leftovers != NULL && 
       con->nextLeftover < con->numLeftovers )
    ret = con->leftovers[con->nextLeftover];
  return ret;
}


const char * vkPoptGetArg( vkPoptContext con )
{
  const char * ret = NULL;
  if ( con && con->leftovers != NULL && 
       con->nextLeftover < con->numLeftovers ) {
    ret = con->leftovers[con->nextLeftover++];
  }
  return ret;
}


const char ** vkPoptGetArgs( vkPoptContext con )
{
  if ( con == NULL || con->leftovers == NULL || 
       con->numLeftovers == con->nextLeftover )
    return NULL;

  /* some apps like [like RPM ;-) ] need this NULL terminated */
  con->leftovers[con->numLeftovers] = NULL;
  return (con->leftovers + con->nextLeftover);
}


vkPoptContext vkPoptGetContext( int argc, const char ** argv,
                                const vkPoptOption * options )
{
  vkPoptContext con = malloc( sizeof(*con) );

  if ( con == NULL ) {
    return NULL;
  }

  memset( con, 0, sizeof(*con) );

  con->os = con->optionStack;
  con->os->argc  = argc;
  con->os->argv  = argv;
  con->os->next  = 1;      /* skip argv[0] */
  con->leftovers = calloc( (argc + 1), sizeof(*con->leftovers) );
  con->options   = options;
  con->flags     = PCONTEXT_POSIXMEHARDER;
  con->finalArgvAlloced = argc * 2;
  con->finalArgv = calloc( con->finalArgvAlloced, 
                           sizeof(*con->finalArgv) );

  return con;
}


static void vkCleanOSE( struct optionStackEntry *os )
{
  os->nextArg = _free( os->nextArg );
  os->argv    = _free( os->argv );
}


static const vkPoptOption * vkFindOption( const vkPoptOption * opt, 
                                          const char * longFlag,  
                                          char shortFlag, 
                                          int singleDash )
{
  /* this happens when a single - is given */
  if ( singleDash && !shortFlag && (longFlag && *longFlag == '\0') )
    shortFlag = '-';

  for (; opt->longFlag || opt->shortFlag || opt->arg; opt++) {

    if ( (opt->argType & ARG_MASK) == ARG_INC_TABLE ) {
      const vkPoptOption * opt2;
      
      /* recurse on included sub-tables. */
      if ( opt->arg == NULL ) 
        continue;
      opt2 = vkFindOption( opt->arg, longFlag, 
                           shortFlag, singleDash );
      if ( opt2 == NULL )
        continue;
      /* sub-table data will be inherited if no data yet. */
      return opt2;
    } else if ( longFlag && opt->longFlag && !singleDash &&
                !strcmp( longFlag, opt->longFlag ) ) {
      break;
    } else if (shortFlag && shortFlag == opt->shortFlag) {
      break;
    }
  }

  if ( !opt->longFlag && !opt->shortFlag )
    return NULL;

  return opt;
}


static const char * vkExpandNextArg( const char * s )
{
  char *t, *te;
  size_t tn = strlen(s) + 1;
  char c;

  te = t = malloc(tn);;
  if ( t == NULL ) 
    return NULL;
  while ((c = *s++) != '\0') {
    *te++ = c;
  }

  *te = '\0';
  /* memory leak, hard to plug */
  t = realloc(t, strlen(t) + 1);
  return t;
}


/* returns 'val' element, -1 on last item, PERROR_* on error */
int vkPoptGetNextOpt( vkPoptContext con, char *arg_val )
{
  const vkPoptOption * opt = NULL;
  int done = 0;

  if ( con == NULL ) {
    return -1;
  }

  while ( !done ) {
    const char * origOptString = NULL;
    const char * longArg = NULL;
    int shorty = 0;

    while ( !con->os->nextCharArg && 
            con->os->next == con->os->argc && 
            con->os > con->optionStack ) {
      vkCleanOSE( con->os-- );
    }

    if ( !con->os->nextCharArg && 
         con->os->next == con->os->argc ) {
      return -1;
    }

    /* process next long option */
    if ( !con->os->nextCharArg ) {
      char * localOptString, * optString;
      int thisopt;
      thisopt = con->os->next;
      if ( con->os->argv != NULL )
        origOptString = con->os->argv[con->os->next++];

      if ( origOptString == NULL ) {
        return PERROR_BADOPT;
      }
      /* FIX: catch cases where eg. -- flag=xx */
      if ( strcmp(origOptString, "--") == 0 ) {
        return PERROR_BADQUOTE;
      }

      if ( con->restLeftover || *origOptString != '-' ) {
        if ( con->flags & PCONTEXT_POSIXMEHARDER )
          con->restLeftover = 1;
        if ( con->leftovers != NULL )
          con->leftovers[con->numLeftovers++] = origOptString;
        continue;
      }

      /* make a copy we can hack at */
      localOptString = optString =
        strcpy(alloca(strlen(origOptString) + 1), origOptString);

      if ( optString[0] == '\0' )
        return PERROR_BADOPT;

      if ( optString[1] == '-' && !optString[2] ) {
        con->restLeftover = 1;
        continue;
      } else {
        char *oe;
        int singleDash;
        optString++;
        if ( *optString == '-' )
          singleDash = 0, optString++;
        else
          singleDash = 1;

        /* Check for "--long=arg" option. */
        for ( oe = optString; *oe && *oe != '='; oe++ )
          { };
        if ( *oe == '=' ) {
          /* FIX: don't use '=' for shortopts */
          if ( singleDash )
            return PERROR_NODASH;
          *oe++ = '\0';
          /* longArg is mapped back to persistent storage. */
          longArg = origOptString + (oe - localOptString);
          /* FIX: catch cases where --longarg=<no-arg> */
          if ( strlen(longArg) == 0 ) {
            //printf("1: returning PERROR_NOARG\n");
            return PERROR_NOARG;
          }
        } 
#if 0
        else if ( singleDash == 0 ) {
          /* FIX: catch cases where we didn't find an '=', 
             and this is a --longarg option */
          //printf("2: returning PERROR_NOARG\n");
          return PERROR_NOARG;
        } 
#endif

        opt = vkFindOption( con->options, optString, 
                            '\0', singleDash );
        if ( !opt && !singleDash ) {
          //printf("returning PERROR_BADOPT\n");
          return PERROR_BADOPT;
        }
      }

      if ( !opt ) {
        con->os->nextCharArg = origOptString + 1;
      } else {
        shorty = 0;
      }
    }

    /* process next short option */
    if ( con->os->nextCharArg ) {
      origOptString = con->os->nextCharArg;
      con->os->nextCharArg = NULL;

      opt = vkFindOption( con->options, NULL, *origOptString, 0 );
      if ( !opt ) {
        return PERROR_BADOPT;
      }
      shorty = 1;
      
      origOptString++;
      if ( *origOptString != '\0' )
        con->os->nextCharArg = origOptString;
    }

    if ( opt == NULL ) 
      return PERROR_BADOPT;

    if ( (opt->argType & ARG_MASK) != ARG_NONE ) {
      con->os->nextArg = _free(con->os->nextArg);
      if ( longArg ) {
        longArg = vkExpandNextArg( longArg );
        con->os->nextArg = longArg;
      } else if ( con->os->nextCharArg ) {
        longArg = vkExpandNextArg( con->os->nextCharArg);
        con->os->nextArg = longArg;
        con->os->nextCharArg = NULL;
      } else {
        while ( con->os->next == con->os->argc &&
                con->os > con->optionStack ) {
          vkCleanOSE( con->os-- );
        }
        if ( con->os->next == con->os->argc ) {
          /* FIX: con->os->argv not defined */
          return PERROR_NOARG;
          con->os->nextArg = NULL;
        } else {
          if ( con->os->argv != NULL ) {
            /* watch out: subtle side-effects live here. */
            longArg = con->os->argv[con->os->next++];
            longArg = vkExpandNextArg( longArg );
            con->os->nextArg = longArg;
          }
        }
      }
      longArg = NULL;

      /* store the argument value for checking */
       if ( con->os->nextArg ) {
        sprintf( arg_val, "%s", con->os->nextArg );
      }
    }     /* end if ! ARG_NONE */

    if ( opt->val ) {
      done = 1;
    }

    if ( (con->finalArgvCount + 2) >= (con->finalArgvAlloced) ) {
      con->finalArgvAlloced += 10;
      con->finalArgv = realloc(con->finalArgv,
                     sizeof(*con->finalArgv) * con->finalArgvAlloced);
    }

    if ( con->finalArgv != NULL ) {
      char *s = malloc((opt->longFlag ? strlen(opt->longFlag) : 0) + 3);
      if ( s != NULL ) {
        if ( opt->longFlag ) {
          sprintf( s, "--%s", opt->longFlag );
        } else {
          sprintf(s, "-%c", opt->shortFlag);
        }
        con->finalArgv[con->finalArgvCount++] = s;
      } else
        con->finalArgv[con->finalArgvCount++] = NULL;
    }

    if (opt->arg && (opt->argType & ARG_MASK) == ARG_NONE)
      ;
    else if ((opt->argType & ARG_MASK) != ARG_NONE) {
      if (con->finalArgv != NULL && con->os->nextArg)
        con->finalArgv[con->finalArgvCount++] =
          xstrdup(con->os->nextArg);
    }
  }  /* end while ( !done ) */

  return ( opt ? opt->val : -1 );
}


vkPoptContext vkPoptFreeContext( vkPoptContext con )
{
  if ( con == NULL ) 
    return con;

  /* poptResetContext( con ); */
  int i;
  while ( con->os > con->optionStack ) {
    vkCleanOSE(con->os--);
  }
  con->os->nextCharArg = NULL;
  con->os->nextArg  = NULL;
  con->os->next     = 1;      /* skip argv[0] */
  con->numLeftovers = 0;
  con->nextLeftover = 0;
  con->restLeftover = 0;

  if ( con->finalArgv != NULL )
    for (i = 0; i < con->finalArgvCount; i++)
      con->finalArgv[i] = _free(con->finalArgv[i]);
  con->finalArgvCount = 0;

  con->leftovers = _free( con->leftovers );
  con->finalArgv = _free( con->finalArgv );

  con = _free( con );
  return con;
}


const char * vkPoptBadOption( vkPoptContext con )
{
  struct optionStackEntry * os = NULL;

  if ( con != NULL )
    os = con->optionStack;

  return ( os && os->argv ? os->argv[os->next - 1] : NULL );
}



/*------ help printing stuff --------------------------------*/
/* set in poptPrintHelp */
static int leftColWidth = -1;

static const char * vkGetHelpDesc( const vkPoptOption * opt )
{
  if ( !(opt->argType & ARG_MASK) )
    return NULL;

  if ( opt->helpdesc )
    return opt->helpdesc;

  return NULL;
}


/* called from singleTableHelp() when only one table is being output */
static void vkSingleOptionHelp( FILE * fp, 
                                const vkPoptOption * opt )
{
  int indentLength  = leftColWidth + 2;
  int lineLength    = 80 - indentLength;
  const char * help = opt->helptxt;
  const char * helpdesc = vkGetHelpDesc( opt );
  int helpLength;
  char * defs = NULL;
  char * left;
  int nb = leftColWidth + 1;

  /* make sure there's more than enough room in target buffer. */
  if ( opt->longFlag )  
    nb += strlen( opt->longFlag );
  if ( helpdesc )  
    nb += strlen( helpdesc );

  left = malloc( nb );
  if ( left == NULL ) 
    return;
  left[0] = '\0';
  left[leftColWidth] = '\0';

  if ( opt->longFlag && opt->shortFlag ) {
    sprintf( left, "-%c, --%s", opt->shortFlag, opt->longFlag);
  } else if (opt->shortFlag != '\0') {
    sprintf( left, "-%c", opt->shortFlag);
  } else if (opt->longFlag) {
    sprintf( left, "--%s", opt->longFlag );
  }

  if ( !*left ) {
    goto out;
  }

  if ( helpdesc ) {
    char * le = left + strlen( left );
    *le++ = '=';
    strcpy( le, helpdesc );
    le += strlen(le);
    *le = '\0';
  }

  if ( help ) {
    fprintf( fp,"  %-*s", leftColWidth, left );
  } else {
    fprintf( fp,"  %s\n", left ); 
    goto out;
  }

  left = _free(left);
  if ( defs ) {
    help = defs;
    defs = NULL;
  }
  
  helpLength = strlen( help );
  while ( helpLength > lineLength ) {
    const char * ch;
    char format[10];

    ch = help + lineLength - 1;
    while ( ch > help && !isspace(*ch) ) 
      ch--;
    if ( ch == help ) 
      break;    /* give up */
    while ( ch > (help + 1) && isspace(*ch) )
      ch--;
    ch++;
    
    sprintf( format, "%%.%ds\n%%%ds", 
             (int) (ch - help), indentLength );
    fprintf( fp, format, help, " " );
    help = ch;
    while ( isspace(*help) && *help ) 
      help++;
    helpLength = strlen( help );
  }

  if ( helpLength ) 
    fprintf( fp, "%s\n", help );

 out:
  defs = _free(defs);
  left = _free(left);
}


static int vkMaxArgWidth( const vkPoptOption * opt )
{
  int max = 0;
  int len = 0;
  const char * s;

  if ( opt != NULL )
    while ( opt->longFlag || opt->shortFlag || opt->arg ) {
      if ((opt->argType & ARG_MASK) == ARG_INC_TABLE) {
        if ( opt->arg )
          len = vkMaxArgWidth( opt->arg );
        if ( len > max ) 
          max = len;
      } else {
        len = sizeof("  ") - 1;
        if ( opt->shortFlag != '\0' )
          len += sizeof("-X")-1;
        if ( opt->shortFlag != '\0' && opt->longFlag ) 
          len += sizeof(", ")-1;
        if (opt->longFlag) {
          len += sizeof("--") - 1;
          len += strlen( opt->longFlag );
        }

        s = vkGetHelpDesc( opt );
        if ( s )
          len += sizeof("=") - 1 + strlen( s );
        if ( len > max ) 
          max = len;
      }
      
      opt++;
    }

  return max;
}


/* this version prints nested tables first. swap 'em over if this
   isn't the behaviour you want. */
static void vkSingleTableHelp( FILE * fp, const vkPoptOption * table )
{
  const vkPoptOption * opt;

  if ( table != NULL ) {
    for (opt = table; ( opt->longFlag || 
                        opt->shortFlag || opt->arg); opt++) {
      if ((opt->argType & ARG_MASK) != ARG_INC_TABLE) {
        continue;
      }
      
      if ( opt->helptxt )
        fprintf( fp, "\n%s\n", opt->helptxt );

      vkSingleTableHelp( fp, opt->arg );
    }
  }

  if ( table != NULL ) {
    for ( opt = table; ( opt->longFlag  || 
                         opt->shortFlag || opt->arg); opt++) {
      if ( (opt->longFlag || opt->shortFlag ) ) {
        vkSingleOptionHelp( fp, opt );
      }
    }
  }

}


/* if tableName == NULL, recurses over all tables;
   else just prints contents of the specified table. */
void vkPoptPrintHelp( vkPoptContext con, FILE * fp,
                      const char * tableName )
{
  const char * fn;

  fprintf( fp, "\nUsage:" );
  fn = con->optionStack->argv[0];
  if ( fn != NULL ) {
    if ( strchr(fn, '/') ) 
      fn = strrchr( fn, '/' ) + 1;
    fprintf( fp, " %s", fn );
  }

  fprintf( fp, " %s", 
           "[valkyrie-opts] [valgrind-opts] [prog-and-args]\n" );

  leftColWidth = vkMaxArgWidth( con->options );

  if ( tableName == NULL ) {
    vkSingleTableHelp( fp, con->options );
  } else {
    /* trawl through con->options till we find the right table */
    const vkPoptOption * opt;
    for ( opt = con->options; opt != NULL; opt++ ) {
      if ( strcmp( opt->helptxt, tableName ) == 0 ) {
        break;
      }
    }
    if ( opt != NULL ) {
      if ( opt->helptxt )
        fprintf( fp, "\n%s\n", opt->helptxt );
      vkSingleTableHelp( fp, opt->arg );
    }
  }

}
