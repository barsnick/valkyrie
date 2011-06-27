/****************************************************************************
** VkSuppressionsDialog definition
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

#ifndef VK_SUPPRESSIONS_DIALOG_H
#define VK_SUPPRESSIONS_DIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>


// ============================================================
class SuppFrame : public QWidget
{
   Q_OBJECT
public:
   SuppFrame( bool isFirst, QWidget* parent=0 );

signals:
   void removeFrame( SuppFrame* w );

private:
   SuppFrame();
   
private slots:
   void buttClicked();
   
public:
   QComboBox* frame_cmb;
   QLineEdit* frame_le;
   QPushButton* frame_but;
};


// ============================================================
class Suppression;
class VkSuppressionsDialog : public QDialog
{
   Q_OBJECT
public:
   VkSuppressionsDialog(QWidget *parent = 0);

   void setSupp( const Suppression& supp );
   const Suppression getUpdatedSupp();
   
private:
   void setupLayout();

private slots:
   void addNewSuppFrame();
   void removeSuppFrame( SuppFrame* w );
   void ToolChanged( int idx );
   void TypeChanged( int idx );

private:
   QVBoxLayout* callChainLayout;

   QLineEdit* name_le;
   QComboBox* tool_cmb;
   QComboBox* type_cmb;
   QLabel* kaux_lbl;
   QLineEdit* kaux_le;
   QList<SuppFrame*> suppFrames;
};

#endif // VK_SUPPRESSIONS_DIALOG_H
