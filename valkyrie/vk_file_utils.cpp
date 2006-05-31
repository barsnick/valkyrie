/* ---------------------------------------------------------------------
 * Custom file dialog                                  vk_file_utils.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_file_utils.h"
#include "filedlg_icons.h"
#include "vk_messages.h"
#include "vk_utils.h"

#include <qapplication.h>
#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qcleanuphandler.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <private/qapplication_p.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcstring.h>
#include <qcursor.h>
#include <qfile.h>
#include <qguardedptr.h>
#include <qhbox.h>
#include <qheader.h>
#include <qmap.h>
#include <qmessagebox.h>
#include <qmime.h>
#include <qnetworkprotocol.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qptrvector.h>
#include <qregexp.h>
#include <qsemimodal.h>
#include <qstrlist.h>
#include <qstyle.h>
#include <qtoolbutton.h>

//RM:
#include <qplatformdefs.h>
#include <qlibrary.h>


// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
#undef truncate
#endif

#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>

static QPixmap * openFolderIcon = 0;
static QPixmap * closedFolderIcon = 0;
static QPixmap * fifteenTransparentPixels = 0;
static QPixmap * symLinkDirIcon = 0;
static QPixmap * symLinkFileIcon = 0;
static QPixmap * fileIcon = 0;
static QPixmap * startCopyIcon = 0;
static QPixmap * endCopyIcon = 0;
static int lastWidth = 0;
static int lastHeight = 0;
static QString * workingDirectory = 0;

static bool bShowHiddenFiles = false;
static int sortFilesBy = (int)QDir::Name;
static bool sortAscending = true;
static bool detailViewMode = false;

static QCleanupHandler<QPixmap> qfd_cleanup_pixmap;
static QCleanupHandler<QString> qfd_cleanup_string;

static bool isDirectoryMode( int m )
{
   return m == FileDialog::Directory || m == FileDialog::DirectoryOnly;
}

static void updateLastSize( FileDialog *that )
{
   int extWidth = 0;
   int extHeight = 0;
   if ( that->extension() && that->extension()->isVisible() ) {
      if ( that->orientation() == Qt::Vertical )
         extHeight = that->extension()->height();
      else
         extWidth = that->extension()->width();
   }
   lastWidth = that->width() - extWidth;
   lastHeight = that->height() - extHeight;
}


static void makeVariables() {
   if ( !openFolderIcon ) {
      workingDirectory = new QString( QDir::currentDirPath() );
      qfd_cleanup_string.add( &workingDirectory );

      openFolderIcon = new QPixmap( (const char **)open_xpm);
      qfd_cleanup_pixmap.add( &openFolderIcon );
      symLinkDirIcon = new QPixmap( (const char **)link_dir_xpm);
      qfd_cleanup_pixmap.add( &symLinkDirIcon );
      symLinkFileIcon = new QPixmap( (const char **)link_file_xpm);
      qfd_cleanup_pixmap.add( &symLinkFileIcon );
      fileIcon = new QPixmap( (const char **)file_xpm);
      qfd_cleanup_pixmap.add( &fileIcon );
      closedFolderIcon = new QPixmap( (const char **)closed_xpm);
      qfd_cleanup_pixmap.add( &closedFolderIcon );
      startCopyIcon = new QPixmap( (const char **)start_xpm );
      qfd_cleanup_pixmap.add( &startCopyIcon );
      endCopyIcon = new QPixmap( (const char **)end_xpm );
      qfd_cleanup_pixmap.add( &endCopyIcon );
      fifteenTransparentPixels = new QPixmap( closedFolderIcon->width(), 1 );
      qfd_cleanup_pixmap.add( &fifteenTransparentPixels );
      QBitmap bm( fifteenTransparentPixels->width(), 1 );
      bm.fill( Qt::color0 );
      fifteenTransparentPixels->setMask( bm );
      bShowHiddenFiles = false;
      sortFilesBy = (int)QDir::Name;
      detailViewMode = false;
   }
}


class FileDialogPrivate 
{
public:
   ~FileDialogPrivate();

   QStringList history;

   bool geometryDirty;
   QComboBox* paths;
   QComboBox* types;
   QLabel* pathL;
   QLabel* fileL;
   QLabel* typeL;

   QVBoxLayout* topLevelLayout;
   QHBoxLayout* buttonLayout, *leftLayout, *rightLayout;
   QPtrList<QHBoxLayout> extraWidgetsLayouts;
   QPtrList<QLabel> extraLabels;
   QPtrList<QWidget> extraWidgets;
   QPtrList<QWidget> extraButtons;
   QPtrList<QButton> toolButtons;

   QWidgetStack* stack;

   QToolButton * cdToParent, *goHome, *previewContents, *goBack;
   QToolButton* mcol_detailView;

   QString currentFileName;

   QListViewItem* last;
   QListBoxItem* lastEFSelected;

   struct File: public QListViewItem {
      File( FileDialogPrivate* dlgp, const QUrlInfo* fi, 
            QListViewItem* parent )
         : QListViewItem( parent, dlgp->last ), info( *fi ), 
           d(dlgp), i( 0 ), hasMimePixmap( false )
      { setup(); dlgp->last = this; }
      File( FileDialogPrivate * dlgp,
            const QUrlInfo * fi, QListView * parent )
         : QListViewItem( parent, dlgp->last ), info( *fi ), 
           d(dlgp), i( 0 ), hasMimePixmap( false )
      { setup(); dlgp->last = this; }
      File( FileDialogPrivate * dlgp,
            const QUrlInfo * fi, QListView * parent, QListViewItem * after )
         : QListViewItem( parent, after ), info( *fi ), 
           d(dlgp), i( 0 ), hasMimePixmap( false )
      { setup(); if ( !nextSibling() ) dlgp->last = this; }
      ~File();
    
      QString text( int column ) const;
      const QPixmap * pixmap( int ) const;

      QUrlInfo info;
      FileDialogPrivate * d;
      QListBoxItem *i;
      bool hasMimePixmap;
   };
  
   class MCItem: public QListBoxItem {
   public:
      MCItem( QListBox *, QListViewItem * item );
      MCItem( QListBox *, QListViewItem * item, QListBoxItem *after );
      QString text() const;
      const QPixmap *pixmap() const;
      int height( const QListBox * ) const;
      int width( const QListBox * ) const;
      void paint( QPainter * );
      QListViewItem * i;
   };

   class UrlInfoList : public QPtrList<QUrlInfo> {
   public:
      UrlInfoList() { setAutoDelete( true ); }
      int compareItems( QPtrCollection::Item n1, 
                        QPtrCollection::Item n2 ) {
         if ( !n1 || !n2 )
            return 0;
      
         QUrlInfo *i1 = ( QUrlInfo *)n1;
         QUrlInfo *i2 = ( QUrlInfo *)n2;

         if ( i1->isDir() && !i2->isDir() )
            return -1;
         if ( !i1->isDir() && i2->isDir() )
            return 1;

         if ( i1->name() == ".." )
            return -1;
         if ( i2->name() == ".." )
            return 1;

         if ( QUrlInfo::equal( *i1, *i2, sortFilesBy ) )
            return 0;
         else if ( QUrlInfo::greaterThan( *i1, *i2, sortFilesBy ) )
            return 1;
         else if ( QUrlInfo::lessThan( *i1, *i2, sortFilesBy ) )
            return -1;
         // can't happen...
         return 0;
      }
      QUrlInfo *operator[]( int i ) {
         return at( i );
      }
   };

   UrlInfoList sortedList;
   QPtrList<File> pendingItems;

   FileDialog::Mode mode;

   QString rw;
   QString ro;
   QString wo;
   QString inaccessible;

   QString symLinkToFile;
   QString file;
   QString symLinkToDir;
   QString dir;
   QString symLinkToSpecial;
   QString special;
   PreviewStack* preview;
   QSplitter* splitter;
   QUrlOperator url, oldUrl;
   bool hadDotDot;
   bool contentsPreview;
   bool ignoreNextKeyPress;
   // ignores the next refresh operation in case the user forced a selection
   bool ignoreNextRefresh;
   bool checkForFilter;
   bool ignoreStop;
  
   const QNetworkOperation *currListChildren;

   /* this is similar to QUrl::encode but does encode "*" and doesn't
      encode whitespaces */
   static QString encodeFileName( const QString& fName ) {
      QString newStr;
      QCString cName = fName.utf8();
      const QCString sChars( "<>#@\"&%$:,;?={}|^~[]\'`\\*" );
      int len = cName.length();
      if ( !len )
         return QString::null;
      for ( int i = 0; i < len ;++i ) {
         uchar inCh = (uchar)cName[ i ];
         if ( inCh >= 128 || sChars.contains(inCh) ) {
            newStr += QChar( '%' );
            ushort c = inCh / 16;
            c += c > 9 ? 'A' - 10 : '0';
            newStr += (char)c;
            c = inCh % 16;
            c += c > 9 ? 'A' - 10 : '0';
            newStr += (char)c;
         } else {
            newStr += (char)inCh;
         }
      }
      return newStr;
   }

   static bool fileExists( const QUrlOperator &url, const QString &name ) {
      QUrl u( url, FileDialogPrivate::encodeFileName(name) );
      if ( u.isLocalFile() ) {
         QFileInfo f( u.path() );
         return f.exists();
      } 
      else {
         QNetworkProtocol* p = QNetworkProtocol::getNetworkProtocol( url.protocol() );
         if ( p && (p->supportedOperations()&QNetworkProtocol::OpListChildren) ) {
            QUrlInfo ui( url, name );
            return ui.isValid();
         }
      }
      return true;
   }

   bool cursorOverride; // Remember if the cursor was overridden or not.
};

FileDialogPrivate::~FileDialogPrivate()
{ }




RenameEdit::RenameEdit( QWidget* parent ) 
	: QLineEdit( parent, "rename_edit" )
{
	m_doRenameAlreadyEmitted = false;
	connect( this, SIGNAL( returnPressed() ), 
            this, SLOT( slotReturnPressed() ) ); 
}

void RenameEdit::keyPressEvent( QKeyEvent *e )
{
   if ( e->key() == Key_Escape )
      emit cancelRename();
   else
      QLineEdit::keyPressEvent( e );
   e->accept();
}

void RenameEdit::focusOutEvent( QFocusEvent * )
{
   if ( !m_doRenameAlreadyEmitted )
      emit doRename();
   else
      m_doRenameAlreadyEmitted = false;
}

void RenameEdit::slotReturnPressed()
{
   m_doRenameAlreadyEmitted = true;
   emit doRename();
}


FileListBox::FileListBox( QWidget* parent, FileDialog* dlg )
   : QListBox( parent, "filelistbox" )
{
   m_filedialog           = dlg;
   m_renaming             = false;
   m_renameItem           = 0;
   m_mousePressed         = false;
   m_firstMousePressEvent = true;

   m_changeDirTimer = new QTimer( this );
   QVBox* box = new QVBox( viewport(), "vbox" );
   box->setFrameStyle( QFrame::Box | QFrame::Plain );
   m_lined = new RenameEdit( box );
   m_lined->setFixedHeight( m_lined->sizeHint().height() );
   box->hide();
   box->setBackgroundMode( PaletteBase );
   m_renameTimer = new QTimer( this );
   connect( m_lined, SIGNAL( doRename() ),
            this, SLOT (rename() ) );
   connect( m_lined, SIGNAL( cancelRename() ),
            this, SLOT( cancelRename() ) );
   connect( m_renameTimer, SIGNAL( timeout() ),
            this, SLOT( doubleClickTimeout() ) );
   connect( this, SIGNAL( contentsMoving( int, int ) ),
            this, SLOT( contentsMoved( int, int ) ) );
}

void FileListBox::show()
{
   setBackgroundMode( PaletteBase );
   viewport()->setBackgroundMode( PaletteBase );
   QListBox::show();
}

void FileListBox::keyPressEvent( QKeyEvent *e )
{
   if ( ( e->key() == Key_Enter ||
          e->key() == Key_Return ) &&
        m_renaming )
      return;

   QString keyPressed = ((QKeyEvent *)e)->text().lower();
   QChar keyChar = keyPressed[0];
   if ( keyChar.isLetterOrNumber() ) {
      QListBoxItem * i = 0;
      if ( currentItem() )
         i = item( currentItem() );
      else
         i = firstItem();
      if ( i->next() )
         i = i->next();
      else
         i = firstItem();
      while ( i != item( currentItem() ) ) {
         QString it = text( index( i ) );
         if ( it[0].lower() == keyChar ) {
            clearSelection();
            setCurrentItem( i );
         } else {
            if ( i->next() )
               i = i->next();
            else
               i = firstItem();
         }
      }
   }
   cancelRename();
   QListBox::keyPressEvent( e );
}

void FileListBox::viewportMousePressEvent( QMouseEvent *e )
{
   m_pressPos = e->pos();
   m_mousePressed = false;

   bool didRename = m_renaming;

   cancelRename();
   if ( !hasFocus() && !viewport()->hasFocus() )
      setFocus();

   if ( e->button() != LeftButton ) {
      QListBox::viewportMousePressEvent( e );
      m_firstMousePressEvent = false;
      return;
   }

   int i = currentItem();
   bool wasSelected = false;
   if ( i != -1 )
      wasSelected = item( i )->isSelected();
   QListBox::mousePressEvent( e );
  
   FileDialogPrivate::MCItem *i1 = (FileDialogPrivate::MCItem*)item( currentItem() );
   if ( i1 )
      m_mousePressed = ( !( (FileDialogPrivate::File*)i1->i )->info.isDir() ) || 
         ( m_filedialog->mode() == FileDialog::Directory ) || 
         ( m_filedialog->mode() == FileDialog::DirectoryOnly );
  
   if ( itemAt( e->pos() ) != item( i ) ) {
      m_firstMousePressEvent = false;
      return;
   }

   if ( !m_firstMousePressEvent && !didRename && 
        i == currentItem() && currentItem() != -1 &&
        wasSelected && QUrlInfo( m_filedialog->d->url, "." ).isWritable() && 
        item( currentItem() )->text() != ".." ) {
      m_renameTimer->start( QApplication::doubleClickInterval(), true );
      m_renameItem = item( i );
   }

   m_firstMousePressEvent = false;
}

