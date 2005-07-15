/* ---------------------------------------------------------------------- 
 * Implementation of ValkyrieOptionsPage        valkyrie_options_page.cpp
 * Subclass of OptionsPage to hold valkyrie-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qfontdialog.h>

#include "valkyrie_options_page.h"
#include "vk_objects.h"
#include "vk_config.h"
#include "vk_utils.h"
#include "main_window.h"
#include "vk_messages.h"


ValkyrieOptionsPage::ValkyrieOptionsPage( QWidget* parent, VkObject* obj )
  : OptionsPage( parent, obj, "valkyrie_options_page" )
{ 
  /* init the QIntDict list, resizing if necessary */
  unsigned int numItems = 11;
  itemList.resize( numItems );

  QVBoxLayout* vbox = new QVBoxLayout( this, margin, -1, "vbox" );

  /* general prefs */
  QGroupBox* group1 = new QGroupBox( " Valkyrie Options ", this, "group1");
  vbox->addWidget( group1, space );

  /* vbox layout for group1; margin = 10; spacing = 25 */
  QVBoxLayout* gvbox = new QVBoxLayout( group1, margin, 25, "gvbox" );

  /* group1: preferences */
  itemList.insert( Valkyrie::TOOLTIP,                  /* checkbox */
                   optionWidget( Valkyrie::TOOLTIP, group1, false ) );
  itemList.insert( Valkyrie::ICONTXT,                  /* checkbox */
                   optionWidget( Valkyrie::ICONTXT, group1, false ) );
  itemList.insert( Valkyrie::PALETTE,                  /* checkbox */
                   optionWidget( Valkyrie::PALETTE, group1, false ) );

  itemList.insert( Valkyrie::FONT_SYSTEM,              /* checkbox */
                   optionWidget( Valkyrie::FONT_SYSTEM, group1, false ));
  connect( itemList[Valkyrie::FONT_SYSTEM], SIGNAL(changed(bool)),
           this,   SLOT(fontClicked(bool)) );
  itemList.insert( Valkyrie::FONT_USER,                /* line edit */
                   optionWidget( Valkyrie::FONT_USER, group1, false ) );
  LeWidget* fontLedit = ((LeWidget*)itemList[Valkyrie::FONT_USER]);
  fontLedit->addButton( group1, this, SLOT(chooseFont()), "Choose:" );
  fontLedit->setReadOnly( true );     /* don't allow direct editing */
  /* start up in correct state */
  bool enable = vkConfig->rdBool("use-system-font", "valkyrie");
  fontLedit->button()->setEnabled( !enable );
  itemList[Valkyrie::FONT_USER]->widget()->setEnabled( !enable );

  /* 1st grid layout for group1 */
  int rows = 0;
  int cols = 3;
  QGridLayout* grid1 = new QGridLayout( gvbox, rows, cols, space );
  grid1->setRowSpacing( 0, topSpace );   /* blank top row */

  grid1->addWidget( itemList[Valkyrie::TOOLTIP]->widget(), 1, 0 );
  grid1->addWidget( itemList[Valkyrie::ICONTXT]->widget(), 1, 2 );
  grid1->addWidget( itemList[Valkyrie::PALETTE]->widget(), 2, 2 );
  grid1->addWidget( itemList[Valkyrie::FONT_SYSTEM ]->widget(), 2, 0 );
  grid1->addMultiCellLayout( 
                itemList[Valkyrie::FONT_USER]->hlayout(), 3,3, 0,2 );

  gvbox->addWidget( sep(group1,"sep1"), 10 );

  itemList.insert( Valkyrie::SRC_LINES,       /* intspin */
                   optionWidget( Valkyrie::SRC_LINES, group1, true ) );
  itemList.insert( Valkyrie::SRC_EDITOR,      /* ledit + button */
                   optionWidget(Valkyrie::SRC_EDITOR, group1, false ) );
  LeWidget* editLedit = ((LeWidget*)itemList[Valkyrie::SRC_EDITOR]);
  editLedit->addButton( group1, this, SLOT(checkEditor()) );
  editLedit->setReadOnly( true );  /* don't allow direct editing */

  itemList.insert( Valkyrie::BINARY, 
                   optionWidget( Valkyrie::BINARY, group1, false ) );
  LeWidget* binLedit = ((LeWidget*)itemList[Valkyrie::BINARY]);
  binLedit->addButton( group1, this, SLOT(getBinary()) );
  connect(binLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));
  itemList.insert( Valkyrie::BIN_FLAGS, 
                   optionWidget( Valkyrie::BIN_FLAGS, group1, true ) );
  LeWidget* binFlgsLedit = ((LeWidget*)itemList[Valkyrie::BIN_FLAGS]);
  connect(binFlgsLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));

  itemList.insert( Valkyrie::VG_EXEC,         /* ledit + button */
                   optionWidget(Valkyrie::VG_EXEC, group1, false ) );
  LeWidget* vgbinLedit = ((LeWidget*)itemList[Valkyrie::VG_EXEC]);
  vgbinLedit->addButton( group1, this, SLOT(getVgExec()) );
  vgbinLedit->setReadOnly( true );   /* don't allow direct editing */
  itemList.insert( Valkyrie::VG_SUPPS_DIR,        /* ledit + button */
                   optionWidget(Valkyrie::VG_SUPPS_DIR, group1, false ) );
  LeWidget* vgsupLedit = ((LeWidget*)itemList[Valkyrie::VG_SUPPS_DIR]);
  vgsupLedit->addButton( group1, this, SLOT(getSuppDir()) );
  vgsupLedit->setReadOnly( true );   /* don't allow direct editing */

  /* 2nd grid layout for group1 */
  rows = 0;
  cols = 2;
  QGridLayout* grid2 = new QGridLayout( gvbox, rows, cols, space );
  grid2->addMultiCellLayout( 
                    itemList[Valkyrie::SRC_LINES]->hlayout(), 0,0, 0,1 );
  grid2->addWidget( editLedit->button(),                      1, 0 );
  grid2->addWidget( editLedit->widget(),                      1, 1 );

  grid2->setRowSpacing( 2, topSpace );
  grid2->addWidget( binLedit->button(),                       3, 0 );
  grid2->addWidget( binLedit->widget(),                       3, 1 );
  grid2->addWidget( itemList[Valkyrie::BIN_FLAGS]->label(),   4, 0 );
  grid2->addWidget( itemList[Valkyrie::BIN_FLAGS]->widget(),  4, 1 );

  grid2->setRowSpacing( 5, topSpace );
  grid2->addWidget( vgbinLedit->button(),                     6, 0 );
  grid2->addWidget( vgbinLedit->widget(),                     6, 1 );
  grid2->addWidget( vgsupLedit->button(),                     7, 0 );
  grid2->addWidget( vgsupLedit->widget(),                     7, 1 );

  vbox->addStretch( space );
  vk_assert( itemList.count() <= numItems );

  QIntDictIterator<OptionWidget> it( itemList );
  for ( ;  it.current(); ++it ) {
    connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
            this,         SLOT(updateEditList( bool, OptionWidget * )));
  }

}



