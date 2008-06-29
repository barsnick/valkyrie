/* --------------------------------------------------------------------- 
 * startup: the main program entry point                        main.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qdir.h>
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <stdlib.h>    /* exit */

#include "vk_utils.h"              // vk_assert(), vkPrint() etc
#include "parse_cmd_args.h"        // parseCmdArgs()
#include "vk_config.h"             // VkConfig
#include "vk_messages.h"           // vkFatal()
#include "main_window.h"           // MainWindow

#include "valkyrie_object.h"
#include "tool_object.h"           // VkRunState

/* catch ctrl-c, and die nicely */
void catch_ctrl_c(int /*sig_num*/)
{
    vkPrint("Interrupted: Cleaning up and exiting...");

    qApp->exit(1);
    qApp->wakeUpGuiThread();
}



/* declare and init globally available objects ------------------------- */
VkConfig* vkConfig = 0;




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

   /* vkConfig ---------------------------------------------------------- 
      Check the configuration dir+file ~/.PACKAGE/PACKAGErc is present,
      and paths are set correctly. If the rc file is corrupted or
      missing or it's an old version, inform user, and (re)create it. 
      Initialises the various valkyrie / valgrind tool objects */
   vkConfig = new VkConfig();
   bool ok = vkConfig->initCfg( &valkyrie );
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
         options with any options we see there (but not to disk yet).
         but if user typed --help || --version || --valgrind-opts,
         or there were parsing errors, do the right thing, then exit
         immediately.
         return values: error < 0, ok = 0, show-help-and-exit = 1 */
      int retval = parseCmdArgs( argc, argv, &valkyrie );
      if ( retval != 0 ) {
         res = ( retval < 0 ) ? EXIT_FAILURE : EXIT_SUCCESS;
         goto cleanup_and_exit;
      }
   }

   /* style ----------------------------------------------------------- */
   app->setStyle( vkConfig->vkStyle() );

   /* font: allow user to specify an app-wide font setting ------------ */
   if ( !vkConfig->rdBool( "font-gen-sys", "valkyrie" ) ) {
      QFont vkfnt = vkConfig->rdFont( "font-gen-user", "valkyrie" );
      app->setFont( vkfnt, true );
   }

   /* palette: allow user to choose between app. default palette
      and the default palette assigned by their system ---------------- */
   if ( vkConfig->rdBool( "use-vk-palette", "valkyrie" ) ) {
      app->setPalette( vkConfig->vkPalette(), true );
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

       QString toolname = vkConfig->rdEntry("tool", "valgrind");
       ToolObjList tools = valkyrie.valgrind()->toolObjList();
       for ( ToolObject* tool=tools.first(); tool; tool=tools.next() ) {
         if ( tool->name() == toolname ) {
           vkWin->showToolView( tool->objId() );
           break;
         }
       }

   }
   qApp->processEvents();

   /* start a run if specified on the cmd-line */
   if (startState != VkRunState::STOPPED) {
      vkWin->run( startState );
   }

   res = app->exec();

 cleanup_and_exit:
   if ( vkWin ) {
      delete vkWin;
      vkWin = 0;
   }
   /* delete vkConfig last */
   if ( vkConfig ) { 
      delete vkConfig;
      vkConfig = 0;
   }

   return res;
}
