/****************************************************************************
** VkSuppressionsDialog implementation
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

#include "help/help_context.h"
#include "help/help_urls.h"
#include "options/suppressions.h"
#include "options/vk_suppressions_dialog.h"
#include "options/vk_options_page.h"
#include "utils/vk_utils.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFontMetrics>
#include <QScrollArea>


#define SIZE_COL1 "XXXXXXXXX" // for a font-dependent size


/*!
  class CallChainFrame
*/
SuppFrame::SuppFrame( bool isFirstFrame, QWidget* parent )
   : QWidget( parent )
{
   setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
   
   QFontMetrics fm( qApp->font() );
   int width_col1 = fm.width( SIZE_COL1 );
   width_col1 -= 5; // compensate for margin of central widget
   
   QHBoxLayout* topHLayout = new QHBoxLayout( this );
   topHLayout->setObjectName( QString::fromUtf8( "topHLayout" ) );
   topHLayout->setMargin(0);
   
   frame_cmb = new QComboBox();
   frame_cmb->setMinimumWidth( width_col1 );
   frame_cmb->addItems( SuppRanges::instance().getFrameTypes() );
   
   frame_le  = new QLineEdit();
   
   frame_but = new QPushButton("-");
   if ( isFirstFrame ) frame_but->hide();
   frame_but->setFixedWidth( 30 );
   connect( frame_but, SIGNAL(clicked()), this, SLOT(buttClicked()) );

   topHLayout->addWidget( frame_cmb );
   topHLayout->addWidget( frame_le );
   topHLayout->addWidget( frame_but );
}

void SuppFrame::buttClicked()
{
   emit removeFrame( this );
}





/*!
  class VkSuppressionsDialog
*/
VkSuppressionsDialog::VkSuppressionsDialog( QWidget *parent )
   : QDialog( parent )
{
   setObjectName( QString::fromUtf8( "VkSuppressionsDialog" ) );
   setWindowTitle( "Valkyrie Suppressions Dialog" );

   QIcon icon_vk;
   icon_vk.addPixmap( QPixmap( QString::fromUtf8( ":/vk_icons/icons/valkyrie.xpm" ) ), QIcon::Normal, QIcon::Off );
   setWindowIcon( icon_vk );

   setMinimumWidth( 500 ); // allow reasonable length paths
   
   setupLayout();

   ContextHelp::addHelp( this, urlValkyrie::optsDlg );
}


