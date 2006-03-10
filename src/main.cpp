/* --------------------------------------------------------------------- 
 * startup: the main program entry point                        main.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qdir.h>
#include <qstylefactory.h>
#include <signal.h>    /* signal name macros, and the signal() prototype */

#include "vk_utils.h"              // vk_assert(), parseCmdArgs()
#include "vk_config.h"             // VkConfig
#include "vk_messages.h"           // vkFatal()
#include "main_window.h"           // MainWindow

#include "valkyrie_object.h"
#include "tool_object.h"           // VkRunState

/* catch Ctrl-C, and die nicely */
void catch_ctrl_c(int /*sig_num*/)
{
    fprintf(stderr, "Ctrl-C caught: Cleaning up and exiting...\n\n");
    fflush(stderr);

    qApp->exit(1);
    qApp->processEvents();
    /* Still hangs around until window system pokes it.. how to bypass that? */
}



/* declare and init globally available objects ------------------------- */
VkConfig* vkConfig = 0;


/* valkyrie's default palette ------------------------------------------ */
QPalette vkPalette()
{
   QColor bg = vkConfig->rdColor( "background" );
   if ( !bg.isValid() ) return qApp->palette();
   QColor base = vkConfig->rdColor( "base" );
   if ( !base.isValid() ) return qApp->palette();
   QColor text = vkConfig->rdColor( "text" );
   if ( !text.isValid() ) return qApp->palette();
   QColor dkgray = vkConfig->rdColor( "dkgray" );
   if ( !dkgray.isValid() ) return qApp->palette();
   QColor hilite = vkConfig->rdColor( "highlight" );
   if ( !hilite.isValid() )  return qApp->palette();

   /* 3 colour groups: active, inactive, disabled */
   QPalette pal( bg, bg );
   /* bg colour for text entry widgets */
   pal.setColor( QPalette::Active,   QColorGroup::Base, base );
   pal.setColor( QPalette::Inactive, QColorGroup::Base, base );
   pal.setColor( QPalette::Disabled, QColorGroup::Base, base );
   /* general bg colour */
   pal.setColor( QPalette::Active,   QColorGroup::Background, bg );
   pal.setColor( QPalette::Inactive, QColorGroup::Background, bg );
   pal.setColor( QPalette::Disabled, QColorGroup::Background, bg );
   /* same as bg */
   pal.setColor( QPalette::Active,   QColorGroup::Button, bg );
   pal.setColor( QPalette::Inactive, QColorGroup::Button, bg );
   pal.setColor( QPalette::Disabled, QColorGroup::Button, bg );
   /* general fg colour - same as Text */
   pal.setColor( QPalette::Active,   QColorGroup::Foreground, text );
   pal.setColor( QPalette::Inactive, QColorGroup::Foreground, text );
   pal.setColor( QPalette::Disabled, QColorGroup::Foreground, dkgray );
   /* same as fg */
   pal.setColor( QPalette::Active,   QColorGroup::Text, text );
   pal.setColor( QPalette::Inactive, QColorGroup::Text, text );
   pal.setColor( QPalette::Disabled, QColorGroup::Text, dkgray );
   /* same as text and fg */
   pal.setColor( QPalette::Active,   QColorGroup::ButtonText, text );
   pal.setColor( QPalette::Inactive, QColorGroup::ButtonText, text );
   pal.setColor( QPalette::Disabled, QColorGroup::ButtonText, dkgray );
   /* highlight */
   pal.setColor( QPalette::Active,   QColorGroup::Highlight, hilite );
   pal.setColor( QPalette::Inactive, QColorGroup::Highlight, hilite );
   pal.setColor( QPalette::Disabled, QColorGroup::Highlight, hilite );
   /* contrast with highlight */
   pal.setColor( QPalette::Active,
                 QColorGroup::HighlightedText, base );
   pal.setColor( QPalette::Inactive,
                 QColorGroup::HighlightedText, base );
   pal.setColor( QPalette::Disabled,
                 QColorGroup::HighlightedText, base );

   return pal;
}


#include "vk_include.h"


