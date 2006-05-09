/* --------------------------------------------------------------------- 
 * Implementation of class Cachegrind              cachegrind_object.cpp
 * Cachegrind-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "cachegrind_object.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "html_urls.h"
#include "vk_messages.h"
#include "vk_popt_option.h"    // PERROR* and friends 


/* class Cachegrind ---------------------------------------------------- */
Cachegrind::~Cachegrind() { }

Cachegrind::Cachegrind( int objId ) 
   : ToolObject( "Cachegrind", "&Cachegrind", Qt::SHIFT+Qt::Key_C, objId ) 
{ 
   /* cachegrind flags */
   addOpt( I1_CACHE,     VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "cachegrind", '\0',                 "I1",
           "<size,assoc,line_size>", "",       "0,0,0",
           "I1 cache configuration:",
           "set I1 cache manually",
           urlCachegrind::Cacheopts );
   addOpt( D1_CACHE,     VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "cachegrind", '\0',                 "D1", 
           "<size,assoc,line_size>", "",       "0,0,0",
           "D1 cache configuration:",        
           "Set D1 cache manually",
           urlCachegrind::Cacheopts );
   addOpt( L2_CACHE,     VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "cachegrind", '\0',                 "L2",
           "<size,assoc,line_size>", "",       "0,0,0",
           "L2 cache configuration:",
           "set L2 cache manually",
           urlCachegrind::Cacheopts );
   /* cachegrind annotate script flags */
   addOpt( PID_FILE,     VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "cachegrind", '\0',                 "pid", 
           "<file.pid>", "",                   "",
           "File to read:",
           "Which <cachegrind.out.pid> file to read (required)",
           urlCachegrind::Pid );
   addOpt( SHOW,         VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "cachegrind", '\0',                 "show",
           "<A,B,C>",    "",                   "all",
           "Show figures for events:",
           "only show figures for events A,B,C",
           urlCachegrind::Show );
   addOpt( SORT,         VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "cachegrind", '\0',                 "sort", 
           "<A,B,C>",    "",                   "event column order",
           "Sort columns by:",
           "sort columns by events A,B,C",
           urlCachegrind::Sort );
   addOpt( THRESH,       VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "cachegrind", '\0',                 "threshold", 
           "<%>",        "0|100",              "99",
           "Threshold percentage:",
           "percentage of counts (of primary sort event) we are interested in",
           urlCachegrind::Threshold );
   addOpt( AUTO,         VkOPTION::ARG_BOOL,   VkOPTION::WDG_CHECK, 
           "cachegrind", '\0',                 "auto",
           "<yes|no>",   "yes|no",             "no",
           "Automatically annotate all relevant source files",
           "annotate all source files containing functions that helped reach the event count threshold",
           urlCachegrind::Auto );
   addOpt( CONTEXT,      VkOPTION::ARG_UINT,   VkOPTION::WDG_SPINBOX, 
           "cachegrind", '\0',                 "context",
           "<number>",   "0|50",               "8",
           "Number of context lines to print:",
           "print <number> lines of context before and after annotated lines",
           urlCachegrind::Context );
   addOpt( INCLUDE,      VkOPTION::ARG_STRING, VkOPTION::WDG_LEDIT, 
           "cachegrind", 'I',                  "include", 
           "<dir>",      "",                   "",
           "Source dirs:",
           "add <dir> to list of directories to search for source files",
           urlCachegrind::Include );
}


/* check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages -------------------- */
int Cachegrind::checkOptArg( int optid, QString& argval )
{ 
   vk_assert( optid >= 0 && optid <= LAST_CMD_OPT );

   int errval = PARSED_OK;
   Option* opt = findOption( optid );

   switch ( optid ) {

   case I1_CACHE:
   case D1_CACHE:
   case L2_CACHE: {
      QStringList aList = QStringList::split( ",", argval );
      if ( aList.count() != 3 ) {
         errval = PERROR_BADARG;
      }  else {
         QStringList possvals;  
         possvals << "4" << "1048576";  // << "8"<< "8192";
         /* 1st number */
         opt->setPossibleValues( possvals );
         if ( opt->isValidArg( &errval, aList[0] ) &&
              Option::isPowerOfTwo( &errval, aList[0].latin1() ) ) {
            /* 2nd number */
            possvals[0] = "0";
            possvals[1] = "8";
            opt->setPossibleValues( possvals );
            if ( opt->isValidArg( &errval, aList[1].latin1() ) ) {
               /* 3rd number */
               possvals[0] = "4";
               possvals[1] = "8192";
               opt->setPossibleValues( possvals );
               if ( opt->isValidArg( &errval, aList[2] ) )
                  Option::isPowerOfTwo( &errval, aList[2].latin1() );
            }
         }
      }
   } break;

   case PID_FILE:
      argval = fileCheck( &errval, argval, true, false );
      break;

   case THRESH:
   case AUTO:
   case CONTEXT:
      opt->isValidArg( &errval, argval.latin1() );
      break;

      /* TODO: not sure how to handle these just yet :( */
   case SHOW:
   case SORT:
      break;

   case INCLUDE: {
      QStringList aList = QStringList::split( ",", argval );
      for ( unsigned int i=0; i<aList.count(); i++ ) {
         QString tmp    = aList[i].simplifyWhiteSpace();
         QString srcdir = dirCheck( &errval, tmp.latin1(), true, false );
         if ( errval == PARSED_OK ) {
            aList[i] = srcdir;
         } else {
            errval = PERROR_DEFAULT;
            break;
         } 
         argval = aList.join( "," );
      } 
   } break;

   }

   return errval;
}


/* Creates this tool's ToolView window,
   and sets up and connections between them */
ToolView* Cachegrind::createView( QWidget* parent )
{
   m_view = new CachegrindView( parent, this->name() );

   /* signals view --> tool */
   //..

   /* signals tool --> view */
   connect( this, SIGNAL(running(bool)),
            m_view, SLOT(setState(bool)) );

   setRunState( VkRunState::STOPPED );
   return m_view;
}


/* called by MainWin::closeToolView() */
bool Cachegrind::isDone()
{
   vk_assert( view() != 0 );

   /* if current process is not yet finished, ask user if they really
      want to close */
   if ( isRunning() ) {
      int ok = vkQuery( view(), "Process Running", "&Abort;&Cancel",
                        "<p>The current process is not yet finished.</p>"
                        "<p>Do you want to abort it ?</p>" );
      if ( ok == MsgBox::vkYes ) {
         bool stopped = stop();        /* abort */
         vk_assert( stopped );         // TODO: what todo if couldn't stop?
      } else if ( ok == MsgBox::vkNo ) {
         return false;                        /* continue */
      }
   }

   if ( !m_fileSaved ) {
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


bool Cachegrind::start( VkRunState::State rs, QStringList /*vgflags*/ )
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


bool Cachegrind::stop()
{
   vk_assert( isRunning() );

   switch ( runState() ) {

   default:
      vk_assert_never_reached();
   }

   // TODO: statusMsg() ?

   return true;
}
