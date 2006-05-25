/* ---------------------------------------------------------------------- 
 * Implementation of ValkyrieOptionsPage        valkyrie_options_page.cpp
 * Subclass of OptionsPage to hold valkyrie-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
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
#include "context_help.h"
#include "html_urls.h"


ValkyrieOptionsPage::ValkyrieOptionsPage( QWidget* parent, VkObject* obj )
   : OptionsPage( parent, obj, "valkyrie_options_page" )
{ 
   /* init the QIntDict list, resizing if necessary */
   unsigned int numItems = 12;
   m_itemList.resize( numItems );

   QVBoxLayout* vbox = new QVBoxLayout( this, m_margin, -1, "vbox" );

   QGroupBox* group1 = new QGroupBox( " Valkyrie Options ", this, "group1");
   ContextHelp::add( group1, urlValkyrie::optsPage );
   vbox->addWidget( group1, m_space );

   /* vbox layout; margin = 10; spacing = 25 */
   QVBoxLayout* gvbox = new QVBoxLayout( group1, m_margin, 25, "gvbox" );

   /* general prefs ------------------------------------------------- */
   m_itemList.insert( Valkyrie::TOOLTIP,                  /* checkbox */
                      optionWidget( Valkyrie::TOOLTIP, group1, false ) );
   m_itemList.insert( Valkyrie::ICONTXT,                  /* checkbox */
                      optionWidget( Valkyrie::ICONTXT, group1, false ) );
   m_itemList.insert( Valkyrie::PALETTE,                  /* checkbox */
                      optionWidget( Valkyrie::PALETTE, group1, false ) );

   /* fonts --------------------------------------------------------- */
   m_itemList.insert( Valkyrie::FNT_GEN_SYS,              /* checkbox */
                      optionWidget( Valkyrie::FNT_GEN_SYS, group1, false ));
   m_itemList.insert( Valkyrie::FNT_GEN_USR,                /* line edit */
                      optionWidget( Valkyrie::FNT_GEN_USR, group1, false ) );
   LeWidget* fontGenLedit = ((LeWidget*)m_itemList[Valkyrie::FNT_GEN_USR]);
   fontGenLedit->addButton( group1, this, SLOT(chooseGenFont()), "Choose:" );
   fontGenLedit->setReadOnly( true );     /* don't allow direct editing */
   /* start up in correct state */
   bool use_sys_font = vkConfig->rdBool("font-gen-sys", "valkyrie");
   fontGenLedit->setDisabled( use_sys_font );
   connect( m_itemList[Valkyrie::FNT_GEN_SYS], SIGNAL(changed(bool)),
            m_itemList[Valkyrie::FNT_GEN_USR], SLOT(setDisabled(bool)) );

   m_itemList.insert( Valkyrie::FNT_TOOL_USR,                /* line edit */
                      optionWidget( Valkyrie::FNT_TOOL_USR, group1, false ) );
   LeWidget* fontToolLedit = ((LeWidget*)m_itemList[Valkyrie::FNT_TOOL_USR]);
   fontToolLedit->addButton( group1, this, SLOT(chooseToolFont()), "Choose:" );
   fontToolLedit->setReadOnly( true );     /* don't allow direct editing */

   /* core ---------------------------------------------------------- */
   m_itemList.insert( Valkyrie::SRC_LINES,       /* intspin */
                      optionWidget( Valkyrie::SRC_LINES, group1, true ) );
   m_itemList.insert( Valkyrie::SRC_EDITOR,      /* ledit + button */
                      optionWidget(Valkyrie::SRC_EDITOR, group1, false ) );
   LeWidget* editLedit = ((LeWidget*)m_itemList[Valkyrie::SRC_EDITOR]);
   editLedit->addButton( group1, this, SLOT(checkEditor()) );
   editLedit->setReadOnly( true );  /* don't allow direct editing */

   m_itemList.insert( Valkyrie::BINARY, 
                      optionWidget( Valkyrie::BINARY, group1, false ) );
   LeWidget* binLedit = ((LeWidget*)m_itemList[Valkyrie::BINARY]);
   binLedit->addButton( group1, this, SLOT(getBinary()) );
   connect(binLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));
   m_itemList.insert( Valkyrie::BIN_FLAGS, 
                      optionWidget( Valkyrie::BIN_FLAGS, group1, true ) );
   LeWidget* binFlgsLedit = ((LeWidget*)m_itemList[Valkyrie::BIN_FLAGS]);
   connect(binFlgsLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));

   m_itemList.insert( Valkyrie::VG_EXEC,         /* ledit + button */
                      optionWidget(Valkyrie::VG_EXEC, group1, false ) );
   LeWidget* vgbinLedit = ((LeWidget*)m_itemList[Valkyrie::VG_EXEC]);
   vgbinLedit->addButton( group1, this, SLOT(getVgExec()) );
   vgbinLedit->setReadOnly( true );   /* don't allow direct editing */


   /* --------------------------------------------------------------- */
   /* Note: not using opt_widget->hlayout()'s
      because button width won't match qlabel width... */
   int rows = 0;
   int cols = 4;
   QGridLayout* grid = new QGridLayout( gvbox, rows, cols, m_space );
   grid->setColStretch(0, 0);
   grid->setColStretch(1, 1);
