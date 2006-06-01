/* ---------------------------------------------------------------------
 * Implementation of HelpAbout                            help_about.cpp
 * small tab dialog showing various information re license etc.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "valkyrie_xpm.h"
#include "help_about.h"
#include "vk_config.h"
#include "vk_utils.h"

#include <qlabel.h>
#include <qpushbutton.h>


/* class HelpAbout ----------------------------------------------------- */
HelpAbout::~HelpAbout() { }

HelpAbout::HelpAbout(  QWidget* parent, TabId tabid )  
   : QDialog( parent, "help_about", true )
{
   title.sprintf( "%s Information", vkConfig->vkName() );
   setCaption( title );

   /* top layout */
   QVBoxLayout* vbox = new QVBoxLayout( this, 10 );

   /* hbox for pic + appname */
   QHBoxLayout* hbox = new QHBoxLayout( vbox, 10 );

   /* pic */
   QLabel* pic = new QLabel( this );
   pic->setPixmap( QPixmap(valkyrie_xpm) );
   pic->setFixedSize( pic->sizeHint() );
   hbox->addWidget( pic );

   /* app info */
   QString str; 
   QLabel* appName = new QLabel( this );
   str.sprintf( "%s %s", vkConfig->vkName(), vkConfig->vkVersion() );
   appName->setText( str );
   appName->setFont(QFont( "Times", 18, QFont::Bold) );
   appName->setFixedSize( appName->sizeHint() );
   hbox->addWidget( appName );

   /* push the pix+info over to the left */
   hbox->addStretch( 10 );

   /* tabwidget */
   tabParent = new QTabWidget( this );
   tabParent->setMargin( 10 );
   connect( tabParent, SIGNAL(currentChanged( QWidget* )),
            this,      SLOT(showTab( QWidget* )) );
   vbox->addWidget( tabParent, 0 );
    
   /* about_vk tab */
   aboutVk = new VkTextBrowser( tabParent, "about_vk" );
   str.sprintf( "About %s", vkConfig->vkName() );
   tabParent->insertTab( aboutVk, str, ABOUT_VK );

   /* about qt tab */
   aboutQt = new VkTextBrowser( tabParent, "about_qt" );
   tabParent->insertTab( aboutQt, "About Qt", ABOUT_QT );

   /* license tab */
   license = new VkTextBrowser( tabParent, "about_gpl" );
   tabParent->insertTab( license, "License Agreement", LICENSE );
  
   /* support tab */
   support = new VkTextBrowser( tabParent, "support" );
   tabParent->insertTab( support, "Support", SUPPORT );

   QPushButton* okButt = new QPushButton( "Close", this);
   okButt->setDefault( true );
   okButt->setFixedSize( okButt->sizeHint() );
   connect( okButt, SIGNAL(clicked() ), this, SLOT(accept()));
   okButt->setFocus();
   vbox->addWidget( okButt, 0, Qt::AlignRight );

   /* setup text-browsers */
   for (int i=0; i<NUM_TABS; ++i) {
      VkTextBrowser* page = (VkTextBrowser*)tabParent->page(i);
      page->setLinkUnderline( true );
      page->mimeSourceFactory()->setFilePath( QStringList() << vkConfig->vkdocDir() );
   }
   aboutVk->setSource( vkConfig->vkdocDir() + "about_vk.html" );
   license->setSource( vkConfig->vkdocDir() + "about_gpl.html" );
   support->setSource( vkConfig->vkdocDir() + "support.html" );
   /* about_qt source needs version args updating: */
   QFile file( vkConfig->vkdocDir() + "about_qt.html" );
   if ( file.open( IO_ReadOnly ) ) {
      QTextStream ts( &file );
      aboutQt->setText( ts.read().arg(QT_VERSION_STR).arg(qVersion()) );
   }

   /* start up with the correct page loaded */
   tabParent->setCurrentPage( tabid );
   /* and start up at a reasonable size */
   resize( 420, 350 );
}

void HelpAbout::showTab( QWidget* tab )
{
   TabId tabid = (TabId)tabParent->indexOf( tab );
   setCaption( title + " : " + tabParent->label( tabid ) );
}
