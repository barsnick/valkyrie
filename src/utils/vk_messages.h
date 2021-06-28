/****************************************************************************
** Definition of class MsgBox
**  - various types of messages: Query, Info ...
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

#ifndef __VK_MESSAGES_H
#define __VK_MESSAGES_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QWidget>



// ============================================================
/* ask user a question */
int vkQuery( QWidget* w, int nbutts, QString hdr, const char*, ... )
__attribute__(( format( printf, 4, 5 ) ) );

/* ask user a question, + set custom button labels.
   Note button ordering - vkYes : vkNo : vkCancel */
int vkQuery( QWidget* w, QString hdr, QString butt_labels, const char*, ... )
__attribute__(( format( printf, 4, 5 ) ) );

/* show information message box */
void vkInfo( QWidget* w, QString hdr, const char* format, ... )
__attribute__(( __format__( __printf__, 3, 4 ) ) );

/* error message box */
void vkError( QWidget* w, QString hdr, const char*, ... )
__attribute__(( format( printf, 3, 4 ) ) );




// ============================================================
// Don't call this class directly: used for standard vkError() etc. functions
class MsgBox : public QDialog
{
   Q_OBJECT
public:
   enum Icon { Query, Info, Error };
   enum rVal { vkYes = 0, vkNo = 1, vkCancel = 2 };
   
   MsgBox( QWidget* parent, Icon icon, QString msg,
           const QString& hdr = QString(), int num_buttons = 1 );
   ~MsgBox();
   
   void setButtonTexts( const QStringList& texts );
   
protected:
   void closeEvent( QCloseEvent* );
   void keyPressEvent( QKeyEvent* );
   
private slots:
   void pbClicked();
   
private:
   int button[3];               /* button types           */
   int escButton;               /* escape button (index)  */
   int numButtons;              /* number of buttons      */
   
   QLabel* msgLabel;            /* label holding msg text */
   QLabel* iconLabel;           /* label holding any icon */
   QPushButton* pb[3];          /* buttons                */
};

#endif
