/****************************************************************************
** main
**  - the program entry point
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
** <info@open-works.co.uk>
**
** This file is part of Valkyrie, a front-end for Valgrind.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file COPYING included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <QApplication>

#include "mainwindow.h"
#include "objects/valkyrie_object.h"
#include "options/vk_parse_cmdline.h"
#include "toolview/toolview.h"
#include "utils/vk_utils.h"
#include "utils/vk_config.h"


VkCfgGlbl* vkCfgGlbl = NULL;  // Singleton VkCfgGlbl (non-project config)
VkCfgProj* vkCfgProj = NULL;  // Singleton VkCfgProj (project config)



/*!
    Main program entry point
    arguments parsed, Qt setup, application setup, the ball set rolling.
*/
int main( int argc, char* argv[] )
{
   int exit_status = EXIT_SUCCESS;
   QApplication* app  = 0;
   MainWindow* vkWin  = 0;
   VGTOOL::ToolProcessId startProcess = VGTOOL::PROC_NONE;

   // ------------------------------------------------------------
   // Create all VkObjects: Vk[ Vg[ Tools[] ] ]
   Valkyrie valkyrie;
   
   // ------------------------------------------------------------
   // Start turning the engine over
   app = new QApplication( argc, argv );
   
   // ------------------------------------------------------------
   // Setup application config settings
   QCoreApplication::setOrganizationName( "OpenWorks" );
   QCoreApplication::setOrganizationDomain( "openworks.co.uk" );
   QCoreApplication::setApplicationName( "Valkyrie" );
   
   QLocale::setDefault( QLocale( QLocale::English, QLocale::UnitedKingdom ) );
   
   bool cfg_ok = VkCfgProj::createConfig( &valkyrie );
   if ( !cfg_ok || vkCfgProj == NULL ) {
      VK_DEBUG( "Failed to initialise project config properly.  "
                "Please check existence/permissions/versions of the "
                "config dir '%s', and its files/sub-directories.</p>",
                qPrintable( VkCfg::cfgDir() ) );
      goto cleanup_and_exit;
   }

   cfg_ok = VkCfgGlbl::createConfig();
   if ( !cfg_ok || vkCfgGlbl == NULL ) {
      VK_DEBUG( "Failed to initialise global config properly.  "
                "Please check existence/permissions of the "
                "config dir '%s', and its files/sub-directories.</p>",
                qPrintable( VkCfg::cfgDir() ) );
      goto cleanup_and_exit;
   }

   //TODO: check docs found

   // ------------------------------------------------------------
   // Command-line parsing
   //  - if a project file is given, the settings will override the initialised vkCfgProj.
   if ( argc > 1 ) {
      // parse cmdline args, overwrite vkCfgProj with any options found.
      bool show_help_and_exit;
      
      if ( ! parseCmdArgs( argc, argv, &valkyrie, show_help_and_exit ) ) {
         exit_status = EXIT_FAILURE;
         goto cleanup_and_exit;
      }
      
      if ( show_help_and_exit ) {
         goto cleanup_and_exit;
      }
   }
   
   // save the working config we've gotten so far.
   vkCfgProj->sync();
   
   
   
   // ------------------------------------------------------------
   // Start up the gui
   vkWin = new MainWindow( &valkyrie );
   
   if ( vkCfgProj->value(
           valkyrie.getOption( VALKYRIE::BINARY )->configKey() ).toString().isEmpty() ) {
      vkWin->openOptions();
   }
   
   vkWin->showToolView( VGTOOL::ID_MEMCHECK );
   vkWin->show();

   // start up a process (run valgrind / view-log / ...) from the command line.
   startProcess = valkyrie.getStartToolProcess();
   if ( startProcess != VGTOOL::PROC_NONE ) {
      vkWin->runTool( startProcess );
   }
   
   qApp->processEvents();
   
   // ------------------------------------------------------------
   // Hand over to QtApp.
   exit_status = app->exec();
   
   
cleanup_and_exit:

   // ------------------------------------------------------------
   // We're done - clean up and return.
   if ( vkWin ) {
      delete vkWin;
   }
   if ( vkCfgProj ) {
      delete vkCfgProj;
   }
   if ( vkCfgGlbl ) {
      delete vkCfgGlbl;
   }
   if ( app ) {
      delete app;
   }
   
   return exit_status;
}
