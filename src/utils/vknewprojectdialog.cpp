/****************************************************************************
** VkNewProjectDialog implementation
** --------------------------------------------------------------------------
**
** Copyright (C) 2011-2011, OpenWorks LLP. All rights reserved.
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

#include "utils/vknewprojectdialog.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"
#include "utils/vk_config.h"

#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#define LBL_STYLE_WARN "QLabel { background-color : red; color : black; }"
#define EDIT_STYLE_WARN "QLineEdit { color: red }"


VkNewProjectDialog::VkNewProjectDialog( QWidget* parent )
   : QDialog( parent )
{
   // ------------------------------------------------------------
   setObjectName( QString::fromUtf8( "VkNewProjectDialog" ) );
   setWindowTitle( "Create New Valkyrie Project" );

   QVBoxLayout* topVLayout = new QVBoxLayout( this );

   // top text
   QLabel* topText = new QLabel( this );
   topText->setText( "Enter a new project name and the directory under which it should be created." );
   topVLayout->addWidget( topText );

   // ------------------------------------------------------------
   // group box
   QGroupBox* groupbox = new QGroupBox( this );
   groupbox->setObjectName( QString::fromUtf8( "groupbox" ) );
   topVLayout->addWidget( groupbox );
   
   // ------------------------------------------------------------
   // grid
   QGridLayout* grid = new QGridLayout( groupbox );
   grid->setObjectName( QString::fromUtf8( "grid" ));
   
   // ------------------------------------------------------------
   // content
   QLabel* lbl_name = new QLabel( "Name: ", groupbox );
   edit_name = new QLineEdit( groupbox );
   connect( edit_name, SIGNAL(textEdited(QString)), this, SLOT( checkInput() ) );

   QLabel* lbl_dir = new QLabel( "Create in: ", groupbox );
   edit_dir = new QLineEdit( groupbox );
   connect( edit_dir, SIGNAL(textEdited(QString)), this, SLOT( checkInput() ) );

   QPushButton* butt_dir = new QPushButton( "Browse...", groupbox );
   connect( butt_dir, SIGNAL( clicked() ), this, SLOT( browseDir() ) );

   lbl_warn = new QLabel( groupbox );

   grid->addWidget( lbl_name,  0, 0, 1, 1 );
   grid->addWidget( edit_name, 0, 1, 1, 2 );
   grid->addWidget( lbl_dir,   1, 0, 1, 1 );
   grid->addWidget( edit_dir,  1, 1, 1, 1 );
   grid->addWidget( butt_dir,  1, 2, 1, 1 );
   grid->addWidget( lbl_warn,  2, 0, 1, 3 );

   // ------------------------------------------------------------
   // buttons
   buttonBox = new QDialogButtonBox( this );
   buttonBox->setObjectName( QString::fromUtf8( "buttonBox" ) );
   buttonBox->setOrientation( Qt::Horizontal );
   buttonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
   connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) ); // Cancel
   connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) ); // Ok
   topVLayout->addWidget( buttonBox );
   

   // ------------------------------------------------------------
   // defaults
   QFileInfo fi( vkCfgGlbl->value( "project_path" ).toString() );
   QString start_dir = QDir::currentPath();
   if ( fi.exists() ) {
      // looks like a bug in Qt: if dir, absolutePath still takes off the last dir!
      start_dir = fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath();
   }
   edit_dir->setText( start_dir );
   
   edit_name->setText( "untitled" );

   edit_style_normal = edit_dir->styleSheet();
   lbl_style_normal = lbl_warn->styleSheet();
}

void VkNewProjectDialog::accept()
{
   QFileInfo fi( getProjectPath() );
   if ( fi.exists() ) {
      int ok = vkQuery( this, "Project already exists", "&Yes;&No",
                        "<p>A project with the same name was found in the given directory.</p>"
                        "<p>Do you want to overwrite it?</p>" );

      if ( ok == MsgBox::vkNo ) {
         return; // try again
      }
      
   }

   // save chosen path to cfg
   vkCfgGlbl->setValue( "project_path", getProjectPath() );

   QDialog::accept();
}

void VkNewProjectDialog::checkInput()
{
   QString warning = "";
   bool name_ok = true;
   bool dir_ok = true;

   // check name
   QString name = edit_name->text();
   if ( name.isEmpty() ) {
      warning = QString( "Name is empty.");
      name_ok = false;
   }
   else {
      int idx = name.lastIndexOf( QRegExp("[^a-zA-Z0-9_-]") );
      if ( idx != -1 ) {
         warning = QString( "invalid character: '%1'").arg( name.at( idx ) );
         name_ok = false;
      }
   }

   // check path - overrides projectname warning.
   QString path = edit_dir->text();
   QFileInfo fi( path );
   if ( !fi.exists() ) {
      warning = QString( "The path '%1' does not exist." ).arg( path );
      dir_ok = false;
   }

   // based on warning string, set the warnings on/off
   edit_dir->setStyleSheet( dir_ok ? edit_style_normal : EDIT_STYLE_WARN );
   edit_name->setStyleSheet( name_ok ? edit_style_normal : EDIT_STYLE_WARN );
   if ( dir_ok && name_ok ) {
      lbl_warn->setStyleSheet( lbl_style_normal );
      lbl_warn->setText( "" );
      buttonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
   }
   else {
      lbl_warn->setStyleSheet( LBL_STYLE_WARN );
      lbl_warn->setText( warning );
      buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
   }
}

void VkNewProjectDialog::browseDir()
{
   QString dir_new = vkDlgGetDir( this, edit_dir->text() );
   
   if ( !dir_new.isEmpty() ) { // user might have clicked Cancel
      edit_dir->setText( dir_new );
   }
}

QString VkNewProjectDialog::getProjectPath()
{
   return edit_dir->text() + "/" + edit_name->text() + "." + VkCfg::filetype();
}
