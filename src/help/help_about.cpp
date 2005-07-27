/* ---------------------------------------------------------------------
 * Implementation of HelpAbout                            help_about.cpp
 * small tab dialog showing various information re licence etc.
 * --------------------------------------------------------------------- 
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "valkyrie_xpm.h"
#include "help_about.h"
#include "vk_config.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>


/* class TextEdit ------------------------------------------------------ */
TextEdit::~TextEdit() { }

TextEdit::TextEdit( QWidget* parent, HelpAbout::TabId tabid, 
                    const char* name ) 
  : QTextEdit( parent, name )
{
  tabId = tabid;
  loaded = false;
  html_file.sprintf("%s%s.html", vkConfig->vkdocDir().latin1(), name );
  setReadOnly( true );
  setTextFormat( Qt::RichText );
}

bool TextEdit::load()
{
  if ( loaded )
    return true;

  if ( !QFile::exists( html_file ) )
    return false;

  QFile file( html_file );
  if ( !file.open( IO_ReadOnly ) )
    return false;

  setUpdatesEnabled( false );

  QTextStream ts( &file );
  switch( tabId ) {
    case HelpAbout::ABOUT_VK:
    case HelpAbout::SUPPORT:
      setText( ts.read() );
      break;
    case HelpAbout::ABOUT_QT:
      setText( ts.read().arg(qVersion()) );
      break;
    case HelpAbout::LICENCE:
      setText( ts.read() );
      break;
  }

  setUpdatesEnabled( true );
  loaded = true;

  return true;
}


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
  aboutVk = new TextEdit( tabParent, ABOUT_VK, "about_vk" );
  str.sprintf( "About %s", vkConfig->vkName() );
  tabParent->insertTab( aboutVk, str, ABOUT_VK );

  /* about qt tab */
  aboutQt = new TextEdit( tabParent, ABOUT_QT, "about_qt" );
  tabParent->insertTab( aboutQt, "About Qt", ABOUT_QT );

  /* licence tab */
  licence = new TextEdit( tabParent, LICENCE, "licence" );
  tabParent->insertTab( licence, "Licence Agreement", LICENCE );
  
  /* support tab */
  support = new TextEdit( tabParent, SUPPORT, "support" );
  tabParent->insertTab( support, "Support", SUPPORT );

  QPushButton* okButt = new QPushButton( "Close", this);
  okButt->setDefault( true );
  okButt->setFixedSize( okButt->sizeHint() );
  connect( okButt, SIGNAL(clicked() ), this, SLOT(accept()));
  okButt->setFocus();
  vbox->addWidget( okButt, 0, Qt::AlignRight );

  /* start up with the correct page loaded */
  tabParent->setCurrentPage( tabid );
  /* and start up at a reasonable size */
  resize( 420, 350 );
}

void HelpAbout::showTab( QWidget* tab )
{
  TabId tabid = (TabId)tabParent->indexOf( tab );

  switch ( tabid ) {
    case ABOUT_VK: aboutVk->load(); break;
    case ABOUT_QT: aboutQt->load(); break;
    case LICENCE:  licence->load(); break;
    case SUPPORT:  support->load(); break;
  }

  setCaption( title + tabParent->label( tabid) );
}
