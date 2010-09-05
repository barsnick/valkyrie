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

#ifndef __VK_HELGRINDLOGVIEW_H
#define __VK_HELGRINDLOGVIEW_H

#include "toolview/vglogview.h"


// ============================================================
class HelgrindLogView : public VgLogView
{
public:
   HelgrindLogView( QTreeWidget* );
   ~HelgrindLogView();

private:
   void updateThreadId( QDomElement elem );

   // Template method functions:
   TopStatusItem* createTopStatus( QTreeWidget* view, QDomElement exe,
                                   QDomElement status, QString _protocol );
   QString toolName();
   bool appendNodeTool( QDomElement elem, QString& errMsg );
};





// ============================================================
class ErrorItemHG : public ErrorItem
{
public:
   ErrorItemHG( VgOutputItem* parent, QTreeWidgetItem* after,
                QDomElement err );
private:
   static ErrorItem::AcronymMap acnymMap;
};




// ============================================================
class TopStatusItemHG : public TopStatusItem
{
public:
   TopStatusItemHG( QTreeWidget* parent, QDomElement exe,
                    QDomElement status, QString _protocol );

   void updateToolStatus( QDomElement err );
};



// ============================================================
class AnnounceThreadItem : public VgOutputItem
{
public:
   AnnounceThreadItem( VgOutputItem* parent, QTreeWidgetItem* after,
                       QDomElement err );

private:
   void setupChildren(); // called by base class
};



/*
//TODO: turn these into QTips

Race
   Data race.  Helgrind will try to show the stacks for both
   conflicting accesses if it can; it will always show the stack
   for at least one of them.
UnlockUnlocked
   Unlocking a not-locked lock
UnlockForeign
   Unlocking a lock held by some other thread
UnlockBogus
   Unlocking an address which is not known to be a lock
PthAPIerror
   One of the POSIX pthread_ functions that are intercepted
   by Helgrind, failed with an error code.  Usually indicates
   something bad happening.
LockOrder
   An inconsistency in the acquisition order of locks was observed;
   dangerous, as it can potentially lead to deadlocks
Misc
   One of various miscellaneous noteworthy conditions was observed
   (eg, thread exited whilst holding locks, "impossible" behaviour
   from the underlying threading library, etc)
*/



#endif // #ifndef __VK_HELGRINDLOGVIEW_H
