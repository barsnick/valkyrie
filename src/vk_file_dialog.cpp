/* ---------------------------------------------------------------------
 * Custom file dialog                                 vk_file_dialog.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_dialogs.h"

static const char* home[] = {
 "16 15 4 1",
 "# c #000000",
 "a c #ffffff",
 "b c #c0c0c0",
 ". c None",
 ".......##.......",
 "..#...####......",
 "..#..#aabb#.....",
 "..#.#aaaabb#....",
 "..##aaaaaabb#...",
 "..#aaaaaaaabb#..",
 ".#aaaaaaaaabbb#.",
 "###aaaaaaaabb###",
 "..#aaaaaaaabb#..",
 "..#aaa###aabb#..",
 "..#aaa#.#aabb#..",
 "..#aaa#.#aabb#..",
 "..#aaa#.#aabb#..",
 "..#aaa#.#aabb#..",
 "..#####.######.."
};

















/*
hand_book.cpp:
QString fn = QFileDialog::getOpenFileName( vkConfig->vkdocDir(),
memcheck_view.cpp:
QString log_file = QFileDialog::getOpenFileName( QString::null,
memcheck_view.cpp
QString merge_file = QFileDialog::getOpenFileName( QString::null,
QString fname = QFileDialog::getSaveFileName( fi.dirPath(),
options_widgets.cpp:
QStringList supp_files = QFileDialog::getOpenFileNames(
cachegrind_options_page.cpp:
QString pidfile = QFileDialog::getOpenFileName( QString::null,
QString incdir = QFileDialog::getExistingDirectory( QString::null,
valkyrie_options_page.cpp:
QString ed_path = QFileDialog::getOpenFileName( fi.dirPath(),
QString binfile = QFileDialog::getOpenFileName( QString::null,
QString supp_dir = QFileDialog::getExistingDirectory( dir.absPath(),
QString vg_exec_path = QFileDialog::getOpenFileName( "/home",
*/

#if 0
/* class VkFileDialog -------------------------------------------------- */
VkFileDialog::~VkFileDialog()
{ }

VkFileDialog::VkFileDialog() : QFileDialog( 0, 0, true )
{
  QToolButton* tb = new QToolButton( this );
  tb->setPixmap( QPixmap( home ) );
  connect( tb, SIGNAL( clicked() ), this, SLOT( goHome() ) );
  addToolButton( tb );
  QToolTip::add( tb, "Go Home!" );

  tb = new QToolButton( this );
  tb->setPixmap( QPixmap( home ) );
  connect( tb, SIGNAL( clicked() ), this, SLOT( showHiddenFiles() ) );
  addToolButton( tb );
  QToolTip::add( tb, "Show hidden files" );

	/* ExistingFile, AnyFile, Directory */
	mode    = QFileDialog::ExistingFile;
  start   = QDir::currentDirPath();
  filter  = QString::null;
  caption = "Choose file...";

	//QFileDialog fd( QString::null, filter, 0, 0, true );
	setMode( mode );
	setCaption( caption );
	setSelection( start );

	PreviewWidget* pw = new PreviewWidget( &fd );
	setContentsPreviewEnabled( true );
	setContentsPreview( pw, pw );
	/* List:   multi-col list of icons+names; 
		 Detail: single-col list of icons+names+size+type+date+attribs */
	setViewMode( QFileDialog::List );
	setPreviewMode( QFileDialog::Contents );
}

void VkFileDialog::goHome()
{
  if ( getenv( "HOME" ) )
    setDir( getenv( "HOME" ) );
  else
    setDir( "/" );
}

void VkFileDialog::showHiddenFiles()
{
  setShowHiddenFiles( !showHiddenFiles() )
}

/* class PixmapView ---------------------------------------------------- */
PixmapView::PixmapView( QWidget *parent )
	: QScrollView( parent )
{ viewport()->setBackgroundMode( PaletteBase ); }

void PixmapView::setPixmap( const QPixmap &pix )
{
  pixmap = pix;
  resizeContents( pixmap.size().width(), pixmap.size().height() );
  viewport()->repaint( FALSE );
}

void PixmapView::drawContents( QPainter *p, int cx, int cy, 
                               int cw, int ch )
{
  p->fillRect( cx, cy, cw, ch, colorGroup().brush( QColorGroup::Base ) );
  p->drawPixmap( 0, 0, pixmap );
}


/* class Preview ------------------------------------------------------- */
Preview::Preview( QWidget* parent )
	: QWidgetStack( parent )
{
  normalText = new QMultiLineEdit( this );
  normalText->setReadOnly( true );
  html   = new QTextView( this );
  pixmap = new PixmapView( this );
  raiseWidget( normalText );
}

void Preview::showPreview( const QUrl& u, int size )
{
  if ( ! u.isLocalFile() ) {
    normalText->setText( "I only show local files!" );
    raiseWidget( normalText );
		return;
	} 

	QString path = u.path();
	QFileInfo fi( path );
	if ( fi.isFile() && (int)fi.size() > size * 1000 ) {
		normalText->setText( tr( "The File\n%1\nis too large, so I don't show it!" ).arg( path ) );
		raiseWidget( normalText );
		return;
	}
	
	QPixmap pix( path );
	if ( !pix.isNull () {
		pixmap->setPixmap( pix );
		raiseWidget( pixmap );
	} else {
		if ( fi.isFile() ) {
			QFile f( path );
			if ( f.open( IO_ReadOnly ) ) {
				QTextStream ts( &f );
				QString text = ts.read();
				f.close();
				if ( fi.extension().lower().contains( "htm" ) ) {
					QString url = html->mimeSourceFactory()->makeAbsolute( path, html->context() );
					html->setText( text, url );   
					raiseWidget( html );
					return;
				} else {
					normalText->setText( text );   
					raiseWidget( normalText );
					return;
				}
			}
		}
		normalText->setText( QString::null );
		raiseWidget( normalText );
	} 

}

/* class PreviewWidget ------------------------------------------------- */
PreviewWidget::PreviewWidget( QWidget *parent )
	: QVBox( parent ), QFilePreview()
{
	setSpacing( 5 );
	setMargin( 5 );
	//QHBox *row = new QHBox( this );
	//row->setSpacing( 5 );
	//(void)new QLabel( tr( "Only show files smaller than: " ), row );
	//sizeSpinBox = new QSpinBox( 1, 10000, 1, row );
	//sizeSpinBox->setSuffix( " KB" );
	//sizeSpinBox->setValue( 64 );
	//row->setFixedHeight( 10 + sizeSpinBox->sizeHint().height() );
	preview = new Preview( this );
}

void PreviewWidget::previewUrl( const QUrl& url )
{ preview->showPreview( url/*, 1000sizeSpinBox->value()*/ ); }
#endif
