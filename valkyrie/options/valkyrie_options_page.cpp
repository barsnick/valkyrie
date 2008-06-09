/* ---------------------------------------------------------------------- 
 * Implementation of ValkyrieOptionsPage        valkyrie_options_page.cpp
 * Subclass of OptionsPage to hold valkyrie-specific options | flags.
 * ---------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
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
   unsigned int numItems = 13;
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
   m_itemList.insert( Valkyrie::BROWSER,                  /* line edit */
                      optionWidget( Valkyrie::BROWSER, group1, false ) );
   LeWidget* brwsrLedit = ((LeWidget*)m_itemList[Valkyrie::BROWSER]);
   brwsrLedit->addButton( group1, this, SLOT(getBrowser()) );
   connect(brwsrLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));

   m_itemList.insert( Valkyrie::DFLT_LOGDIR,    /* ledit + button */
                      optionWidget(Valkyrie::DFLT_LOGDIR, group1, false ) );
   LeWidget* dirLogSave = ((LeWidget*)m_itemList[Valkyrie::DFLT_LOGDIR]);
   dirLogSave->addButton( group1, this, SLOT(getDfltLogDir()) );
   connect(dirLogSave, SIGNAL(returnPressed()), this, SIGNAL(apply()));

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
   editLedit->addButton( group1, this, SLOT(getEditor()) );
   connect(editLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));

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
   connect(vgbinLedit, SIGNAL(returnPressed()), this, SIGNAL(apply()));


   /* --------------------------------------------------------------- */
   /* Note: not using opt_widget->hlayout()'s
      because button width won't match qlabel width... */
   int rows = 0;
   int cols = 4;
   int i=0;
   QGridLayout* grid = new QGridLayout( gvbox, rows, cols, m_space );
   grid->setColStretch(0, 0);
   grid->setColStretch(1, 1);
#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( i++, m_topSpace );   /* blank top row */
#else // QT_VERSION < 3.2
   grid->addRowSpacing( i++, m_topSpace );   /* blank top row */
#endif

   grid->addMultiCellWidget( m_itemList[Valkyrie::TOOLTIP]->widget(), i,i, 0,1 );
   grid->addWidget( m_itemList[Valkyrie::ICONTXT]->widget(), i++, 2 );
   grid->addMultiCellWidget( m_itemList[Valkyrie::PALETTE]->widget(), i,i, 0,1 );
   i++;

   grid->addWidget( brwsrLedit->button(),                             i, 0 );
   grid->addMultiCellWidget( brwsrLedit->widget(),                    i,i, 1,3 );
   i++;

   grid->addWidget( dirLogSave->button(),                             i, 0 );
   grid->addMultiCellWidget( dirLogSave->widget(),                    i,i, 1,3 );
   i++;

   grid->addMultiCellWidget( sep(group1,"sep0"), i,i, 0,3 );
#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( i++, 8 );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( i++, 8 );
#endif

   /* --------------------------------------------------------------- */
   QLabel* fntLblGen = new QLabel("General Font:", group1);
   grid->addMultiCellWidget( fntLblGen,                           i,i, 0,1 );
   grid->addWidget( m_itemList[Valkyrie::FNT_GEN_SYS ]->widget(), i++, 2 );
   grid->addWidget( fontGenLedit->button(),                       i, 0 );
   grid->addMultiCellWidget( fontGenLedit->widget(),              i,i, 1,3 );
   i++;

   QLabel* fntLblTool = new QLabel("Tool Font:", group1);
   grid->addMultiCellWidget( fntLblTool,                          i,i, 0,1 );
   i++;
   grid->addWidget( fontToolLedit->button(),                      i, 0 );
   grid->addMultiCellWidget( fontToolLedit->widget(),             i,i, 1,3 );
   i++;

   grid->addMultiCellWidget( sep(group1,"sep1"), i,i, 0,3 );
#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( i++, 8 );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( i++, 8 );
#endif

   /* --------------------------------------------------------------- */
   grid->addMultiCellLayout( m_itemList[Valkyrie::SRC_LINES]->hlayout(),  i,i, 0,3 );
   i++;
   grid->addWidget( editLedit->button(),                       i, 0 );
   grid->addMultiCellWidget( editLedit->widget(),              i,i, 1,3 );
   i++;

#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( i++, m_topSpace );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( i++, m_topSpace );
#endif
   grid->addWidget( binLedit->button(),                        i, 0 );
   grid->addMultiCellWidget( binLedit->widget(),               i,i, 1,3 );
   i++;
   grid->addWidget( binFlgsLedit->label(),                     i, 0 );
   grid->addMultiCellWidget( binFlgsLedit->widget(),           i,i, 1,3 );
   i++;

