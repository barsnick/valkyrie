/* ---------------------------------------------------------------------- 
 * Definition of class VkObject and subclass ToolObject    vk_objects.cpp
 *
 * Essential functionality is contained within a VkObject / ToolObject.
 * Each ToolObject has an associated ToolView for displaying output.
 * 
 * To add a new valgrind tool:
 * - add a new enum value to enum VkObject::ObjectId{ ... }.
 * - create the subclass in its own files in the src/core/ directory.
 *   see the Example below w.r.t. addOpt(...)
 * - add new stuff to the various VkObject fns where necessary.
 * - in VkConfig::initVkObjects() [vk_config.cpp], 
 *   add the new tool to the list of objects to be created at startup
 * - in [/core/valkyrie_object.cpp], add the new tool where necessary.
 * - create a new options page for the Options dialog, and add this
 *   into OptionsWindow::mkOptionsPage(int) in /options/options_window.cpp
 * That's all, folks.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_objects.h"
#include "vk_config.h"
#include "vk_popt_option.h"    // PERROR* and friends 
#include "vk_messages.h"       // vkInfo() and friends
#include "html_urls.h"

#include "valkyrie_object.h"   // for configEntries() && checkArg()
#include "valgrind_object.h"   // modFlags(), checkArg()
#include "memcheck_object.h"   // only for checkArg()
#include "cachegrind_object.h"
#include "massif_object.h"

#include <qdir.h>
#include <qfileinfo.h>
#include <qtimer.h>

/* Example:
addOpt( 
  LEAK_CHECK,                            int opt_key
  Option::ARG_BOOL,                      Option::ArgType arg_type
  Option::CHECK,                         Option::WidgetType w_type
  "memcheck",                            QString cfg_group     // cfgGroup()
  '\0',                                  QChar   short_flag
  "leak-check",                          QString long_flag     // cfgKey()
  "<no|summary|full>",                   QString flag_desc     // cmd-line
  "no|summary|full",                     QString poss_vals
  "summary",                             QString default_val
  "Search for memory leaks at exit",     QString shelp         // gui
  "search for memory leaks at exit?",    QString lhelp         // cmd-line
  "manual.html#leak-check" );            QString url
*/


/* class VkObject ------------------------------------------------------ */
VkObject::~VkObject() 
{ 
  optList.setAutoDelete( true );
  optList.clear();
  optList.setAutoDelete( false );
}


VkObject::VkObject( ObjectId id, const QString& capt, const QString& txt,
                    const QKeySequence& key, bool is_tool ) 
  : QObject( 0, capt )
{
  objectId  = id;
  caption   = capt;
  accelText = txt;
  is_Tool   = is_tool;
  accel_Key = key;

  usingGui   = false;
}


void VkObject::addOpt( 
     int opt_key,  Option::ArgType arg_type, Option::WidgetType w_type, 
     QString cfg_group, QChar short_flag,         QString long_flag, 
     QString flag_desc, QString poss_vals,        QString default_val, 
     QString shelp,     QString lhelp,            const char* url )
{
  optList.append( new Option( opt_key,   arg_type,   w_type, 
                              cfg_group, short_flag, long_flag, 
                              flag_desc, poss_vals,  default_val, 
                              shelp,     lhelp,      url ) );
}


Option * VkObject::findOption( int optkey )
{
  Option* opt = NULL;
  for ( opt=optList.first(); opt; opt=optList.next() ) {
    if ( opt->key == optkey )
      break;
  }
  vk_assert( opt != NULL );

  return opt;
}


/* writes the argval for this option to vkConfig. */
void VkObject::writeOptionToConfig( Option* opt, QString argval )
{
  opt->modified = true;
  vkConfig->wrEntry( argval, opt->longFlag, opt->cfgGroup() );
}


/* called from VkConfig::mkConfigFile() when we need to create the
   valkyrierc file for the very first time. */
QString VkObject::configEntries()
{
  QString cfgEntry = "\n[" + name() + "]\n";
  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {

    /* skip these entirely */
    if ( opt->key == Valkyrie::HELP_OPT ) continue;

    cfgEntry += opt->longFlag + "=" + opt->defaultValue + "\n";
  }

  return cfgEntry;
}

/* called from VkConfig::modFlags() when a toolview needs to know what
   flags to set || pass to a process. */
QStringList VkObject::modifiedFlags()
{
  QStringList modFlags;
  QString defVal, cfgVal;

  for ( Option* opt = optList.first(); opt; opt = optList.next() ) {

    /* config already has this 'cos we need it to be first in the queue */
    if ( opt->key == Valgrind::TOOL )
      continue;

    defVal = opt->defaultValue;     /* opt holds the default */
    cfgVal = vkConfig->rdEntry( opt->longFlag, name() );

    if ( defVal != cfgVal )
      modFlags << "--" + opt->longFlag + "=" + cfgVal;

  }

  return modFlags;
}