void VkSuppressionsDialog::setupLayout()
{
   QFontMetrics fm( qApp->font() );
   int width_col1 = fm.width( SIZE_COL1 );

   // ------------------------------------------------------------
   QVBoxLayout* topVLayout = new QVBoxLayout( this );
   topVLayout->setObjectName( QString::fromUtf8( "topVLayout" ) );
   setLayout( topVLayout );
   
   // ------------------------------------------------------------
   QWidget* layoutWidget = new QWidget( this );
   layoutWidget->setObjectName( QString::fromUtf8( "layoutWidget" ) );
   topVLayout->addWidget( layoutWidget, 0 );
   
   // Note: not using opt_widget->hlayout()'s as button width won't match qlabel width.
   QGridLayout* gridLayout = new QGridLayout( layoutWidget );
   gridLayout->setColumnStretch( 0, 0 );
   gridLayout->setColumnStretch( 1, 1 );
   gridLayout->setColumnMinimumWidth( 0, width_col1 );
   gridLayout->setMargin(0);
   
   // ------------------------------------------------------------
   // Name
   QLabel* name_lbl = new QLabel("Name:");
   name_le = new QLineEdit();

   // Kind-Tool
   QLabel* tool_lbl = new QLabel("Kind-Tool:");
   tool_cmb = new QComboBox();
   tool_cmb->addItems( SuppRanges::instance().getKindTools() );
   connect( tool_cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(ToolChanged(int)) );

   // Kind-Type
   QLabel* type_lbl = new QLabel("Kind-Type:");
   type_cmb = new QComboBox();
   connect( type_cmb, SIGNAL(currentIndexChanged(int)), this, SLOT(TypeChanged(int)) );

   // Kind-Aux
   kaux_lbl = new QLabel("Kind-Aux:");
   kaux_le = new QLineEdit();

   // Trigger Tool change setup Type and Aux
   ToolChanged( 0 );
   
   // (start of) Call chain
   QLabel* cchn_lbl   = new QLabel("Call Chain:");
   QPushButton* cchn_but = new QPushButton("Add Frame");
   connect( cchn_but, SIGNAL(clicked()), this, SLOT(addNewSuppFrame()) );

   QWidget* pb_addFrame = new QWidget();
   QHBoxLayout* hLayout = new QHBoxLayout( pb_addFrame );
   hLayout->setMargin(0);
   hLayout->addWidget( cchn_but );
   hLayout->addStretch(10);
   
   // ------------------------------------------------------------
   // layout
   int i = 0;
   // gridLayout->setRowMinimumHeight( i++, lineHeight / 2 ); // blank row
   gridLayout->addWidget( name_lbl, i,   0, Qt::AlignRight );
   gridLayout->addWidget( name_le,  i++, 1 );

   gridLayout->addWidget( VkOptionsPage::sep( layoutWidget ), i++, 0, 1, 2 );
   
   gridLayout->addWidget( tool_lbl, i,   0, Qt::AlignRight );
   gridLayout->addWidget( tool_cmb, i++, 1 );
   gridLayout->addWidget( type_lbl, i,   0, Qt::AlignRight );
   gridLayout->addWidget( type_cmb, i++, 1 );
   gridLayout->addWidget( kaux_lbl, i,   0, Qt::AlignRight );
   gridLayout->addWidget( kaux_le,  i++, 1 );

   gridLayout->addWidget( VkOptionsPage::sep( layoutWidget ), i++, 0, 1, 2 );

   gridLayout->addWidget( cchn_lbl, i, 0, Qt::AlignRight );
   gridLayout->addWidget( pb_addFrame, i++, 1, 1, 2 );

   // ------------------------------------------------------------
   // scrollarea for call chain
   QScrollArea* scroll = new QScrollArea( this );
   scroll->setObjectName( QString::fromUtf8( "scroll" ) );
   topVLayout->addWidget( scroll, 1 );
         
   callChainLayout = new QVBoxLayout( scroll );
   callChainLayout->setObjectName( QString::fromUtf8( "callChainLayout" ) );
   callChainLayout->setMargin(5);
   // this needed else internal widgets resize to nothing!
   callChainLayout->setSizeConstraint( QLayout::SetMinAndMaxSize );

   QWidget* callChainWidget = new QWidget( scroll );
   callChainWidget->setObjectName( QString::fromUtf8( "callChainWidget" ) );
   callChainWidget->setLayout( callChainLayout );
         
   scroll->setWidget( callChainWidget );
   scroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
   scroll->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
   scroll->setMinimumHeight( 200 );
   scroll->setFrameStyle( QFrame::Box | QFrame::Plain );
   // allow internal widgets to resize when needed
   scroll->setWidgetResizable(true);

   // call chain (always have at least ONE frame)
   SuppFrame* suppFrm = new SuppFrame( true/*isFirst*/ );
   suppFrames.append( suppFrm );
   callChainLayout->addWidget( suppFrm );

   // ------------------------------------------------------------
   // *** other suppFrames dynimically placed here ***

   // push frames to top of scrollarea
   callChainLayout->addStretch( 1 );

   // ------------------------------------------------------------
   // Standard buttons: Ok, Cancel   
   QDialogButtonBox* buttbox = new QDialogButtonBox();
   buttbox->setObjectName( QString::fromUtf8( "buttbox" ) );
   buttbox->setOrientation( Qt::Horizontal );
   buttbox->setStandardButtons( QDialogButtonBox::Ok |
                                QDialogButtonBox::Cancel );

   QPushButton* pb_ok1 = buttbox->button( QDialogButtonBox::Ok );
   QPushButton* pb_cancel1 = buttbox->button( QDialogButtonBox::Cancel );
   connect( pb_ok1, SIGNAL( clicked() ), this, SLOT( accept() ) );
   connect( pb_cancel1, SIGNAL( clicked() ), this, SLOT( reject() ) );

   topVLayout->addWidget( buttbox, 0 );
}

