/* ---------------------------------------------------------------------
 * vkPoptOption struct + enum values for error strings  vk_popt_option.h
 *
 * This file contains bits and pieces which both vk_popt.* and
 * vk_object.* need to be aware of.
 *
 * parseErrString() is defined in vk_option.cpp
 * ---------------------------------------------------------------------
 * popt is Copyright (c) 1998 Red Hat Software and distributed under
 * an X11-style license, which is in turn compatible the GNU GPL v.2.
 */

#ifndef __VK_POPT_OPTION_H
#define __VK_POPT_OPTION_H

namespace VkOPTION {
   /* NOT_POPT: ignore options in cmdline option parsing
      ARG_*   : arg type for this option (NONE = no args) */
   enum ArgType { NOT_POPT=-1, ARG_NONE=0, ARG_STRING=1,
                  ARG_UINT=2, ARG_PWR2=3, ARG_BOOL=4 };

   /* these are kept in here in order to avoid depending on the gui */
   enum WidgetType { WDG_NONE=-1, WDG_CHECK=0, WDG_RADIO,
                     WDG_LEDIT, WDG_COMBO, WDG_LISTBOX, WDG_SPINBOX };
}


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

#endif // #ifndef __VK_POPT_OPTION_H