/* determine the total no. of option structs required by counting the
   no. of entries in the optList.  Note: num_options = optList+1
   because we need a NULL option entry to terminate each option array. */
vkPoptOption * VkObject::poptOpts()
{
  //printOptList();
  int num_options  = optList.count() + 1;
  size_t nbytes    = sizeof(vkPoptOption) * num_options;
  vkPoptOption * vkopts = (vkPoptOption*)malloc(nbytes);

  int idx = 0;
  Option *opt;
  QString tmp;
  for ( opt = optList.first(); opt; opt = optList.next() ) {
    vk_assert( opt != NULL );
    vk_assert( vkopts != NULL );

    /* not a popt option */
    if ( opt->argType != Option::NOT_POPT ) {
      vkopts[idx].argType    = opt->argType;
      vkopts[idx].shortFlag  = opt->shortFlag.latin1();
      vkopts[idx].longFlag   = opt->longFlag.latin1();
      vkopts[idx].arg        = 0;
      if ( opt->key == Valkyrie::HELP_OPT ) {
        vkopts[idx].val      = opt->shortFlag.latin1();
      } else {
        vkopts[idx].val      = opt->key;
      }
      if ( opt->defaultValue.isEmpty() ) {
        vkopts[idx].helptxt  = opt->longHelp;
      } else {
        tmp = opt->longHelp + " [" + opt->defaultValue + "]";
        vkopts[idx].helptxt  = vk_strdup( tmp.latin1() );
      }
      vkopts[idx].helpdesc   = opt->flagDescrip.latin1();
      idx++;
    }
  }
  /* we need a null entry to terminate the table */
  vkopts[idx].argType   = 0;
  vkopts[idx].shortFlag = '\0';
  vkopts[idx].longFlag  = NULL;
  vkopts[idx].arg       = 0;
  vkopts[idx].val       = 0;
  vkopts[idx].helptxt   = NULL;
  vkopts[idx].helpdesc  = NULL;

  return vkopts;
}


/* tread carefully: when creating the struct in poptOpts(), the
   helpdesc field was only malloc'd if there was a default value.  
   so we check to see if the original string and the copy are the
   same.  if yes, leave it alone, as it only contains a ptr to the
   original string.  */
void VkObject::freePoptOpts( vkPoptOption * vkopts )
{
  int num_options  = optList.count();
  if ( num_options == PARSED_OK )
    return;

  int xx = 0;
  for ( int yy=0; yy<num_options; yy++ ) {

    /* beware: don't use strcmp on empty strings :( */
    if ( optList.at(yy)->argType != Option::NOT_POPT && 
         !optList.at(yy)->longHelp.isEmpty()  ) {
      const char* tmp = optList.at(yy)->longHelp.latin1();
      if ( !vk_strcmp( vkopts[xx].helptxt, tmp ) ) {
        vk_str_free( vkopts[xx].helptxt );
      }
      xx++;
    }
  }

  vk_free( vkopts );
  vkopts = NULL;
}


/* static fn, called from parseCmdArgs() in parse_cmd_args.cpp */
int VkObject::checkArg(int optid, const char* argval, bool use_gui/*=false*/)
{
  VkObject * vkObj = NULL;
  if ( optid >= Valkyrie::HELP_OPT && optid <= Valkyrie::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "valkyrie" );
  else if ( optid >= Valgrind::FIRST_CMD_OPT && optid <= Valgrind::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "valgrind" );
  else if ( optid >= Memcheck::FIRST_CMD_OPT && optid <= Memcheck::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "memcheck" );
  else if ( optid >= Cachegrind::FIRST_CMD_OPT && optid <= Cachegrind::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "cachegrind" );
  else if ( optid >= Massif::FIRST_CMD_OPT && optid <= Massif::LAST_CMD_OPT )
    vkObj = vkConfig->vkObject( "massif" );
  else
    vk_assert_never_reached();

  vk_assert( vkObj != NULL );
  return vkObj->checkOptArg( optid, argval, use_gui );
}




/* class ToolObject ---------------------------------------------------- */
ToolObject::~ToolObject() 
{ 
  killProc();
}

ToolObject::ToolObject( ObjectId id, const QString& capt, 
                        const QString& txt, const QKeySequence& key )
  : VkObject( id, capt, txt, key, true )
{
  m_view = 0;
  proc = 0;

  is_Running = false;
  fileSaved  = true;
}

bool ToolObject::isRunning() 
{ return is_Running; }


/* kill proc if it is running */
void ToolObject::killProc()
{
  if ( proc != 0 ) {
    if ( proc->isRunning() ) {
      /* if this view is closed, don't leave the process running */
      proc->tryTerminate();
      QTimer::singleShot( 5000, proc, SLOT( kill() ) );
    }
    delete proc;
    proc = 0;
  }
}

void ToolObject::deleteView()
{
  emit message( "" );  /* clear the status bar */
  vk_assert( m_view != 0 );
  // CAB: which is correct: close or delete ?
  m_view->close( true );
  //  delete memcheckView;
  m_view = 0;
}