void VkSuppressionsDialog::addNewSuppFrame()
{
   if ( suppFrames.count() >= MAX_SUPP_FRAMES ) {
      vkPrintErr("Maximum allowed number of frames reached.");
      return;
   }
   
   // add new frame just above our 'add frame' button
   SuppFrame* frm = new SuppFrame( false );
   suppFrames.append( frm );
   // last widget in callchain is a 'stretch': insert before that. 
   callChainLayout->insertWidget( callChainLayout->count()-1, frm );
   connect( frm, SIGNAL(removeFrame(SuppFrame*)),
           this, SLOT(removeSuppFrame(SuppFrame*)) );
}

void VkSuppressionsDialog::removeSuppFrame( SuppFrame* frm )
{
   callChainLayout->removeWidget( frm );
   suppFrames.removeAll( frm );
   delete frm;
}

void VkSuppressionsDialog::ToolChanged( int idx )
{
   type_cmb->clear();
   type_cmb->addItems( SuppRanges::instance().getKindTypes()[idx] );
}

void VkSuppressionsDialog::TypeChanged( int )
{
   QRegExp re("^Param$", Qt::CaseInsensitive);
   bool hasAux = type_cmb->currentText().contains(re);
   kaux_lbl->setEnabled( hasAux );
   kaux_le->setEnabled( hasAux );
}


void VkSuppressionsDialog::setSupp( const Suppression& supp )
{
   // Name
   name_le->setText( supp.getName() );

   // Kind
   QStringList kind = supp.getKind().split(":");
   vk_assert( kind.count() == 2 );
   tool_cmb->setCurrentIndex( tool_cmb->findText(kind[0], Qt::MatchExactly) );
   type_cmb->setCurrentIndex( type_cmb->findText(kind[1], Qt::MatchExactly) );

   // Kaux
   if ( !supp.getKAux().isEmpty() )
      kaux_le->setText( supp.getKAux() );

   // Call-Chain
   QStringList frames = supp.getFrames();
   vk_assert( frames.count() > 0 );
   for (int i=0; i<frames.count(); i++) {
      // new widgets for new frames, apart from first
      if ( i>0) addNewSuppFrame();
      
      QStringList frame = frames.at(i).split(":");
      vk_assert( frame.count() == 2 );
      
      QComboBox* cmb = suppFrames[i]->frame_cmb;
      cmb->setCurrentIndex( cmb->findText( frame[0], Qt::MatchExactly ) );
      QLineEdit* le = suppFrames[i]->frame_le;
      le->setText( frame[1] );
   }
}

const Suppression VkSuppressionsDialog::getUpdatedSupp()
{
   Suppression supp;

   // Name
   QString name = name_le->text();
   supp.setName( name.isEmpty() ? "<unknown>" : name );

   // Kind
   supp.setKind( tool_cmb->currentText() + ":" + type_cmb->currentText() );
   
   // Kaux
   if ( kaux_le->isEnabled() && !kaux_le->text().isEmpty() )
      supp.setKindAux( kaux_le->text() );

   // Call Chain
   for (int i=0; i<suppFrames.count(); i++) {
      QString type = suppFrames[i]->frame_cmb->currentText();
      QString data = suppFrames[i]->frame_le->text();
      if ( i==0 && data.isEmpty() )
         data = "<unknown>";
      if ( !data.isEmpty() )
         supp.addFrame(  type + ":" + data );
   }
   
   return supp;
}
