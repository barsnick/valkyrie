/****************************************************************************
** Memcheck definition
**  - Memcheck-specific options / flags / fns
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

#ifndef __MEMCHECK_OBJECT_H
#define __MEMCHECK_OBJECT_H

#include "objects/tool_object.h"


// ============================================================
namespace MEMCHECK
{

/*!
  enum identification of all options for this object
*/
enum mcOptId {
   LEAK_CHECK,
   LEAK_RES,
   SHOW_REACH,
   //UNDEF_VAL,
   TRACK_ORI,
   PARTIAL,
   FREELIST,
   GCC_296,
   ALIGNMENT,
   NUM_OPTS
};
}


// ============================================================
// class Memcheck
class Memcheck : public ToolObject
{
   Q_OBJECT
public:
   Memcheck();
   ~Memcheck();

   ToolView* createToolView( QWidget* parent );
   VkOptionsPage* createVkOptionsPage();

   int checkOptArg( int optid, QString& argval );
   unsigned int maxOptId() { return MEMCHECK::NUM_OPTS; }

private:
   void setupOptions();
   void statusMsg( QString msg );
};


#endif
