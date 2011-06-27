/****************************************************************************
** LbWidget implementation
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

#include <QDir>
#include <QFileDialog>
#include <QMenu>

#include "help/help_context.h"
#include "help/help_urls.h"
#include "options/widgets/opt_lb_widget.h"
#include "options/vk_option.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"


/***************************************************************************/
/*!
  class MyListWidget
  - sends signals for row add/delete
*/
void MyListWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
   vk_assert( start == end );
   QListWidget::rowsInserted(parent, start, end);
   emit rowsChanged( true, start );
}

void MyListWidget::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
   vk_assert( start == end );
   QListWidget::rowsAboutToBeRemoved(parent, start, end);
   emit rowsChanged( false, start );
}



/*!
   Constructs a LbWidget object
   has-a QListWidget

   Note: This widget was specifically written to handle suppression files
   stuff and nothing else. This because of the difference in stored
   (with comma's, for opt) and displayed (list)
*/
LbWidget::LbWidget( QWidget* parent, VkOption* vkopt, bool mklabel )
   : OptionWidget( parent, vkopt, mklabel )
{
   this->setObjectName( "lb_widget" );
   
   m_lbox = new MyListWidget( parent );
   m_lbox->setObjectName( QString::fromUtf8( "list_box" ) );
   m_widg = m_lbox;
   m_lbox->setSelectionMode( QAbstractItemView::SingleSelection );
   connect( m_lbox, SIGNAL(rowsChanged(bool,int)),
           this, SLOT(updateValueFromView(bool, int)));
   
   m_sep  = VkCfg::sepChar();
   
   update( m_currentValue );
            
   // not added if the url is empty
   ContextHelp::addHelp( m_widg, m_opt->urlAddress );
}


LbWidget::~LbWidget()
{
   if ( m_lbox ) {
      delete m_lbox;
      m_lbox = 0;
   }
}


/*!
  reload lbox from m_currentValue
 */
void LbWidget::update( const QString& txt )
{
   setCurrValue( txt );

   m_lbox->clear();
   QStringList sfiles = m_currentValue.split( m_sep, QString::SkipEmptyParts );

   for ( int i = 0; i < sfiles.count(); i++ ) {
      m_lbox->addItem( sfiles[i] );
   }

   // auto-select the top row   
   m_lbox->setCurrentRow( 0 );
}


/*!
  Update underlying value from lbox
  Triggered by signals rowsInserted, rowsToBeDeleted
   - inserted row is easy: just concat the items
   - to_be_deleted row isn't yet gone: skip that row.
 */
void LbWidget::updateValueFromView( bool isInserted, int row )
{
   QStringList items;
   for ( int i=0; i < m_lbox->count(); i++ ) {
      if (!isInserted && i == row) continue;   // 'row' is to be removed
      items += m_lbox->item( i )->text();
   }
   QString listViewText = items.join( m_sep );

   setCurrValue( listViewText );
}


/*!
  Pretty print current value
*/
QString LbWidget::printCurrValue()
{
   return "For one (or more) of:<br>" + currValue().replace( m_sep, "<br>" );
}
