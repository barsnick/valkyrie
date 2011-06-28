/****************************************************************************
** MemcheckLogView implementation
**  - links QDomElements with QTreeWidgetItems
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

#include "toolview/memcheck_logview.h"
#include "utils/vk_utils.h"


// ============================================================
/*!
  map Error::kind to a three-letter acronym
*/
static ErrorItem::AcronymMap setupErrAcronymMap()
{
   ErrorItem::AcronymMap amap;
//TODO: where did this go?
//   amap["CoreMemError"]        = "CRM";
   amap["InvalidFree"]         = "IVF"; // free/delete/delete[] on an invalid pointer
   amap["MismatchedFree"]      = "MMF"; // free/delete/delete[] does not match allocation function
   amap["InvalidRead"]         = "IVR"; //
   amap["InvalidWrite"]        = "IVW";
   amap["InvalidJump"]         = "IVJ";
   amap["Overlap"]             = "OVL";
   amap["InvalidMemPool"]      = "IMP";
   amap["UninitCondition"]     = "UNC";
   amap["UninitValue"]         = "UNV";
   amap["SyscallParam"]        = "SCP";
   amap["ClientCheck"]         = "CCK";
   amap["Leak_DefinitelyLost"] = "LDL";
   amap["Leak_IndirectlyLost"] = "LIL";
   amap["Leak_PossiblyLost"]   = "LPL";
   amap["Leak_StillReachable"] = "LSR";
   return amap;
}
ErrorItem::AcronymMap ErrorItemMC::acnymMap = setupErrAcronymMap();


// ============================================================
/*!
  ErrorItem for Memcheck
*/
ErrorItemMC::ErrorItemMC( VgOutputItem* parent, QTreeWidgetItem* after,
                          QDomElement err )
      : ErrorItem( parent, after, err, acnymMap )
{
}




// ============================================================
/*!
  TopStatus: first item in listview
  as two text lines:
  status, client exe
  errcounts(num_errs), leak_errors(num_bytes++, num_blocks++)
*/
TopStatusItemMC::TopStatusItemMC( QTreeWidget* parent, QDomElement exe,
                                  QDomElement status, QString _protocol )
   : TopStatusItem( parent, exe, status, ",   Leaked Bytes: 0", _protocol ),
   num_bytes( 0 ), num_blocks( 0 )
{
   // leaks, in addition to the basic errorcounts.
   errcounts_tmplt = ",   Leaked Bytes: %1 in %2 blocks";
}


void TopStatusItemMC::updateToolStatus( QDomElement err )
{
   QString kind = err.firstChildElement( "kind" ).text();
   if ( !kind.startsWith( "Leak_" ) ) {
      // Update general error count
      // Note: this may be _way_ off, 'cos we don't see repeated errors
      // until we get an ERRORCOUNTS element
      num_errs++;
      updateText();
      return;
   }
   else {
      // Update Leak_* error counts
      QDomElement xwhat = err.firstChildElement( "xwhat" );
      if ( xwhat.isNull() ) {
         vkPrintErr( "TopStatusItemMC::updateToolStatus(): missing xwhat element for leak error" );
      }
      else {
#if 1  //TODO: still needed?

         /* HACK ALERT!
            VALGRIND_DO_LEAK_CHECK gives repeated leaks...
            taking apart error::what to get record number
            - if '1' then reset counters
         */
         QDomElement text = xwhat.firstChildElement( "text" );
         if ( ! text.isNull() ) {
            QString text_str = text.text();
            QString lossrec_str = text_str.mid( text_str.indexOf( "in loss record " ) );

            if ( !lossrec_str.isEmpty() ) {
               QString record = lossrec_str.split( " ", QString::SkipEmptyParts )[3];

               if ( record == "1" ) {
                  num_bytes = num_blocks = 0;
               }
            }
            else {
               VK_DEBUG( "Unexpected string value for 'text' element: %s",
                         qPrintable( text_str ) );
            }
         }
         else {
            VK_DEBUG( "No 'text' child for 'xwhat' element" );
         }
#endif

         QDomElement leakedbytes  = xwhat.firstChildElement( "leakedbytes" );
         QDomElement leakedblocks = xwhat.firstChildElement( "leakedblocks" );

         if ( !leakedbytes.isNull() && !leakedblocks.isNull() ) {
            num_bytes  += leakedbytes.text().toUInt();
            num_blocks += leakedblocks.text().toUInt();

            toolstatus_str = errcounts_tmplt
                             .arg( num_bytes )
                             .arg( num_blocks );
            updateText();
         }
      }
   }
}




// ============================================================
/*!
  MemcheckLogView
*/
MemcheckLogView::MemcheckLogView( QTreeWidget* view )
   : VgLogView( view )
{}

MemcheckLogView::~MemcheckLogView()
{}

QString MemcheckLogView::toolName()
{
   return "memcheck";
}

/*!
  Populate our model (QDomDocument) and the view (QListWidget)
   - top-level xml elements are pushed to us from the parser
   - node is reparented to the QDomDocument log
*/
bool MemcheckLogView::appendNodeTool( QDomElement elem, QString& errMsg )
{
   switch ( VgOutputItem::elemType( elem.tagName() ) ) {
   case VG_ELEM::PROTOCOL_VERSION : {
      if ( elem.text() != "4" ) {
         errMsg = "Memcheck tool doesn't support XML protocol version: (" + elem.text() + ")";
         vkPrintErr( "%s", qPrintable( "MemcheckLogView::appendNodeTool(): " + errMsg ) );
         return false;
      }
      break;
   }

   case VG_ELEM::ERROR: {
      QDomElement err = elem;
      lastItem = new ErrorItemMC( topStatus, lastItem, err );

      // update topStatus
      topStatus->updateToolStatus( err );
      break;
   }

   default:
      break;
   }

   return true;
}


TopStatusItem* MemcheckLogView::createTopStatus( QTreeWidget* view,
                                                 QDomElement exe,
                                                 QDomElement status,
                                                 QString _protocol )
{
   return new TopStatusItemMC( view, exe, status, _protocol );
}