/* called when user clicks "Apply" or "Ok" button.  
   also called when Cancel button is clicked, to reset toggled values */
bool ValkyrieOptionsPage::applyOptions( int optId, bool undo/*=false*/ )
{ 
  bool retval = true;

  switch ( optId ) {

    case Valkyrie::TOOLTIP: {
      MainWindow* vkWin = (MainWindow*)qApp->mainWidget();
      vkWin->toggleToolTips();
    } break;

    case Valkyrie::ICONTXT: {
      MainWindow* vkWin = (MainWindow*)qApp->mainWidget();
      vkWin->toggleToolbarLabels();
    } break;

    case Valkyrie::FONT_USER: {
      QFont fnt; 
      fnt.fromString( itemList[Valkyrie::FONT_USER]->currValue() );
      qApp->setFont( fnt, true );
    } break;

    default:
      if ( !undo ) {
        const char* argval = itemList[optId]->currValue().latin1();
        int errval = vkObj->checkOptArg( optId, argval, true );
        if ( errval != PARSED_OK ) {
          vkError( this, "Invalid Entry", "%s:\n\"%s\"", 
                   parseErrString(errval), argval );
          itemList[optId]->cancelEdit();
          retval = false;
        }
      } break;

  }

  return retval;
}


/* dis/enable the button depending on state of checkbox */
void ValkyrieOptionsPage::fontClicked( bool state )
{ 
  LeWidget* fontLedit = ((LeWidget*)itemList[Valkyrie::FONT_USER]);
  fontLedit->widget()->setEnabled( !state );
  fontLedit->button()->setEnabled( !state );
}


/* called by pbFont: conjures up a QFontDialog */
void ValkyrieOptionsPage::chooseFont()
{
  LeWidget* fontLedit = ((LeWidget*)itemList[Valkyrie::FONT_USER]);

  QFont afont;
  afont.fromString( fontLedit->initValue() );
  bool ok;
  QFont user_font = QFontDialog::getFont( &ok, afont, this );
  if ( ok ) {
    fontLedit->setCurrValue( user_font.toString() );
  } else {      /* user clicked cancel */
    itemList[Valkyrie::FONT_SYSTEM]->reset();
  }
}