void FileListBox::viewportMouseReleaseEvent( QMouseEvent *e )
{
   QListBox::viewportMouseReleaseEvent( e );
   m_mousePressed = false;
}

void FileListBox::viewportMouseDoubleClickEvent( QMouseEvent *e )
{
   m_renameTimer->stop();
   QListBox::viewportMouseDoubleClickEvent( e );
}

void FileListBox::doubleClickTimeout()
{
   startRename();
   m_renameTimer->stop();
}

void FileListBox::startRename( bool check )
{
   if ( check && ( !m_renameItem ||
                   m_renameItem != item( currentItem() ) ) )
      return;

   int i = currentItem();
   setSelected( i, true );
   QRect r = itemRect( item( i ) );
   int bdr = item( i )->pixmap() ?
      item( i )->pixmap()->width() : 16;
   int x = r.x() + bdr;
   int y = r.y();
   int w = item( i )->width( this ) - bdr;
   int h = QMAX( m_lined->height() + 2, r.height() );
   y = y + r.height() / 2 - h / 2;
  
   m_lined->parentWidget()->setGeometry( x, y, w + 6, h );
   m_lined->setFocus();
   m_lined->setText( item( i )->text() );
   m_lined->selectAll();
   m_lined->setFrame( false );
   m_lined->parentWidget()->show();
   viewport()->setFocusProxy( m_lined );
   m_renaming = true;
}

void FileListBox::clear()
{
   cancelRename();
   QListBox::clear();
}

void FileListBox::rename()
{
   if ( !m_lined->text().isEmpty() ) {
      QString file = currentText();

      if ( m_lined->text() != file )
         m_filedialog->d->url.rename( file, m_lined->text() );
   }
   cancelRename();
}

void FileListBox::cancelRename()
{
   m_renameItem = 0;
   m_lined->parentWidget()->hide();
   viewport()->setFocusProxy( this );
   m_renaming = false;
   updateItem( currentItem() );
   if ( m_lined->hasFocus() )
      viewport()->setFocus();
}

void FileListBox::contentsMoved( int, int )
{
   m_changeDirTimer->stop();
}

FileListView::FileListView( QWidget* parent, FileDialog* dlg )
   : QListView( parent, "file_listview" )
{
   m_renaming             = false;
   m_renameItem           = 0;
   m_filedialog           = dlg;
   m_mousePressed         = false;
   m_firstMousePressEvent = true;
   m_sortcolumn           = 0;
   m_ascending            = true;

   m_changeDirTimer = new QTimer( this );
   QVBox* box = new QVBox( viewport(), "vbox" );
   box->setFrameStyle( QFrame::Box | QFrame::Plain );
   m_lined = new RenameEdit( box );
   m_lined->setFixedHeight( m_lined->sizeHint().height() );
   box->hide();
   box->setBackgroundMode( PaletteBase );
   m_renameTimer = new QTimer( this );
   connect( m_lined, SIGNAL( doRename() ),
            this, SLOT (rename() ) );
   connect( m_lined, SIGNAL( cancelRename() ),
            this, SLOT( cancelRename() ) );
   header()->setMovingEnabled( false );
   connect( m_renameTimer, SIGNAL( timeout() ),
            this, SLOT( doubleClickTimeout() ) );
   disconnect( header(), SIGNAL( sectionClicked( int ) ),
               this, SLOT( changeSortColumn( int ) ) );
   connect( header(), SIGNAL( sectionClicked( int ) ),
            this, SLOT( changeSortColumn2( int ) ) );
   connect( this, SIGNAL( contentsMoving( int, int ) ),
            this, SLOT( contentsMoved( int, int ) ) );
}

void FileListView::setSorting( int column, bool increasing )
{
   if ( column == -1 ) {
      QListView::setSorting( column, increasing );
      return;
   }

   sortAscending = m_ascending = increasing;
   m_sortcolumn = column;
   switch ( column ) {
   case 0:
      sortFilesBy = QDir::Name;
      break;
   case 1:
      sortFilesBy = QDir::Size;
      break;
   case 3:
      sortFilesBy = QDir::Time;
      break;
   default:
      sortFilesBy = QDir::Name; // #### ???
      break;
   }

   m_filedialog->resortDir();
}

void FileListView::changeSortColumn2( int column )
{
   int lcol = header()->mapToLogical( column );
   setSorting( lcol, m_sortcolumn == lcol ? !m_ascending : true );
}

void FileListView::keyPressEvent( QKeyEvent *e )
{
   if ( ( e->key() == Key_Enter ||
          e->key() == Key_Return ) &&
        m_renaming )
      return;

   QString keyPressed = e->text().lower();
   QChar keyChar = keyPressed[0];
   if ( keyChar.isLetterOrNumber() ) {
      QListViewItem * i = 0;
      if ( currentItem() )
         i = currentItem();
      else
         i = firstChild();
      if ( i->nextSibling() )
         i = i->nextSibling();
      else
         i = firstChild();
      while ( i != currentItem() ) {
         QString it = i->text(0);
         if ( it[0].lower() == keyChar ) {
            clearSelection();
            ensureItemVisible( i );
            setCurrentItem( i );
         } else {
            if ( i->nextSibling() )
               i = i->nextSibling();
            else
               i = firstChild();
         }
      }
      return;
   }

   cancelRename();
   QListView::keyPressEvent( e );
}

void FileListView::viewportMousePressEvent( QMouseEvent *e )
{
   m_pressPos = e->pos();
   m_mousePressed = false;

   bool didRename = m_renaming;
   cancelRename();
   if ( !hasFocus() && !viewport()->hasFocus() )
      setFocus();

   if ( e->button() != LeftButton ) {
      QListView::viewportMousePressEvent( e );
      m_firstMousePressEvent = false;
      return;
   }

   QListViewItem *i = currentItem();
   QListView::viewportMousePressEvent( e );

   FileDialogPrivate::File *i1 = (FileDialogPrivate::File*)currentItem();
   if ( i1 )
      m_mousePressed = !i1->info.isDir() || 
         ( m_filedialog->mode() == FileDialog::Directory ) || 
         ( m_filedialog->mode() == FileDialog::DirectoryOnly );

   if ( itemAt( e->pos() ) != i ||
        e->x() + contentsX() > columnWidth( 0 ) ) {
      m_firstMousePressEvent = false;
      return;
   }

   if ( !m_firstMousePressEvent && !didRename && 
        i == currentItem() && currentItem() &&
        QUrlInfo( m_filedialog->d->url, "." ).isWritable() && 
        currentItem()->text( 0 ) != ".." ) {
      m_renameTimer->start( QApplication::doubleClickInterval(), true );
      m_renameItem = currentItem();
   }

   m_firstMousePressEvent = false;
}

void FileListView::viewportMouseDoubleClickEvent( QMouseEvent *e )
{
   m_renameTimer->stop();
   QListView::viewportMouseDoubleClickEvent( e );
}

void FileListView::viewportMouseReleaseEvent( QMouseEvent *e )
{
   QListView::viewportMouseReleaseEvent( e );
   m_mousePressed = false;
}

void FileListView::doubleClickTimeout()
{
   startRename();
   m_renameTimer->stop();
}

void FileListView::startRename( bool check )
{
   if ( check && ( !m_renameItem || m_renameItem != currentItem() ) )
      return;

   QListViewItem *i = currentItem();
   setSelected( i, true );

   QRect r = itemRect( i );
   int bdr = i->pixmap( 0 ) ?
      i->pixmap( 0 )->width() : 16;
   int x = r.x() + bdr;
   int y = r.y();
   int w = columnWidth( 0 ) - bdr;
   int h = QMAX( m_lined->height() + 2, r.height() );
   y = y + r.height() / 2 - h / 2;

   m_lined->parentWidget()->setGeometry( x, y, w + 6, h );
   m_lined->setFocus();
   m_lined->setText( i->text( 0 ) );
   m_lined->selectAll();
   m_lined->setFrame( false );
   m_lined->parentWidget()->show();
   viewport()->setFocusProxy( m_lined );
   m_renaming = true;
}

void FileListView::clear()
{
   cancelRename();
   QListView::clear();
}

void FileListView::rename()
{
   if ( !m_lined->text().isEmpty() ) {
      QString file = currentItem()->text( 0 );

      if ( m_lined->text() != file )
         m_filedialog->d->url.rename( file, m_lined->text() );
   }
   cancelRename();
}

void FileListView::cancelRename()
{
   m_renameItem = 0;
   m_lined->parentWidget()->hide();
   viewport()->setFocusProxy( this );
   m_renaming = false;
   if ( currentItem() )
      currentItem()->repaint();
   if ( m_lined->hasFocus() )
      viewport()->setFocus();
}

void FileListView::contentsMoved( int, int )
{
   m_changeDirTimer->stop();
}


FileDialogPrivate::File::~File()
{
   if ( d->pendingItems.findRef( this ) )
      d->pendingItems.removeRef( this );
}

QString FileDialogPrivate::File::text( int column ) const
{
   makeVariables();

   switch( column ) {
   case 0:
      return info.name();
   case 1:
      if ( info.isFile() ) {
         uint size = info.size();
#if defined(QT_LARGEFILE_SUPPORT) && defined(Q_OS_UNIX)
         // ### the following code should not be needed as soon
         // ### as QUrlInfo::size() can return 64-bit
         if ( size > INT_MAX ) {
            struct stat buffer;
            if ( ::stat( QFile::encodeName(info.name()), &buffer ) == 0 ) {
#if (QT_VERSION-0 >= 0x030200)
               Q_ULLONG size64 = (Q_ULLONG)buffer.st_size;
               return QString::number(size64);
#else // QT_VERSION < 3.2
               off_t n = buffer.st_size;
               // ### use QString::number() instead when it has support
               // ### for 64-bit integers (off_t) on all platforms
               if ( n > 0 ) {
                  char charbuf[21*sizeof(QChar)];
                  QChar *buf = (QChar*)charbuf;
                  QChar *p = &buf[20];
                  uint len = 0;
                  do {
                     *--p = "0123456789"[(int)(n%10)];
                     n /= 10;
                     ++len;
                  } while ( n );
                  return QString( p, len );
               }
#endif
            }
         }
#endif
         return QString::number(size);
      } else {
         return QString::fromLatin1("");
      }
   case 2:
      if ( info.isFile() && info.isSymLink() ) {
         return d->symLinkToFile;
      } else if ( info.isFile() ) {
         return d->file;
      } else if ( info.isDir() && info.isSymLink() ) {
         return d->symLinkToDir;
      } else if ( info.isDir() ) {
         return d->dir;
      } else if ( info.isSymLink() ) {
         return d->symLinkToSpecial;
      } else {
         return d->special;
      }
   case 3: {
      return info.lastModified().toString( Qt::LocalDate );
   }
   case 4:
      if ( info.isReadable() )
         return info.isWritable() ? d->rw : d->ro;
      else
         return info.isWritable() ? d->wo : d->inaccessible;
   }
  
   return QString::fromLatin1("<--->");
}

const QPixmap * FileDialogPrivate::File::pixmap( int column ) const
{
   if ( column ) {
      return 0;
   } else if ( QListViewItem::pixmap( column ) ) {
      return QListViewItem::pixmap( column );
   } else if ( info.isSymLink() ) {
      if ( info.isFile() )
         return symLinkFileIcon;
      else
         return symLinkDirIcon;
   } else if ( info.isDir() ) {
      return closedFolderIcon;
   } else if ( info.isFile() ) {
      return fileIcon;
   } else {
      return fifteenTransparentPixels;
   }
}

FileDialogPrivate::MCItem::MCItem( QListBox * lb, QListViewItem * item )
   : QListBoxItem()
{
   i = item;
   if ( lb )
      lb->insertItem( this );
}

FileDialogPrivate::MCItem::MCItem( QListBox* lb, QListViewItem* item, 
                                   QListBoxItem* after )
   : QListBoxItem()
{
   i = item;
   if ( lb )
      lb->insertItem( this, after );
}

QString FileDialogPrivate::MCItem::text() const
{
   return i->text( 0 );
}


const QPixmap *FileDialogPrivate::MCItem::pixmap() const
{
   return i->pixmap( 0 );
}


int FileDialogPrivate::MCItem::height( const QListBox * lb ) const
{
   int hf = lb->fontMetrics().height();
   int hp = pixmap() ? pixmap()->height() : 0;
   return QMAX(hf, hp) + 2;
}


int FileDialogPrivate::MCItem::width( const QListBox * lb ) const
{
   QFontMetrics fm = lb->fontMetrics();
   int w = 2;
   if ( pixmap() )
      w += pixmap()->width() + 4;
   else
      w += 18;
   w += fm.width( text() );
   w += -fm.minLeftBearing();
   w += -fm.minRightBearing();
   w += 6;
   return w;
}


