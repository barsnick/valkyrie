/****************************************************************************
** VkOptionsDialog implementation
**  - A container class for each tool's options / flags 'pane'.
**  - Not modal, so user can keep it open and change flags as they work.
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
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

#include "help/help_context.h"
#include "help/help_urls.h"
#include "mainwindow.h"
#include "objects/vk_objects.h"
#include "options/vk_options_dialog.h"
#include "options/vk_options_page.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"

#include <QApplication>
#include <QLabel>
#include <QPushButton>



/***************************************************************************/
/*!
    Constructs a VkOptionsDialog
*/
VkOptionsDialog::VkOptionsDialog( QWidget* parent )
   : QDialog( parent )
{
   // ------------------------------------------------------------
   // basic dialog setup
   setObjectName( QString::fromUtf8( "VkOptionsDialog" ) );
   setWindowTitle( "[*]Valkyrie Options Dialog" ); // [*] == 'windowModified' placeholder
   setupLayout();
   
   // ------------------------------------------------------------
   // Add categories, and the pages
   // Note: both the pages and categories list use the same 'index',
   // which is how we keep them in sync.
   // TODO: if any complaints re speed, load the pages on demand.
   VkObjectList objList = (( MainWindow* )parent )->getValkyrie()->vkObjList();
   
   for ( int i = 0; i < objList.size(); ++i ) {
   
      // Allow the VkObject to create the appropriate options page
      // Pass 'this' so constructor widgets auto-size correctly.
      VkObject* obj = objList.at( i );
      VkOptionsPage* page = obj->createVkOptionsPage();
      vk_assert( page != 0 );
      page->init();
      connect( page, SIGNAL( modified() ), this, SLOT( pageModified() ) );
      // handle e.g. user pressing return in an ledit
      connect( page, SIGNAL( apply() ), this, SLOT( apply() ) );
      
      // Set list item entry
      QListWidgetItem* item = new QListWidgetItem( contentsListWidget );
      QString itemName = obj->objectName();
      itemName[0] = itemName[0].toUpper();
      item->setText( itemName );

      QFont font = item->font();
      font.setBold( true );
      font.setPointSize( font.pointSize() * 1.2 );
      item->setFont( font );

      // insert into stack (takes ownership)
      optionPages->addWidget( page );
   }
   
   contentsListWidget->setCurrentRow( 0 );
   contentsListWidget->setFocus();
   optionPages->setCurrentIndex( 0 );
   
   // Give a max to our contentsList, based on hints from the list-items.
   // TODO: surely this can be done automatically?
   // - QSizePolicy::* don't seem to do the job :-(
   contentsListWidget->setMaximumWidth( 40 + contentsListWidget->sizeHintForColumn( 0 ) );
   
   ContextHelp::addHelp( this, urlValkyrie::optsDlg );
}


/*!
  Nothing to cleanup: Qt's object-parenting does it all for us.
*/
VkOptionsDialog::~VkOptionsDialog()
{
}


/*!
  A return/enter keypress in an option widget isn't eaten up by that
  widgets' event handler - it's propogated to the QDialog parent.
  Enter/Return keypresses will then call the QDialog default-button,
  which is the 'Ok' button.

  We don't know for sure if changes have been applied already, and
  if the edits failed, we don't want to leave.

  Until this is worked out better, we're just going to ignore these
  events: shortcuts & mouse are then the only way to accept().
  TODO: better way of doing this?
*/
void VkOptionsDialog::keyPressEvent( QKeyEvent* event )
{
   if ( event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter ) {
       // eat keypress return/enter event, so dialog doesn't close
       return;
   }
   QDialog::keyPressEvent( event );
}


