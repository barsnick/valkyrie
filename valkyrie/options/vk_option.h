/* --------------------------------------------------------------------- 
 * Definition of class Option                                vk_option.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_OPTION_H
#define __VK_OPTION_H

#include <qstring.h>
#include <qstringlist.h>

namespace VkOPTION {
   /* NOT_POPT: ignore this option in cmdline option parsing
      ARG_*   : arg type for this option (ARG_NONE = no args) */
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



/* helper functions ---------------------------------------------------- */
QString fileCheck( int* err_val, const char* fpath, 
                   bool rd_perms, bool wr_perms );
QString binaryCheck( int* err_val, const char* exe_name );
QString dirCheck( int* err_val, const char* fpath,
                  bool rd_perms, bool wr_perms );

const char* parseErrString( const int error );


/* class Option -------------------------------------------------------- */
class Option 
{
public:
   Option( int opt_key, VkOPTION::ArgType arg_type, VkOPTION::WidgetType w_type, 
           QString cfg_group, QChar short_flag,  QString long_flag, 
           QString flag_desc, QString poss_vals, QString default_val, 
           QString shelp,     QString lhelp,     const char* url );

   QString cfgKey()    { return m_longFlag; }
   QString cfgGroup()  { return m_configGroup; }
   QString defValue()  { return m_defaultValue; }
   QString url() const { return m_urlHelp; }

   bool isValidArg( int* err_val, const char* argval );
   static bool isPowerOfTwo( int* err_val, const char* argval );

   void setPossibleValues( QStringList vals );
   void print();                /* for debugging */

public:
   VkOPTION::ArgType    m_argType;  /* option type, eg. ARG_UINT   */
   VkOPTION::WidgetType m_widgType; /* eg. VkOPTION::WDG_LEDIT     */
   int         m_key;               /* eg. LEAK_CHK                */
   QString     m_configGroup;       /* eg. [valkyrie]              */
   QChar       m_shortFlag;         /* eg. --l                     */
   QString     m_longFlag;          /* eg. --leak-resolution       */
   QString     m_flagDescrip;       /* eg. <file>                  */
   QString     m_shortHelp;         /* txt for OptionsWindow       */
   QString     m_longHelp;          /* txt for help --> stdout     */
   QString     m_urlHelp;           /* context help url            */
   QString     m_defaultValue;      /* eg. [low]                   */
   QStringList m_possValues;        /* eg. low | med | high        */
};



#endif