void FileDialogPrivate::MCItem::paint( QPainter * ptr )
{
   QFontMetrics fm = ptr->fontMetrics();

   int h;

   if ( pixmap() )
      h = QMAX( fm.height(), pixmap()->height()) + 2;
   else
      h = fm.height() + 2;

   const QPixmap * pm = pixmap();
   if ( pm )
      ptr->drawPixmap( 2, 1, *pm );

   ptr->drawText( pm ? pm->width() + 4 : 22, h - fm.descent() - 2,
                  text() );
}

static QStringList makeFiltersList( const QString &filter )
{
   if ( filter.isEmpty() )
      return QStringList();

   int i = filter.find( ";;", 0 );
   QString sep( ";;" );
   if ( i == -1 ) {
      if ( filter.find( "\n", 0 ) != -1 ) {
         sep = "\n";
         i = filter.find( sep, 0 );
      }
   }

   return QStringList::split( sep, filter );
}



/* ------------------------------------------------------------------ */
extern const char qt_file_dialog_filter_reg_exp[] =
"([a-zA-Z0-9 ]*)\\(([a-zA-Z0-9_.*? +;#\\[\\]]*)\\)$";

FileDialog::FileDialog( QWidget *parent, const char *name, 
                        bool modal )
   : QDialog( parent, name, modal,
              (modal ? (WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu) : 0) )
{
   init();
   d->mode = ExistingFile;
   d->types->insertItem( "All Files (*)" );
   d->cursorOverride = false;
   emit dirEntered( d->url.dirPath() );
   rereadDir();
}


FileDialog::FileDialog( const QString &dirName, 
                        const QString & filter,
                        QWidget *parent, const char *name, 
                        bool modal )
   : QDialog( parent, name, modal,
              (modal ? (WStyle_Customize | WStyle_DialogBorder | WStyle_Title | WStyle_SysMenu) : 0) )
{
   init();
   d->mode = ExistingFile;
   rereadDir();
   QUrlOperator u( dirName );
   if ( !dirName.isEmpty() && 
        ( !u.isLocalFile() || QDir( dirName ).exists() ) )
      setSelection( dirName );
   else if ( workingDirectory && !workingDirectory->isEmpty() )
      setDir( *workingDirectory );
  
   if ( !filter.isEmpty() ) {
      setFilters( filter );
      if ( !dirName.isEmpty() ) {
         int dotpos = dirName.find( QChar('.'), 0, false );
         if ( dotpos != -1 ) {
            for ( int b=0 ; b<d->types->count() ; b++ ) {
               if ( d->types->text(b).contains( dirName.right( dirName.length() - dotpos ) ) ) {
                  d->types->setCurrentItem( b );
                  setFilter( d->types->text( b ) );
                  return;
               }
            }
         }
      }
   } 
   else {
      d->types->insertItem( "All Files (*)" );
   }
}


void FileDialog::init()
{
   setSizeGripEnabled( true );
   d = new FileDialogPrivate();
   d->mode = AnyFile;
   d->last = 0;
   d->lastEFSelected = 0;
   d->contentsPreview = true;
   d->hadDotDot = false;
   d->ignoreNextKeyPress = false;
   d->checkForFilter = false;
   d->ignoreNextRefresh = false;
   d->ignoreStop = false;
   d->pendingItems.setAutoDelete( false );
   d->cursorOverride = false;
   d->url = QUrlOperator( QDir::currentDirPath() );
   d->oldUrl = d->url;
   d->currListChildren = 0;

   connect( &d->url, SIGNAL( start( QNetworkOperation * ) ),
            this, SLOT( urlStart( QNetworkOperation * ) ) );
   connect( &d->url, SIGNAL( finished( QNetworkOperation * ) ),
            this, SLOT( urlFinished( QNetworkOperation * ) ) );
   connect( &d->url, SIGNAL( newChildren( const QValueList<QUrlInfo> &, QNetworkOperation * ) ),
            this, SLOT( insertEntry( const QValueList<QUrlInfo> &, QNetworkOperation * ) ) );
   connect( &d->url, SIGNAL( removed( QNetworkOperation * ) ),
            this, SLOT( removeEntry( QNetworkOperation * ) ) );
   connect( &d->url, SIGNAL( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ),
            this, SLOT( createdDirectory( const QUrlInfo &, QNetworkOperation * ) ) );
   connect( &d->url, SIGNAL( itemChanged( QNetworkOperation * ) ),
            this, SLOT( itemChanged( QNetworkOperation * ) ) );
   connect( &d->url, SIGNAL( dataTransferProgress( int, int, QNetworkOperation * ) ),
            this, SLOT( dataTransferProgress( int, int, QNetworkOperation * ) ) );
  
   m_nameEdit = new QLineEdit( this, "name/filter editor" );
   m_nameEdit->setMaxLength( 255 ); //_POSIX_MAX_PATH
   connect( m_nameEdit, SIGNAL(textChanged(const QString&)),
            this,  SLOT(fileNameEditDone()) );
   m_nameEdit->installEventFilter( this );
  
   d->splitter = new QSplitter( this, "qt_splitter" );
  
   d->stack = new QWidgetStack( d->splitter, "files and more files" );
  
   d->splitter->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
  
   m_files = new FileListView( d->stack, this );
   QFontMetrics fm = fontMetrics();
   m_files->addColumn( "Name" );
   m_files->addColumn( "Size" );
   m_files->setColumnAlignment( 1, AlignRight );
   m_files->addColumn( "Type" );
   m_files->addColumn( "Date" );
   m_files->addColumn( "Attribs" );
   m_files->header()->setStretchEnabled( true, 0 );
  
   m_files->setMinimumSize( 50, 25 + 2*fm.lineSpacing() );
  
   connect( m_files, SIGNAL( selectionChanged() ),
            this, SLOT( detailViewSelectionChanged() ) );
   connect( m_files, SIGNAL(currentChanged(QListViewItem *)),
            this, SLOT(updateFileNameEdit(QListViewItem *)) );
   connect( m_files, SIGNAL(doubleClicked(QListViewItem *)),
            this, SLOT(selectDirectoryOrFile(QListViewItem *)) );
   connect( m_files, SIGNAL(returnPressed(QListViewItem *)),
            this, SLOT(selectDirectoryOrFile(QListViewItem *)) );
   connect( m_files, SIGNAL(rightButtonPressed(QListViewItem *,
                                               const QPoint &, int)),
            this, SLOT(popupContextMenu(QListViewItem *,
                                        const QPoint &, int)) );
  
   m_files->installEventFilter( this );
   m_files->viewport()->installEventFilter( this );

   m_moreFiles = new FileListBox( d->stack, this );
   m_moreFiles->setRowMode( QListBox::FitToHeight );
   m_moreFiles->setVariableWidth( true );
  
   connect( m_moreFiles, SIGNAL(selected(QListBoxItem *)),
            this, SLOT(selectDirectoryOrFile(QListBoxItem *)) );
   connect( m_moreFiles, SIGNAL( selectionChanged() ),
            this, SLOT( listBoxSelectionChanged() ) );
   connect( m_moreFiles, SIGNAL(highlighted(QListBoxItem *)),
            this, SLOT(updateFileNameEdit(QListBoxItem *)) );
   connect( m_moreFiles, SIGNAL( rightButtonPressed( QListBoxItem *, const QPoint & ) ),
            this, SLOT( popupContextMenu( QListBoxItem *, const QPoint & ) ) );
  
   m_moreFiles->installEventFilter( this );
   m_moreFiles->viewport()->installEventFilter( this );
  
   m_okB = new QPushButton( "&OK", this, "OK" ); //### Or "Save (see other "OK")
   m_okB->setDefault( true );
   m_okB->setEnabled( false );
   connect( m_okB, SIGNAL(clicked()), this, SLOT(okClicked()) );
   m_cancelB = new QPushButton( "Cancel" , this, "Cancel" );
   connect( m_cancelB, SIGNAL(clicked()), this, SLOT(cancelClicked()) );
  
   d->paths = new QComboBox( true, this, "directory history/editor" );
   d->paths->setDuplicatesEnabled( false );
   d->paths->setInsertionPolicy( QComboBox::NoInsertion );
   const QFileInfoList * rootDrives = QDir::drives();
   QFileInfoListIterator it( *rootDrives );
   QFileInfo *fi;
   makeVariables();
  
   while ( (fi = it.current()) != 0 ) {
      ++it;
      d->paths->insertItem( *openFolderIcon, fi->absFilePath() );
   }
  
   if ( !!QDir::homeDirPath() ) {
      if ( !d->paths->listBox()->findItem( QDir::homeDirPath() ) )
         d->paths->insertItem( *openFolderIcon, QDir::homeDirPath() );
   }
  
   connect( d->paths, SIGNAL(activated(const QString&)),
            this, SLOT(setDir(const QString&)) );
  
   d->paths->installEventFilter( this );
   QObjectList *ol = d->paths->queryList( "QLineEdit" );
   if ( ol && ol->first() )
      ( (QLineEdit*)ol->first() )->installEventFilter( this );
   delete ol;

   d->geometryDirty = true;
   d->types = new QComboBox( true, this, "file types" );
   d->types->setDuplicatesEnabled( false );
   d->types->setEditable( false );
   connect( d->types, SIGNAL(activated(const QString&)),
            this, SLOT(setFilter(const QString&)) );
   connect( d->types, SIGNAL(activated(const QString&)),
            this, SIGNAL(filterSelected(const QString&)) );

   d->pathL = new QLabel( d->paths, "Look &in:", this, "qt_looin_lbl" );
   d->fileL = new QLabel( m_nameEdit, "File &name:", this, "qt_filename_lbl" );
   d->typeL = new QLabel( d->types, "File &type:", this, "qt_filetype_lbl" );
  
   d->goBack = new QToolButton( this, "go back" );
   d->goBack->setEnabled( false );
   d->goBack->setFocusPolicy( TabFocus );
	d->goBack->setIconSet( QPixmap(back_xpm) );
   connect( d->goBack, SIGNAL( clicked() ), this, SLOT( goBack() ) );
   QToolTip::add( d->goBack, "Back" );

   d->cdToParent = new QToolButton( this, "cd to parent" );
   d->cdToParent->setFocusPolicy( TabFocus );
   QToolTip::add( d->cdToParent, "One directory up" );
   d->cdToParent->setIconSet( QPixmap(cdtoparent_xpm) );
   connect( d->cdToParent, SIGNAL(clicked()),
            this, SLOT(cdUpClicked()) );
  
	d->goHome = new QToolButton( this, "go home" );
   d->goHome->setFocusPolicy( TabFocus );
	QToolTip::add( d->goHome, "Go Home" );
   d->goHome->setIconSet( QPixmap(go_home_xpm) );
   connect( d->goHome, SIGNAL(clicked()),
            this, SLOT(goHomeClicked()) );
  
   d->stack->addWidget( m_moreFiles, ListView );
   d->stack->addWidget( m_files, DetailView );

   d->mcol_detailView = new QToolButton( this, "mcol_detail_view" );
   d->mcol_detailView->setFocusPolicy( TabFocus );
	QToolTip::add( d->mcol_detailView, "Toggle between List and Detail View" );
	QIconSet iconset = d->mcol_detailView->iconSet();
	iconset.setPixmap( QPixmap(mclistview_xpm), 
                      QIconSet::Automatic, QIconSet::Normal, QIconSet::On );
	iconset.setPixmap( QPixmap(detailedview_xpm), 
                      QIconSet::Automatic, QIconSet::Normal, QIconSet::Off );
	d->mcol_detailView->setIconSet( iconset );
	d->mcol_detailView->setToggleButton( true );
   connect( d->mcol_detailView, SIGNAL(clicked()),
            this,               SLOT(changeMode()) );

   d->previewContents = new QToolButton( this, "preview info view" );
	d->goBack->setAutoRaise( true );
	d->cdToParent->setAutoRaise( true );
	d->goHome->setAutoRaise( true );
	d->mcol_detailView->setAutoRaise( true ); 
	d->previewContents->setAutoRaise( true );
   d->previewContents->setFocusPolicy( TabFocus );
   d->previewContents->setIconSet(QPixmap(previewcontentsview_xpm)  );
   d->previewContents->setToggleButton( true );
	connect( d->previewContents, SIGNAL(clicked()),
            this,               SLOT(changeMode()) );

   QToolTip::add( d->previewContents, "Preview File Contents" );
	connect( d->mcol_detailView, SIGNAL( clicked() ),
            m_moreFiles,        SLOT( cancelRename() ) );
	connect( d->mcol_detailView, SIGNAL( clicked() ),
            m_files,            SLOT( cancelRename() ) );

   d->stack->raiseWidget( m_moreFiles );
   d->mcol_detailView->setOn( false ); /* startup in ViewMode::List */
  
   QHBoxLayout *lay = new QHBoxLayout( this );
   lay->setMargin( 6 );
   d->leftLayout = new QHBoxLayout( lay, 5 );
   d->topLevelLayout = new QVBoxLayout( (QWidget*)0, 5 );
   lay->addLayout( d->topLevelLayout, 1 );
   d->extraWidgetsLayouts.setAutoDelete( false );
   d->extraLabels.setAutoDelete( false );
   d->extraWidgets.setAutoDelete( false );
   d->extraButtons.setAutoDelete( false );
   d->toolButtons.setAutoDelete( false );
  
   QHBoxLayout * h;
  
   d->preview = new PreviewStack( d->splitter, "preview" );
  
   h = new QHBoxLayout( 0 );
   d->buttonLayout = h;
   d->topLevelLayout->addLayout( h );
   h->addWidget( d->pathL );
   h->addSpacing( 8 );
   h->addWidget( d->paths );
   h->addSpacing( 8 );
   if ( d->goBack )
      h->addWidget( d->goBack );
   h->addWidget( d->cdToParent );
   h->addSpacing( 2 );
   h->addWidget( d->goHome );

   h->addSpacing( 4 );
   h->addWidget( d->mcol_detailView );
   h->addWidget( d->previewContents );

   d->topLevelLayout->addWidget( d->splitter );

   h = new QHBoxLayout();
   d->topLevelLayout->addLayout( h );
   h->addWidget( d->fileL );
   h->addWidget( m_nameEdit );
   h->addSpacing( 15 );
   h->addWidget( m_okB );

   h = new QHBoxLayout();
   d->topLevelLayout->addLayout( h );
   h->addWidget( d->typeL );
   h->addWidget( d->types );
   h->addSpacing( 15 );
   h->addWidget( m_cancelB );

   d->rightLayout = new QHBoxLayout( lay, 5 );
   d->topLevelLayout->setStretchFactor( d->mcol_detailView, 1 );
   d->topLevelLayout->setStretchFactor( m_files, 1 );

   updateGeometries();

   if ( d->goBack ) {
      setTabOrder( d->paths, d->goBack );
      setTabOrder( d->goBack, d->cdToParent );
   } else {
      setTabOrder( d->paths, d->cdToParent );
   }
	setTabOrder( d->cdToParent, d->goHome );
   setTabOrder( d->goHome, d->mcol_detailView );
   setTabOrder( d->mcol_detailView, m_moreFiles );

   setTabOrder( m_moreFiles, m_files );
   setTabOrder( m_files, m_nameEdit );
   setTabOrder( m_nameEdit, d->types );
   setTabOrder( d->types, m_okB );
   setTabOrder( m_okB, m_cancelB );
  
   d->rw = " -rw-";    
   d->ro = " -r--";     
   d->wo = " --w-";    
   d->inaccessible = " ----";

   d->symLinkToFile = " -> File";
   d->symLinkToDir = " -> Directory";
   d->symLinkToSpecial = " -> Special";
   d->file = " File";
   d->dir = " Dir";
   d->special = " Special";
  
   if ( lastWidth == 0 ) {
      QRect screen = QApplication::desktop()->screenGeometry( pos() );
      if ( screen.width() < 1024 || screen.height() < 768 ) {
         resize( QMIN(screen.width(), 420), QMIN(screen.height(), 236) );
      } else {
         QSize s = m_files->sizeHint();
         s = QSize( s.width() + 300, s.height() + 82 );
      
         if ( s.width() * 3 > screen.width() * 2 )
            s.setWidth( screen.width() * 2 / 3 );
      
         if ( s.height() * 3 > screen.height() * 2 )
            s.setHeight( screen.height() * 2 / 3 );
         else if ( s.height() * 3 < screen.height() )
            s.setHeight( screen.height() / 3 );
      
         resize( s );
      }
      updateLastSize(this);
   } else {
      resize( lastWidth, lastHeight );
   }
  
   if ( detailViewMode ) {
      d->stack->raiseWidget( m_files );
      d->mcol_detailView->setOn( true );
   }
  
   d->preview->hide();
   m_nameEdit->setFocus();
  
   connect( m_nameEdit, SIGNAL( returnPressed() ),
            this, SLOT( fileNameEditReturnPressed() ) );
}

