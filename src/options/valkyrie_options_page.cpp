/****************************************************************************
** ValkyrieOptionsPage implementation
**  - subclass of VkOptionsPage to hold valkyrie-specific options
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
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
#include <QFileDialog>
#include <QFontDialog>
#include <QLabel>
#include <QVBoxLayout>

#include "help/help_context.h"
#include "help/help_urls.h"
#include "objects/valkyrie_object.h"            // access to valkyrie object
#include "options/widgets/opt_base_widget.h"
#include "options/widgets/opt_ck_widget.h"
#include "options/widgets/opt_le_widget.h"
#include "options/valkyrie_options_page.h"
#include "utils/vk_utils.h"
#include "utils/vk_config.h"

#if 0
#include "vk_objects.h"
#include "vk_messages.h"
#endif


/***************************************************************************/
/*!
  Valkyrie Options Page
*/
ValkyrieOptionsPage::ValkyrieOptionsPage( VkObject* obj )
   : VkOptionsPage( obj )
{
}


void ValkyrieOptionsPage::setupOptions()
{
   group1 = new QGroupBox( " Valkyrie Options ", this );
   group1->setObjectName( QString::fromUtf8( "ValkyrieOptionsPage_group1" ) );
   pageTopVLayout->addWidget( group1 );
   pageTopVLayout->addStretch( 1 );
   
   ContextHelp::addHelp( group1, urlValkyrie::optsPage );
   
   // Note: not using opt_widget->hlayout()'s as button width won't match qlabel width.
   QGridLayout* grid = new QGridLayout( group1 );
   grid->setColumnStretch( 0, 0 );
   grid->setColumnStretch( 1, 1 );
   grid->setColumnStretch( 2, 1 );
   grid->setColumnStretch( 3, 1 );
   
   int i = 0;
   grid->setRowMinimumHeight( i++, lineHeight / 2 ); // blank row
   
   // ------------------------------------------------------------
   // target setup - options
   insertOptionWidget( VALKYRIE::BINARY, group1, false );  // ledit + button
   LeWidget* binLedit = (( LeWidget* )m_itemList[VALKYRIE::BINARY] );
   binLedit->addButton( group1, this, SLOT( getBinary() ) );
   
   insertOptionWidget( VALKYRIE::BIN_FLAGS, group1, true );  // ledit
   LeWidget* binFlgsLedit = (( LeWidget* )m_itemList[VALKYRIE::BIN_FLAGS] );
   
   insertOptionWidget( VALKYRIE::WORKING_DIR, group1, false );  // line edit + button
   LeWidget* dirWorking = (( LeWidget* )m_itemList[VALKYRIE::WORKING_DIR] );
   dirWorking->addButton( group1, this, SLOT( getWorkingDir() ) );
   
   // target setup - layout
   grid->addWidget( binLedit->button(),     i, 0 );
   grid->addWidget( binLedit->widget(),     i++, 1, 1, 3 );
   grid->addWidget( binFlgsLedit->label(),  i, 0 );
   grid->addWidget( binFlgsLedit->widget(), i++, 1, 1, 3 );
   grid->addWidget( dirWorking->button(),   i, 0 );
   grid->addWidget( dirWorking->widget(),   i++, 1, 1, 3 );
   
   grid->addWidget( sep( group1 ), i++, 0, 1, 4 );
   
   
   // ------------------------------------------------------------
   // general prefs - options
   insertOptionWidget( VALKYRIE::SRC_EDITOR, group1, false );  // ledit + button
   LeWidget* editLedit = (( LeWidget* )m_itemList[VALKYRIE::SRC_EDITOR] );
   editLedit->addButton( group1, this, SLOT( getEditor() ) );

   insertOptionWidget( VALKYRIE::SRC_LINES, group1, true );    // intspin
   
   insertOptionWidget( VALKYRIE::BROWSER, group1, false );  // line edit
   LeWidget* brwsrLedit = (( LeWidget* )m_itemList[VALKYRIE::BROWSER] );
   brwsrLedit->addButton( group1, this, SLOT( getBrowser() ) );
   
   insertOptionWidget( VALKYRIE::DFLT_LOGDIR, group1, false );  // line edit + button
   LeWidget* dirLogSave = (( LeWidget* )m_itemList[VALKYRIE::DFLT_LOGDIR] );
   dirLogSave->addButton( group1, this, SLOT( getDfltLogDir() ) );
   
   insertOptionWidget( VALKYRIE::VG_EXEC, group1, false );  // ledit + button
   LeWidget* vgbinLedit = (( LeWidget* )m_itemList[VALKYRIE::VG_EXEC] );
   vgbinLedit->addButton( group1, this, SLOT( getVgExec() ) );
   
   // general prefs - layout
   grid->addWidget( editLedit->button(), i, 0 );
   grid->addWidget( editLedit->widget(), i++, 1, 1, 3 );
   grid->addLayout( m_itemList[VALKYRIE::SRC_LINES]->hlayout(),  i++, 0, 1, 4 );
   
   grid->addWidget( brwsrLedit->button(), i, 0 );
   grid->addWidget( brwsrLedit->widget(), i++, 1, 1, 3 );
   grid->addWidget( dirLogSave->button(), i, 0 );
   grid->addWidget( dirLogSave->widget(), i++, 1, 1, 3 );
   grid->addWidget( vgbinLedit->button(), i, 0 );
   grid->addWidget( vgbinLedit->widget(), i++, 1, 1, 3 );
   
   grid->addWidget( sep( group1 ), i++, 0, 1, 4 );
   
   
   // ------------------------------------------------------------
   // look 'n feel - options
   insertOptionWidget( VALKYRIE::TOOLTIP, group1, false );  // checkbox
   insertOptionWidget( VALKYRIE::ICONTXT, group1, false );  // checkbox
   insertOptionWidget( VALKYRIE::PALETTE, group1, false );  // checkbox
   
   insertOptionWidget( VALKYRIE::FNT_GEN_SYS, group1, false );  // checkbox
   LeWidget* fontGenSysLedit = (( LeWidget* )m_itemList[VALKYRIE::FNT_GEN_SYS] );
   insertOptionWidget( VALKYRIE::FNT_GEN_USR, group1, false );  // line edit
   LeWidget* fontGenLedit = (( LeWidget* )m_itemList[VALKYRIE::FNT_GEN_USR] );
   fontGenLedit->addButton( group1, this, SLOT( chooseGenFont() ) );
   fontGenLedit->setReadOnly( true );     // don't allow direct editing
   
   // start up in correct state
   bool use_sys_font = vkCfgProj->value( "valkyrie/font-gen-sys" ).toBool();
   fontGenLedit->setDisabled( use_sys_font );
   connect( fontGenSysLedit, SIGNAL( changed( bool ) ),
            fontGenLedit, SLOT( setDisabled( bool ) ) );
            
   insertOptionWidget( VALKYRIE::FNT_TOOL_USR, group1, false );  // line edit
   LeWidget* fontToolLedit = (( LeWidget* )m_itemList[VALKYRIE::FNT_TOOL_USR] );
   fontToolLedit->addButton( group1, this, SLOT( chooseToolFont() ) );
   fontToolLedit->setReadOnly( true );     // don't allow direct editing
   
   // look 'n feel - layout
   grid->addWidget( m_itemList[VALKYRIE::TOOLTIP]->widget(), i++, 0, 1, 2 );
   grid->addWidget( m_itemList[VALKYRIE::ICONTXT]->widget(), i++, 0, 1, 2 );
   grid->addWidget( m_itemList[VALKYRIE::PALETTE]->widget(), i++, 0, 1, 2 );
   grid->addWidget( fontGenSysLedit->widget(), i++, 0, 1, 4 );
   grid->addWidget( fontGenLedit->button(),    i, 0 );
   grid->addWidget( fontGenLedit->widget(),    i++, 1, 1, 3 );
   grid->addWidget( fontToolLedit->button(),   i, 0 );
   grid->addWidget( fontToolLedit->widget(),   i++, 1, 1, 3 );
   
   vk_assert( m_itemList.count() <= VALKYRIE::NUM_OPTS );


   // ------------------------------------------------------------
   // tooltips
   // TODO: put these in the options.
   QString tip_editor = tr( "Tip: \"%n\" will be replaced with "
                            "the source code line number.<br>"
                            "Set the appropriate editor flag to support "
                            "opening the source at this line." );
   editLedit->button()->setToolTip( tip_editor );
   editLedit->widget()->setToolTip( tip_editor );

   QString tip_logdir = tr( "Tip: All files in this temporary directory are "
                            "deleted by Valkyrie on startup and exit.<br>"
                            "Don't save your own files here!" );
   dirLogSave->button()->setToolTip( tip_logdir );
   dirLogSave->widget()->setToolTip( tip_logdir );
}


