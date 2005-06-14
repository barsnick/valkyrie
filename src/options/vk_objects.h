/* ---------------------------------------------------------------------- 
 * Options / flags                                         vk_options.cpp
 * A 'VkObject' is merely a way of storing the various flags / options
 * relevant to valkyrie, valgrind and valgrind's tools.
 * ----------------------------------------------------------------------
 * This is the only place where the various options/flags for
 * valkyrie, valgrind and valgrind tools are stored.
 *
 * When adding / changing / removing flags, it is important to ensure
 * that the enum values in the various subclasses DO NOT OVERLAP.
 * 
 * To add a new valgrind tool:
 *
 * - add a new enum value to enum VkObject::ObjectId{ ... }.
 *
 * - create the subclass in this vkobjects.h and the vkobjects.cpp file,
 *   adding new stuff to the various VkObject fns where necessary.
 *
 * - in vk_config.cpp, in bool VkConfig::initVkObjects(),
 *   add the new tool to the list of objects to be created at startup
 *
 * - Create a new options page for the Options dialog, and add this
 *   into VkObject::optionsPage() in vk_objects.cpp
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
//RM: #include "options_page.h"       /* */



/* IMPORTANT: when adding new objects, keep in numerical sequence. If
   you change the ordering of the enum values, you must change the
   insertion order in VkConfig::initVkObjects() as well. */
class VkObject : public QObject 
{ 
public: 
  enum ObjectId { 
    VALKYRIE=0, VALGRIND=1, MEMCHECK=2, CACHEGRIND=3, MASSIF=4 
  };

  VkObject( ObjectId id, const QString& capt, const QString& txt,
            const QKeySequence& key, bool is_tool=true );
  ~VkObject();

  int  id()               { return objectId;        }
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

  QString configEntries();
  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool gui=false );

  /* whether we are parsing a file, or output from valgrind */
  enum RunMode{ NOT_SET=0, PARSE_LOG, PARSE_OUTPUT };
  RunMode runMode;

  enum vkOpts {
    FIRST_CMD_OPT = 14,
    HELP_OPT      = 1,
    TOOLTIP       = 2,
    MENUBAR       = 3,
    PALETTE       = 4,
    ICONTXT       = 5,
    FONT_SYSTEM   = 6,
    FONT_USER     = 7,
    SRC_EDITOR    = 8,
    SRC_LINES     = 9,
    VG_EXEC       = 10,          // /usr/bin/valgrind
    VG_DOCS_DIR   = 11,          // /usr/share/doc/packages/valgrind/
    /* list of user suppressions _and_ default suppression, inc. path */
    ALL_SUPPS     = 12,
    /* just the currently selected suppressions */
    SEL_SUPPS     = 13,
    BINARY        = FIRST_CMD_OPT,
    BIN_FLAGS     = 15,
    VIEW_LOG      = 16,
		USE_GUI       = 17,
    LAST_CMD_OPT  = USE_GUI
  };

};


/* class Valgrind ------------------------------------------------------ */
class Valgrind : public VkObject
{
public:
  Valgrind();
  ~Valgrind() { }

  int checkOptArg( int optid, const char* argval, bool gui=false );

  enum vgOpts {        /* no's 30 .. 49 reserved for valgrind */
    FIRST_CMD_OPT = Valkyrie::LAST_CMD_OPT + 1,
    TOOL          = FIRST_CMD_OPT, 
    VERBOSITY,
    TRACE_CH,
    TRACK_FDS,
    TIME_STAMP,
    RUN_LIBC,
    WEIRD,
    PTR_CHECK,
    EM_WARNS,
    LAST_CMD_OPT = EM_WARNS
  };

};


/* class Memcheck ------------------------------------------------------ */
class Memcheck : public VkObject
{
public:
  Memcheck();
  ~Memcheck() { } 

  QStringList modifiedFlags();
  int checkOptArg( int optid, const char* argval, bool gui=false );

  enum mcOpts {        /* no's 50 .. 79 reserved for memcheck */
    FIRST_CMD_OPT = Valgrind::LAST_CMD_OPT + 1,

    XML          = FIRST_CMD_OPT,  /* this may apply to more tools later */
    /* this set of flags theoretically applies to memcheck, addrcheck
       and helgrind, but since addrcheck + helgrind are feeling unwell
       pro tem, we are assuming they are only relevant for memcheck */
    LOG_FD,
    LOG_PID,
    LOG_FILE,
    LOG_SOCKET,
    DEMANGLE,
    NUM_CALLERS,
    ERROR_LIMIT,
    SHOW_BELOW,
    SUPPS,
    GEN_SUPP,
    DB_ATTACH,
    DB_COMMAND,
    INPUT_FD,
    MAX_SFRAME,
    PARTIAL,     /* <-- memcheck-specific flags start here */
    FREELIST,
    LEAK_CHECK,
    LEAK_RES,
    SHOW_REACH,
    GCC_296,
    ALIGNMENT,
    STRLEN,
    LAST_CMD_OPT = STRLEN
  };

};


/* class Cachegrind ---------------------------------------------------- */
class Cachegrind : public VkObject
{
public:
  Cachegrind();
  ~Cachegrind() { }

  int checkOptArg( int optid, const char* argval, bool gui=false );

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

  int checkOptArg( int optid, const char* argval, bool gui=false );

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