/*  */
void FileDialog::fileNameEditReturnPressed()
{
   d->oldUrl = d->url;
   if ( !isDirectoryMode( d->mode ) ) {
      okClicked();
   } 
   else {
      d->currentFileName = QString::null;
      if ( m_nameEdit->text().isEmpty() ) {
         emit fileSelected( selectedFile() );
         accept();
      } 
      else {
         QUrlInfo f;
         FileDialogPrivate::File *c = (FileDialogPrivate::File *)m_files->currentItem();
         if ( c && m_files->isSelected(c) )
            f = c->info;
         else
            f = QUrlInfo( d->url, m_nameEdit->text() );
         if ( f.isDir() ) {
            setUrl( QUrlOperator( d->url,
                                  FileDialogPrivate::encodeFileName(m_nameEdit->text() + "/" ) ) );
            d->checkForFilter = true;
            trySetSelection( true, d->url, true );
            d->checkForFilter = false;
         }
      }
      m_nameEdit->setText( QString::null );
   }
}

void FileDialog::updatePreviews( const QUrl& url )
{
	if ( d->preview->isVisible() )
		d->preview->previewUrl( url );
}


/* either d->mcol_detailView or d->previewContents was clicked */
void FileDialog::changeMode()
{
   if ( d->mcol_detailView->isOn() ) {
      d->stack->raiseWidget( m_files );        /* listview */
	} else {
		d->stack->raiseWidget( m_moreFiles );  /* listbox */
	}

   if ( d->previewContents->isOn() ) {
      d->preview->show();
	} else {
      d->preview->hide();
	}
}


FileDialog::~FileDialog()
{
   /* since clear might call setContentsPos which would emit a signal
      and thus cause a recompute of sizes... */
   m_files->blockSignals( true );
   m_moreFiles->blockSignals( true );
   m_files->clear();
   m_moreFiles->clear();
   m_moreFiles->blockSignals( false );
   m_files->blockSignals( false );

   if ( d->cursorOverride )
      QApplication::restoreOverrideCursor();

   delete d;
   d = 0;
}


QString FileDialog::selectedFile() const
{
   QString s = d->currentFileName;
   // remove the protocol because we do not want to encode it...
   QString prot = QUrl( s ).protocol();
   if ( !prot.isEmpty() ) {
      prot += ":";
      s.remove( 0, prot.length() );
   }
   QUrl u( prot + FileDialogPrivate::encodeFileName( s ) );
   if ( u.isLocalFile() ) {
      QString s = u.toString();
      if ( s.left( 5 ) == "file:" )
         s.remove( (uint)0, 5 );
      return s;
   }
   return d->currentFileName;
}

QString FileDialog::selectedFilter() const
{
   return d->types->currentText();
}

void FileDialog::setSelectedFilter( int n )
{
   d->types->setCurrentItem( n );
   QString f = d->types->currentText();
   QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
   int index = r.search( f );
   if ( index >= 0 )
      f = r.cap( 2 );
   d->url.setNameFilter( f );
   rereadDir();
}

void FileDialog::setSelectedFilter( const QString& mask )
{
   int n;
   for ( n = 0; n < d->types->count(); n++ ) {
      if ( d->types->text( n ).contains( mask, false ) ) {
         d->types->setCurrentItem( n );
         QString f = mask;
         QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
         int index = r.search( f );
         if ( index >= 0 )
            f = r.cap( 2 );
         d->url.setNameFilter( f );
         rereadDir();
         return;
      }
   }
}


QStringList FileDialog::selectedFiles() const
{
   QStringList lst;

   if ( mode() == ExistingFiles ) {
      QStringList selectedLst;
      QString selectedFiles = m_nameEdit->text();
      selectedFiles.truncate( selectedFiles.findRev( '\"' ) );
      selectedLst = selectedLst.split( QString("\" "), selectedFiles );
      for ( QStringList::Iterator it = selectedLst.begin(); it != selectedLst.end(); ++it ) {
         QUrl u;
         if ( (*it)[0] == '\"' ) {
            u = QUrl( d->url, FileDialogPrivate::encodeFileName( (*it).mid(1) ) );
         } 
         else {
            u = QUrl( d->url, FileDialogPrivate::encodeFileName( (*it) ) );
         }
         if ( u.isLocalFile() ) {
            QString s = u.toString();
            if ( s.left( 5 ) == "file:" )
               s.remove( (uint)0, 5 );
            lst << s;
         } else {
            lst << u.toString();
         }
      }
   }

   return lst;
}


void FileDialog::setSelection( const QString & filename )
{
   d->oldUrl = d->url;
   QString nf = d->url.nameFilter();
   if ( QUrl::isRelativeUrl( filename ) )
      d->url = QUrlOperator( d->url, FileDialogPrivate::encodeFileName( filename ) );
   else
      d->url = QUrlOperator( filename );
   d->url.setNameFilter( nf );
   d->checkForFilter = true;
   bool isDirOk;
   bool isDir = d->url.isDir( &isDirOk );
   if ( !isDirOk )
      isDir = d->url.path().right( 1 ) == "/";
   if ( !isDir ) {
      QUrlOperator u( d->url );
      d->url.setPath( d->url.dirPath() );
      trySetSelection( false, u, true );
      d->ignoreNextRefresh = true;
      m_nameEdit->selectAll();
      rereadDir();
      emit dirEntered( d->url.dirPath() );
   } 
   else {
      if ( !d->url.path().isEmpty() &&
           d->url.path().right( 1 ) != "/" ) {
         QString p = d->url.path();
         p += "/";
         d->url.setPath( p );
      }
      trySetSelection( true, d->url, false );
      rereadDir();
      emit dirEntered( d->url.dirPath() );
      m_nameEdit->setText( QString::fromLatin1("") );
   }
   d->checkForFilter = false;
}


QString FileDialog::dirPath() const
{
   return d->url.dirPath();
}


void FileDialog::setFilter( const QString & newFilter )
{
   if ( newFilter.isEmpty() )
      return;
   QString f = newFilter;
   QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
   int index = r.search( f );
   if ( index >= 0 )
      f = r.cap( 2 );
   d->url.setNameFilter( f );
   if ( d->types->count() == 1 )  {
      d->types->clear();
      d->types->insertItem( newFilter );
   } else {
      for ( int i = 0; i < d->types->count(); ++i ) {
         if ( d->types->text( i ).left( newFilter.length() ) == newFilter ||
              d->types->text( i ).left( f.length() ) == f ) {
            d->types->setCurrentItem( i );
            break;
         }
      }
   }
   rereadDir();
}


void FileDialog::setDir( const QString & pathstr )
{
   QString dr = pathstr;
   if ( dr.isEmpty() )
      return;

#if defined(Q_OS_UNIX)
	vkPrint("setDir() - defined(Q_OS_UNIX)\n");
   if ( dr.length() && dr[0] == '~' ) {
      int i = 0;
      while( i < (int)dr.length() && dr[i] != '/' )
         i++;
      QCString user;
      if ( i == 1 ) {
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
      
#  ifndef _POSIX_LOGIN_NAME_MAX
#    define _POSIX_LOGIN_NAME_MAX 9
#  endif
      
         char name[_POSIX_LOGIN_NAME_MAX];
         if ( ::getlogin_r( name, _POSIX_LOGIN_NAME_MAX ) == 0 )
            user = name;
         else
#else
				vkPrint("setDir() - else\n");
         user = ::getlogin();
         if ( !user )
#endif
            user = getenv( "LOGNAME" );
      } else
         user = dr.mid( 1, i-1 ).local8Bit();
      dr = dr.mid( i, dr.length() );
      struct passwd *pw;
#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_FREEBSD)
      struct passwd mt_pw;
      char buffer[2048];
      if ( ::getpwnam_r( user, &mt_pw, buffer, 2048, &pw ) == 0 && pw == &mt_pw )
#else
         pw = ::getpwnam( user );
      if ( pw )
#endif
         dr.prepend( QString::fromLocal8Bit(pw->pw_dir) );
   }
#endif

   setUrl( dr );
}

const QDir *FileDialog::dir() const
{
   if ( d->url.isLocalFile() )
      return  new QDir( d->url.path() );
   else
      return 0;
}


void FileDialog::setDir( const QDir &dir )
{
   d->oldUrl = d->url;
   QString nf( d->url.nameFilter() );
   d->url = dir.canonicalPath();
   d->url.setNameFilter( nf );
   QUrlInfo i( d->url, m_nameEdit->text() );
   d->checkForFilter = true;
   trySetSelection( i.isDir(), QUrlOperator( d->url, 
                                             FileDialogPrivate::encodeFileName(m_nameEdit->text() ) ), 
                    false );
   d->checkForFilter = false;
   rereadDir();
   emit dirEntered( d->url.path() );
}


void FileDialog::setUrl( const QUrlOperator &url )
{
   d->oldUrl = d->url;
   QString nf = d->url.nameFilter();

   QString operatorPath = url.toString( false, false );
   if ( QUrl::isRelativeUrl( operatorPath ) ) {
      d->url = QUrl( d->url, operatorPath );
   } else {
      d->url = url;
   }
   d->url.setNameFilter( nf );

   d->checkForFilter = true;
   if ( !d->url.isDir() ) {
      QUrlOperator u = d->url;
      d->url.setPath( d->url.dirPath() );
      trySetSelection( false, u, false );
      rereadDir();
      emit dirEntered( d->url.dirPath() );
      QString fn = u.fileName();
      m_nameEdit->setText( fn );
   } 
   else {
      trySetSelection( true, d->url, false );
      rereadDir();
      emit dirEntered( d->url.dirPath() );
   }
   d->checkForFilter = false;
}


void FileDialog::rereadDir()
{
   if ( !d->cursorOverride ) {
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
      d->cursorOverride = true;
   }
   d->pendingItems.clear();
   d->currListChildren = d->url.listChildren();
   if ( d->cursorOverride ) {
      QApplication::restoreOverrideCursor();
      d->cursorOverride = false;
   }
}

extern bool qt_resolve_symlinks; // defined in qapplication.cpp
bool qt_use_native_dialogs = true;

