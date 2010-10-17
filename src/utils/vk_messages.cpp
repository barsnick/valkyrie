/****************************************************************************
** Definition of class MsgBox
**  - various types of messages: Query, Info ...
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
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

#include "utils/vk_messages.h"
#include "utils/vk_config.h"

#include <stdarg.h>           /* va_start, va_end */
#include <stdlib.h>           /* exit errno */
#include <stdio.h>            /* vsnprintf */

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QObjectList>
#include <QPixmap>
#include <QStyle>


/***************************************************************************/
/*!
  message dialogs
*/
#define VK_BUFLEN 8196

/*!
  vkQuery: ask user a question
  Modal dialog box
*/
int vkQuery( QWidget* w, int nbutts, QString hdr,
             const char* msg, ... )
{
   // setup message
   char buf[VK_BUFLEN];
   va_list ap;
   va_start( ap, msg );
   vsnprintf( buf, VK_BUFLEN, msg, ap );
   va_end( ap );
   
   // setup and show dialog box
   MsgBox mb( w, MsgBox::Query, buf, hdr, nbutts );
   QStringList names;
   names << "&Yes" << "&No" << "&Cancel";
   mb.setButtonTexts( names );
   return mb.exec();
}


/*!
  vkQuery: ask user a question + set custom button labels
  Modal dialog box
*/
int vkQuery( QWidget* w, QString hdr,
             QString labels, const char* msg, ... )
{
   // setup message
   char buf[VK_BUFLEN];
   va_list ap;
   va_start( ap, msg );
   vsnprintf( buf, VK_BUFLEN, msg, ap );
   va_end( ap );
   
   // setup and show dialog box
   QStringList buttonLabels( labels.split( ";", QString::SkipEmptyParts ) );
   int nbutts = buttonLabels.count();
   MsgBox mb( w, MsgBox::Query, buf, hdr, nbutts );
   mb.setButtonTexts( buttonLabels );
   return mb.exec();
}


/*!
  vkInfo: show information message
  Modal dialog box
*/
void vkInfo( QWidget* w, QString hdr, const char* msg, ... )
{
   // setup message
   char buf[VK_BUFLEN];
   va_list ap;
   va_start( ap, msg );
   vsnprintf( buf, VK_BUFLEN, msg, ap );
   va_end( ap );
   
   // setup and show dialog box
   MsgBox mb( w, MsgBox::Info, buf, hdr, 1 );
   mb.setButtonTexts( QStringList( "O&K" ) );
   mb.exec();
}


/*!
  vkError: error message box
  Modal dialog box
*/
void vkError( QWidget* w, QString hdr, const char* msg, ... )
{
   // setup message
   char buf[VK_BUFLEN];
   va_list ap;
   va_start( ap, msg );
   vsnprintf( buf, VK_BUFLEN, msg, ap );
   va_end( ap );
   
   // setup and show dialog box
   MsgBox mb( w, MsgBox::Error, buf, hdr );
   mb.setButtonTexts( QStringList( "O&K" ) );
   mb.exec();
}





/***************************************************************************/
/*!
  Base dialog box implementation
*/
MsgBox::MsgBox( QWidget* parent, Icon icon, QString msg,
                const QString& hdr, int num_buttons )
   : QDialog( parent )
{
   setObjectName( QString::fromUtf8( "msgbox" ) );
   
   msg = "<p>" + msg + "</p>";
   
   if ( !hdr.isEmpty() ) {
      msg = "<b>" + hdr + "</b>" + msg;
   }
   
   numButtons = num_buttons;
   int defButton  = 0;
   button[0]  = MsgBox::vkYes;
   
   QString caption;
   QPixmap pm_file;
   
   switch ( icon ) {
   case Query:
      caption = "Query";
      defButton = 1;  /* No */
      pm_file = QPixmap( QString::fromUtf8( ":/vk_icons/icons/msgbox_query.xpm" ) );
      break;
   case Info:
      caption = "Information";
      pm_file = QPixmap( QString::fromUtf8( ":/vk_icons/icons/msgbox_info.xpm" ) );
      break;
   case Error:
      caption = "Error";
      pm_file = QPixmap( QString::fromUtf8( ":/vk_icons/icons/msgbox_error.xpm" ) );
      break;
   default:
      break;
   }
   
   setWindowTitle( caption );
   
   switch ( numButtons ) {
   case 1:        // ok button
      escButton = -1;
      break;
   case 2:        // yes + no
      button[1] = MsgBox::vkNo;
      escButton = 1;
      break;
   case 3:       // yes + no + cancel
      button[1] = MsgBox::vkNo;
      button[2] = MsgBox::vkCancel;
      escButton = 2;
      break;
   }
   
   
   QVBoxLayout* vLayout = new QVBoxLayout( this );
   vLayout->setObjectName( QString::fromUtf8( "vlayout" ) );
   vLayout->setAlignment( Qt::AlignTop );
   
   QHBoxLayout* hLayout = new QHBoxLayout();
   hLayout->setObjectName( QString::fromUtf8( "hlayout" ) );
   hLayout->setSpacing( 14 );
   vLayout->addLayout( hLayout );
   
   iconLabel = new QLabel( this );
   iconLabel->setObjectName( "icon_lbl" );
   iconLabel->setAlignment( Qt::AlignTop );
   iconLabel->setPixmap( pm_file );
   hLayout->addWidget( iconLabel );
   
   msgLabel = new QLabel( this );  // default: left aligned, tabs expanded.
   msgLabel->setObjectName( "msg_lbl" );
   msgLabel->setText( msg );
   hLayout->addWidget( msgLabel );
   hLayout->addStretch( 1 );
   
   
   QHBoxLayout* hLayoutButtons = new QHBoxLayout();
   hLayoutButtons->setObjectName( QString::fromUtf8( "hLayoutButtons" ) );
   hLayoutButtons->addStretch( 1 );
   vLayout->addSpacing( 18 );
   vLayout->addStretch( 1 );
   vLayout->addLayout( hLayoutButtons );
   
   for ( int i = 0; i < numButtons; i++ ) {
      pb[i] = new QPushButton( this );
      pb[i]->setObjectName( "button" + QString::number( i + 1 ) );
      hLayoutButtons->addWidget( pb[i] );
      
      if ( defButton == i ) {
         pb[i]->setDefault( true );
         pb[i]->setFocus();
      }
      
      pb[i]->setAutoDefault( true );
      connect( pb[i], SIGNAL( clicked() ),
               this,  SLOT( pbClicked() ) );
   }
}



MsgBox::~MsgBox()
{ }



void MsgBox::setButtonTexts( const QStringList& btexts )
{
   for ( int i = 0; i < numButtons; i++ ) {
      pb[i]->setText( btexts[i] );
   }
}


void MsgBox::pbClicked()
{
   int result = 0;
   const QObject* s = sender();
   
   for ( int i = 0; i < numButtons; i++ ) {
      if ( pb[i] == s ) {
         result = button[i];
         break;
      }
   }
   
   done( result );
}


// An escape key triggers the assigned 'default-escape' button
void MsgBox::keyPressEvent( QKeyEvent* e )
{
   if ( e->key() == Qt::Key_Escape ) {
      if ( escButton >= 0 ) {
         pb[escButton]->animateClick();
         e->accept();
         return;
      }
   }
   
   QDialog::keyPressEvent( e );
}

// Any close event other than via pbClicked() gives 'default-escape' button result.
void MsgBox::closeEvent( QCloseEvent* ce )
{
   QDialog::closeEvent( ce );
   
   if ( escButton != -1 ) {
      setResult( button[escButton] );
   }
}