#if (QT_VERSION-0 >= 0x030200)
   grid->setRowSpacing( i++, m_topSpace );
#else // QT_VERSION < 3.2
   grid->addRowSpacing( i++, m_topSpace );
#endif
   grid->addWidget( vgbinLedit->button(),                      i, 0 );
   grid->addMultiCellWidget( vgbinLedit->widget(),             i,i, 1,3 );
   i++;

   vbox->addStretch( m_space );
   vk_assert( m_itemList.count() <= numItems );

   QIntDictIterator<OptionWidget> it( m_itemList );
   for ( ;  it.current(); ++it ) {
      connect(it.current(), SIGNAL(valueChanged( bool, OptionWidget * )),
              this,         SLOT(updateEditList( bool, OptionWidget * )));
   }
}


/* called when user clicks "Apply" / "Ok" / "Reset" buttons.  */
void ValkyrieOptionsPage::applyOption( int optId )
{ 
   vk_assert( optId >= 0 && optId < Valkyrie::NUM_OPTS );

   OptionWidget* optWidg = m_itemList[optId];
   vk_assert( optWidg != 0 );
   QString argval = optWidg->currValue();

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

   case Valkyrie::SRC_EDITOR: {
      /* if no "%n", give warning */
      if (argval.find("%n") == -1) {
         QFileInfo fi( QStringList::split(" ",argval ).first() );
         if ( !fi.fileName().startsWith("emacs") && !fi.fileName().startsWith("nedit") ) {
            vkInfo( this, "Unknown Source Editor",
                    "If possible, set an editor flag to allow the \
                     editor to be opened at a target line-number, \
                     where %%n will be replaced with the line-number." );
         }
      }
   } break;

   default:
      break;
   }
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


void ValkyrieOptionsPage::getEditor()
{
   /* try and start up somewhere sensible */
   QString ed = m_itemList[Valkyrie::SRC_EDITOR]->currValue();
   QString ed_file = QStringList::split(" ", ed).first();
   QFileInfo fi( ed_file );

   QString ed_path =
      QFileDialog::getOpenFileName( fi.dirPath(), "All Files (*)",
                                    this, "fdlg", "Select Source Editor" );
   if ( ed_path.isEmpty() ) { /* user might have clicked Cancel */
      return;
   }

   fi.setFile( ed_path );

   ed = ed_path;
   if ( fi.fileName().startsWith("emacs") ||
        fi.fileName().startsWith("nedit") ) {
      /* add go-to-line flag + replacement string (%n) */
      ed += " +%n";
   }

   ((LeWidget*)m_itemList[Valkyrie::SRC_EDITOR])->setCurrValue(ed);
   checkOption( Valkyrie::SRC_EDITOR );
}


/* allows user to select executable-to-debug */
void ValkyrieOptionsPage::getBinary()
{
   QString binfile =
      QFileDialog::getOpenFileName( QString::null, "All Files (*)",
                                    this, "fdlg", "Select Executable" );
   if ( !binfile.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Valkyrie::BINARY])->setCurrValue(binfile);
      checkOption( Valkyrie::BINARY );
   }
}


/* allows user to select default browser */
void ValkyrieOptionsPage::getBrowser()
{
   QString brwsr =
      QFileDialog::getOpenFileName( QString::null, "All Files (*)",
                                    this, "fdlg", "Select Browser" );
   if ( !brwsr.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Valkyrie::BROWSER])->setCurrValue(brwsr);
      checkOption( Valkyrie::BROWSER );
   }
}


/* RM: allows user to specify which valgrind version to use.  the guts
   of this fn are essentially the same as the one in config.tests/valgrind.test */
void ValkyrieOptionsPage::getVgExec()
{
   QString vg_exec_path =
      QFileDialog::getOpenFileName( QString::null, "All Files (*)",
                                    this, "fdlg", "Select Valgrind" );
   if ( !vg_exec_path.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Valkyrie::VG_EXEC])->setCurrValue( vg_exec_path );
      checkOption( Valkyrie::VG_EXEC );
   }
}


/* RM: allows user to specify which valgrind version to use.  the guts
   of this fn are essentially the same as the one in config.tests/valgrind.test */
void ValkyrieOptionsPage::getDfltLogDir()
{
   QString currdir = m_itemList[Valkyrie::DFLT_LOGDIR]->currValue();
   QString dir_logsave =
      QFileDialog::getExistingDirectory( currdir, this,
                                         "get default log-save dir",
                                          "Choose a directory", TRUE );
   if ( !dir_logsave.isEmpty() ) { /* user might have clicked Cancel */
      ((LeWidget*)m_itemList[Valkyrie::DFLT_LOGDIR])->setCurrValue( dir_logsave );
      checkOption( Valkyrie::DFLT_LOGDIR );
   }
}
