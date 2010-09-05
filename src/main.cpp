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


VkConfig* vkConfig = NULL;




/*!
    Main program entry point
    arguments parsed, Qt setup, application setup, the ball set rolling.
*/
int main( int argc, char* argv[] )
{
   int exit_status = EXIT_SUCCESS;
   QApplication* app  = 0;
   MainWindow* vkWin  = 0;
   
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
   
   VkConfig::createConfig( &valkyrie, &vkConfig );
   
   if ( vkConfig == NULL ) {
      VK_DEBUG( "Failed to initialise config properly: Aborting" );
      goto cleanup_and_exit;
   }
   
   
   // ------------------------------------------------------------
   // Command-line parsing
   //  - if a project file is given, the settings will override the initialised vkConfig.
   if ( argc > 1 ) {
      // parse cmdline args, overwrite vkConfig with any options found.
      bool show_help_and_exit;
      
      if ( ! parseCmdArgs( argc, argv, &valkyrie, show_help_and_exit ) ) {
         exit_status = EXIT_FAILURE;
         goto cleanup_and_exit;
      }
      
      if ( show_help_and_exit ) {
         goto cleanup_and_exit;
      }
   }
   
   // save the working config
   vkConfig->sync();
   
   
   
   // ------------------------------------------------------------
   // Start up the gui
   vkWin = new MainWindow( &valkyrie );
   
   if ( vkConfig->value(
           valkyrie.getOption( VALKYRIE::BINARY )->configKey() ).toString().isEmpty() ) {
      vkWin->openOptions();
   }
   
   vkWin->showToolView( VGTOOL::ID_MEMCHECK );
   vkWin->show();
   
   
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
   
   if ( vkConfig ) {
      delete vkConfig;
   }
   
   if ( app ) {
      delete app;
   }
   
   return exit_status;
}
