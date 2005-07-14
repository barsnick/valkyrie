/* --------------------------------------------------------------------- 
 * Implementation of class Cachegrind              cachegrind_object.cpp
 * Cachegrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "cachegrind_object.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "html_urls.h"
#include "vk_messages.h"


/* class Cachegrind ---------------------------------------------------- */
Cachegrind::~Cachegrind() { }

Cachegrind::Cachegrind() 
  : ToolObject(CACHEGRIND, "Cachegrind", "&Cachegrind", Qt::SHIFT+Qt::Key_C) 
{ 
  /* cachegrind flags */
  addOpt( I1_CACHE,     Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "I1",
          "<size,assoc,line_size>", "",     "0,0,0",
          "I1 cache configuration:",
          "set I1 cache manually",
          urlCachegrind::Cacheopts );
  addOpt( D1_CACHE,     Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "D1", 
          "<size,assoc,line_size>", "",     "0,0,0",
          "D1 cache configuration:",        
          "Set D1 cache manually",
          urlCachegrind::Cacheopts );
  addOpt( L2_CACHE,     Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "L2",
          "<size,assoc,line_size>", "",     "0,0,0",
          "L2 cache configuration:",
          "set L2 cache manually",
          urlCachegrind::Cacheopts );
  /* cachegrind annotate script flags */
  addOpt( PID_FILE,     Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", '\0',               "pid", 
          "<file.pid>", "",                 "",
          "File to read:",
          "Which <cachegrind.out.pid> file to read (required)",
          urlCachegrind::Pid );
  addOpt( SORT,         Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", '\0',               "sort", 
          "<A,B,C>",    "",                 "event column order",
          "Sort columns by:",
          "sort columns by events A,B,C",
          urlCachegrind::Sort );
  addOpt( SHOW,         Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", '\0',               "show",
          "<A,B,C>",    "",                 "all",
          "Show figures for events:",
          "only show figures for events A,B,C",
          urlCachegrind::Show );
  addOpt( THRESH,       Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "threshold", 
          "<%>",        "0|100",            "99",
          "Threshold percentage:",
          "percentage of counts (of primary sort event) we are interested in",
          urlCachegrind::Threshold );
  addOpt( AUTO,         Option::ARG_BOOL,   Option::CHECK, 
          "cachegrind", '\0',               "auto",
          "<yes|no>",   "yes|no",           "no",
          "Automatically annotate all relevant source files",
          "Annotate all source files containing functions that helped reach the event count threshold",
          urlCachegrind::Auto );
  addOpt( CONTEXT,      Option::ARG_UINT,   Option::SPINBOX, 
          "cachegrind", '\0',               "context",
          "<number>",   "0|50",             "8",
          "Number of context lines to print:",
          "print <number> lines of context before and after annotated lines",
          urlCachegrind::Context );
  addOpt( INCLUDE,      Option::ARG_STRING, Option::LEDIT, 
          "cachegrind", 'I',                "include", 
          "<dir1,dir2>", "",                "",
          "Source dirs:",
          "List of directories to search for source files",
          urlCachegrind::Include );
}


int Cachegrind::checkOptArg( int optid, const char* argval, 
                             bool use_gui/*=false*/ )
{ 
  vk_assert( optid >= FIRST_CMD_OPT && optid <= LAST_CMD_OPT );

  int errval = PARSED_OK;
  QString argVal( argval );
  Option* opt = findOption( optid );

  switch ( optid ) {

  case I1_CACHE:
  case D1_CACHE:
  case L2_CACHE: {
    QStringList aList = QStringList::split( ",", argVal );
    if ( aList.count() != 3 ) {
      errval = PERROR_BADARG;
    }  else {
      QStringList possvals;  
      possvals << "4" << "1048576";  // << "8"<< "8192";
      /* 1st number */
      opt->setPossibleValues( possvals );
      if ( opt->isPowerOfTwo( &errval, aList[0].latin1() ) ) {
        /* 2nd number */
        possvals[0] = "0";
        possvals[1] = "8";
        opt->setPossibleValues( possvals );
        if ( opt->isValidArg( &errval, aList[1].latin1() ) ) {
          /* 3rd number */
          possvals[0] = "4";
          possvals[1] = "8192";
          opt->setPossibleValues( possvals );
          opt->isPowerOfTwo( &errval, aList[2].latin1() );
        }
      }
    }
  } break;

  case PID_FILE:
    argVal = fileCheck( &errval, argVal, true, false );
    break;

  case THRESH:
  case AUTO:
  case CONTEXT:
    opt->isValidArg( &errval, argVal );
    break;

  /* TODO: not sure how to handle these just yet :( */
  case SHOW:
  case SORT:
    break;

  case INCLUDE: {
    QStringList aList = QStringList::split( ",", argVal );
    for ( unsigned int i=0; i<aList.count(); i++ ) {
      QString tmp    = aList[i].simplifyWhiteSpace();
      QString srcdir = dirCheck( &errval, tmp.latin1(), true, false );
      if ( errval == PARSED_OK ) {
        aList[i] = srcdir;
      } else {
        if ( !use_gui ) {
          vkPrint( "Parse error [--include=%s] : invalid dir path\n"
                   "The offending directory is '%s'", 
                   argVal.latin1(), tmp.latin1() );
        }
        errval = PERROR_DEFAULT;
        break;
      } 
      argVal = aList.join( "," );
    } 
  } break;

  }

  /* if this option has been called from the cmd-line, save its value
     in valkyrierc if it has passed the checks. */
  if ( errval == PARSED_OK && use_gui == false ) {
    writeOptionToConfig( opt, argVal );
  }

  return errval;
}


/* returns the ToolView window (memcheckView) for this tool */
ToolView* Cachegrind::createView( QWidget* parent )
{
  usingGui = true;
  m_view = new CachegrindView( parent, this );
  view()->setState( is_Running );
  return m_view;
}


void Cachegrind::emitRunning( bool run )
{
  is_Running = run;
  emit running( is_Running );

  if ( usingGui ) {
    view()->setState( is_Running );
  }
}


/* called by MainWin::closeToolView() */
bool Cachegrind::isDone()
{
  vk_assert( view() != 0 );

  /* if current process is not yet finished, ask user if they really
     want to close */
  if ( is_Running ) {
    int ok = vkQuery( view(), "Process Running", "&Abort;&Cancel",
                      "<p>The current process is not yet finished.</p>"
                      "<p>Do you want to abort it ?</p>" );
    if ( ok == MsgBox::vkYes ) {
      bool stopped = stop( Valkyrie::modeNotSet );     /* abort */
      vk_assert( stopped );         // TODO: what todo if couldn't stop?
    } else if ( ok == MsgBox::vkNo ) {
      return false;                                    /* continue */
    }
  }

  if ( !fileSaved ) {
    /* currently loaded / parsed stuff isn't saved to disk */
    int ok = vkQuery( view(), "Unsaved File", 
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


bool Cachegrind::stop( Valkyrie::RunMode )
{
  VK_DEBUG("TODO: %s::stop()", name().latin1() );
  return true;
}
