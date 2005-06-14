/*---------------------------------------------------------------------- 
  Startup: the main program file                                main.cpp
  ----------------------------------------------------------------------
*/

#include <qapplication.h>
#include <qdir.h>
#include <qstylefactory.h>

#include "vk_include.h"
#include "vk_utils.h"              // vk_assert(), parseCmdArgs()
#include "vk_config.h"             // VkConfig
#include "vk_msgbox.h"             // vkFatal()
#include "vk_objects.h"            // VkObjects()
#include "main_window.h"           // MainWindow


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


/* See /src/kernel/qapplication.cpp #740 re starting non-gui apps */

int main ( int argc, char* argv[] )
{
  int res = EXIT_SUCCESS;
  MainWindow* vkWin = 0;

  /* start turning the engine over... ---------------------------------- */
  QApplication app( argc, argv );
  /* TODO: allow user to chang default application style */
  app.setStyle( QStyleFactory::create( "windows" ) );


  /* vkConfig ---------------------------------------------------------- 
     Check the configuration dir+file ~/.PACKAGE/PACKAGErc is present,
     and paths are set correctly. If the rc file is corrupted or
     missing or it's an old version, inform user, and (re)create it. 
     The very first time valkyrie is run, tries to find where valgrind
     lives; if fails, asks user; checks the version is okay.
     Initialises the various valkyrie / valgrind tool objects */
  bool ok = false;
  vkConfig = new VkConfig( &ok );
  if ( !ok ) {
    res = EXIT_FAILURE;
    goto cleanup_and_exit;
  }
  vk_assert( vkConfig != 0 );


  /* Command-line parsing ---------------------------------------------- 
     If no command-line flags are present, skip parsing.  Assume that
     either the user wishes to set things up from inside valkryie, or
     this is a re-run which uses the settings stored in vkConfig. */
  if ( argc > 1 ) {
    /* Parse the command-line args, and overwrite vkConfig's current
       options with any options we see there.  But if the user typed
       --help || --version || -V, or there were parsing errors, do the
       right thing, then exit immediately. 
       Return values: error = -1, show-help-and-exit = 0, ok = 1 */
    int retval = parseCmdArgs( argc, argv );
    if ( retval <= 0 ) {
      vkConfig->dontSync();
      res = ( retval == -1 ) ? EXIT_FAILURE : EXIT_SUCCESS;
      goto cleanup_and_exit;
    }
  }


  /* Font -------------------------------------------------------------- 
     allow user to specify an app-wide font setting */
  if ( !vkConfig->rdBool( "use-system-font", "Prefs" ) ) {
    QFont vkfnt = vkConfig->rdFont( "user-font", "Prefs" );
    app.setFont( vkfnt, true );
  }


  /* palette -----------------------------------------------------------
     allow user to choose between app. default palette and the default
     palette assigned by their system. */
  if ( vkConfig->rdBool("use-vk-palette", "Prefs") ) {
    app.setPalette( vkPalette(), true );
  }


  /* we have lift-off - start up the gui ------------------------------- */
  vkWin = new MainWindow();
  app.setMainWidget( vkWin );
  app.connect( &app, SIGNAL(lastWindowClosed()), 
               &app, SLOT(quit()) );

#if 1
  vkWin->resize( vkConfig->rdInt("width", "MainWin"),
                 vkConfig->rdInt("height","MainWin") );
  vkWin->move( vkConfig->rdInt("x-pos", "MainWin"),
               vkConfig->rdInt("y-pos", "MainWin") );
#else
  vkWin->setGeometry ( vkConfig->rdInt("x-pos", "MainWin"), 
                       vkConfig->rdInt("y-pos", "MainWin"), 
                       vkConfig->rdInt("width", "MainWin"), 
                       vkConfig->rdInt("height","MainWin") );
#endif

  vkWin->show();

  res = app.exec();

 cleanup_and_exit:
  if ( vkConfig ) { 
    delete vkConfig; 
    vkConfig = 0;
  }

  return res;
}
