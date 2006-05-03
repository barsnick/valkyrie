/* ---------------------------------------------------------------------
 * Custom file dialog                                    vk_file_utils.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_FILE_UTILS_H
#define __VK_FILE_UTILS_H


class QPushButton;
class QButton;
class QLabel;
class QWidget;
class FileDialog;
class QTimer;
class QNetworkOperation;
class QListViewItem;
class QListBoxItem;
class FileDialogPrivate;

#include <qdir.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qsplitter.h>
#include <qtextview.h>
#include <qurloperator.h>
#include <qurlinfo.h>
#include <qwidgetstack.h>

/* class PixmapView ---------------------------------------------------- */
class PixmapView : public QScrollView
{
   Q_OBJECT
public:
   PixmapView( QWidget *parent );
   void setPixmap( const QPixmap &pix );
   void drawContents( QPainter *p, int, int, int, int );

private:
   QPixmap m_pixmap;
};

/* class PreviewStack -------------------------------------------------- */
class PreviewStack : public QWidgetStack
{
   Q_OBJECT
public:
   PreviewStack( QSplitter* parent, const char* name );
   void previewUrl( const QUrl& u );
private:
   QTextView*  m_textView;
   QTextView*  m_htmlView;
   PixmapView* m_pixmapView;
};


/* class RenameEdit ---------------------------------------------------- */
class RenameEdit : public QLineEdit
{
   Q_OBJECT
public:
   RenameEdit( QWidget* parent );
protected:
   void keyPressEvent( QKeyEvent *e );
   void focusOutEvent( QFocusEvent *e );
signals:
   void cancelRename();
   void doRename();
private slots:
   void slotReturnPressed();
private:
   bool m_doRenameAlreadyEmitted;
};



/* class FileListBox --------------------------------------------------- */
class FileListBox : public QListBox
{
   Q_OBJECT
public:
   FileListBox( QWidget* parent, FileDialog* d );

   void clear();
   void show();
   void startRename( bool check=true );

   bool          m_renaming;
   RenameEdit*   m_lined;
   QListBoxItem* m_renameItem;

private:
   void keyPressEvent( QKeyEvent* ke );
   void viewportMousePressEvent( QMouseEvent* me );
   void viewportMouseReleaseEvent( QMouseEvent* me );
   void viewportMouseDoubleClickEvent( QMouseEvent* me );

private slots:
   void rename();
   void cancelRename();
   void doubleClickTimeout();
   void contentsMoved( int, int );

private:
   FileDialog*  m_filedialog;
   QTimer*      m_renameTimer;
   QTimer*      m_changeDirTimer;

   int          m_urls;
   bool         m_mousePressed;
   bool         m_firstMousePressEvent;
	//RM ??
   QPoint       m_pressPos;
   QPoint       m_oldDragPos;
   QString      m_startDragDir;
   QUrlOperator m_startDragUrl;
};



/* class FileListView -------------------------------------------------- */
class FileListView : public QListView
{
   Q_OBJECT
public:
   FileListView( QWidget* parent, FileDialog* d );

   void clear();
   void startRename( bool check = true );
   void setSorting( int column, bool increasing = true );

   bool           m_renaming;
   RenameEdit*    m_lined;
   QListViewItem* m_renameItem;

private:
   void keyPressEvent( QKeyEvent *e );
   void viewportMousePressEvent( QMouseEvent *e );
   void viewportMouseDoubleClickEvent( QMouseEvent *e );
   void viewportMouseReleaseEvent( QMouseEvent *e );

private slots:
   void rename();
   void cancelRename();
   void changeSortColumn2( int column );
   void doubleClickTimeout();
   void contentsMoved( int, int );

private:
   FileDialog* m_filedialog;
   QTimer*     m_renameTimer;
   QTimer*     m_changeDirTimer;
   QPoint      m_pressPos;
   QPoint      m_oldDragPos;
   QString     m_startDragDir;

   int  m_urls;
   int  m_sortcolumn;
   bool m_mousePressed;
   bool m_firstMousePressEvent;
   bool m_ascending;

   QUrlOperator m_startDragUrl;
};



/* class FileDialog ---------------------------------------------------- */
class FileDialog : public QDialog
{
   Q_OBJECT
public:
   FileDialog( const QString &dirName, 
               const QString &filter=QString::null,
               QWidget *parent=0, const char *name=0, 
               bool modal=false );
   FileDialog( QWidget *parent=0, const char *name=0, bool modal=false );
   ~FileDialog();

