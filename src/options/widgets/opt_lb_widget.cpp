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
    Constructs a LbWidget object
    has-a QListBox

   Note: This widget was specifically written to handle suppression files
   stuff and nothing else.
*/

static const char* sel_supp_xpm[] = {
   "11 11 8 1",
   "   c None",
   ".  c #024266",
   "+  c #5A9AB8",
   "@  c #1B5F8E",
   "#  c #79B7CD",
   "$  c #5A97B5",
   "%  c #AEDDE9",
   "&  c #8ECADC",
   "     .     ",
   "  . .+. .  ",
   " .+.@#@.+. ",
   "  .#$%$#.  ",
   " .@$&%&$@. ",
   ".+#%%%%%#+.",
   " .@$&%&$@. ",
   "  .#$%$#.  ",
   " .+.@#@.+. ",
   "  . .+. .  ",
   "     .     "
};


LbWidget::LbWidget( QWidget* parent, VkOption* vkopt, bool mklabel )
   : OptionWidget( parent, vkopt, mklabel )
{
   this->setObjectName( "lb_widget" );
   
   m_lbox = new QListWidget( parent );
   m_lbox->setObjectName( QString::fromUtf8( "list_box" ) );
   m_widg = m_lbox;
   m_lbox->setSelectionMode( QAbstractItemView::SingleSelection );//QListWidget::Single );
   
   m_sep  = VkCfg::sepChar();
   
   update( m_currentValue );
   m_lbox->setContextMenuPolicy( Qt::CustomContextMenu );
   connect( m_lbox, SIGNAL( customContextMenuRequested( const QPoint& ) ),
            this,     SLOT( popupMenu( const QPoint& ) ) );
            
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
}


/*!
  Pretty print current value
*/
QString LbWidget::printCurrValue()
{
   return "For one (or more) of:<br>" + currValue().replace( m_sep, "<br>" );
}


/*!
  return all contents concat'd with m_sep
*/
QString LbWidget::lbText()
{
   QStringList items;

   for ( int i = 0; i < m_lbox->count(); i++ ) {
      items += m_lbox->item( i )->text();
   }
   
   return ( items.count() == 0 ) ? "" : items.join( m_sep );
}


/*!
  different menus and stuff for the different modes
*/
void LbWidget::popupMenu( const QPoint& pos )
{
   QListWidgetItem* lb_item = m_lbox->itemAt( pos );
   
   QAction actRemove( QPixmap( sel_supp_xpm ), "Remove File", this );
   QAction actAdd( QPixmap( sel_supp_xpm ), "Add File", this );

   if ( !lb_item ) {
      actRemove.setEnabled( false );
   }
   
   QMenu menu( m_lbox );
   menu.addAction( &actRemove );
   menu.addAction( &actAdd );
   QAction* act = menu.exec( m_lbox->mapToGlobal( pos ) );
   
   if ( act == &actRemove ) {
      vk_assert( lb_item );
      m_lbox->takeItem( m_lbox->row( lb_item ) );
      setCurrValue( lbText() );
   }
   else if ( act == &actAdd ) {
      QString supp_file =
         QFileDialog::getOpenFileName( m_lbox,
                                       tr( "Choose Suppression File" ),
                                       "./", tr("Suppression Files (*.supp)") );
                            
      if ( ! supp_file.isEmpty() ) { // user clicked Cancel ?
         m_lbox->addItem( supp_file );
         setCurrValue( lbText() );
      }
   }
}