int main ( int argc, char* argv[] )
{
   int res = EXIT_SUCCESS;
   QApplication* app  = 0;
   MainWindow* vkWin  = 0;
   VkRunState::State startState = VkRunState::STOPPED;

   /* catch Ctrl-C */
   signal(SIGINT, catch_ctrl_c);

   /* Create all VkObjects: Vk[ Vg[ Tools ] ] */
   Valkyrie valkyrie;

   /* start turning the engine over... ---------------------------------- */
   app = new QApplication( argc, argv );

   /* style ----------------------------------------------------------- */
   app->setStyle( QStyleFactory::create( "windows" ) );

   /* vkConfig ---------------------------------------------------------- 
      Check the configuration dir+file ~/.PACKAGE/PACKAGErc is present,
      and paths are set correctly. If the rc file is corrupted or
      missing or it's an old version, inform user, and (re)create it. 
      Initialises the various valkyrie / valgrind tool objects */
   bool ok = false;
   vkConfig = new VkConfig( &valkyrie, &ok );
   if ( !ok ) {
      res = EXIT_FAILURE;
      goto cleanup_and_exit;
   }
   vk_assert( vkConfig != 0 );

   /* command-line parsing ---------------------------------------------- 
      if no command-line flags are present, skip parsing.  assume that
      either the user wishes to set things up from inside valkryie, or
      this is a re-run which uses the settings stored in vkConfig. */
   if ( argc > 1 ) {
      /* parse the command-line args, and overwrite vkConfig's current
         options with any options we see there.  but if the user typed
         --help || --version || --valgrind-opts, or there were parsing
         errors, do the right thing, then exit immediately.
         return values: error < 0, ok = 0, show-help-and-exit = 1 */
      int retval = parseCmdArgs( argc, argv, &valkyrie );
      if ( retval != 0 ) {
         vkConfig->dontSync();
         res = ( retval < 0 ) ? EXIT_FAILURE : EXIT_SUCCESS;
         goto cleanup_and_exit;
      }
   }

   /* font: allow user to specify an app-wide font setting ------------ */
   if ( !vkConfig->rdBool( "use-system-font", "valkyrie" ) ) {
      QFont vkfnt = vkConfig->rdFont( "user-font", "valkyrie" );
      app->setFont( vkfnt, true );
   }

   /* palette: allow user to choose between app. default palette
      and the default palette assigned by their system ---------------- */
   if ( vkConfig->rdBool( "use-vk-palette", "valkyrie" ) ) {
      app->setPalette( vkPalette(), true );
   }
    
   /* we have lift-off: start up the gui ------------------------------ */
   vkWin = new MainWindow( &valkyrie );
   app->setMainWidget( vkWin );
   app->connect( app, SIGNAL(lastWindowClosed()), 
                 app, SLOT(quit()) );

   vkWin->resize( vkConfig->rdInt("width", "MainWin"),
                  vkConfig->rdInt("height","MainWin") );
   vkWin->move( vkConfig->rdInt("x-pos", "MainWin"),
                vkConfig->rdInt("y-pos", "MainWin") );
   vkWin->show();

   startState = valkyrie.startRunState();
   if ( startState != VkRunState::STOPPED &&
        startState != VkRunState::VALGRIND ) {
      /* other run states currently only makes sense for memcheck,
         so override the default/given tool */
      vkWin->showToolView( valkyrie.valgrind()->toolObjId( "memcheck" ) );
   } else {
      /* start up with the tool currently set in vkConfig (either the
         default, the last-used, or whatever was set on the cmd-line) */
      vkWin->showToolView( vkConfig->mainToolObjId() );
   }
   qApp->processEvents();

   /* start a run if specified on the cmd-line */
   if (startState != VkRunState::STOPPED) {
      vkWin->run( startState );
   }

   res = app->exec();

 cleanup_and_exit:
   if ( vkConfig ) { 
      delete vkConfig;
      vkConfig = 0;
   }
   if ( vkWin ) {
      delete vkWin;
      vkWin = 0;
   }

   return res;
}