/*!
  Allows user to select a general font
*/
void ValkyrieOptionsPage::chooseGenFont()
{
   LeWidget* fontLedit = (( LeWidget* )m_itemList[VALKYRIE::FNT_GEN_USR] );
   
   QFont afont;
   afont.fromString( fontLedit->currValue() );
   bool ok;
   QFont font = QFontDialog::getFont( &ok, afont, this, "Select General Font" );
   
   if ( ok ) {
      fontLedit->setValue( font.toString() );
   }
}


/*!
  Allows user to select a tool font
*/
void ValkyrieOptionsPage::chooseToolFont()
{
   LeWidget* fontLedit = (( LeWidget* )m_itemList[VALKYRIE::FNT_TOOL_USR] );
   
   QFont afont;
   afont.fromString( fontLedit->currValue() );
   bool ok;
   QFont font = QFontDialog::getFont( &ok, afont, this, "Select Tool Font" );
   
   if ( ok ) {
      fontLedit->setValue( font.toString() );
   }
}


/*!
  Allows user to select the source editor
*/
void ValkyrieOptionsPage::getEditor()
{
   // get path of current editor
   QString ed_curr = m_itemList[VALKYRIE::SRC_EDITOR]->currValue();
   if ( !ed_curr.isEmpty() )
      ed_curr = ed_curr.split( " ", QString::SkipEmptyParts ).first();
   
   QString ed_new = vkDlgGetFile( this, ed_curr );
   
   if ( !ed_new.isEmpty() ) { // user might have clicked Cancel
      (( LeWidget* )m_itemList[VALKYRIE::SRC_EDITOR] )->setValue( ed_new );
      // update triggers checkOption()
   }
}