/*!
  Setup our basic widget layout, ready for the pages
*/
void VkOptionsDialog::setupLayout()
{
   // ------------------------------------------------------------
   // top layout
   QVBoxLayout* vLayout = new QVBoxLayout( this );
   vLayout->setObjectName( QString::fromUtf8( "vlayout" ) );
   
   // ------------------------------------------------------------
   // parent widget for the contents + pages
   QWidget* hLayoutWidget = new QWidget( this );
   hLayoutWidget->setObjectName( QString::fromUtf8( "hLayoutWidget" ) );
   vLayout->addWidget( hLayoutWidget );
   
   // ------------------------------------------------------------
   // contents + pages layout
   QHBoxLayout* hLayout = new QHBoxLayout( hLayoutWidget );
   hLayout->setObjectName( QString::fromUtf8( "hLayout" ) );
   hLayout->setContentsMargins( 0, 0, 0, 0 );
   
   // ------------------------------------------------------------
   // The contents list
   // Note: give this its maximum width once filled with items
   contentsListWidget = new QListWidget( hLayoutWidget );
   contentsListWidget->setObjectName( QString::fromUtf8( "contentsListWidget" ) );
   contentsListWidget->setEditTriggers( QAbstractItemView::NoEditTriggers );
   contentsListWidget->setSortingEnabled( false );
   QSizePolicy sizePolicyContents( QSizePolicy::Maximum, QSizePolicy::Expanding );
   contentsListWidget->setSizePolicy( sizePolicyContents );
   contentsListWidget->setSelectionMode( QAbstractItemView::SingleSelection );
   hLayout->addWidget( contentsListWidget );
   
   optionPages = new QStackedWidget( hLayoutWidget );
   optionPages->setObjectName( QString::fromUtf8( "optionPages" ) );
   optionPages->setFrameShape( QFrame::StyledPanel );
   optionPages->setFrameShadow( QFrame::Raised );
   hLayout->addWidget( optionPages );
   
   
   // ------------------------------------------------------------
   // parent widget for the buttons
   QWidget* hButtonWidget = new QWidget( this );
   hButtonWidget->setObjectName( QString::fromUtf8( "hButtonWidget" ) );
   vLayout->addWidget( hButtonWidget );
   
   // ------------------------------------------------------------
   // options button box
   QHBoxLayout* hLayoutButtons = new QHBoxLayout( hButtonWidget );
   hLayoutButtons->setObjectName( QString::fromUtf8( "hLayoutButtons" ) );
   
   updateDefaultsButton = new QPushButton( QPixmap( ":/vk_icons/icons/filesave.png" ),
                                     "Save As Project Default" );
   hLayoutButtons->addWidget( updateDefaultsButton );
   hLayoutButtons->addStretch( 1 );
   
   optionsButtonBox = new QDialogButtonBox( hButtonWidget );
   optionsButtonBox->setObjectName( QString::fromUtf8( "optionsButtonBox" ) );
   optionsButtonBox->setOrientation( Qt::Horizontal );
   optionsButtonBox->setStandardButtons( QDialogButtonBox::Apply |
                                         QDialogButtonBox::Cancel |
                                         QDialogButtonBox::Ok );
   hLayoutButtons->addWidget( optionsButtonBox );
   
   // ------------------------------------------------------------
   // signals / slots
   connect( contentsListWidget, SIGNAL( itemSelectionChanged() ),
            this,                 SLOT( showPage() ) );

   QPushButton* applyButton  = optionsButtonBox->button( QDialogButtonBox::Apply );
   QPushButton* cancelButton = optionsButtonBox->button( QDialogButtonBox::Cancel );
   QPushButton* okButton     = optionsButtonBox->button( QDialogButtonBox::Ok );
   connect( applyButton,      SIGNAL( released() ), this, SLOT( apply()  ) ); // Apply
   connect( optionsButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) ); // Cancel
   connect( optionsButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) ); // Ok
   connect( updateDefaultsButton, SIGNAL( released() ), this, SLOT( overwriteDefaultConfig() ) );

   // ------------------------------------------------------------
   // setup default state
   applyButton->setEnabled( false );
   cancelButton->setEnabled( false );
   okButton->setDefault( true );
}