#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( 0, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   grid->addRowSpacing( 0, m_topSpace );   /* blank top row */
#endif

   grid->addMultiCellWidget( m_itemList[Valkyrie::TOOLTIP]->widget(), 1,1, 0,1 );
   grid->addWidget( m_itemList[Valkyrie::ICONTXT]->widget(), 1, 2 );
   grid->addMultiCellWidget( m_itemList[Valkyrie::PALETTE]->widget(), 2,2, 0,1 );

   grid->addMultiCellWidget( sep(group1,"sep0"), 3,3, 0,3 );
#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( 3, 8 );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( 3, 8 );
#endif

   /* --------------------------------------------------------------- */
   QLabel* fntLblGen = new QLabel("General Font:", group1);
   grid->addMultiCellWidget( fntLblGen, 4,4, 0,1 );
   grid->addWidget( m_itemList[Valkyrie::FNT_GEN_SYS ]->widget(), 4, 2 );
   grid->addWidget( fontGenLedit->button(),                       5, 0 );
   grid->addMultiCellWidget( fontGenLedit->widget(),              5,5, 1,3 );

   QLabel* fntLblTool = new QLabel("Tool Font:", group1);
   grid->addMultiCellWidget( fntLblTool, 6,6, 0,1 );
   grid->addWidget( fontToolLedit->button(),                      7, 0 );
   grid->addMultiCellWidget( fontToolLedit->widget(),             7,7, 1,3 );

   grid->addMultiCellWidget( sep(group1,"sep1"), 8,8, 0,3 );
#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( 8, 8 );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( 8, 8 );
#endif

   /* --------------------------------------------------------------- */
   grid->addMultiCellLayout( m_itemList[Valkyrie::SRC_LINES]->hlayout(),  9,9, 0,3 );
   grid->addWidget( editLedit->button(),                       10, 0 );
   grid->addMultiCellWidget( editLedit->widget(),              10,10, 1,3 );

#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( 11, m_topSpace );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( 11, m_topSpace );
#endif
   grid->addWidget( binLedit->button(),                        12, 0 );
   grid->addMultiCellWidget( binLedit->widget(),               12,12, 1,3 );
   grid->addWidget( binFlgsLedit->label(),                     13, 0 );
   grid->addMultiCellWidget( binFlgsLedit->widget(),           13,13, 1,3 );

#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( 14, m_topSpace );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( 14, m_topSpace );
#endif
   grid->addWidget( vgbinLedit->button(),                      15, 0 );
   grid->addMultiCellWidget( vgbinLedit->widget(),             15,15, 1,3 );


   vbox->addStretch( m_space );
   vk_assert( m_itemList.count() <= numItems );

   QIntDictIterator<OptionWidget> it( m_itemList );
   for ( ;  it.current(); ++it ) {
      connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
              this,         SLOT(updateEditList( bool, OptionWidget * )));
   }
}