/*!
  Allows user to select executable-to-debug
*/
void ValkyrieOptionsPage::getBinary()
{
   QString bin_curr = m_itemList[VALKYRIE::BINARY]->currValue();

   QString bin_new = vkDlgGetFile( this, bin_curr );
   
   if ( !bin_new.isEmpty() ) {   // user might have clicked Cancel
      (( LeWidget* )m_itemList[VALKYRIE::BINARY] )->setValue( bin_new );
      // update triggers checkOption()
   }
}


/*!
  Allows user to select the default browser
*/
void ValkyrieOptionsPage::getBrowser()
{
   QString browser_curr = m_itemList[VALKYRIE::BINARY]->currValue();

   QString browser_new = vkDlgGetFile( this, browser_curr );

   if ( !browser_new.isEmpty() ) { // user might have clicked Cancel
      (( LeWidget* )m_itemList[VALKYRIE::BROWSER] )->setValue( browser_new );
      // update triggers checkOption()
   }
}


/*!
  Allows user to specify which valgrind version to use.
*/
void ValkyrieOptionsPage::getVgExec()
{
   QString vg_curr = m_itemList[VALKYRIE::VG_EXEC]->currValue();

   QString vg_new = vkDlgGetFile( this, vg_curr );
                                    
   if ( !vg_new.isEmpty() ) { // user might have clicked Cancel
      (( LeWidget* )m_itemList[VALKYRIE::VG_EXEC] )->setValue( vg_new );
      // update triggers checkOption()
   }
}


/*!
  Allows user to specify which default log dir to use
*/
void ValkyrieOptionsPage::getDfltLogDir()
{
   QString dir_curr = m_itemList[VALKYRIE::DFLT_LOGDIR]->currValue();

   QString dir_new = vkDlgGetDir( this, dir_curr );
   
   if ( !dir_new.isEmpty() ) { // user might have clicked Cancel
      (( LeWidget* )m_itemList[VALKYRIE::DFLT_LOGDIR] )->setValue( dir_new );
      // update triggers checkOption()
   }
}

/*!
  Allows user to specify which dir to run valgrind under
*/
void ValkyrieOptionsPage::getWorkingDir()
{
   QString dir_curr = m_itemList[VALKYRIE::WORKING_DIR]->currValue();

   QString dir_new = vkDlgGetDir( this, dir_curr );
      
   if ( !dir_new.isEmpty() ) { // user might have clicked Cancel
      (( LeWidget* )m_itemList[VALKYRIE::WORKING_DIR] )->setValue( dir_new );
      // update triggers checkOption()
   }
}