/*!
  Show the chosen options page.

  If any edits in last page, ask user what to do:
   - Accept edits and move on
   - Reject edits and move on
   - Cancel (i.e. cancel move to new page), and remain where we were.

  Note: have to use listwidget signal itemSelectionChanged(), to make
    sure the item has already been updated - else our update of
    current-item would get overridden.
*/
void VkOptionsDialog::showPage()
{
   int nextIdx = contentsListWidget->currentRow();
   int prevIdx = optionPages->currentIndex();
   //cerr << "itemSelectionChanged: old->new: " << prevIdx << "->" << nextIdx << endl;

   if ( nextIdx == prevIdx ) {
      // get here on startup, and whenever we revert to last list item
      // - easier than disconnecting signals etc.
      return;
   }

   // check no outstanding edits in prevItem.
   bool continueToNext = true;
   VkOptionsPage* prevPage = (VkOptionsPage*)optionPages->currentWidget();
   vk_assert( prevPage );

   if ( prevPage->isModified() ) {
      // choose to apply/reset edits
      QString pageName = contentsListWidget->item( prevIdx )->text();

      int ret_qry =
         vkQuery( this, "Apply/Reset Edits",
                  "&Apply;&Reset;&Cancel",
                  "<p>The <b><i>%s</i></b> option page has non-committed edits.<br/>"
                  "Would you like to <b>Apply</b> or <b>Reset</b> these edits?<br/>"
                  "Choose <b>Cancel</b> to stay on the edited page.</p>",
                  qPrintable( pageName ) );

      switch ( ret_qry ) {
      case MsgBox::vkYes:    apply(); break;                 // apply & move on
      case MsgBox::vkNo:     prevPage->rejectEdits(); break; // reject & move on
      case MsgBox::vkCancel: continueToNext = false; break;  // remain in prevPage
      default:
         vk_assert_never_reached();
      }
   }

   if ( continueToNext ) {
      // All done with last page: open next page
      optionPages->setCurrentIndex( nextIdx );
   }
   else {
      // Revert current item in contents list.
      // This will trigger this function to be called again, but with
      // nextidx == previdx, so easy to ignore.
      contentsListWidget->setCurrentRow( prevIdx );
   }
}


/*!
  reject edits
   - showPage() ensures only current page can be in an edited state.
*/
void VkOptionsDialog::reject()
{
   //   std::cerr << "VkOptionsDialog::reject()" << std::endl;
   
   VkOptionsPage* page = ( VkOptionsPage* )optionPages->currentWidget();
   vk_assert( page );

   page->rejectEdits();

   // close up shop.
   QDialog::reject();
}



/*!
   Apply edits
   The 'apply' button is important for look and feel options to be tried out
   Only current page can be in an edited state.
*/
bool VkOptionsDialog::apply()
{
   //   std::cerr << "VkOptionsDialog::apply()" << std::endl;
   
   VkOptionsPage* page = ( VkOptionsPage* )optionPages->currentWidget();
   vk_assert( page );
   
   if ( !page->applyEdits() ) {
      VK_DEBUG( "Failed to apply edits" );
      return false;
   }
   
   // ensure changes saved to disc
   vkCfgProj->sync();
   
   return true;
}


/*!
  Accept: apply edits and quit if no problems.
   - save the settings to the Project cfg file if specified
*/
void VkOptionsDialog::accept()
{
   //   std::cerr << "VkOptionsDialog::apply()" << std::endl;
   
   if ( apply() ) {
      // close up shop.
      QDialog::accept();
   }
   
   // Else, we have a problem.
   // Best to let the user know changes didn't get committed, and let them cancel.
}


/*!
  save applied edits to Default-Project-Config file
   - only enabled when no edits outstanding, to prevent any confusion.
*/
void VkOptionsDialog::overwriteDefaultConfig()
{
   int ok =
      vkQuery( this, 2, "Overwrite Default Config",
               "<p>Are you <b>sure</b> you want to overwrite the default project config ?</p>"
               "<p><i>Note: Valkyrie will regenerate factory default settings<br>"
               "if the default project config file is removed:<br>"
               "%s</i></p>", qPrintable( VkCfg::projDfltPath() ) );

   if ( ok == MsgBox::vkYes ) {
      vkCfgProj->saveToDefaultCfg();
   }
}


/*!
 Enable/disable buttons.
 This slot is called by page->modified() signal
  - only current page can have been modified.
*/
void VkOptionsDialog::pageModified()
{
   VkOptionsPage* page = ( VkOptionsPage* )optionPages->currentWidget();
   vk_assert( page );
   bool modified = page->isModified();
   
   QPushButton* applyButton  = optionsButtonBox->button( QDialogButtonBox::Apply );
   QPushButton* cancelButton = optionsButtonBox->button( QDialogButtonBox::Cancel );
   applyButton->setEnabled( modified );
   cancelButton->setEnabled( modified );
   // enable update-defaults only when no edits
   updateDefaultsButton->setEnabled( !modified );
   
   // updates the window title to indicate modified.
   this->setWindowModified( modified );
}
