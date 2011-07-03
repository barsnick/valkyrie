/****************************************************************************
** LogViewFilterMC implementation
** --------------------------------------------------------------------------
**
** Copyright (C) 2011-2011, OpenWorks LLP. All rights reserved.
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

#include "toolview/logviewfilter_mc.h"
#include "utils/vk_utils.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMap>
#include <QTimer>



LogViewFilterMC::LogViewFilterMC( QWidget *parent, QTreeWidget* view )
   : QWidget(parent), m_view( view )
{
   setObjectName( QString::fromUtf8( "LogViewFilterMC" ) );
   
   // ------------------------------------------------------------
   // widgets
   QIcon ico_filter( QString::fromUtf8( ":/vk_icons/icons/refresh.png" ) );
   butt_refresh = new QPushButton( ico_filter, "" );
   butt_refresh->setFixedWidth( 30 );
   connect( butt_refresh, SIGNAL(clicked()), this, SLOT(refresh()) );

   // XML tag types
   combo_xmltag = new QComboBox();
   connect( combo_xmltag, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()) );
   
   // Compare functions
   cmpWidgStack = new QStackedWidget();
   cmpWidgStack->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );
   QComboBox* combo_cmp[3];    // for each of [CMP_KND, CMP_STR, CMP_INT]
   for ( int i=0; i<3; ++i ) {
      combo_cmp[i] = new QComboBox();
      combo_cmp[i]->setSizeAdjustPolicy( QComboBox::AdjustToContents );
      connect( combo_cmp[i], SIGNAL(currentIndexChanged(int)), this, SLOT(edited()) );
      cmpWidgStack->addWidget( combo_cmp[i] );
   }
   vk_assert( cmpWidgStack->indexOf( combo_cmp[CMP_KND] ) == CMP_KND );
   vk_assert( cmpWidgStack->indexOf( combo_cmp[CMP_STR] ) == CMP_STR );
   vk_assert( cmpWidgStack->indexOf( combo_cmp[CMP_INT] ) == CMP_INT );

   // Filter values (combo/lineedits)
   QComboBox* combo_filter  = new QComboBox();
   connect( combo_filter, SIGNAL(currentIndexChanged(int)), this, SLOT(edited()) );
   QLineEdit* ledit_strfilter  = new QLineEdit();
   connect( ledit_strfilter, SIGNAL(textChanged(QString)), this, SLOT(edited()) );
   connect( ledit_strfilter, SIGNAL(editingFinished()), this, SLOT(refresh()) );
   QLineEdit* ledit_intfilter  = new QLineEdit();
   ledit_intfilter->setValidator( new QIntValidator(this) ); // only accept integers.
   connect( ledit_intfilter, SIGNAL(textChanged(QString)), this, SLOT(edited()) );
   connect( ledit_intfilter, SIGNAL(editingFinished()), this, SLOT(refresh()) );
   
   filterWidgStack = new QStackedWidget();
   filterWidgStack->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum );
   filterWidgStack->addWidget( combo_filter );
   filterWidgStack->addWidget( ledit_strfilter );
   filterWidgStack->addWidget( ledit_intfilter );
   vk_assert( filterWidgStack->indexOf( combo_filter    ) == CMP_KND );
   vk_assert( filterWidgStack->indexOf( ledit_strfilter ) == CMP_STR );
   vk_assert( filterWidgStack->indexOf( ledit_intfilter ) == CMP_INT );
   
   // ------------------------------------------------------------
   // layout
   QGridLayout* gridLayout = new QGridLayout( this );
   gridLayout->setColumnStretch( 0, 0 );
   gridLayout->setColumnStretch( 1, 0 );
   gridLayout->setColumnStretch( 2, 0 );
   gridLayout->setColumnStretch( 3, 1 );
   gridLayout->setMargin(0);
   gridLayout->addWidget( butt_refresh,    0, 0 );
   gridLayout->addWidget( combo_xmltag,    0, 1 );
   gridLayout->addWidget( cmpWidgStack,    0, 2 );
   gridLayout->addWidget( filterWidgStack, 0, 3 );

         
   // ------------------------------------------------------------
   // initialise xml tag combobox along with all compare types
   combo_xmltag->addItem( "Function",      XML_FUN );
   combo_xmltag->addItem( "Object",        XML_OBJ );
   combo_xmltag->addItem( "Directory",     XML_DIR );
   combo_xmltag->addItem( "File",          XML_FIL );
   combo_xmltag->addItem( "Line",          XML_LIN );
   combo_xmltag->addItem( "Leaked Bytes",  XML_LBY );
   combo_xmltag->addItem( "Leaked Blocks", XML_LBL );
   combo_xmltag->addItem( "Kind",          XML_KND );
   connect( combo_xmltag, SIGNAL(currentIndexChanged(int)), this, SLOT( setupFilter(int) ) );

   // map xmltags to compare types
   map_xmltag_cmptype.insert( XML_KND, CMP_KND );
   map_xmltag_cmptype.insert( XML_LBY, CMP_INT );
   map_xmltag_cmptype.insert( XML_LBL, CMP_INT );
   map_xmltag_cmptype.insert( XML_OBJ, CMP_STR );
   map_xmltag_cmptype.insert( XML_FUN, CMP_STR );
   map_xmltag_cmptype.insert( XML_DIR, CMP_STR );
   map_xmltag_cmptype.insert( XML_FIL, CMP_STR );
   map_xmltag_cmptype.insert( XML_LIN, CMP_INT );

   // setup compare function comboboxes, along with their enums
   combo_cmp[CMP_KND]->addItem( "==",            FUN_EQL   );
   combo_cmp[CMP_KND]->addItem( "!=",            FUN_NEQL  );
   combo_cmp[CMP_STR]->addItem( "==",            FUN_EQL   );
   combo_cmp[CMP_STR]->addItem( "!=",            FUN_NEQL  );
   combo_cmp[CMP_STR]->addItem( "contains",      FUN_CONT  );
   combo_cmp[CMP_STR]->addItem( "! contain",     FUN_NCONT );
   combo_cmp[CMP_STR]->addItem( "starts with",   FUN_STRT  );
   combo_cmp[CMP_STR]->addItem( "! starts with", FUN_NSTRT );
   combo_cmp[CMP_STR]->addItem( "ends with",     FUN_END   );
   combo_cmp[CMP_STR]->addItem( "! ends with",   FUN_NEND  );
   combo_cmp[CMP_INT]->addItem( "==",            FUN_EQL   );
   combo_cmp[CMP_INT]->addItem( "!=",            FUN_NEQL  );
   combo_cmp[CMP_INT]->addItem( "<",             FUN_LSTHN );
   combo_cmp[CMP_INT]->addItem( ">",             FUN_GRTHN );
   
   // initialise filter combobox with all 'kind' types (display & matching text)
   combo_filter->addItem( "", "" );
   combo_filter->addItem( "IVF - InvalidFree",         "InvalidFree"         );
   combo_filter->addItem( "MMF - MismatchedFree",      "MismatchedFree"      );
   combo_filter->addItem( "IVR - InvalidRead",         "InvalidRead"         );
   combo_filter->addItem( "IVW - InvalidWrite",        "InvalidWrite"        );
   combo_filter->addItem( "IVJ - InvalidJump",         "InvalidJump"         );
   combo_filter->addItem( "OVL - Overlap",             "Overlap"             );
   combo_filter->addItem( "IMP - InvalidMemPool",      "InvalidMemPool"      );
   combo_filter->addItem( "UNC - UninitCondition",     "UninitCondition"     );
   combo_filter->addItem( "UNV - UninitValue",         "UninitValue"         );
   combo_filter->addItem( "SCP - SyscallParam",        "SyscallParam"        );
   combo_filter->addItem( "CCK - ClientCheck",         "ClientCheck"         );
   combo_filter->addItem( "LDL - Leak_DefinitelyLost", "Leak_DefinitelyLost" );
   combo_filter->addItem( "LIL - Leak_IndirectlyLost", "Leak_IndirectlyLost" );
   combo_filter->addItem( "LPL - Leak_PossiblyLost",   "Leak_PossiblyLost"   );
   combo_filter->addItem( "LSR - Leak_StillReachable", "Leak_StillReachable" );
   
   // initialise filters
   setupFilter( 0 );
   ((QComboBox*)cmpWidgStack->currentWidget())->setCurrentIndex( 2 );
  
   //TODO: ContextHelp::addHelp( this, urlValkyrie::XYZ);
}

void LogViewFilterMC::setupFilter( int idx )
{
//   vkDebug( "LogViewFilterMC::setupFilter( %d )", idx );
   
   XmlTagType xmltag = (XmlTagType)combo_xmltag->itemData( idx ).toInt();
   CmpType cmp_type = map_xmltag_cmptype[ xmltag ];
      
   // compares: combobox
   cmpWidgStack->setCurrentIndex( cmp_type );

   // filter values: combobox/lineedit
   filterWidgStack->setCurrentIndex( cmp_type );
}


void LogViewFilterMC::edited()
{
//   vkDebug( "LogViewFilterMC::edited()" );

   butt_refresh->setEnabled( true );
}

void LogViewFilterMC::refresh()
{
//   vkDebug( "LogViewFilterMC::refresh()" );
   
   butt_refresh->setEnabled( false );
   
   updateView();
}




void LogViewFilterMC::updateView()
{
//   vkDebug( "LogViewFilterMC::updateView()" );

   if ( m_view == NULL ) {
      vkPrintErr( "No treeview - This shouldn't happen!" );
      return;
   }
            
   VgOutputItem* vgItemTop = (VgOutputItem*)m_view->topLevelItem( 0 );
   if ( vgItemTop == NULL ) {
//      vkDebug( "No items in treeview." );
      return;
   }

   // iterate over all the first-child items
   for ( int i=0; i<vgItemTop->childCount(); ++i ) {
      VgOutputItem* child = (VgOutputItem*)vgItemTop->child( i );

      if ( child->elemType() == VG_ELEM::ERROR ) {

         if ( this->isHidden() ) {     // show all items if filter is inactive
            child->setHidden( false );
         }
         else {                        // filter active: go filter!
            showHideItem( child );
         }
      }
   }
}


void LogViewFilterMC::showHideItem( VgOutputItem* item )
{
//   vkDebug( "LogViewFilterMC::showHideItem: %s", qPrintable( item->text(0) ) );

   // sanity checks
   if ( !item ) {
      vkPrintErr( "NULL item. This shouldn't happen!");
      return;
   }
   
   if ( item->elemType() != VG_ELEM::ERROR ) {
      vkPrintErr( "Not an ERROR item. This shouldn't happen!");
      return;
   }
   
   // get filter from widgets
   // - first get and test the filter value: if empty -> no filter.
   QString str_flt;
   if ( filterWidgStack->currentIndex() == CMP_KND ) { // => combobox
      QComboBox* combo = (QComboBox*)filterWidgStack->currentWidget();
      str_flt = combo->itemData( combo->currentIndex() ).toString();
   }
   else {                                           // => lineedit
      QLineEdit* le = (QLineEdit*)filterWidgStack->currentWidget();
      str_flt = le->text();
   }
   
   if ( str_flt.isEmpty() ) {
//      vkDebug( "Filter value empty -> empty filter" );
      item->setHidden( false );
   }
   else {
      QStringList xml_list;
      
      // get the compare function
      QComboBox* comboCmpFun = (QComboBox*)cmpWidgStack->currentWidget();
      int idx = comboCmpFun->currentIndex();
      CmpFunType cmpFun = (CmpFunType)comboCmpFun->itemData( idx ).toInt();
      
      // get the type to compare
      idx = combo_xmltag->currentIndex();
      XmlTagType xmltag = (XmlTagType)combo_xmltag->itemData( idx ).toInt();
      CmpType cmp_type = map_xmltag_cmptype[ xmltag ];
      
      // get the appropriate xml tag, and compare it with our filter
      //  - a positive match means the error item remains.
      //  - a negative match means the error item is hidden.
      bool res_cmp = false;
      switch ( xmltag ) {
      case XML_KND: // Kind
         vk_assert( cmp_type == CMP_KND );
         res_cmp = xmlCompare( item, "kind", str_flt, cmpFun, cmp_type );         
         break;
      case XML_LBY: // Leaked Bytes
         vk_assert( cmp_type == CMP_INT );
         res_cmp = xmlCompare( item, "leakedbytes", str_flt, cmpFun, cmp_type );         
         break;
      case XML_LBL: // Leaked Blocks
         vk_assert( cmp_type == CMP_INT );
         res_cmp = xmlCompare( item, "leakedblocks", str_flt, cmpFun, cmp_type );         
         break;
      case XML_OBJ: // Object
         vk_assert( cmp_type == CMP_STR );
         res_cmp = xmlCompare( item, "obj", str_flt, cmpFun, cmp_type );         
         break;
      case XML_FUN: // Function
         vk_assert( cmp_type == CMP_STR );
         res_cmp = xmlCompare( item, "fn", str_flt, cmpFun, cmp_type );         
         break;
      case XML_DIR: // Directory
         vk_assert( cmp_type == CMP_STR );
         res_cmp = xmlCompare( item, "dir", str_flt, cmpFun, cmp_type );         
         break;
      case XML_FIL: // File
         vk_assert( cmp_type == CMP_STR );
         res_cmp = xmlCompare( item, "file", str_flt, cmpFun, cmp_type );         
         break;
      case XML_LIN: // Line
         vk_assert( cmp_type == CMP_INT );
         res_cmp = xmlCompare( item, "line", str_flt, cmpFun, cmp_type );         
         break;
      default:
         vk_assert_never_reached();
      }
      
      item->setHidden( !res_cmp );
   }
}


void LogViewFilterMC::enableFilter( bool enable )
{
   if ( enable )
      this->show();
   else
      this->hide();
   
   updateView();
}





bool LogViewFilterMC::xmlCompare( VgOutputItem* errItem, const QString& tag,
                                 const QString& str_flt, CmpFunType cmpFun, CmpType cmp_type )
{
   // extract the relevant XML data
   // no idea if this is good enough... sometimes maybe better to get first tag only?
   QStringList list_xml;
   QDomNodeList nodelist = errItem->getElement().elementsByTagName( tag );
   for (int i=0; i<nodelist.count(); ++i ) {
      list_xml << nodelist.at(i).toElement().text();
   }

   // compare strings or integers?
   bool res_cmp = false;
   switch (cmp_type) {
   case CMP_KND:
   case CMP_STR:
      res_cmp = compare_strings( list_xml, str_flt, cmpFun );
      break;
   case CMP_INT:
      res_cmp = compare_integers( list_xml, str_flt, cmpFun );
      break;
   default:
      vk_assert_never_reached();
   }
   
//   vkDebug( "LogViewFilterMC::xmlCompare: res_cmp: '%d'", res_cmp );
   
   return res_cmp;
}



/*!
  Compare strings.
  If _any_ of the strings in lst_xml match str_flt, return true (don't hide)
  If no match, return false (hide)
  Any problems, just return true (i.e. don't hide)
*/
bool LogViewFilterMC::compare_strings( const QStringList& lst_xml,
                                        const QString& str_flt,
                                        CmpFunType cmpfuntype )
{

   bool res_cmp = false;
   foreach ( QString str_xml, lst_xml ) {
//      vkDebug( "LogViewFilterMC::compare_strings(%d): '%s'' - '%s'", cmpfuntype, qPrintable( str_xml ), qPrintable( str_flt ) );
      
      switch ( cmpfuntype ) {
      case FUN_EQL:   res_cmp = ( str_xml ==          str_flt  ); break;
      case FUN_NEQL:  res_cmp = ( str_xml !=          str_flt  ); break;
      case FUN_CONT:  res_cmp = ( str_xml.contains(   str_flt )); break;
      case FUN_NCONT: res_cmp = (!str_xml.contains(   str_flt )); break;
      case FUN_STRT:  res_cmp = ( str_xml.startsWith( str_flt )); break;
      case FUN_NSTRT: res_cmp = (!str_xml.startsWith( str_flt )); break;
      case FUN_END:   res_cmp = ( str_xml.endsWith(   str_flt )); break;
      case FUN_NEND:  res_cmp = (!str_xml.endsWith(   str_flt )); break;
         break;
      default:
         vk_assert_never_reached();
      }

      // match found - don't filter out this item!
      if ( res_cmp )
         return true;
   }
   
   // no match: filter out (hide) this item
   return false;
}


