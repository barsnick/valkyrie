/****************************************************************************
** HelpAbout implementation
**  - small tab dialog showing various information re license etc.
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

#include "help/help_about.h"
#include "utils/vk_config.h"
#include "utils/vk_utils.h"

#include <QFile>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QPushButton>
#include <QTextStream>

/*!
    Constructs a HelpAbout dialog with the given \a parent and \a tabid.
*/
HelpAbout::~HelpAbout() { }

HelpAbout::HelpAbout( QWidget* parent, HELPABOUT::TabId tabid )
   : QDialog( parent )
{
   setObjectName( QString::fromUtf8( "HelpAbout" ) );
   title = VkCfg::appName() + " Information";
   setWindowTitle( title );
   
   // top layout
   QVBoxLayout* vbox = new QVBoxLayout( this );
   vbox->setObjectName( QString::fromUtf8( "vbox" ) );
   
   // widget for the top part
   QWidget* hLayoutWidget = new QWidget( this );
   hLayoutWidget->setObjectName( QString::fromUtf8( "hLayoutWidget" ) );
   vbox->addWidget( hLayoutWidget );
   
   // hbox for pic + appname
   QHBoxLayout* hbox = new QHBoxLayout( hLayoutWidget );
   hbox->setObjectName( QString::fromUtf8( "hbox" ) );
   
   // pic
   QLabel* pic = new QLabel( this );
   pic->setPixmap( QPixmap( ":/vk_icons/icons/valkyrie.xpm" ) );
   pic->setFixedSize( pic->sizeHint() );
   hbox->addWidget( pic );
   
   // app info
   QLabel* appName = new QLabel( this );
   QString str = VkCfg::appName() + " v" + VkCfg::appVersion();
   appName->setText( str );
   appName->setFont( QFont( "Times", 18, QFont::Bold ) );
   appName->setFixedSize( appName->sizeHint() );
   hbox->addWidget( appName );
   
   // push the pix+info over to the left
   hbox->addStretch( 10 );
   
   // tabwidget
   tabParent = new QTabWidget( this );
   tabParent->setObjectName( QString::fromUtf8( "tabParent" ) );
   connect( tabParent, SIGNAL( currentChanged( int ) ),
            this,      SLOT( showTab( int ) ) );
   vbox->addWidget( tabParent );
   
   // about_vk tab
   aboutVk = new QTextBrowser( tabParent );
   aboutVk->setObjectName( QString::fromUtf8( "aboutVk" ) );
   QString VkName = VkCfg::appName();
   VkName.replace( 0, 1, VkName[0].toUpper() );
   str = "About " + VkName;
   tabParent->insertTab( HELPABOUT::ABOUT_VK, aboutVk, str );
   
   // license tab
   license = new QTextBrowser( tabParent );
   license->setObjectName( QString::fromUtf8( "license" ) );
   tabParent->insertTab( HELPABOUT::LICENSE, license, "License Agreement" );
   
   // support tab
   support = new QTextBrowser( tabParent );
   support->setObjectName( QString::fromUtf8( "support" ) );
   tabParent->insertTab( HELPABOUT::SUPPORT, support, "Support" );
   
   
   QPushButton* okButt = new QPushButton( "Close", this );
   okButt->setDefault( true );
   okButt->setFixedSize( okButt->sizeHint() );
   connect( okButt, SIGNAL( clicked() ), this, SLOT( accept() ) );
   okButt->setFocus();
   vbox->addWidget( okButt, 0, Qt::AlignRight );
   
   // setup text-browsers
   license->setSource( VkCfg::docDir() + "/about_gpl.html" );
   support->setSource( VkCfg::docDir() + "/support.html" );
   // about_vk source needs version args updating:
   QFile file( VkCfg::docDir() + "/about_vk.html" );
   
   if ( file.open( QIODevice::ReadOnly ) ) {
      QTextStream ts( &file );
      aboutVk->setText( ts.readAll().arg( QT_VERSION_STR ).arg( qVersion() ) );
   }
   
   // start up with the correct page loaded
   tabParent->setCurrentIndex( tabid );
   // and start up at a reasonable size
   resize( 600, 380 );
}

void HelpAbout::showTab( int idx )
{
   setWindowTitle( title + " : " + tabParent->tabText( idx ) );
}
