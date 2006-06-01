/* --------------------------------------------------------------------- 
 * Implementation of class Massif                      massif_object.cpp
 * Massif-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "massif_object.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "html_urls.h"
#include "vk_messages.h"
#include "vk_option.h"         // PERROR* and friends 


/* class Massif -------------------------------------------------------- */
Massif::~Massif() { }

Massif::Massif( int objId ) 
   : ToolObject( "Massif", "Ma&ssif", Qt::SHIFT+Qt::Key_S, objId ) 
{
   /* massif flags */
   addOpt( HEAP,        VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,
           "massif",    '\0',                 "heap",
           "<yes|no>",  "yes|no",             "yes",
           "Profile heap blocks",
           "profile heap blocks",           
           urlMassif::Heap );
   addOpt(  HEAP_ADMIN, VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
            "massif",    '\0',                 "heap-admin", 
            "<number>",  "4|15",               "8",
            "Average admin bytes per heap block:",
            "average admin bytes per heap block", 
            urlMassif::HeapAdmin );
   addOpt( STACKS,      VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK,
           "massif",    '\0',                 "stacks",
           "<yes|no>",  "yes|no",             "yes",
           "Profile stack(s)",
           "profile stack(s)",
           urlMassif::Stacks );
   addOpt(  DEPTH,      VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
            "massif",    '\0',                "depth", 
            "<number>",  "1|20",              "3",
            "Depth of contexts:",
            "depth of contexts", 
            urlMassif::Depth );
   addOpt( ALLOC_FN,    VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "massif",    '\0',                 "alloc-fn", 
           "<name>",    "",                   "",
           "Specify <fn> as an alloc function:",
           "specify <fn> as an alloc function", 
           urlMassif::AllocFn );
   /* TODO: generate xml format for output */
   addOpt( FORMAT,      VkOPTION::ARG_STRING,   VkOPTION::WDG_COMBO,  
           "massif",    '\0',                   "format",
           "<text|html|xml>", "text|html|xml",  "text",
           "Output Format:",
           "format of textual output",
           urlMassif::Format );
   addOpt( ALIGNMENT,  VkOPTION::ARG_PWR2,   VkOPTION::WDG_SPINBOX, 
           "massif",    '\0',                 "alignment", 
           "<number>",  "8|1048576",          "8",
           "Minimum alignment of allocations:",
           "set minimum alignment of allocations", 
           urlVgCore::Alignment );
}


/* check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages -------------------- */
int Massif::checkOptArg( int optid, QString& argval )
{
   vk_assert( optid >= 0 && optid < NUM_OPTS );

   int errval = PARSED_OK;
   Option* opt = findOption( optid );

   switch ( optid ) {

   case HEAP:
   case HEAP_ADMIN:
   case STACKS:
   case DEPTH:
   case FORMAT:
   case ALIGNMENT:
      opt->isValidArg( &errval, argval );
      break;

      /* how on earth can this be checked ? */
   case ALLOC_FN:
      break;

   default:
      vk_assert_never_reached();
   }

   return errval;
}


/* Creates this tool's ToolView window,
   and sets up and connections between them */
ToolView* Massif::createView( QWidget* parent )
{
   m_view = new MassifView( parent, this->name() );
   /* signals view --> tool */
   // ...

   /* signals tool --> view */
   connect( this, SIGNAL(running(bool)),
            m_view, SLOT(setState(bool)) );

   setRunState( VkRunState::STOPPED );
   return m_view;
}


/* called by MainWin::closeToolView() */
bool Massif::queryDone()
{
   vk_assert( view() != 0 );

   /* if current process is not yet finished, ask user if they really
      want to close */
   if ( isRunning() ) {
      int ok = vkQuery( this->view(), "Process Running", "&Abort;&Cancel",
                        "<p>The current process is not yet finished.</p>"
                        "<p>Do you want to abort it ?</p>" );
      /* Note: process may have finished while waiting for user */
      if ( ok == MsgBox::vkYes ) {
         stop();                              /* abort */
         vk_assert( !isRunning() );
      } else if ( ok == MsgBox::vkNo ) {
         return false;                        /* continue */
      }
   }

   /* currently loaded / parsed stuff isn't saved to disk */
   if ( !m_fileSaved ) {
      int ok = vkQuery( this->view(), "Unsaved File", 
                        "&Save;&Discard;&Cancel",
                        "<p>The current output is not saved."
                        "Do you want to save it ?</p>" );
      if ( ok == MsgBox::vkYes ) {
         VK_DEBUG("TODO: save();\n");         /* save */
      } else if ( ok == MsgBox::vkCancel ) {
         return false;                        /* procrastinate */
      }
   }

   return true;
}


bool Massif::start( VkRunState::State rs, QStringList /*vgflags*/ )
{
   bool ok = false;
   vk_assert( rs != VkRunState::STOPPED );
   vk_assert( !isRunning() );

   switch ( rs ) {

   default:
      vk_assert_never_reached();
   }
   return ok;
}


void Massif::stop()
{
   if ( !isRunning() )
      return;

   switch ( runState() ) {

   default:
      vk_assert_never_reached();
   }

   // TODO: statusMsg() ?

   return;
}
