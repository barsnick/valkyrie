/* ---------------------------------------------------------------------
 * vkPoptOption struct + enum values for error strings  vk_popt_option.h
 *
 * This file contains bits and pieces which both vk_popt.* and
 * vk_object.* need to be aware of.
 *
 * parseErrString() is defined in vk_option.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_POPT_OPTION_H
#define __VK_POPT_OPTION_H


typedef struct {
  int  argType;            /* option type: ARG_***       */
  char shortFlag;          /* '\0' || 'h'                */
  const char* longFlag;    /* NULL || --help             */
  void* arg;               /* 0 for Options, 
                              'tablename' for tables eg. 'vkOptions' */
  int val;                 /* 'h' || optid eg. VIEW-LOG  */
  const char* helptxt;     /* help text                  */
  const char* helpdesc;    /* eg. <file>                 */
} vkPoptOption;


/* Error return values */
#define PARSED_OK             0  /* value passed the parsing checks */
#define PERROR_DEFAULT      - 1  /* don't print parseError() */
#define PERROR_NODASH       - 2  /* don't use '=' for short opts */
#define PERROR_NOARG        -10  /* missing argument */
#define PERROR_BADOPT       -11  /* unknown option */
#define PERROR_BADARG       -12  /* bad argument given */
#define PERROR_BADQUOTE     -15  /* error in paramter quoting */
#define PERROR_BADNUMBER    -17  /* invalid numeric value */
#define PERROR_OUTOFRANGE   -18  /* outside min|max values */
#define PERROR_OVERFLOW     -19  /* number too large or too small */
#define PERROR_BADOPERATION -20  /* mutually exclusive logical
                                    operations requested */
#define PERROR_NULLARG      -21  /* opt->arg should not be NULL */
#define PERROR_MALLOC       -22  /* memory allocation failed */
#define PERROR_BADDIR       -23  /* dir does not exist */
#define PERROR_BADFILE      -24  /* file does not exist */
#define PERROR_BADNUMFILES  -25  /* invalid no. of files */
#define PERROR_BADFILERD    -26  /* user hasn't got rd permission */
#define PERROR_BADFILEWR    -27  /* user hasn't got wr permission */
#define PERROR_BADEXEC      -28  /* user hasn't got exec permission */
#define PERROR_DB_CONFLICT  -29  /* db-attach -v- trace-children */
#define PERROR_DB_OUTPUT    -30  /* using db-attach, but not sending
                                    output to stderr */
#define PERROR_POWER_OF_TWO -31  /* number not a power of two */



const char* parseErrString( const int error );

#endif