QString FileDialog::getOpenFileName( const QString & startWith,
                                     const QString& filter,
                                     QWidget *parent, 
                                     const char* name,
                                     const QString& caption,
                                     QString *selectedFilter,
                                     bool resolveSymlinks )
{
   bool save_qt_resolve_symlinks = qt_resolve_symlinks;
   qt_resolve_symlinks = resolveSymlinks;

   QStringList filters;
   if ( !filter.isEmpty() )
      filters = makeFiltersList( filter );

   makeVariables();
   QString initialSelection;
   if ( !startWith.isEmpty() ) {
      QUrlOperator u( FileDialogPrivate::encodeFileName( startWith ) );
      if ( u.isLocalFile() && QFileInfo( u.path() ).isDir() ) {
         *workingDirectory = startWith;
      } else {
         if ( u.isLocalFile() ) {
            QFileInfo fi( u.dirPath() );
            if ( fi.exists() ) {
               *workingDirectory = u.dirPath();
               initialSelection = u.fileName();
            }
         } else {
            *workingDirectory = u.toString();
            initialSelection = QString::null;
         }
      }
   }

   if ( workingDirectory->isNull() )
      *workingDirectory = QDir::currentDirPath();

   FileDialog *dlg = new FileDialog( *workingDirectory, QString::null, parent, name ? name : "qt_filedlg_gofn", true );

   Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
   if ( !caption.isNull() )
      dlg->setCaption( caption );
   else
      dlg->setCaption( FileDialog::tr( "Open" ) );
#endif

   dlg->setFilters( filters );
   if ( selectedFilter )
      dlg->setFilter( *selectedFilter );
   dlg->setMode( FileDialog::ExistingFile );
   QString result;
   if ( !initialSelection.isEmpty() )
      dlg->setSelection( initialSelection );
   if ( dlg->exec() == QDialog::Accepted ) {
      result = dlg->selectedFile();
      *workingDirectory = dlg->d->url;
      if ( selectedFilter )
         *selectedFilter = dlg->selectedFilter();
   }

   delete dlg;
   qt_resolve_symlinks = save_qt_resolve_symlinks;
   return result;
}

QString FileDialog::getSaveFileName( const QString & startWith,
                                     const QString& filter,
                                     QWidget *parent, 
                                     const char* name,
                                     const QString& caption,
                                     QString *selectedFilter,
                                     bool resolveSymlinks)
{
   bool save_qt_resolve_symlinks = qt_resolve_symlinks;
   qt_resolve_symlinks = resolveSymlinks;

   QStringList filters;
   if ( !filter.isEmpty() )
      filters = makeFiltersList( filter );

   makeVariables();
   QString initialSelection;
   if ( !startWith.isEmpty() ) {
      QUrlOperator u( FileDialogPrivate::encodeFileName( startWith ) );
      if ( u.isLocalFile() && QFileInfo( u.path() ).isDir() ) {
         *workingDirectory = startWith;
      } else {
         if ( u.isLocalFile() ) {
            QFileInfo fi( u.dirPath() );
            if ( fi.exists() ) {
               *workingDirectory = u.dirPath();
               initialSelection = u.fileName();
            }
         } else {
            *workingDirectory = u.toString();
            initialSelection = QString::null;
         }
      }
   }
  
   if ( workingDirectory->isNull() )
      *workingDirectory = QDir::currentDirPath();


   FileDialog *dlg = new FileDialog( *workingDirectory, QString::null, 
                                     parent, name ? name : "qt_filedlg_gsfn", true );

   Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
   if ( !caption.isNull() )
      dlg->setCaption( caption );
   else
      dlg->setCaption( FileDialog::tr( "Save As" ) );
#endif
  
   QString result;
   dlg->setFilters( filters );
   if ( selectedFilter )
      dlg->setFilter( *selectedFilter );
   dlg->setMode( FileDialog::AnyFile );
   if ( !initialSelection.isEmpty() )
      dlg->setSelection( initialSelection );
   if ( dlg->exec() == QDialog::Accepted ) {
      result = dlg->selectedFile();
      *workingDirectory = dlg->d->url;
      if ( selectedFilter )
         *selectedFilter = dlg->selectedFilter();
   }
   delete dlg;

   qt_resolve_symlinks = save_qt_resolve_symlinks;

   return result;
}


void FileDialog::okClicked()
{
   QString fn( m_nameEdit->text() );

   if ( fn.contains( "*" ) ) {
      addFilter( fn );
      m_nameEdit->blockSignals( true );
      m_nameEdit->setText( "" );
      m_nameEdit->blockSignals( false );
      return;
   }

   *workingDirectory = d->url;
   detailViewMode = m_files->isVisible();
   updateLastSize(this);

   if ( isDirectoryMode( d->mode ) ) {
      QUrlInfo f( d->url, m_nameEdit->text() );
      if ( f.isDir() ) {
         d->currentFileName = d->url;
         if ( d->currentFileName.right(1) != "/" )
            d->currentFileName += '/';
         if ( f.name() != "." )
            d->currentFileName += f.name();
         accept();
         return;
      }
      /* Since it's not a directory and we clicked ok, we don't really
         want to do anything else */
      return;
   }

   /* if we're in multi-selection mode and something is selected,
      accept it and be done. */
   if ( mode() == ExistingFiles ) {
      if ( ! m_nameEdit->text().isEmpty() ) {
         QStringList sf = selectedFiles();
         bool isdir = false;
         if ( sf.count() == 1 ) {
            QUrlOperator u( d->url, sf[0] );
            bool ok;
            isdir = u.isDir(&ok) && ok;
         }
         if ( !isdir ) {
            emit filesSelected( sf );
            accept();
            return;
         }
      }
   }
  
   if ( mode() == AnyFile ) {
      QUrlOperator u( d->url, 
                      FileDialogPrivate::encodeFileName(m_nameEdit->text()) );
      if ( !u.isDir() ) {
         d->currentFileName = u;
         emit fileSelected( selectedFile() );
         accept();
         return;
      }
   }
  
   if ( mode() == ExistingFile ) {
      if ( !FileDialogPrivate::fileExists( d->url, m_nameEdit->text() ) )
         return;
   }
  
   /* if selection is valid, return it, else try using selection as a
      directory to change to. */
   if ( !d->currentFileName.isNull() && !d->currentFileName.contains("*") ) {
      emit fileSelected( selectedFile() );
      accept();
   } else {
      QUrlInfo f;
      FileDialogPrivate::File* c = (FileDialogPrivate::File*)m_files->currentItem();
      FileDialogPrivate::MCItem* m = (FileDialogPrivate::MCItem *)m_moreFiles->item( m_moreFiles->currentItem() );
      if ( c && m_files->isVisible() && m_files->hasFocus() ||
           m && m_moreFiles->isVisible() && m_moreFiles->hasFocus() ) {
         if ( c && m_files->isVisible() )
            f = c->info;
         else
            f = ( (FileDialogPrivate::File*)m->i )->info;
      } else {
         f = QUrlInfo( d->url, m_nameEdit->text() );
      }
      if ( f.isDir() ) {
         setUrl( QUrlOperator( d->url, FileDialogPrivate::encodeFileName(f.name() + "/" ) ) );
         d->checkForFilter = true;
         trySetSelection( true, d->url, true );
         d->checkForFilter = false;
      } else {
         if ( !m_nameEdit->text().contains( "/" ) && 
              !m_nameEdit->text().contains( "\\" ) )
            addFilter( m_nameEdit->text() );
         else if ( m_nameEdit->text()[ 0 ] == '/' ||
                   m_nameEdit->text()[ 0 ] == '\\' )
            setDir( m_nameEdit->text() );
         else if ( m_nameEdit->text().left( 3 ) == "../" || m_nameEdit->text().left( 3 ) == "..\\" )
            setDir( QUrl( d->url.toString(), FileDialogPrivate::encodeFileName(m_nameEdit->text() ) ).toString() );
      }
      m_nameEdit->setText( "" );
   }
}


void FileDialog::cancelClicked()
{
   *workingDirectory = d->url;
   detailViewMode = m_files->isVisible();
   updateLastSize(this);
   reject();
}


void FileDialog::resizeEvent( QResizeEvent * e )
{
   QDialog::resizeEvent( e );
   updateGeometries();
}


bool FileDialog::trySetSelection( bool isDir, const QUrlOperator &u, 
                                  bool updatelined )
{
   if ( !isDir && !u.path().isEmpty() && u.path().right( 1 ) == "/" )
      isDir = true;
   if ( u.fileName().contains( "*") && d->checkForFilter ) {
      QString fn( u.fileName() );
      if ( fn.contains( "*" ) ) {
         addFilter( fn );
         d->currentFileName = QString::null;
         d->url.setFileName( QString::null );
         m_nameEdit->setText( "" );
         return false;
      }
   }

   if ( isDir && d->preview && d->preview->isVisible() )
      updatePreviews( u );

   QString old = d->currentFileName;

   if ( isDirectoryMode( mode() ) ) {
      if ( isDir )
         d->currentFileName = u;
      else
         d->currentFileName = QString::null;
   } else if ( !isDir && mode() == ExistingFiles ) {
      d->currentFileName = u;
   } else if ( !isDir || ( mode() == AnyFile && !isDir ) ) {
      d->currentFileName = u;
   } else {
      d->currentFileName = QString::null;
   }
   if ( updatelined && !d->currentFileName.isEmpty() ) {
      /* If the selection is valid, or if its a directory, allow OK. */
      if ( !d->currentFileName.isNull() || isDir ) {
         if ( u.fileName() != ".." ) {
            QString fn = u.fileName();
            m_nameEdit->setText( fn );
         } else {
            m_nameEdit->setText( "" );
         }
      } else
         m_nameEdit->setText( "" );
   }
  
   if ( !d->currentFileName.isNull() || isDir ) {
      m_okB->setEnabled( true );
   } else if ( !isDirectoryMode( d->mode ) ) {
      m_okB->setEnabled( false );
   }

   if ( d->currentFileName.length() && old != d->currentFileName )
      emit fileHighlighted( selectedFile() );
  
   return !d->currentFileName.isNull();
}


/*  Make sure the minimum and maximum sizes of everything are sane. */
void FileDialog::updateGeometries()
{
   if ( !d || !d->geometryDirty )
      return;

   d->geometryDirty = false;
   QSize r, t;

   /* we really should have a QSize::unite() */
#define RM r.setWidth( QMAX(r.width(),t.width()) ); \
r.setHeight( QMAX(r.height(),t.height()) )

   // labels first
   r = d->pathL->sizeHint();
   t = d->fileL->sizeHint();
   RM;
   t = d->typeL->sizeHint();
   RM;
   d->pathL->setFixedSize( d->pathL->sizeHint() );
   d->fileL->setFixedSize( r );
   d->typeL->setFixedSize( r );

   // single-line input areas
   r = d->paths->sizeHint();
   t = m_nameEdit->sizeHint();
   RM;
   t = d->types->sizeHint();
   RM;
   r.setWidth( t.width() * 2 / 3 );
   t.setWidth( QWIDGETSIZE_MAX );
   t.setHeight( r.height() );
   d->paths->setMinimumSize( r );
   d->paths->setMaximumSize( t );
   m_nameEdit->setMinimumSize( r );
   m_nameEdit->setMaximumSize( t );
   d->types->setMinimumSize( r );
   d->types->setMaximumSize( t );

   // buttons on top row
   r = QSize( 0, d->paths->minimumSize().height() );
   t = QSize( 21, 20 );
   RM;
   if ( r.height()+1 > r.width() )
      r.setWidth( r.height()+1 );
   if ( d->goBack )
      d->goBack->setFixedSize( r );
   d->cdToParent->setFixedSize( r );
   d->goHome->setFixedSize( r );
   d->mcol_detailView->setFixedSize( r );

   QButton *b = 0;
   if ( !d->toolButtons.isEmpty() ) {
      for ( b = d->toolButtons.first(); b; b = d->toolButtons.next() )
         b->setFixedSize( b->sizeHint().width(), r.height() );
   }

   if ( d->contentsPreview ) {
      d->previewContents->show();
      d->previewContents->setFixedSize( r );
   } else {
      d->previewContents->hide();
      d->previewContents->setFixedSize( QSize( 0, 0 ) );
   }

   // open/save, cancel
   r = QSize( 75, 20 );
   t = m_okB->sizeHint();
   RM;
   t = m_cancelB->sizeHint();
   RM;
  
   m_okB->setFixedSize( r );
   m_cancelB->setFixedSize( r );
  
   d->topLevelLayout->activate();

#undef RM
}


void FileDialog::updateFileNameEdit( QListViewItem * newItem )
{
   if ( !newItem )
      return;

   if ( mode() == ExistingFiles ) {
      detailViewSelectionChanged();
      QUrl u( d->url, 
              FileDialogPrivate::encodeFileName( ((FileDialogPrivate::File*)m_files->currentItem())->info.name() ) );
      QFileInfo fi( u.toString( false, false ) );
      if ( !fi.isDir() )
         emit fileHighlighted( u.toString( false, false ) );
   } 
   else if ( m_files->isSelected( newItem ) ) {
      FileDialogPrivate::File *i = (FileDialogPrivate::File *)newItem;
      if ( i && i->i && !i->i->isSelected() ) {
         m_moreFiles->blockSignals( true );
         m_moreFiles->setSelected( i->i, true );
         m_moreFiles->blockSignals( false );
      }
      /* Encode the filename in case it had any special characters in it */
      QString encFile = FileDialogPrivate::encodeFileName( newItem->text( 0 ) );
      trySetSelection( i->info.isDir(), 
                       QUrlOperator( d->url, encFile ), true );
   }
}


