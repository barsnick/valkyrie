/****************************************************************************
** MemcheckOptionsPage definition
**  - subclass of VkOptionsPage to hold memcheck-specific options
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


#ifndef __MEMCHECK_OPTIONS_PAGE_H
#define __MEMCHECK_OPTIONS_PAGE_H

#include "options/vk_options_page.h"


// ============================================================
class MemcheckOptionsPage : public VkOptionsPage
{
   Q_OBJECT
   
public:
   MemcheckOptionsPage( VkObject* obj );
   
private:
   void setupOptions();
};


#endif
