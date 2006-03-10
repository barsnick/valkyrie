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
#include "vk_popt_option.h"

/* helper functions ---------------------------------------------------- */
QString fileCheck( int* err_val, const char* fpath, 
                   bool rd_perms, bool wr_perms );
QString binaryCheck( int* err_val, const char* exe_name );
QString dirCheck( int* err_val, const char* fpath,
                  bool rd_perms, bool wr_perms );


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
   bool        m_modified;
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