   static QString getOpenFileName( const QString &initially = QString::null,
                                   const QString &filter = QString::null,
                                   QWidget *parent = 0, const char* name = 0,
                                   const QString &caption = QString::null,
                                   QString *selectedFilter = 0,
                                   bool resolveSymlinks = true);
   static QString getSaveFileName( const QString &initially = QString::null,
                                   const QString &filter = QString::null,
                                   QWidget *parent = 0, const char* name = 0,
                                   const QString &caption = QString::null,
                                   QString *selectedFilter = 0,
                                   bool resolveSymlinks = true);
   static QString getExistingDirectory( const QString &dir = QString::null,
                                        QWidget *parent = 0,
                                        const char* name = 0,
                                        const QString &caption = QString::null,
                                        bool dirOnly = true,
                                        bool resolveSymlinks = true);
   static QStringList getOpenFileNames( const QString &filter= QString::null,
                                        const QString &dir = QString::null,
                                        QWidget *parent = 0,
                                        const char* name = 0,
                                        const QString &caption = QString::null,
                                        QString *selectedFilter = 0,
                                        bool resolveSymlinks = true);

   QString selectedFile() const;
   QString selectedFilter() const;
   virtual void setSelectedFilter( const QString& );
   virtual void setSelectedFilter( int );

   void setSelection( const QString &);
   void selectAll( bool b );

   QStringList selectedFiles() const;
   QString dirPath() const;

   void setDir( const QDir & );
   const QDir *dir() const;

   void rereadDir();
   void resortDir();

   enum Mode { 
		AnyFile, 
		ExistingFile, 
		Directory, 
		ExistingFiles, 
		DirectoryOnly 
	};
   void setMode( Mode );
   Mode mode() const;

   enum ViewMode { 
		Detail = 0, /* name, size, type, date, attribs */
		List   = 1  /* files + dirs */
	};

   //RM: ??
	enum StackId {
      DetailView = 0,  /* files     */
		ListView   = 1,  /* moreFiles */
      PreView    = 2   /* */
	};

   enum ButtonId {
      ListDetails = 0,     /* mcol_detailView */
		PreviewContents = 2  /* previewContents */
	};

   bool eventFilter( QObject *, QEvent * );

   QUrl url() const;

   void addFilter( const QString &filter );

public slots:
   void done( int );
   void setDir( const QString& );
   void setUrl( const QUrlOperator &url );
   void setFilter( const QString& );
   void setFilters( const QString& );
   void setFilters( const char ** );
   void setFilters( const QStringList& );

protected:
   void resizeEvent( QResizeEvent * );
   void keyPressEvent( QKeyEvent * );

signals:
   void fileHighlighted( const QString& );
   void fileSelected( const QString& );
   void filesSelected( const QStringList& );
   void dirEntered( const QString& );
   void filterSelected( const QString& );

private slots:
   void detailViewSelectionChanged();
   void listBoxSelectionChanged();
   void changeMode();
   void fileNameEditReturnPressed();
   void stopCopy();

   void updateFileNameEdit( QListViewItem *);
   void selectDirectoryOrFile( QListViewItem * );
   void popupContextMenu( QListViewItem *, const QPoint &, int );
   void popupContextMenu( QListBoxItem *, const QPoint & );
   void updateFileNameEdit( QListBoxItem *);
   void selectDirectoryOrFile( QListBoxItem * );
   void fileNameEditDone();

   void okClicked();
   void cancelClicked();
   void cdUpClicked();
   void goHomeClicked();

   void fixupNameEdit();

   void updateGeometries();
   void urlStart( QNetworkOperation *op );
   void urlFinished( QNetworkOperation *op );
   void dataTransferProgress( int bytesDone, int bytesTotal, 
                              QNetworkOperation * );
   void insertEntry( const QValueList<QUrlInfo> &fi, 
                     QNetworkOperation *op );
   void removeEntry( QNetworkOperation * );
   void createdDirectory( const QUrlInfo &info, QNetworkOperation * );
   void itemChanged( QNetworkOperation * );
   void goBack();

private:
   enum PopupAction {
      PA_Open = 0,
      PA_Delete,
      PA_Rename,
      PA_SortName,
      PA_SortSize,
      PA_SortType,
      PA_SortDate,
      PA_SortNone,
      PA_Cancel,
      PA_Reload,
      PA_Hidden
   };

   void init();
   bool trySetSelection( bool isDir, const QUrlOperator &, bool );
   void deleteFile( const QString &filename );
   void popupContextMenu( const QString &filename, bool withSort,
                          PopupAction &action, const QPoint &p );
   void updatePreviews( const QUrl &u );

   //  QDir    m_reserved; // was cwd
   //  QString m_fileName;

   friend class FileListView;
   friend class FileListBox;

   FileDialogPrivate* d;

   FileListView* m_files;
   FileListBox*  m_moreFiles;

   QLineEdit*    m_nameEdit; // also filter
   QPushButton*  m_okB;
   QPushButton*  m_cancelB;
};

#endif // #ifndef __VK_FILE_UTILS_H

