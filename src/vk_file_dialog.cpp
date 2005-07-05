/* ---------------------------------------------------------------------
 * Custom file dialog                                 vk_file_dialog.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_file_dialog.h"
#include "filedlg_icons.h"

#include <qheader.h>
#include <qtooltip.h>



FileDialog::~FileDialog() 
{ 
	// #2822
  
}


FileDialog::FileDialog( QWidget* parent, const char* name )
	: QDialog( parent, name, true, WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu )
{
  setSizeGripEnabled( true );
  geometryDirty = true;
  
  /* layout stuff */
  nameEdit = new QLineEdit( this, "name/filter editor" );
  nameEdit->setMaxLength( 255 );  /*_POSIX_MAX_PATH */
	// connect #2427
  //nameEdit->installEventFilter( this );
  
  splitter = new QSplitter( this, "splitter" );
	stack    = new QWidgetStack( splitter, "files and more files" );
  splitter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, 
																				QSizePolicy::Expanding ) );

  files = new FileDialogFileListView( stack, this );
  QFontMetrics fm = fontMetrics();
  files->addColumn( "Name" );
  files->addColumn( "Size" );
  files->setColumnAlignment( 1, AlignRight );
  files->addColumn( "Type" );
  files->addColumn( "Date" );
  files->addColumn( "Attributes" );
  files->header()->setStretchEnabled( true, 0 );
  files->setMinimumSize( 50, 25 + 2*fm.lineSpacing() );
  //files->installEventFilter( this );
  //files->viewport()->installEventFilter( this );
	// connect #2449
  
  moreFiles = new FileListBox( stack, this );
  moreFiles->setRowMode( QListBox::FitToHeight );
  moreFiles->setVariableWidth( true );
	// connect #2469
  //moreFiles->installEventFilter( this );
  //moreFiles->viewport()->installEventFilter( this );
	
  okB = new QPushButton( "&OK", this, "OK" ); 
	// or "Save (see other "OK")
  okB->setDefault( true );
  okB->setEnabled( false );
  //connect( okB, SIGNAL(clicked()), this, SLOT(okClicked()) );
  cancelB = new QPushButton( "Cancel" , this, "Cancel" );
  //connect( cancelB, SIGNAL(clicked()), this, SLOT(cancelClicked()) );

  paths = new QComboBox( true, this, "directory history/editor" );
  paths->setDuplicatesEnabled( false );
  paths->setInsertionPolicy( QComboBox::NoInsertion );
  // QFileInfoList ... #2491
  //paths->installEventFilter( this );

	types = new QComboBox( true, this, "file types" );
  types->setDuplicatesEnabled( false );
  types->setEditable( false );
	// connect #2519

  pathL = new QLabel( paths,    "Look &in:",   this, "lookin_lbl" );
  fileL = new QLabel( nameEdit, "File &name:", this, "filename_lbl" );
  typeL = new QLabel( types,    "File &type:", this, "filetype_lbl" );
  
  goBack = new QToolButton( this, "go back" );
  goBack->setEnabled( false );
  goBack->setFocusPolicy( TabFocus );
  //connect( goBack, SIGNAL( clicked() ), this, SLOT( goBack() ) );
  //RM: goBack->setIconSet( *goBackIcon );
  goBack->setPixmap( QPixmap(back_xpm) );
  goBack->setAutoRaise( true );
  QToolTip::add( goBack, "Back" );

  cdToParent = new QToolButton( this, "cd to parent" );
  cdToParent->setFocusPolicy( TabFocus );
  //RM: cdToParent->setIconSet( *cdToParentIcon );
  cdToParent->setPixmap( QPixmap(cdtoparent_xpm) );
  cdToParent->setAutoRaise( true );
  QToolTip::add( cdToParent, "One directory up" );
	// connect #2543

  modeButtons = new QButtonGroup( 0, "invisible group" );
  connect( modeButtons, SIGNAL(destroyed()),
           this,        SLOT(modeButtonsDestroyed()) );
  modeButtons->setExclusive( true );
	// connect #2559

  mcView = new QToolButton( this, "mclistbox view" );
  mcView->setFocusPolicy( TabFocus );
  //RM: mcView->setIconSet( *multiColumnListViewIcon );
  mcView->setPixmap( QPixmap(mclistview_xpm) );
  mcView->setToggleButton( true );
  mcView->setAutoRaise( true );
  mcView->setOn( true );
  QToolTip::add( mcView, "List View" );
  stack->addWidget( moreFiles, modeButtons->insert( mcView ) );

  detailView = new QToolButton( this, "list view" );
  detailView->setFocusPolicy( TabFocus );
  //RM: detailView->setIconSet( *detailViewIcon );
  detailView->setPixmap( QPixmap(detailedview_xpm) );
  detailView->setToggleButton( true );
  detailView->setAutoRaise( true );
  QToolTip::add( detailView, "Detail View" );
  stack->addWidget( files, modeButtons->insert( detailView ) );

  previewContents = new QToolButton( this, "preview info view" );
  previewContents->setFocusPolicy( TabFocus );
  previewContents->setAutoRaise( true );
  //RM: previewContents->setIconSet( *previewContentsViewIcon );
  previewContents->setPixmap( QPixmap(previewcontentsview_xpm) );
  previewContents->setToggleButton( true );
  QToolTip::add( previewContents, "Preview File Contents" );
  modeButtons->insert( previewContents );

  stack->raiseWidget( moreFiles );

	// at #2625
}

