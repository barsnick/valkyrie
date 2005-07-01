/* ---------------------------------------------------------------------- 
 * Options / flags                                         vk_options.cpp
 * A 'VkObject' is merely a way of storing the various flags / options
 * relevant to valkyrie, valgrind and valgrind's tools.
 * ----------------------------------------------------------------------
 * This is the only place where the various options/flags for
 * valkyrie, valgrind and valgrind tools are stored.
 * 
 * To add a new valgrind tool:
 *
 * - add a new enum value to enum VkObject::ObjectId{ ... }.
 *
 * - create the subclass in the vk_objects.h and vk_objects.cpp files,
 *   adding new stuff to the various VkObject fns where necessary.
 *
 * - in vk_config.cpp, in bool VkConfig::initVkObjects(),
 *   add the new tool to the list of objects to be created at startup
 *
 * - Create a new options page for the Options dialog, and add this
 *   into OptionsWindow::mkOptionsPage(int) in /options/options_window.cpp
 *
 * That's all, folks.
 */


#ifndef __VALKYRIE_OBJECTS_H
#define __VALKYRIE_OBJECTS_H

#include <qkeysequence.h>
#include <qobject.h>
#include <qptrlist.h>

#include "vk_option.h"          /* class Option */
#include "vk_popt_option.h"



/* IMPORTANT: when adding new objects, keep in numerical sequence. If
   you change the ordering of the enum values, you must change the
   insertion order in VkConfig::initVkObjects() as well. */
class VkObject : public QObject 
{
public:
  enum ObjectId { 
    INVALID=-1, VALKYRIE=0, VALGRIND=1, MEMCHECK=2, CACHEGRIND=3, MASSIF=4 
  };

  VkObject( ObjectId id, const QString& capt, const QString& txt,
            const QKeySequence& key, bool is_tool=true );
  ~VkObject();

  //RM: int id()          { return objectId;        }
  VkObject::ObjectId id() { return objectId;        }
  bool isTool()           { return is_Tool;         }
  QString name()          { return caption.lower(); }
  QString title()         { return caption;         }
  QString accelTitle()    { return accelText;       }
  QKeySequence accelKey() { return accel_Key;       }

  /* called by parseCmdArgs() in parse_cmd_args.cpp, and from the gui
     options pages; calls checkOptArg() */
  static int checkArg(int optid, const char* argval, bool gui=false);
  virtual int checkOptArg(int optid, const char* argval, bool gui=false) = 0; 

  /* returns a list of non-default flags to pass to valgrind */
  virtual QStringList modifiedFlags();
  /* returns a list of options to be written to the config file */
  virtual QString configEntries();
  /* command-line + optionsWindow help and parsing stuff */
  vkPoptOption * poptOpts();
  void freePoptOpts( vkPoptOption * );
  /* also called by OptionsPage::optionWidget() */
  Option * findOption( int optid );

protected:
  /* writes the value of the option to vkConfig */
  void writeOptionToConfig( Option* opt, QString argval );

  void addOpt( int key, Option::ArgType arg_type, Option::WidgetType w_type,
               QString cfg_group,  QChar   short_flag, QString long_flag, 
               QString flag_desc,  QString poss_vals,  QString default_val,
               QString shelp,      QString lhelp,      const char* url );

protected:
  bool is_Tool;              /* not valkyrie or valgrind-core */
  ObjectId objectId;         /* eg. MEMCHECK */
  QString caption;           /* eg. Memcheck */

  QString accelText;         /* eg. &Memcheck */
  QKeySequence accel_Key;    /* accelerator key */

  QPtrList<Option> optList;  /* list of options for this object */
};



/* class Valkyrie ------------------------------------------------------
   Note: the very first option must be > 0, otherwise it conflicts
   with arg_flags in popt. */
class Valkyrie : public VkObject
{
public:
  Valkyrie();
  ~Valkyrie() { }

  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  /* modeNotSet:      no cmd-line options given
     modeParseLog:    read file from disk
     modeMergeLogs:   read file-list from disk, and merge the contents
     modeParseOutput: read output direct from valgrind 
  */
	enum RunMode { modeNotSet=0, modeParseLog, modeMergeLogs, modeParseOutput };
  RunMode runMode;

