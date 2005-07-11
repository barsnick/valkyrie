/* --------------------------------------------------------------------- 
 * startup: the main program entry point                        main.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qapplication.h>
#include <qdir.h>
#include <qstylefactory.h>

#include "vk_utils.h"              // vk_assert(), parseCmdArgs()
#include "vk_config.h"             // VkConfig
#include "vk_messages.h"           // vkFatal()
#include "main_window.h"           // MainWindow

#include "vk_objects.h"            // VkObjects()
#include "valkyrie_object.h"



/* declare and init globally available objects ------------------------- */
VkConfig* vkConfig = 0;


/* See /src/kernel/qapplication.cpp #740 re starting non-gui apps */

int main ( int argc, char* argv[] )
{
  bool usingGui;
  int res = EXIT_SUCCESS;
  Valkyrie* valkyrie = 0;
  QApplication* app  = 0;
  MainWindow* vkWin  = 0;

  /* vkConfig ---------------------------------------------------------- 
     Check the configuration dir+file ~/.PACKAGE/PACKAGErc is present,
     and paths are set correctly. If the rc file is corrupted or
     missing or it's an old version, inform user, and (re)create it. 
     Initialises the various valkyrie / valgrind tool objects */
  bool ok = false;
  vkConfig = new VkConfig( &ok );
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
       return values: error = -1, show-help-and-exit = 0, ok = 1 */
    int retval = parseCmdArgs( argc, argv );
    if ( retval <= 0 ) {
      vkConfig->dontSync();
      res = ( retval == -1 ) ? EXIT_FAILURE : EXIT_SUCCESS;
      goto cleanup_and_exit;
    }
  }

  /* find out if we are in gui || non-gui mode ------------------------- */
  usingGui = vkConfig->rdBool( "gui", "valkyrie" );
  /* get hold of valkyrie */
  valkyrie = (Valkyrie*)vkConfig->vkObject( "valkyrie" );
  /* set usingGui + get ptrs to all tools */
  valkyrie->init();

  /* start turning the engine over... ---------------------------------- */
  app = new QApplication( argc, argv, usingGui );

  if ( !usingGui ) {
    /* strut your stuff, girl */
    valkyrie->runTool();
  } else {

    /* style ----------------------------------------------------------- */
    app->setStyle( QStyleFactory::create( "windows" ) );

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
    vkWin = new MainWindow( valkyrie );
    app->setMainWidget( vkWin );
    app->connect( app, SIGNAL(lastWindowClosed()), 
                  app, SLOT(quit()) );

    vkWin->resize( vkConfig->rdInt("width", "MainWin"),
                   vkConfig->rdInt("height","MainWin") );
    vkWin->move( vkConfig->rdInt("x-pos", "MainWin"),
                 vkConfig->rdInt("y-pos", "MainWin") );
    vkWin->show();
    /* start up with the tool currently set in vkConfig (either the
       default, the last-used, or whatever was set on the cmd-line) */
    vkWin->showToolView( vkConfig->currentToolId() );
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
