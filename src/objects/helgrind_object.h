/****************************************************************************
** Helgrind definition
**  - Helgrind-specific options / flags / fns
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

#ifndef __HELGRIND_OBJECT_H
#define __HELGRIND_OBJECT_H

#include "objects/tool_object.h"


// ============================================================
namespace HELGRIND
{
/*!
   enum identification of all options for this object
*/
enum mcOptId {
   // No tool-specific options
   NUM_OPTS
};
}


// ============================================================
// class Helgrind
class Helgrind : public ToolObject
{
   Q_OBJECT
public:
   Helgrind();
   ~Helgrind();

   ToolView* createToolView( QWidget* parent );
   VkOptionsPage* createVkOptionsPage();

   int checkOptArg( int optid, QString& argval );
   unsigned int maxOptId() { return HELGRIND::NUM_OPTS; }

private:
   void setupOptions();
   void statusMsg( QString msg );
};


#endif  // __HELGRIND_OBJECT_H
