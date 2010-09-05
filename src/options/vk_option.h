/****************************************************************************
** VkOption definition
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __VK_OPTION_H
#define __VK_OPTION_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariant>


// ============================================================
class VkOption;


// ============================================================
typedef QHash<int, VkOption*> OptionHash;
typedef QHash<int, VkOption*>::iterator Iter_OptionHash;
typedef QHash<int, VkOption*>::const_iterator ConstIter_OptionHash;

namespace VkOPT
{
/*!
   Argument type: tells us how to parse and test the argument value.
   NOT_POPT: ignore this option in cmdline option parsing
   ARG_*   : arg type for this option (ARG_NONE = no args)
*/
enum ArgType {
   NOT_POPT = -1,
   ARG_NONE = 0,
   ARG_STRING,
   ARG_UINT,
   ARG_PWR2,
   ARG_BOOL
};

/*!
   Widget type to use for a particular option
   note: the argument value setup must match the widget type used
*/
enum WidgType {
   WDG_NONE = -1,
   WDG_CHECK = 0,
   WDG_RADIO,
   WDG_LEDIT,
   WDG_COMBO,
   WDG_LISTBOX,
   WDG_SPINBOX
};
}


// ============================================================
// Error return values
#define PARSED_OK             0  // value passed the parsing checks
#define PERROR_DEFAULT      - 1  // don't print parseError()
#define PERROR_NODASH       - 2  // don't use '=' for short opts
#define PERROR_NOARG        -10  // missing argument
#define PERROR_BADOPT       -11  // unknown option
#define PERROR_BADARG       -12  // bad argument given
#define PERROR_BADQUOTE     -15  // error in paramter quoting
#define PERROR_BADNUMBER    -17  // invalid numeric value
#define PERROR_OUTOFRANGE   -18  // outside min|max values
#define PERROR_OVERFLOW     -19  // number too large or too small
#define PERROR_BADOPERATION -20  // mutually exclusive logical operations requested
#define PERROR_NULLARG      -21  // opt->arg should not be NULL
#define PERROR_MALLOC       -22  // memory allocation failed
#define PERROR_BADDIR       -23  // dir does not exist
#define PERROR_BADFILE      -24  // file does not exist
#define PERROR_BADFILENAME  -25  // filename un-good
#define PERROR_BADNUMFILES  -26  // invalid no. of files
#define PERROR_BADFILERD    -27  // user hasn't got rd permission
#define PERROR_BADFILEWR    -28  // user hasn't got wr permission
#define PERROR_BADEXEC      -29  // user hasn't got exec permission
#define PERROR_DB_CONFLICT  -30  // db-attach -v- trace-children
#define PERROR_DB_OUTPUT    -31  // using db-attach, but not sending output to stderr
#define PERROR_POWER_OF_TWO -32  // number not a power of two
#define PERROR_BADVERSION   -33  // bad program version
#define PERROR_DUPLICATE    -34  // duplicate entries

const char* parseErrString( const int error );


// ============================================================
class VkOptionHash
{
public:
   VkOptionHash();
   ~VkOptionHash();
   
   VkOption* getOption( int optid );
   
   OptionHash& getOptionHash() {
      return optionHash;
   }
   
   
   void addOpt( int optid,
                QString cfg_group,
                QString long_flag,
                QChar   short_flag,
                QString flag_desc,
                QString poss_vals,
                QVariant default_val,
                QString shelp,
                QString lhelp,
                QString url,
                VkOPT::ArgType arg_type,
                VkOPT::WidgType w_type
              );
              
protected:
   OptionHash optionHash;
};



// ============================================================
class VkOption : public QObject
{
   Q_OBJECT
public:
   VkOption( int optid,
             QString cfg_group,     // use for config->'group'
             QString long_flag,     // use for config->'key'
             QChar   short_flag,
             QString flag_desc,
             QString poss_vals,
             QVariant default_val,
             QString shelp,
             QString lhelp,
             QString url,
             VkOPT::ArgType arg_type,
             VkOPT::WidgType w_type
           );

   bool isValidArg( int* err_val, QString argval );
   static bool isPowerOfTwo( QString argval, int* err_val=0 );

signals:
   void valueChanged();
   
   
public:
   int             optid;         // for easy identification
   QString         configGrp;     // eg. [valkyrie]
   QString         longFlag;      // eg. --leak-resolution
   QChar           shortFlag;     // eg. --l
   QString         flagDescr;     // eg. <file>
   QStringList     possValues;    // eg. low | med | high
   QVariant        dfltValue;     // eg. [low]
                                  // - QVariant to support QSettings::setValue()
                                  //   See this->updateConfig()
   QString         shortHelp;     // txt for OptionsWindow
   QString         longHelp;      // txt for help --> stdout
   QString         urlAddress;    // context help url
   VkOPT::ArgType  argType;       // eg. ARG_UINT
   VkOPT::WidgType widgType;      // eg. VkOPTION::WDG_LEDIT
   
   QString configKey();
   
   void updateConfig( QVariant argVal );

private:
   void print();
};

#endif  // __VK_OPTION_H
