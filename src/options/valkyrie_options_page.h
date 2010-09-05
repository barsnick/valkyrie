/****************************************************************************
** ValkyrieOptionsPage definition
**  - subclass of VkOptionsPage to hold valkyrie-specific options
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2009, OpenWorks LLP. All rights reserved.
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

#ifndef __VALKYRIE_OPTIONS_PAGE_H
#define __VALKYRIE_OPTIONS_PAGE_H

#include <QGroupBox>

#include "options/vk_options_page.h"


// ============================================================
class ValkyrieOptionsPage : public VkOptionsPage
{
   Q_OBJECT
public:
   ValkyrieOptionsPage( VkObject* obj );
   
   bool saveToProjectCfg();
   
private slots:
   void chooseGenFont();
   void chooseToolFont();
   void getEditor();
   void getBinary();
   void getBrowser();
   void getDfltLogDir();
   void getWorkingDir();
   void getVgExec();
   
private:
   void setupOptions();
   
private:
   QGroupBox* group1;
};

#endif  // __VALKYRIE_OPTIONS_PAGE_H