void ValkyrieOptionsPage::checkEditor()
{
  /* try and start up somewhere sensible */
  QString ed_file = itemList[Valkyrie::SRC_EDITOR]->currValue();
  QFileInfo fi( ed_file );

  QString ed_path = QFileDialog::getOpenFileName( fi.dirPath(),
          "All Files (*)", this, "fdlg", "Select Source Editor" );
  if ( ed_path.isEmpty() ) { /* user might have clicked Cancel */
    return;
  }

  /* let's see what we have here ... */
  fi.setFile( ed_path );

  if ( fi.fileName() != "emacs" && fi.fileName() != "nedit" ) {
    vkInfo( this, "Source Editor Warning",
            "Valkyrie has not been tested with "
            "editors other than Emacs and Nedit.<br>"
            "Caveat emptor applies hereon in." );
  }

  ((LeWidget*)itemList[Valkyrie::SRC_EDITOR])->setCurrValue(ed_path);
  applyOptions( Valkyrie::SRC_EDITOR );
}


/* allows user to select executable-to-debug */
void ValkyrieOptionsPage::getBinary()
{
  QString binfile = QFileDialog::getOpenFileName( QString::null,
          "All Files (*)", this, "fdlg", "Select Executable" );
  if ( !binfile.isEmpty() ) { /* user might have clicked Cancel */
    ((LeWidget*)itemList[Valkyrie::BINARY])->setCurrValue(binfile);
    applyOptions( Valkyrie::BINARY );
  }
}


void ValkyrieOptionsPage::getSuppDir()
{
  /* VG_EXEC is guaranteed to never be empty, so start up the file
     dialog with that dir as the default place to start looking */
  QString vgpath = itemList[Valkyrie::VG_EXEC]->currValue();
  QFileInfo fi( vgpath );
  QDir dir( fi.dir() );
  dir.cd( ".." );

  QString supp_dir = QFileDialog::getExistingDirectory( dir.absPath(),
                                this, "get_supp_dir",
                                "Choose Suppressions Directory", 
                                true );
  if ( !supp_dir.isEmpty() ) {    /* user might have clicked Cancel */
    dir.setPath( supp_dir );
    /* see if we have any *.supp files in here - if so, grab 'em while
       the going's good */
    QStringList supp_list = dir.entryList( "*.supp", QDir::Files );

    /* if we don't, then this isn't a valid suppressions dir */
    if ( supp_list.count() == 0 ) {
      vkInfo( this, "Invalid Suppressions Directory", 
              "<p>This directory does not seem to contain any "
              "suppression files.</p>"
              "<p>Reverting to previous setting.</p>" );
    } else {
      /* it does contain some, so grab 'em for later digestion */
      QString supp_files = "";
      for ( unsigned int i=0; i<supp_list.count(); i++ ) {
        supp_files += dir.absPath() + "/" + supp_list[i] + ';';
      }
      /* chop off the last ';' */
      supp_files.truncate( supp_files.length() - 1 );
      /* add the list of found .supp files onto existing ones */
      vkConfig->addEntry( supp_files, "supps-all", "valgrind" );

      ((LeWidget*)itemList[Valkyrie::VG_SUPPS_DIR])->setCurrValue(supp_dir);
      applyOptions( Valkyrie::VG_SUPPS_DIR );
    }
  }
}


/* allows user to specify which valgrind version to use.  the guts of
this fn are essentially the same as the one in config.tests/valgrind.test */
void ValkyrieOptionsPage::getVgExec()
{
  QString vg_exec_path = QFileDialog::getOpenFileName( "/home", 
          "All Files (*)", this, "fdlg", "Select Valgrind" );
  if ( vg_exec_path.isEmpty() ) { /* user might have clicked Cancel */
    return;
  }

  /* quick and dirty check to see if we have an executable with rwx 
     permissions */
  LeWidget* vgbinLedit = ((LeWidget*)itemList[Valkyrie::VG_EXEC]);
  vgbinLedit->setCurrValue( vg_exec_path );
  if ( ! applyOptions( Valkyrie::VG_EXEC ) ) {
    return;
  }

  /* now check the version */
  QString cmd, vg_version, tmp_fname;
  tmp_fname = vk_mkstemp( "vg-version", vkConfig->rcDir() );
  cmd.sprintf( "%s --version | sed \"s/valgrind-//g\" > %s", 
               vg_exec_path.latin1(), tmp_fname.latin1() );
  system( cmd.latin1() );
  QFile file( tmp_fname );
  if ( file.open( IO_ReadOnly ) ) {
    file.readLine( vg_version, 100 );
  }
  file.remove();   /* close and delete the temporary file */

  /* do some fancy stuff */
  vg_version = vg_version.simplifyWhiteSpace();
  int found = str2hex( vg_version );
  int reqd  = str2hex( "3.0.0" );
  if ( found < reqd ) {
    itemList[Valkyrie::VG_EXEC]->cancelEdit();
    vkInfo( this, "Invalid Valgrind Version",
            "<p>Valgrind version >= 3.0.0 is required.</p>" );
  } 

}


