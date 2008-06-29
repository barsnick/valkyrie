/* ---------------------------------------------------------------------- 
 * Definition of class MsgBox                                 vk_msgbox.h
 * Various types of messages: Query, Warn, Fatal, Info ...
 * ----------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#ifndef __VK_MESSAGES_H
#define __VK_MESSAGES_H

#include <qdialog.h>
#include <qlabel.h>



class MsgBox : public QDialog
{
   Q_OBJECT
public:
   enum Icon { Query=0, Info=1, Warning=2, Error=3, Fatal=4, About=5 };
   enum rVal { vkYes=0, vkNo=1, vkCancel=2 };

   MsgBox( QWidget* parent, Icon icon, QString msg, 
           const QString& hdr=QString::null, int num_buttons=1 );
   ~MsgBox();

   static int  query( QWidget* parent, QString hdr, QString msg, 
                      int nbutts=2 );
   static int  query( QWidget* parent, QString hdr, QString msg,
                      QString buttonNames );
   static void info( QWidget* parent, QString hdr, QString msg );
   static int warning( QWidget* parent, QString hdr, QString msg );
   static void error( QWidget* parent, QString hdr, QString msg );
   static void fatal( QWidget* parent, QString hdr, QString msg );
   static void about( QWidget* parent );

   void setButtonTexts( const QStringList &texts );

protected:
   void resizeEvent( QResizeEvent * );
   void closeEvent( QCloseEvent * );
   void keyPressEvent( QKeyEvent * );

private slots:
   void pbClicked();

private:
   void adjustSize();
   void resizeButtons();

private:
   int minWidth;

   enum { ButtonMask=0xff, Default=0x100, Escape=0x200 };
   int button[3];          /* button types           */
   int defButton;          /* default button (index) */
   int escButton;          /* escape button (index)  */
   int numButtons;         /* number of buttons      */

   QSize buttonSize;       /* button size            */
   QLabel* msgLabel;       /* label holding the message text */
   QLabel* iconLabel;      /* label holding any icon */
   QPushButton *pb[3];     /* buttons                */
};


/* message handling fns, defined in vk_msgbox.cpp ---------------------- */

/* show information message box */
extern void vkInfo( QWidget* w, QString hdr, 
                    const char* format, ...) 
     __attribute__((__format__ (__printf__, 3, 4)));

/* show fatal message and exit */
extern int vkFatal( QWidget* w, QString hdr,
                         const char* format, ...) 
     __attribute__((__format__ (__printf__, 3, 4)));

/* ask user a question */
extern int vkQuery( QWidget* w, int nbutts,
                    QString hdr, const char*, ... )
     __attribute__ (( format( printf, 4, 5 ) ));

/* ask user a question, + set custom button labels.
   Note button ordering: 
   - button[0] = vkYes;
   - button[1] = vkNo;
   - button[2] = vkCancel | MsgBox::Escape;
*/
extern int vkQuery( QWidget* w, QString hdr, 
                    QString butt_labels, const char*, ... )
     __attribute__ (( format( printf, 4, 5 ) ));

/* warning message box */
extern int vkWarn( QWidget* w, QString hdr,
                   const char*, ... )
     __attribute__ (( format( printf, 3, 4 ) ));

/* error message box */
extern void vkError( QWidget* w, QString hdr,
                     const char*, ... )
     __attribute__ (( format( printf, 3, 4 ) ));


#endif