void FileDialog::detailViewSelectionChanged()
{
   if ( d->mode != ExistingFiles )
      return;

   m_nameEdit->clear();
   QString str;
   QListViewItem* item = m_files->firstChild();
   m_moreFiles->blockSignals( true );
   while ( item ) {
      if ( m_moreFiles && isVisible() ) {
         FileDialogPrivate::File* f = (FileDialogPrivate::File*)item;
         if ( f->i && f->i->isSelected() != item->isSelected() )
            m_moreFiles->setSelected( f->i, item->isSelected() );
      }
      if ( item->isSelected() && 
           !( (FileDialogPrivate::File*)item )->info.isDir() )
         str += QString( "\"%1\" " ).arg( item->text( 0 ) );
      item = item->nextSibling();
   }
   m_moreFiles->blockSignals( false );
   m_nameEdit->setText( str );
   m_nameEdit->setCursorPosition( str.length() );
   m_okB->setEnabled( true );
   if ( d->preview && d->preview->isVisible() && m_files->currentItem() ) {
      QUrl u = QUrl( d->url, FileDialogPrivate::encodeFileName( ((FileDialogPrivate::File*)m_files->currentItem())->info.name() ) );
      updatePreviews( u );
   }
}

void FileDialog::listBoxSelectionChanged()
{
   if ( d->mode != ExistingFiles )
      return;

   if ( d->ignoreNextRefresh ) {
      d->ignoreNextRefresh = false;
      return;
   }

   m_nameEdit->clear();
   QString str;
   QListBoxItem * i = m_moreFiles->item( 0 );
   QListBoxItem * j = 0;
   int index = 0;
   m_files->blockSignals( true );
   while( i ) {
      FileDialogPrivate::MCItem *mcitem = (FileDialogPrivate::MCItem *)i;
      if ( m_files && isVisible() ) {
         if ( mcitem->i->isSelected() != mcitem->isSelected() ) {
            m_files->setSelected( mcitem->i, mcitem->isSelected() );

            /* What happens here is that we want to emit signal
               highlighted for newly added items.  But QListBox apparently
               emits selectionChanged even when a user clicks on the same
               item twice.  So, basically emulate the behaivor we have in
               the "Details" view which only emits highlighted the first
               time we click on the item.  Perhaps at some point we should
               have a call to updateFileNameEdit(QListViewItem) which also
               emits fileHighlighted() for ExistingFiles.  For better or
               for worse, this clones the behaivor of the "Details" view
               quite well. */
            if ( mcitem->isSelected() && i != d->lastEFSelected ) {
               QUrl u( d->url, FileDialogPrivate::encodeFileName( ((FileDialogPrivate::File*)(mcitem)->i)->info.name()) );
               d->lastEFSelected = i;
               emit fileHighlighted( u.toString(false, false) );
            }
         }
      }
      if ( m_moreFiles->isSelected( i )
           && !( (FileDialogPrivate::File*)(mcitem)->i )->info.isDir() ) {
         str += QString( "\"%1\" " ).arg( i->text() );
         if ( j == 0 )
            j = i;
      }
      i = m_moreFiles->item( ++index );
   }

   m_files->blockSignals( false );
   m_nameEdit->setText( str );
   m_nameEdit->setCursorPosition( str.length() );
   m_okB->setEnabled( true );
   if ( d->preview && d->preview->isVisible() && j ) {
      QUrl u = QUrl( d->url,
                     FileDialogPrivate::encodeFileName( ( (FileDialogPrivate::File*)( (FileDialogPrivate::MCItem*)j )->i )->info.name() ) );
      updatePreviews( u );
   }
}


void FileDialog::updateFileNameEdit( QListBoxItem * newItem )
{
   if ( !newItem )
      return;
   FileDialogPrivate::MCItem * i = (FileDialogPrivate::MCItem *)newItem;
   if ( i->i ) {
      i->i->listView()->setSelected( i->i, i->isSelected() );
      updateFileNameEdit( i->i );
   }
}


void FileDialog::fileNameEditDone()
{
   QUrlInfo f( d->url, m_nameEdit->text() );
   if ( mode() != FileDialog::ExistingFiles ) {
      QUrlOperator u( d->url, FileDialogPrivate::encodeFileName( m_nameEdit->text() ) );
      trySetSelection( f.isDir(), u, false );
      if ( d->preview && d->preview->isVisible() )
         updatePreviews( u );
   }
}


void FileDialog::selectDirectoryOrFile( QListViewItem * newItem )
{
   *workingDirectory = d->url;
   detailViewMode = m_files->isVisible();
   updateLastSize(this);

   if ( !newItem )
      return;

   if ( d->url.isLocalFile() ) {
      QFileInfo fi( d->url.path() + newItem->text(0) );
   }

   FileDialogPrivate::File *i = (FileDialogPrivate::File *)newItem;
   QString oldName = m_nameEdit->text();

   if ( i->info.isDir() ) {
      setUrl( QUrlOperator( d->url, 
                            FileDialogPrivate::encodeFileName( i->info.name() ) + "/" ) );
      if ( isDirectoryMode( mode() ) ) {
         QUrlInfo f ( d->url, QString::fromLatin1( "." ) );
         trySetSelection( f.isDir(), d->url, true );
      }
   } else if ( newItem->isSelectable() &&
               trySetSelection( i->info.isDir(), QUrlOperator( d->url, 
                                                               FileDialogPrivate::encodeFileName( i->info.name() ) ), true ) ) {
      if ( !isDirectoryMode( mode() ) ) {
         if ( mode() == ExistingFile ) {
            if ( FileDialogPrivate::fileExists( d->url, m_nameEdit->text() ) ) {
               emit fileSelected( selectedFile() );
               accept();
            }
         } else {
            emit fileSelected( selectedFile() );
            accept();
         }
      }
   } else if ( isDirectoryMode( d->mode ) ) {
      d->currentFileName = d->url;
      accept();
   }
   if ( !oldName.isEmpty() && !isDirectoryMode( mode() ) )
      m_nameEdit->setText( oldName );
}


void FileDialog::selectDirectoryOrFile( QListBoxItem * newItem )
{
   if ( !newItem )
      return;

   FileDialogPrivate::MCItem *i = (FileDialogPrivate::MCItem *)newItem;
   if ( i->i ) {
      i->i->listView()->setSelected( i->i, i->isSelected() );
      selectDirectoryOrFile( i->i );
   }
}


void FileDialog::popupContextMenu( QListViewItem* item, const QPoint& p, int )
{
   if ( item ) {
      m_files->setCurrentItem( item );
      m_files->setSelected( item, true );
   }

   PopupAction action;
   popupContextMenu( item ? item->text(0) : QString::null, true, action, p );

   if ( action == PA_Open )
      selectDirectoryOrFile( item );
   else if ( action == PA_Rename )
      m_files->startRename( false );
   else if ( action == PA_Delete )
      deleteFile( item ? item->text( 0 ) : QString::null );
   else if ( action == PA_Reload )
      rereadDir();
   else if ( action == PA_Hidden ) {
      bShowHiddenFiles = !bShowHiddenFiles;
      rereadDir();
   } else if ( action == PA_SortName ) {
      sortFilesBy = (int)QDir::Name;
      sortAscending = true;
      resortDir();
   } else if ( action == PA_SortSize ) {
      sortFilesBy = (int)QDir::Size;
      sortAscending = true;
      resortDir();
   } else if ( action == PA_SortDate ) {
      sortFilesBy = (int)QDir::Time;
      sortAscending = true;
      resortDir();
   } else if ( action == PA_SortNone ) {
      sortFilesBy = (int)QDir::Unsorted;
      sortAscending = true;
      resortDir();
   }
}

void FileDialog::popupContextMenu( QListBoxItem* item, const QPoint& p )
{
   PopupAction action;
   popupContextMenu( item ? item->text() : QString::null, false, action, p );

   if ( action == PA_Open )
      selectDirectoryOrFile( item );
   else if ( action == PA_Rename )
      m_moreFiles->startRename( false );
   else if ( action == PA_Delete )
      deleteFile( item->text() );
   else if ( action == PA_Reload )
      rereadDir();
   else if ( action == PA_Hidden ) {
      bShowHiddenFiles = !bShowHiddenFiles;
      rereadDir();
   } else if ( action == PA_SortName ) {
      sortFilesBy = (int)QDir::Name;
      sortAscending = true;
      resortDir();
   } else if ( action == PA_SortSize ) {
      sortFilesBy = (int)QDir::Size;
      sortAscending = true;
      resortDir();
   } else if ( action == PA_SortDate ) {
      sortFilesBy = (int)QDir::Time;
      sortAscending = true;
      resortDir();
   } else if ( action == PA_SortNone ) {
      sortFilesBy = (int)QDir::Unsorted;
      sortAscending = true;
      resortDir();
   }
}

void FileDialog::popupContextMenu( const QString &filename, bool,
                                   PopupAction& action, const QPoint& pt )
{
	enum { OPEN_SAVE=0, RENAME, DELETE, RELOAD, HIDDEN,
          SORT_NAME, SORT_SIZE, SORT_DATE, SORT_NONE };

   action = PA_Cancel;

	QString open_save = "&Open";
   if ( !QUrlInfo( d->url, filename ).isDir() && mode() == AnyFile ) 
      open_save = "&Save";

   QPopupMenu popup( 0, "popup context menu" );
   popup.setCheckable( true );
   popup.insertItem( open_save, OPEN_SAVE );
   popup.insertSeparator();
   popup.insertItem( "&Rename", RENAME );
   popup.insertItem( "&Delete", DELETE );
   popup.insertSeparator();
	QPopupMenu popup2( 0, "sort menu" );
   popup2.insertItem( "Sort by &Name", SORT_NAME );
   popup2.insertItem( "Sort by &Size", SORT_SIZE );
   popup2.insertItem( "Sort by &Date", SORT_DATE );
   popup2.insertItem( "&Unsorted",     SORT_NONE );
   popup.insertItem( "Sort",  &popup2 );
   popup.insertItem( "R&eload",            RELOAD );
   popup.insertItem( "Show &hidden files", HIDDEN );
   popup.setItemChecked( HIDDEN, bShowHiddenFiles );

	if ( filename.isEmpty() || filename == ".." || 
        !QUrlInfo( d->url, filename ).isWritable() ) {
		if ( filename.isEmpty() || !QUrlInfo( d->url, filename ).isReadable() )
			popup.setItemEnabled( OPEN_SAVE, false );
		popup.setItemEnabled( RENAME, false );
		popup.setItemEnabled( DELETE, false );
	}
	switch ( sortFilesBy ) {
   case QDir::Name:     popup2.setItemChecked( SORT_NAME, true );  break;
   case QDir::Size:     popup2.setItemChecked( SORT_SIZE, true );  break;
   case QDir::Time:     popup2.setItemChecked( SORT_DATE, true );  break;
   case QDir::Unsorted: popup2.setItemChecked( SORT_NONE, true );  break;
	}

	popup.move( pt );
	int res = popup.exec();
	switch ( res ) {
   case OPEN_SAVE: action = PA_Open;      break;
   case RENAME:    action = PA_Rename;    break;
   case DELETE:    action = PA_Delete;    break;
   case RELOAD:    action = PA_Reload;    break;
   case HIDDEN:    action = PA_Hidden;    break;
   case SORT_NAME: action = PA_SortName;  break;
   case SORT_DATE: action = PA_SortDate;  break;
   case SORT_SIZE: action = PA_SortSize;  break;
   case SORT_NONE: action = PA_SortNone;  break;
   }

}

void FileDialog::deleteFile( const QString& filename )
{
	if ( !filename.isEmpty() ) {
		QUrlInfo fi( d->url, FileDialogPrivate::encodeFileName(filename) );

		QString ftype = "file";
		if ( fi.isDir() )        ftype = "directory";
		else if ( fi.isSymLink() ) ftype = "symlink";

		int ok = vkQuery( this, QString("Delete %1").arg(ftype), "&Yes;&No",
                        "<p>Are you sure you wish to delete the "
                        "%s '%s' ?</p>", ftype.latin1(), filename.latin1() );
		if ( ok == MsgBox::vkYes ) {
			d->url.remove( FileDialogPrivate::encodeFileName(filename) );
		}
	}
}


void FileDialog::cdUpClicked()
{
   QString oldName = m_nameEdit->text();
   setUrl( QUrlOperator( d->url, ".." ) );
   if ( !oldName.isEmpty() )
      m_nameEdit->setText( oldName );
}

void FileDialog::goHomeClicked()
{
   if ( getenv( "HOME" ) )
      setDir( getenv( "HOME" ) );
   else
      setDir( "/" );
}

void FileDialog::createdDirectory( const QUrlInfo &info, QNetworkOperation * )
{
   resortDir();
   if ( m_moreFiles->isVisible() ) {
      for ( unsigned int i=0; i<m_moreFiles->count(); ++i ) {
         if ( m_moreFiles->text(i) == info.name() ) {
            m_moreFiles->setCurrentItem(i);
            m_moreFiles->startRename( false );
            break;
         }
      }
   } else {
      QListViewItem *item = m_files->firstChild();
      while ( item ) {
         if ( item->text( 0 ) == info.name() ) {
            m_files->setSelected( item, true );
            m_files->setCurrentItem( item );
            m_files->startRename( false );
            break;
         }
         item = item->nextSibling();
      }
   }
}