/*!
  Compare integers.
  First convert from strings to integers.
  If _any_ of the strings in lst_xml match str_flt, return true (don't hide)
  If no match, return false (hide)
  Any problems, just return true (i.e. don't hide)
*/
bool LogViewFilterMC::compare_integers( const QStringList& lst_xml,
                                        const QString& str_flt,
                                        CmpFunType cmpfuntype )
{
   bool res_cmp = false;
   foreach ( QString str_xml, lst_xml ) {

      bool ok = true;
      int int_flt = str_flt.toInt( &ok );
      if ( !ok ) {
//         vkDebug( "Failed conversion (str_flt) to integer: '%s'", qPrintable(str_flt) );
         continue;
      }

      int int_xml = str_xml.toInt( &ok );
      if ( !ok ) {
//         vkDebug( "Failed conversion (str_xml) to integer: '%s'", qPrintable(str_xml) );
         continue;
      }
      
//      vkDebug( "LogViewFilterMC::compare_integers: '%d'' - '%d'", int_xml, int_flt );
      
      switch ( cmpfuntype ) {
      case FUN_EQL:   res_cmp = (int_xml == int_flt); break;
      case FUN_NEQL:  res_cmp = (int_xml != int_flt); break;
      case FUN_LSTHN: res_cmp = (int_xml  < int_flt); break;
      case FUN_GRTHN: res_cmp = (int_xml  > int_flt); break;
      default:
         vk_assert_never_reached();
      }
      
      // match found - don't filter out this item!
      if ( res_cmp )
         return true;
   }

   // no match: filter out (hide) this item
   return false;
}
