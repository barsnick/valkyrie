/****************************************************************************
** ValgrindOptionsPage definition
**  - subclass of VkOptionsPage to hold valgrind-specific options
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

#ifndef __VALGRIND_OPTIONS_PAGE_H
#define __VALGRIND_OPTIONS_PAGE_H

#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QStringList>

#include "options/vk_options_page.h"
#include "options/suppressions.h"


// ============================================================
class ValgrindOptionsPage : public VkOptionsPage
{
   Q_OBJECT
public:
   ValgrindOptionsPage( VkObject* obj );
   
   void setCurrentTab( int idx );
   void suppNewFromStr( const QString& str );
   
public slots:
   void suppNew();
   void suppEdit();
   
private slots:
   void getDbBin();
   void setSuppFileBtns();
   void suppfileNew();
   void suppfileAdd();
   void suppfileRemove();
   void suppfileUp();
   void suppfileDown();
   void setSuppBtns();
   void suppLoad();
   void suppEdit( QListWidgetItem* );
   void suppDelete();
   
private:
   void setupOptions();
   
private:
   QTabWidget* tabWidget;
   QGroupBox* group1;
   QListWidget* lwSupps;
   QPushButton* btn_suppfile_up;
   QPushButton* btn_suppfile_dwn; 
   QPushButton* btn_suppfile_new;
   QPushButton* btn_suppfile_add;
   QPushButton* btn_suppfile_rmv;
   QPushButton* btn_supp_new;
   QPushButton* btn_supp_edt;
   QPushButton* btn_supp_del;

   SuppList supplist;
};


#endif
