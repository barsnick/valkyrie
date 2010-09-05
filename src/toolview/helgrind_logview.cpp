/****************************************************************************
** HelgrindLogView implementation
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

#include "toolview/helgrind_logview.h"
#include "utils/vk_utils.h"


// ============================================================
/*!
  setup static class maps
*/
static ErrorItem::AcronymMap setupErrAcronymMap()
{
   ErrorItem::AcronymMap amap;
   amap["Race"]           = "RAC"; // Data race.
   amap["UnlockUnlocked"] = "ULU"; // Unlocking a not-locked lock
   amap["UnlockForeign"]  = "ULF"; // Unlocking a lock held by some other thread
   amap["UnlockBogus"]    = "ULB"; // Unlocking an address not known to be a lock
   amap["PthAPIerror"]    = "PTH"; // Failure of pthread_ function
   amap["LockOrder"]      = "LOR"; // Lock acuisition order inconsistency
   amap["Misc"]           = "MSC"; // Misc.
   return amap;
}
ErrorItem::AcronymMap ErrorItemHG::acnymMap = setupErrAcronymMap();


// ============================================================
/*!
  ErrorItem for Helgrind
*/
ErrorItemHG::ErrorItemHG( VgOutputItem* parent, QTreeWidgetItem* after,
                          QDomElement err )
      : ErrorItem( parent, after, err, acnymMap )
{
}



// ============================================================
/*!
  TopStatus: first item in listview
*/
TopStatusItemHG::TopStatusItemHG( QTreeWidget* parent, QDomElement exe,
                                  QDomElement status, QString _protocol )
   : TopStatusItem( parent, exe, status, "", _protocol )
{
}


void TopStatusItemHG::updateToolStatus( QDomElement /*err*/ )
{
   // Update general error count
   // Note: this may be _way_ off, 'cos we don't see repeated errors
   // until we get an ERRORCOUNTS element
   num_errs++;
   updateText();
}



// ============================================================
/*!
  AnnounceThreadItem

  The relative position in the log does not reflect the actual
  thread creation point - this is simply when Helgrind 'announces' it,
  for use in a subsequent ERROR.

  TODO: put that in a tooltip, or sthng.
*/
AnnounceThreadItem::AnnounceThreadItem( VgOutputItem* parent,
                                        QTreeWidgetItem* after,
                                        QDomElement err )
: VgOutputItem( parent, after, err )
{

   QDomElement hthreadid = elem.firstChildElement();
#ifdef DEBUG_ON
   if ( hthreadid.tagName() != "hthreadid" ) {
      vkPrintErr( "AnnounceThreadItem::AnnounceThreadItem(): unexpected tagName: %s",
                  qPrintable( hthreadid.tagName() ) );
   }
#endif
   QString hthreadid_str = hthreadid.text();
   setText( "Thread Announce: #HG_" + hthreadid_str );

   isExpandable = true;
}


void AnnounceThreadItem::setupChildren()
{
   if ( childCount() == 0 ) {
      QDomElement e = elem.firstChildElement();
      e = e.nextSiblingElement();
#ifdef DEBUG_ON
      if ( e.tagName() != "stack" ) {
         vkPrintErr( "AnnounceThreadItem::setupChildren(): unexpected tagName: %s",
                     qPrintable( e.tagName() ) );
      }
#endif
      VgOutputItem* stack = new StackItem( this, this, e );
      stack->openChildren();
   }
}



// ============================================================
/*!
  HelgrindLogView
*/
HelgrindLogView::HelgrindLogView( QTreeWidget* view )
   : VgLogView( view )
{}

HelgrindLogView::~HelgrindLogView()
{}

QString HelgrindLogView::toolName()
{
   return "helgrind";
}


/*!
  replace "#" with "#HG_", to distinguish from real thread id's.
*/
void HelgrindLogView::updateThreadId( QDomElement elem )
{
   if ( elem.isNull() ) {
      return;
   }
   QDomNode n = elem.firstChild();
   if ( !n.isText() ) {
      VK_DEBUG( "Node not of type QDomText: %s", qPrintable( n.nodeName() ) );
      return;
   }
   n.setNodeValue( n.nodeValue().replace( "hread #", "hread #HG_" ) );
}


/*!
  Populate our model (QDomDocument) and the view (QListWidget)
   - top-level xml elements are pushed to us from the parser
   - node is reparented to the QDomDocument log
*/
bool HelgrindLogView::appendNodeTool( QDomElement elem, QString& errMsg )
{
   switch ( VgOutputItem::elemType( elem.tagName() ) ) {
   case VG_ELEM::PROTOCOL_VERSION : {
      if ( elem.text() != "4" ) {
         errMsg = "Helgrind tool doesn't support XML protocol version: (" + elem.text() + ")";
         vkPrintErr( "%s", qPrintable( "HelgrindLogView::appendNodeTool(): " + errMsg ) );
         return false;
      }
      break;
   }

   case VG_ELEM::ERROR: {
      QDomElement err = elem;

      // update thread id description, to distinguish from real thread id's.
      updateThreadId( err.firstChildElement( "xwhat" ).firstChildElement( "text" ) );
      updateThreadId( err.firstChildElement( "xauxwhat" ).firstChildElement( "text" ) );
      updateThreadId( err.firstChildElement( "what" ) );
      updateThreadId( err.firstChildElement( "auxwhat" ) );

      lastItem = new ErrorItemHG( topStatus, lastItem, err );

      // update topStatus
      topStatus->updateToolStatus( err );
      break;
   }

   case VG_ELEM::ANNOUNCETHREAD: {
      QDomElement announcethread = elem;
      lastItem = new AnnounceThreadItem( topStatus, lastItem, announcethread );
      break;
   }

   default:
      break;
   }

   return true;
}


TopStatusItem* HelgrindLogView::createTopStatus( QTreeWidget* view,
                                                 QDomElement exe,
                                                 QDomElement status,
                                                 QString _protocol )
{
   return new TopStatusItemHG( view, exe, status, _protocol );
}