  enum vkOpts {
    FIRST_CMD_OPT = 11,
    HELP_OPT      = 1,
    TOOLTIP       = 2,
    PALETTE       = 3,
    ICONTXT       = 4,
    FONT_SYSTEM   = 5,
    FONT_USER     = 6,
    SRC_EDITOR    = 7,
    SRC_LINES     = 8,
    /* path to valgrind executable (/usr/bin/valgrind) */
    VG_EXEC       = 9,
    /* path to supp. files dir [def = /usr/lib/valgrind/] */
    VG_SUPPS_DIR  = 10,
    BINARY        = FIRST_CMD_OPT,
    BIN_FLAGS     = 12,
    VIEW_LOG      = 13,
    MERGE_LOGS    = 14,
    USE_GUI       = 15,
    LAST_CMD_OPT  = USE_GUI
  };

};


/* class Valgrind ------------------------------------------------------ */
class Valgrind : public VkObject
{
public:
  Valgrind();
  ~Valgrind() { }

  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum vgOpts {
    FIRST_CMD_OPT = Valkyrie::LAST_CMD_OPT + 1,
    TOOL          = FIRST_CMD_OPT,
    /* common options relevant to all tools */
    VERBOSITY,
    XML_OUTPUT,     /* this may apply to more tools later */
    TRACE_CH,
    TRACK_FDS,
    TIME_STAMP,
    /* uncommon options relevant to all tools */
    RUN_LIBC,
    WEIRD,
    PTR_CHECK,
    ELAN_HACKS,
    EM_WARNS,
    /* options relevant to error-reporting tools */
    LOG_FD,
    LOG_PID,
    LOG_FILE,
    LOG_SOCKET,
    DEMANGLE,
    NUM_CALLERS,
    ERROR_LIMIT,
    SHOW_BELOW,
    SUPPS_SEL,    /* the currently selected suppression(s) */
    SUPPS_ALL,    /* list of all supp. files ever found, inc. paths */
		SUPPS_DEF,    /* as above, but never gets changed -> defaults */
    GEN_SUPP,
    DB_ATTACH,
    DB_COMMAND,
    INPUT_FD,
    MAX_SFRAME,
    LAST_CMD_OPT = MAX_SFRAME
  };

};


/* class Memcheck ------------------------------------------------------ */
class Memcheck : public VkObject
{
public:
  Memcheck();
  ~Memcheck() { } 

  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum mcOpts { 
    FIRST_CMD_OPT = Valgrind::LAST_CMD_OPT + 1,
    PARTIAL       = FIRST_CMD_OPT,
    FREELIST,
    LEAK_CHECK,
    LEAK_RES,
    SHOW_REACH,
    GCC_296,
    ALIGNMENT,
    STRLEN,
    LAST_CMD_OPT  = STRLEN
  };

};


/* class Cachegrind ---------------------------------------------------- */
class Cachegrind : public VkObject
{
public:
  Cachegrind();
  ~Cachegrind() { }

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum cgOpts {
    FIRST_CMD_OPT = Memcheck::LAST_CMD_OPT + 1,
    I1_CACHE      = FIRST_CMD_OPT, 
    D1_CACHE,
    L2_CACHE,
    PID_FILE,
    SHOW,
    SORT,
    THRESH,
    AUTO,
    CONTEXT,
    INCLUDE,
    LAST_CMD_OPT = INCLUDE 
  };

};


/* class Massif -------------------------------------------------------- */
class Massif : public VkObject
{
public:
  Massif();
  ~Massif() { }

  int checkOptArg( int optid, const char* argval, bool use_gui=false );

  enum msOpts {
    FIRST_CMD_OPT = Cachegrind::LAST_CMD_OPT + 1,
    HEAP          = FIRST_CMD_OPT, 
    HEAP_ADMIN,
    STACKS,
    DEPTH,
    ALLOC_FN,
    FORMAT,
    ALIGNMENT,
    LAST_CMD_OPT  = ALIGNMENT
  };

};

#endif


