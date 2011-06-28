/****************************************************************************
** VkOptionsDialog definition
**  - A container class for each tool's options / flags 'pane'.
**  - Not modal, so user can keep it open and change flags as they work.
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

#ifndef __VK_OPTIONS_DIALOG_H
#define __VK_OPTIONS_DIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QStackedWidget>
#include <QWidget>


// ============================================================
class VkOptionsDialog : public QDialog
{
   Q_OBJECT
public:
   VkOptionsDialog( QWidget* );
   ~VkOptionsDialog();
   
   // setup and return new current page
   QWidget* setCurrentPage( int idx );
   
private:
   void setupLayout();
   void keyPressEvent( QKeyEvent* event ); // overloaded

private slots:
   bool apply();
   void accept();  // overloaded
   void reject();  // overloaded
   void showPage();
   void pageModified();
   void overwriteDefaultConfig();

signals:
   void flagsChanged();
   
private:
   QListWidget*      contentsListWidget;
   QStackedWidget*   optionPages;
   QDialogButtonBox* optionsButtonBox;
   QPushButton*      updateDefaultsButton;
};


#endif  // __VK_OPTIONS_DIALOG_H
