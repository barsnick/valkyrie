/****************************************************************************
** MemcheckLogView definition
**  - links QDomElements with QTreeWidgetItems
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

#ifndef __VK_MEMCHECKLOGVIEW_H
#define __VK_MEMCHECKLOGVIEW_H

#include "toolview/vglogview.h"


// ============================================================
class MemcheckLogView : public VgLogView
{
public:
   MemcheckLogView( QTreeWidget* );
   ~MemcheckLogView();

private:
   // Template method functions:
   TopStatusItem* createTopStatus( QTreeWidget* view, QDomElement exe,
                                   QDomElement status, QString _protocol );
   QString toolName();
   bool appendNodeTool( QDomElement elem, QString& errMsg );
};





// ============================================================
class ErrorItemMC : public ErrorItem
{
public:
   ErrorItemMC( VgOutputItem* parent, QTreeWidgetItem* after,
                QDomElement err );
private:
   static ErrorItem::AcronymMap acnymMap;
};




// ============================================================
class TopStatusItemMC : public TopStatusItem
{
public:
   TopStatusItemMC( QTreeWidget* parent, QDomElement exe,
                    QDomElement status, QString _protocol );

   void updateToolStatus( QDomElement err );

private:
   int num_bytes, num_blocks;
   QString errcounts_tmplt;
};




/*
//TODO: turn these into QTips

InvalidFree
   free/delete/delete[] on an invalid pointer
MismatchedFree
   free/delete/delete[] does not match allocation function
InvalidRead
   read of an invalid address
InvalidWrite
   write of an invalid address
InvalidJump
   jump to an invalid address
Overlap
   args overlap other otherwise bogus in eg memcpy
InvalidMemPool
   invalid mem pool specified in client request
UninitCondition
   conditional jump/move depends on undefined value
UninitValue
   other use of undefined value (primarily memory addresses)
SyscallParam
   system call params are undefined or point to undefined/unaddressible memory
ClientCheck
   "error" resulting from a client check request
Leak_DefinitelyLost
   memory leak; the referenced blocks are definitely lost
Leak_IndirectlyLost
   memory leak; the referenced blocks are lost because all pointers to them are also in leaked blocks
Leak_PossiblyLost
   memory leak; only interior pointers to referenced blocks were found
Leak_StillReachable
   memory leak; pointers to un-freed blocks are still available
*/



#endif // #ifndef __VK_MEMCHECKLOGVIEW_H