QString FileDialog::getExistingDirectory( const QString & dir,
                                          QWidget *parent,
                                          const char* name,
                                          const QString& caption,
                                          bool dirOnly,
                                          bool resolveSymlinks)
{
   bool save_resolve_symlinks = qt_resolve_symlinks;
   qt_resolve_symlinks = resolveSymlinks;

   makeVariables();
   QString wd;
   if ( workingDirectory )
      wd = *workingDirectory;

   FileDialog* dlg = new FileDialog( parent, 
                                     name ? name : "filedlg_ged", true );
   Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
   if ( !caption.isNull() )
      dlg->setCaption( caption );
   else
      dlg->setCaption( FileDialog::tr("Find Directory") );
#endif
   dlg->setMode( dirOnly ? DirectoryOnly : Directory );
   dlg->d->types->clear();
   dlg->d->types->insertItem( FileDialog::tr("Directories") );
   dlg->d->types->setEnabled( false );

   QString dir_( dir );
   dir_ = dir_.simplifyWhiteSpace();
   if ( dir_.isEmpty() && !wd.isEmpty() )
      dir_ = wd;
   QUrlOperator u( dir_ );
   if ( u.isLocalFile() ) {
      if ( !dir_.isEmpty() ) {
         QFileInfo f( u.path() );
         if ( f.exists() )
            if ( f.isDir() ) {
               dlg->setDir( dir_ );
               wd = dir_;
            }
      } else if ( !wd.isEmpty() ) {
         QUrl tempUrl( wd );
         QFileInfo f( tempUrl.path() );
         if ( f.isDir() ) {
            dlg->setDir( wd );
         }
      } else {
         QString theDir = dir_;
         if ( theDir.isEmpty() ) {
            theDir = QDir::currentDirPath();
         } if ( !theDir.isEmpty() ) {
            QUrl tempUrl( theDir );
            QFileInfo f( tempUrl.path() );
            if ( f.isDir() ) {
               wd = theDir;
               dlg->setDir( theDir );
            }
         }
      }
   } else {
      dlg->setUrl( dir_ );
   }

   QString result;
   dlg->setSelection( dlg->d->url.toString() );

   if ( dlg->exec() == QDialog::Accepted ) {
      result = dlg->selectedFile();
      wd = result;
   }
   delete dlg;

   if ( !result.isEmpty() && result.right( 1 ) != "/" )
      result += "/";

   qt_resolve_symlinks = save_resolve_symlinks;
  
   return result;
}


void FileDialog::setMode( Mode newMode )
{
   if ( d->mode != newMode ) {
      d->mode = newMode;
      QString sel = d->currentFileName;
      int maxnamelen = 255; // _POSIX_MAX_PATH
      if ( isDirectoryMode( newMode ) ) {
         m_files->setSelectionMode( QListView::Single );
         m_moreFiles->setSelectionMode( QListBox::Single );
         if ( sel.isNull() )
            sel = QString::fromLatin1(".");
         d->types->setEnabled( false );
      } else if ( newMode == ExistingFiles ) {
         maxnamelen = INT_MAX;
         m_files->setSelectionMode( QListView::Extended );
         m_moreFiles->setSelectionMode( QListBox::Extended );
         d->types->setEnabled( true );
      } else {
         m_files->setSelectionMode( QListView::Single );
         m_moreFiles->setSelectionMode( QListBox::Single );
         d->types->setEnabled( true );
      }
      m_nameEdit->setMaxLength(maxnamelen);
      rereadDir();
      QUrlInfo f( d->url, "." );
      trySetSelection( f.isDir(), d->url, false );
   }

   QString okt;
   bool changeFilters = false;
   if ( mode() == AnyFile ) {
      okt = "&Save";
      d->fileL->setText( "File &name:" );
      if ( d->types->count() == 1 ) {
         d->types->setCurrentItem( 0 );
         if ( d->types->currentText() == "Directories" ) {
            changeFilters = true;
         }
      }
   }
   else if ( mode() == Directory || mode() == DirectoryOnly ) {
      okt = "&OK";
      d->fileL->setText( "Directory:" );
      d->types->clear();
      d->types->insertItem( "Directories" );
   }
   else {
      okt = "&Open";
      d->fileL->setText( "File &name:" );
      if ( d->types->count() == 1 ) {
         d->types->setCurrentItem( 0 );
         if ( d->types->currentText() == "Directories" ) {
            changeFilters = true;
         }
      }
   }

   if ( changeFilters ) {
      d->types->clear();
      d->types->insertItem( "All Files (*)" );
   }

   m_okB->setText( okt );
}


FileDialog::Mode FileDialog::mode() const
{ return d->mode; }


void FileDialog::done( int i )
{
   if ( i == QDialog::Accepted && (d->mode == ExistingFile || 
                                   d->mode == ExistingFiles) ) {
      QStringList selection = selectedFiles();
      for ( uint f = 0; f < selection.count(); f++ ) {
         QString file = selection[f];
         if ( file.isNull() )
            continue;
         if ( d->url.isLocalFile() && !QFile::exists( file ) ) {
				vkInfo( this, "Error",  "<p>%s<br>File not found.<br>"
                    "Check path and filename.</p>", file.latin1() );
            return;
         }
      }
   }

   QDialog::done( i );
}

void FileDialog::keyPressEvent( QKeyEvent* ke )
{
   if ( !d->ignoreNextKeyPress && ke && ( ke->key() == Key_Enter ||
                                          ke->key() == Key_Return ) ) {
      ke->ignore();
      if ( d->paths->hasFocus() ) {
         ke->accept();
         if ( d->url == QUrl(d->paths->currentText()) )
            m_nameEdit->setFocus();
      } else if ( d->types->hasFocus() ) {
         ke->accept();
         /* is there a suitable condition for this?  only valid wildcards? */
         m_nameEdit->setFocus();
      } else if ( m_nameEdit->hasFocus() ) {
         if ( d->currentFileName.isNull() ) {
            /* maybe change directory */
            QUrlInfo ui( d->url, m_nameEdit->text() );
            if ( ui.isDir() ) {
               m_nameEdit->setText( "" );
               setDir( QUrlOperator( d->url, 
                                     FileDialogPrivate::encodeFileName(ui.name()) ) );
            }
            ke->accept();
         } else if ( mode() == ExistingFiles ) {
            QUrlInfo ui( d->url, m_nameEdit->text() );
            if ( ui.isFile() ) {
               QListViewItem* item = m_files->firstChild();
               while ( item && m_nameEdit->text() != item->text( 0 ) )
                  item = item->nextSibling();
               if ( item )
                  m_files->setSelected( item, true );
               else
                  ke->accept(); // strangely, means to ignore that event
            }
         }
      } else if ( m_files->hasFocus() || m_moreFiles->hasFocus() ) {
         ke->accept();
      }
   } else if ( ke->key() == Key_Escape ) {
      ke->ignore();
   }

   d->ignoreNextKeyPress = false;

   if ( !ke->isAccepted() ) {
      QDialog::keyPressEvent( ke );
   }
}


bool FileDialog::eventFilter( QObject* obj, QEvent* ev )
{
	QKeyEvent* ke = ((QKeyEvent*)ev);

	if ( ev->type() == QEvent::KeyPress && ke->key() == Key_F5 ) {
      rereadDir();
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress && ke->key() == Key_F2 &&
             ( obj == m_files || obj == m_files->viewport() ) ) {
      if ( m_files->isVisible() && m_files->currentItem() ) {
         if ( QUrlInfo( d->url, "." ).isWritable() && 
              m_files->currentItem()->text( 0 ) != ".." ) {
            m_files->m_renameItem = m_files->currentItem();
            m_files->startRename( true );
         }
      }
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress &&  ke->key() == Key_F2 &&
             ( obj == m_moreFiles || obj == m_moreFiles->viewport() ) ) {
      if ( m_moreFiles->isVisible() && m_moreFiles->currentItem() != -1 ) {
         if ( QUrlInfo( d->url, "." ).isWritable() &&
              m_moreFiles->item( m_moreFiles->currentItem() )->text() != ".." ) {
            m_moreFiles->m_renameItem = m_moreFiles->item( m_moreFiles->currentItem() );
            m_moreFiles->startRename( true );
         }
      }
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress && m_moreFiles->m_renaming ) {
      m_moreFiles->m_lined->setFocus();
      QApplication::sendEvent( m_moreFiles->m_lined, ev );
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress && m_files->m_renaming ) {
      m_files->m_lined->setFocus();
      QApplication::sendEvent( m_files->m_lined, ev );
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress && ke->key() == Key_Backspace &&
             ( obj == m_files || obj == m_moreFiles || 
               obj == m_files->viewport() || obj == m_moreFiles->viewport() ) ) {
      cdUpClicked();
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress && ke->key() == Key_Delete &&
             ( obj == m_files || obj == m_files->viewport() ) ) {
      if ( m_files->currentItem() )
         deleteFile( m_files->currentItem()->text( 0 ) );
      ke->accept();
      return true;
   } 
   else if ( ev->type() == QEvent::KeyPress && ke->key() == Key_Delete &&
             ( obj == m_moreFiles || obj == m_moreFiles->viewport() ) ) {
      int c = m_moreFiles->currentItem();
      if ( c >= 0 )
         deleteFile( m_moreFiles->item( c )->text() );
      ke->accept();
      return true;
   } 
   else if ( obj == m_files && ev->type() == QEvent::FocusOut && 
             m_files->currentItem() ) {
   } 
   else if ( obj == m_files && ev->type() == QEvent::KeyPress ) {
      QTimer::singleShot( 0, this, SLOT(fixupNameEdit()) );
   } 
   else if ( obj == m_nameEdit && ev->type() == QEvent::KeyPress && 
             d->mode != AnyFile ) {
      if ( ( m_nameEdit->cursorPosition() == (int)m_nameEdit->text().length() || 
             m_nameEdit->hasSelectedText() ) && isprint( ke->ascii() ) ) {
         QString nt( m_nameEdit->text() );
         nt.truncate( m_nameEdit->cursorPosition() );
         nt += (char)(ke->ascii());
         QListViewItem* item = m_files->firstChild();
         while( item && item->text( 0 ).left(nt.length()) != nt )
            item = item->nextSibling();
         if ( item ) {
            nt = item->text( 0 );
            int cp = m_nameEdit->cursorPosition() + 1;
            m_nameEdit->validateAndSet( nt, cp, cp, nt.length() );
            return true;
         }
      }
   } 
   else if ( obj == m_nameEdit && ev->type() == QEvent::FocusIn ) {
      fileNameEditDone();
   } 
   else if ( m_moreFiles->m_renaming && obj != m_moreFiles->m_lined && 
             ev->type() == QEvent::FocusIn ) {
      m_moreFiles->m_lined->setFocus();
      return true;
   } 
   else if ( m_files->m_renaming && obj != m_files->m_lined && 
             ev->type() == QEvent::FocusIn ) {
      m_files->m_lined->setFocus();
      return true;
   } 
   else if ( ( obj == m_moreFiles || obj == m_moreFiles->viewport() ) &&
             ev->type() == QEvent::FocusIn ) {
      if ( obj == m_moreFiles->viewport() && !m_moreFiles->viewport()->hasFocus() ||
           obj == m_moreFiles && !m_moreFiles->hasFocus() )
         ((QWidget*)obj)->setFocus();
      return false;
   }
  
   return QDialog::eventFilter( obj, ev );
}


void FileDialog::setFilters( const QString &filters )
{
   QStringList lst = makeFiltersList( filters );
   setFilters( lst );
}


void FileDialog::setFilters( const char ** types )
{
   if ( !types || !*types )
      return;

   d->types->clear();
   while( types && *types ) {
      d->types->insertItem( QString::fromLatin1(*types) );
      types++;
   }
   d->types->setCurrentItem( 0 );
   setFilter( d->types->text( 0 ) );
}


void FileDialog::setFilters( const QStringList & types )
{
   if ( types.count() < 1 )
      return;

   d->types->clear();
   for ( QStringList::ConstIterator it = types.begin(); it != types.end(); ++it )
      d->types->insertItem( *it );
   d->types->setCurrentItem( 0 );
   setFilter( d->types->text( 0 ) );
}


void FileDialog::addFilter( const QString &filter )
{
   if ( filter.isEmpty() )
      return;
   QString f = filter;
   QRegExp r( QString::fromLatin1(qt_file_dialog_filter_reg_exp) );
   int index = r.search( f );
   if ( index >= 0 )
      f = r.cap( 2 );
   for ( int i = 0; i < d->types->count(); ++i ) {
      QString f2( d->types->text( i ) );
      int index = r.search( f2 );
      if ( index >= 0 )
         f2 = r.cap( 1 );
      if ( f2 == f ) {
         d->types->setCurrentItem( i );
         setFilter( f2 );
         return;
      }
   }

   d->types->insertItem( filter );
   d->types->setCurrentItem( d->types->count() - 1 );
   setFilter( d->types->text( d->types->count() - 1 ) );
}


QStringList FileDialog::getOpenFileNames( const QString & filter,
                                          const QString& dir,
                                          QWidget *parent,
                                          const char* name,
                                          const QString& caption,
                                          QString *selectedFilter,
                                          bool resolveSymlinks )
{
   bool save_resolve_symlinks = qt_resolve_symlinks;
   qt_resolve_symlinks = resolveSymlinks;

   QStringList filters;
   if ( !filter.isEmpty() )
      filters = makeFiltersList( filter );

   makeVariables();

   if ( workingDirectory->isNull() )
      *workingDirectory = QDir::currentDirPath();

   if ( !dir.isEmpty() ) {
      /* only works correctly for local files */
      QUrlOperator u( dir );
      if ( u.isLocalFile() && QFileInfo( u ).isDir() ) {
         *workingDirectory = dir;
      } else {
         *workingDirectory = u.toString();
      }
   }

   FileDialog* dlg = new FileDialog( *workingDirectory, QString::null, 
                                     parent, name ? name : "filedlg_gofns", true );
   Q_CHECK_PTR( dlg );
#ifndef QT_NO_WIDGET_TOPEXTRA
   if ( !caption.isNull() )
      dlg->setCaption( caption );
   else
      dlg->setCaption( FileDialog::tr("Open") );
#endif

   dlg->setFilters( filters );
   if ( selectedFilter )
      dlg->setFilter( *selectedFilter );
   dlg->setMode( FileDialog::ExistingFiles );
   QString result;
   QStringList lst;
   if ( dlg->exec() == QDialog::Accepted ) {
      lst = dlg->selectedFiles();
      *workingDirectory = dlg->d->url;
      if ( selectedFilter )
         *selectedFilter = dlg->selectedFilter();
   }
   delete dlg;
   qt_resolve_symlinks = save_resolve_symlinks;
   return lst;
}


