/* --------------------------------------------------------------------- 
 * Implementation of class Massif                      massif_object.cpp
 * Massif-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "massif_object.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "html_urls.h"
#include "vk_messages.h"


/* class Massif -------------------------------------------------------- */
Massif::~Massif() { }

Massif::Massif() 
  : ToolObject( MASSIF, "Massif", "Ma&ssif", Qt::SHIFT+Qt::Key_S ) 
{
  /* massif flags */
  addOpt( HEAP,        Option::ARG_BOOL,   Option::CHECK,
          "massif",    '\0',               "heap",
          "<yes|no>",  "yes|no",           "yes",
          "Profile heap blocks",
          "profile heap blocks",           
          urlMassif::Heap );
  addOpt(  HEAP_ADMIN, Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "heap-admin", 
          "<number>",  "4|15",             "8",
          "Average admin bytes per heap block:",
          "average admin bytes per heap block", 
           urlMassif::HeapAdmin );
  addOpt( STACKS,      Option::ARG_BOOL,   Option::CHECK,
          "massif",    '\0',               "stacks",
          "<yes|no>",  "yes|no",           "yes",
          "Profile stack(s)",
          "profile stack(s)",
          urlMassif::Stacks );
  addOpt(  DEPTH,      Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "depth", 
          "<number>",  "1|20",             "3",
          "Depth of contexts:",
          "depth of contexts", 
           urlMassif::Depth );
  addOpt( ALLOC_FN,    Option::ARG_STRING, Option::LEDIT, 
          "massif",    '\0',               "alloc-fn", 
          "<name>",    "",                 "",
          "Specify <fn> as an alloc function:",
          "specify <fn> as an alloc function", 
          urlMassif::AllocFn );
  addOpt( FORMAT,      Option::ARG_STRING,   Option::COMBO,  
          "massif",    '\0',                 "format",
          "<text|html|xml>", "text|html|xml", "text",
          "Output Format:",
          "format of textual output",
          urlMassif::Format );
  addOpt( ALIGNMENT,  Option::ARG_UINT,   Option::SPINBOX, 
          "massif",    '\0',               "alignment", 
          "<number>",  "8|1048576",        "8",
          "Minimum alignment of allocations:",
          "set minimum alignment of allocations", 
           urlVgCore::Alignment );
}


int Massif::checkOptArg( int optid, const char* argval, 
                         bool use_gui/*=false*/ )
{
  vk_assert( optid >= FIRST_CMD_OPT && optid <= LAST_CMD_OPT );

  int errval = PARSED_OK;
  Option * opt = findOption( optid );

  switch ( optid ) {

    /* how on earth can this be checked ? */
    case ALLOC_FN:
      break;

    case HEAP:
    case HEAP_ADMIN:
    case STACKS:
    case DEPTH:
    case FORMAT:
      opt->isValidArg( &errval, argval );
      break;

    /* check it's really a number ... then if is a power of two */
    case ALIGNMENT:
      opt->isPowerOfTwo( &errval, argval ); 
      break;

  }

  /* if this option has been called from the cmd-line, save its value
     in valkyrierc if it has passed the checks. */
  if ( errval == PARSED_OK && use_gui == false ) {
    writeOptionToConfig( opt, argval );
  }

  return errval;
}


/* returns the ToolView window (memcheckView) for this tool */
ToolView* Massif::createView( QWidget* parent )
{
  usingGui = true;
  m_view = new MassifView( parent, this);
  view()->setState( is_Running );
  return m_view;
}


void Massif::emitRunning( bool run )
{
  is_Running = run;
  emit running( is_Running );

  if ( usingGui ) {
    view()->setState( is_Running );
  }
}


/* called by MainWin::closeToolView() */
bool Massif::isDone()
{
  vk_assert( view() != 0 );

  /* if current process is not yet finished, ask user if they really
     want to close */
  if ( is_Running ) {
    int ok = vkQuery( this->view(), "Process Running", "&Abort;&Cancel",
                      "<p>The current process is not yet finished.</p>"
                      "<p>Do you want to abort it ?</p>" );
    if ( ok == MsgBox::vkYes ) {
      bool stopped = stop( Valkyrie::modeNotSet );     /* abort */
      vk_assert( stopped );          // TODO: what todo if couldn't stop?
    } else if ( ok == MsgBox::vkNo ) {
      return false;                                    /* continue */
    }
  }

  if ( !fileSaved ) {
    /* currently loaded / parsed stuff isn't saved to disk */
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


bool Massif::stop( Valkyrie::RunMode )
{
  VK_DEBUG("TODO: %s::stop()", name().latin1() );
  return true;
}