/* called when user clicks "Apply" / "Ok" / "Reset" buttons.  */
bool ValkyrieOptionsPage::applyOptions( int optId )
{ 
   vk_assert( optId <= Valkyrie::LAST_CMD_OPT );

   /* check option */
   QString argval = m_itemList[optId]->currValue();
   int errval = m_vkObj->checkOptArg( optId, argval );
   if ( errval != PARSED_OK ) {
      vkError( this, "Invalid Entry", "%s:\n\"%s\"", 
               parseErrString(errval), argval.latin1() );
      m_itemList[optId]->cancelEdit();
      return false;
   }

   /* apply option */
   switch ( optId ) {
   case Valkyrie::TOOLTIP: {
      MainWindow* vkWin = (MainWindow*)qApp->mainWidget();
      vkWin->toggleToolTips();
   } break;

   case Valkyrie::ICONTXT: {
      MainWindow* vkWin = (MainWindow*)qApp->mainWidget();
      vkWin->toggleToolbarLabels();
   } break;

   case Valkyrie::FNT_GEN_USR:
   case Valkyrie::FNT_GEN_SYS: {
      /* one or both of these could end up here on 'Apply'
         - so making sure not to apply changes more than once */
      bool useSysFont = ((CkWidget*)m_itemList[Valkyrie::FNT_GEN_SYS])->isOn();
      QFont fnt;
      if (useSysFont)
         fnt = vkConfig->defaultAppFont();
      else
         fnt.fromString( m_itemList[Valkyrie::FNT_GEN_USR]->currValue() );
      if (qApp->font() != fnt)
         qApp->setFont( fnt, true );
   } break;

   case Valkyrie::FNT_TOOL_USR: {
      QFont fnt;
      fnt.fromString( m_itemList[Valkyrie::FNT_TOOL_USR]->currValue() );

      /* set font for all tool views */
      ToolObjList tools = ((Valkyrie*)m_vkObj)->valgrind()->toolObjList();
      for ( ToolObject* tool = tools.first(); tool; tool = tools.next() ) {
         tool->view()->setToolFont( fnt );
      }
   } break;

   case Valkyrie::PALETTE: {
      bool useVkPalette = ((CkWidget*)m_itemList[Valkyrie::PALETTE])->isOn();
      if ( useVkPalette ) {
         QApplication::setPalette( vkConfig->vkPalette(), true );
      } else {
         /* setting qapp style resets palette: better way to do this? */
         QApplication::setStyle( vkConfig->vkStyle() );
      }
   } break;

   default:
      break;
   }

   return true;
}


/* called by pbFont: conjures up a QFontDialog */
void ValkyrieOptionsPage::chooseGenFont()
{
   LeWidget* fontLedit = ((LeWidget*)m_itemList[Valkyrie::FNT_GEN_USR]);

   QFont afont;
   afont.fromString( fontLedit->initValue() );
   bool ok;
   QFont font = QFontDialog::getFont( &ok, afont, this );
   if ( ok ) {
      fontLedit->setCurrValue( font.toString() );
   } else {      /* user clicked cancel */
      m_itemList[Valkyrie::FNT_GEN_SYS]->reset();
   }
}


/* called by pbFont: conjures up a QFontDialog */
void ValkyrieOptionsPage::chooseToolFont()
{
   LeWidget* fontLedit = ((LeWidget*)m_itemList[Valkyrie::FNT_TOOL_USR]);

   QFont afont;
   afont.fromString( fontLedit->initValue() );
   bool ok;
   QFont font = QFontDialog::getFont( &ok, afont, this );
   if ( ok ) {
      fontLedit->setCurrValue( font.toString() );
   }
}


void ValkyrieOptionsPage::checkEditor()
{
   /* try and start up somewhere sensible */
   QString ed_file = m_itemList[Valkyrie::SRC_EDITOR]->currValue();
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

   ((LeWidget*)m_itemList[Valkyrie::SRC_EDITOR])->setCurrValue(ed_path);
   applyOptions( Valkyrie::SRC_EDITOR );
}


/* allows user to select executable-to-debug */
void ValkyrieOptionsPage::getBinary()
{
   QString binfile = QFileDialog::getOpenFileName( QString::null,
                                                   "All Files (*)", this, "fdlg", "Select Executable" );
   if ( !binfile.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Valkyrie::BINARY])->setCurrValue(binfile);
      applyOptions( Valkyrie::BINARY );
   }
}


/* RM: allows user to specify which valgrind version to use.  the guts
   of this fn are essentially the same as the one in config.tests/valgrind.test */
void ValkyrieOptionsPage::getVgExec()
{
   LeWidget* vgbinLedit = ((LeWidget*)m_itemList[Valkyrie::VG_EXEC]);

   QString startdir = vgbinLedit->currValue();
   if (startdir.isEmpty())
      startdir = QDir::currentDirPath();

   QString vg_exec_path = QFileDialog::getOpenFileName( startdir, 
                                                        "All Files (*)",
                                                        this, "fdlg",
                                                        "Select Valgrind" );
   if ( vg_exec_path.isEmpty() ) /* user clicked Cancel ? */
      return;

   /* quick and dirty check to see if we have an executable with rwx 
      permissions */
   vgbinLedit->setCurrValue( vg_exec_path );
   if ( ! applyOptions( Valkyrie::VG_EXEC ) )
      return;


   /* now check the version */
   QString cmd, vg_version, tmp_fname;
   tmp_fname = vk_mkstemp( vkConfig->rcDir() + "vg-version" );
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
      m_itemList[Valkyrie::VG_EXEC]->cancelEdit();
      vkInfo( this, "Invalid Valgrind Version",
              "<p>Valgrind version >= 3.0.0 is required.</p>" );
   } 
}