void FileDialog::fixupNameEdit()
{
   if ( m_files->currentItem() ) {
      if ( ( (FileDialogPrivate::File*)m_files->currentItem() )->info.isFile() )
         m_nameEdit->setText( m_files->currentItem()->text( 0 ) );
   }
}


QUrl FileDialog::url() const
{ return d->url; }


static bool isRoot( const QUrl& u )
{
   if ( u.path() == "/" )
      return true;

   if ( !u.isLocalFile() && u.path() == "/" )
      return true;

   return false;
}

void FileDialog::urlStart( QNetworkOperation *op )
{
   if ( !op ) return;

   if ( op->operation() == QNetworkProtocol::OpListChildren ) {

		if ( !d->cursorOverride ) {
			QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
			d->cursorOverride = true;
		}

		bool root = isRoot( d->url );
      d->cdToParent->setEnabled( !root );

		d->sortedList.clear();
		d->pendingItems.clear();
		m_moreFiles->clearSelection();
		m_files->clearSelection();
		m_moreFiles->clear();
		m_files->clear();
		m_files->setSorting( -1 );

      QString url_str = d->url.toString( false, false );
      bool found = false;
      for ( int i = 0; i < d->paths->count(); ++i ) {
			if ( d->paths->text( i ) == url_str ) {
				found = true;
				d->paths->setCurrentItem( i );
				break;
			}
		}
		if ( !found ) {
			d->paths->insertItem( *openFolderIcon, url_str, -1 );
			d->paths->setCurrentItem( d->paths->count() - 1 );
		}
		d->last = 0;
		d->hadDotDot = false;

		if ( d->goBack && d->history.last() != d->url.toString() ) {
			d->history.append( d->url.toString() );
			if ( d->history.count() > 1 )
				d->goBack->setEnabled( true );
		}
	}
}

void FileDialog::urlFinished( QNetworkOperation *op )
{
   if ( !op ) return;

   if ( op->operation() == QNetworkProtocol::OpListChildren &&
        d->cursorOverride ) {
		QApplication::restoreOverrideCursor();
		d->cursorOverride = false;
	}
  
   if ( op->state() == QNetworkProtocol::StFailed ) {
      if ( d->paths->hasFocus() )
         d->ignoreNextKeyPress = true;

		vkWarn( this, "Error", op->protocolDetail().latin1() );
      switch ( op->errorCode() ) {
      case QNetworkProtocol::ErrParse:
      case QNetworkProtocol::ErrValid:
      case QNetworkProtocol::ErrListChildren:
      case QNetworkProtocol::ErrHostNotFound:
      case QNetworkProtocol::ErrUnknownProtocol:
      case QNetworkProtocol::ErrLoginIncorrect:
      case QNetworkProtocol::ErrFileNotExisting:
         if ( d->url != d->oldUrl ) {
            d->url = d->oldUrl;
            rereadDir();
         } break;
      default: break;
		}

	} else if ( op->operation() == QNetworkProtocol::OpListChildren &&
               op == d->currListChildren ) {
      if ( !d->hadDotDot && !isRoot( d->url ) ) {
			QUrlInfo ui( d->url, ".." );
			ui.setName( ".." );
			ui.setDir( true );
			ui.setFile( false );
			ui.setSymLink( false );
			ui.setSize( 0 );
			QValueList<QUrlInfo> lst;
			lst << ui;
			insertEntry( lst, 0 );
      }
      resortDir();
   } else if ( op->operation() == QNetworkProtocol::OpPut ) {
      rereadDir();
   }

}
 
void FileDialog::dataTransferProgress( int bytesDone, int bytesTotal,
                                       QNetworkOperation *op )
{
   if ( !op )
      return;

   QString label;
   QUrl u( op->arg( 0 ) );
   if ( u.isLocalFile() ) {
      label = u.path();
   } 
   else {
      label = QString( "%1 (on %2)" );
      label = label.arg( u.path() ).arg( u.host() );
   }

	vkPrint("bytesDone = %d, bytesTotal = %d\n", bytesDone, bytesTotal );
}

void FileDialog::insertEntry( const QValueList<QUrlInfo> &lst, 
                              QNetworkOperation *op )
{
   if ( op && op->operation() == QNetworkProtocol::OpListChildren &&
        op != d->currListChildren )
      return;
   QValueList<QUrlInfo>::ConstIterator it = lst.begin();
   for ( ; it != lst.end(); ++it ) {
      const QUrlInfo &inf = *it;
      if ( d->mode == DirectoryOnly && !inf.isDir() )
         continue;
      if ( inf.name() == ".." ) {
         d->hadDotDot = true;
         if ( isRoot( d->url ) )
            continue;
      } else if ( inf.name() == "." )
         continue;
    
      if ( !bShowHiddenFiles && inf.name() != ".." ) {
         if ( inf.name()[ 0 ] == QChar( '.' ) )
            continue;
      }
      if ( !d->url.isLocalFile() ) {
         FileDialogPrivate::File * i = 0;
         FileDialogPrivate::MCItem *i2 = 0;
         i = new FileDialogPrivate::File( d, &inf, m_files );
         i2 = new FileDialogPrivate::MCItem( m_moreFiles, i );
      
         if ( d->mode == ExistingFiles && inf.isDir() ||
              ( isDirectoryMode( d->mode ) && inf.isFile() ) ) {
            i->setSelectable( false );
            i2->setSelectable( false );
         }
      
         i->i = i2;
      }
    
      d->sortedList.append( new QUrlInfo( inf ) );
   }
}

void FileDialog::removeEntry( QNetworkOperation *op )
{
   if ( !op )
      return;

   QUrlInfo *i = 0;
   QListViewItemIterator it( m_files );
   bool ok1 = false, ok2 = false;
   for ( i=d->sortedList.first(); it.current(); 
         ++it, i=d->sortedList.next() ) {
      if ( ( (FileDialogPrivate::File*)it.current() )->info.name() 
           == op->arg( 0 ) ) {
         d->pendingItems.removeRef( (FileDialogPrivate::File*)it.current() );
         delete ( (FileDialogPrivate::File*)it.current() )->i;
         delete it.current();
         ok1 = true;
      }
      if ( i && i->name() == op->arg( 0 ) ) {
         d->sortedList.removeRef( i );
         i = d->sortedList.prev();
         ok2 = true;
      }
      if ( ok1 && ok2 )
         break;
   }
}

void FileDialog::itemChanged( QNetworkOperation *op )
{
   if ( !op )
      return;

   QUrlInfo* i = 0;
   QListViewItemIterator it1( m_files );
   bool ok1 = false, ok2 = false;
   /* first check whether the new file replaces an existing file */
   for ( i = d->sortedList.first(); it1.current(); 
         ++it1, i = d->sortedList.next() ) {
      if ( ( (FileDialogPrivate::File*)it1.current() )->info.name() == op->arg( 1 ) ) {
         delete ( (FileDialogPrivate::File*)it1.current() )->i;
         delete it1.current();
         ok1 = true;
      }
      if ( i && i->name() == op->arg( 1 ) ) {
         d->sortedList.removeRef( i );
         i = d->sortedList.prev();
         ok2 = true;
      }
      if ( ok1 && ok2 )
         break;
   }

   i = 0;
   QListViewItemIterator it( m_files );
   ok1 = false;
   ok2 = false;
   for ( i = d->sortedList.first(); it.current(); 
         ++it, i = d->sortedList.next() ) {
      if ( ( (FileDialogPrivate::File*)it.current() )->info.name() == op->arg( 0 ) ) {
         ( (FileDialogPrivate::File*)it.current() )->info.setName( op->arg( 1 ) );
         ok1 = true;
      }
      if ( i && i->name() == op->arg( 0 ) ) {
         i->setName( op->arg( 1 ) );
         ok2 = true;
      }
      if ( ok1 && ok2 )
         break;
   }

   resortDir();
}


void FileDialog::resortDir()
{
   d->pendingItems.clear();

   FileDialogPrivate::File* item = 0;
   FileDialogPrivate::MCItem* item2 = 0;

   d->sortedList.sort();

   if ( m_files->childCount() > 0 || m_moreFiles->count() > 0 ) {
      m_moreFiles->clear();
      m_files->clear();
      d->last = 0;
      m_files->setSorting( -1 );
   }

   QUrlInfo *i = sortAscending ? d->sortedList.first() : d->sortedList.last();
   for ( ; i; i=sortAscending ? d->sortedList.next() : d->sortedList.prev() ) {
      item = new FileDialogPrivate::File( d, i, m_files );
      item2 = new FileDialogPrivate::MCItem( m_moreFiles, item, item2 );
      item->i = item2;
      d->pendingItems.append( item );
      if ( d->mode == ExistingFiles && item->info.isDir() ||
           ( isDirectoryMode( d->mode ) && item->info.isFile() ) ) {
         item->setSelectable( false );
         item2->setSelectable( false );
      }
   }

}


/* Stops the current copy operation. 
   FIXME: this was invoked by a progress dialog. hook up to esc key */
void FileDialog::stopCopy()
{
   if ( d->ignoreStop )
      return;

   d->url.blockSignals( true );
   d->url.stop();
   d->url.blockSignals( false );
}


/* If b is true then all the files in the current directory are
   selected; otherwise, they are deselected.  */
void FileDialog::selectAll( bool b )
{
   if ( d->mode != ExistingFiles )
      return;
   m_moreFiles->selectAll( b );
   m_files->selectAll( b );
}


void FileDialog::goBack()
{
   if ( !d->goBack || !d->goBack->isEnabled() )
      return;
   d->history.remove( d->history.last() );
   if ( d->history.count() < 2 )
      d->goBack->setEnabled( false );
   setUrl( d->history.last() );
}



/* --------------------------------------------------------------------- */
PixmapView::PixmapView( QWidget *parent )
	: QScrollView( parent )
{
   viewport()->setBackgroundMode( PaletteBase );
}

void PixmapView::setPixmap( const QPixmap &pix )
{
   m_pixmap = pix;
   resizeContents( m_pixmap.size().width(), m_pixmap.size().height() );
   viewport()->repaint( false );
}

/* draw the pic in the centre of the widget */
void PixmapView::drawContents( QPainter* p, int cx, int cy, 
                               int cw, int ch )
{
   int x = ( viewport()->width() / 2 )  - ( m_pixmap.width() / 2 );
	int y = ( viewport()->height() / 2 ) - ( m_pixmap.height() / 2 );
   p->fillRect( cx, cy, cw, ch, 
                colorGroup().brush( QColorGroup::Base ) );
   p->drawPixmap( x, y, m_pixmap );
}



/* --------------------------------------------------------------------- */
PreviewStack::PreviewStack( QSplitter* parent, const char* name )
	: QWidgetStack( parent, name )
{
   m_textView   = new QTextView( this );
   m_htmlView   = new QTextView( this );
   m_pixmapView = new PixmapView( this );
   raiseWidget( m_textView );
}

void PreviewStack::previewUrl( const QUrl& url )
{
   if ( ! url.isLocalFile() ) {
      m_textView->setText( "I only show local files" );
      raiseWidget( m_textView );
		return;
	}

	QString path = url.path();
	QFileInfo fi( path );
	/* refuse to show binaries */
	if ( fi.isExecutable() ) {
		m_textView->setText( "I don't show executables" );
		raiseWidget( m_textView );
		return;
	}

	QPixmap pix( path );
	if ( ! pix.isNull() ) {
		m_pixmapView->setPixmap( pix );
		raiseWidget( m_pixmapView );
		return;
	} 

	if ( fi.isFile() ) {
		QFile f( path );
		if ( f.open( IO_ReadOnly ) ) {
			QTextStream ts( &f );
			QString text = ts.read();
			f.close();
			if ( fi.extension().lower().contains( "htm" ) ) {
				QString abs_url = m_htmlView
               ->mimeSourceFactory()
               ->makeAbsolute( path, m_htmlView->context() );
				m_htmlView->setText( text, abs_url );   
				raiseWidget( m_htmlView );
				return;
			} else {
				m_textView->setText( text );   
				raiseWidget( m_textView );
				return;
			}
		}
	}

	m_textView->setText( "blarg"/*QString::null*/ );
	raiseWidget( m_textView );
}





/* ------------------------------------------------------------------ */
bool FileCopy(const QString& in, const QString& out)
{
   const int bufSize = 16384; // 16Kb buffer
   char *buf = new char[bufSize];
   
   QFile fin(in);
   if( !fin.open(IO_ReadOnly) )
      return false;
   QFile fout(out);
   if( !fout.open(IO_WriteOnly) )
      return false;

   int len = fin.readBlock(buf, bufSize);
   while (len > 0) {
      if (fout.writeBlock(buf, len) == -1)
         return false;
      len = fin.readBlock(buf, len);
   }
   if (len == -1)
      return false;

   fin.close();
   fout.close();
   delete[] buf;

   return true;
}
