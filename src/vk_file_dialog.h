/* ---------------------------------------------------------------------
 * Custom file dialog                                   vk_file_dialog.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_FILE_DIALOG_H
#define __VK_FILE_DIALOG_H
























//#include <qscrollview.h>
//#include <qfiledialog.h>
//#include <qwidgetstack.h>
//#include <qvbox.h>
//#include <qurl.h>
//#include <qpixmap.h>
//#include <qstringlist.h>


/* custom file/dir dialog 
class VkFileDialog : public QDialog
{
  Q_OBJECT
public:
  VkFileDialog();
  ~VkFileDialog();
private slots:
	void goHome();
  void showHiddenFiles();
private:
  QFileDialog::Mode mode;
  QString start;
  QString filter;
  QString caption;
};
*/

/* shows an image in the preview widget 
class PixmapView : public QScrollView
{
  Q_OBJECT
public:
  PixmapView( QWidget* parent );
  void setPixmap( const QPixmap& pix );
  void drawContents( QPainter* p, int, int, int, int );
private:
  QPixmap pixmap;
};
*/

/* stack of widgets: each one previews a different type of file
class Preview : public QWidgetStack
{
  Q_OBJECT
public:
  Preview( QWidget* parent );
  void showPreview( const QUrl& u, int size );
private:
  QMultiLineEdit* normalText;
  QTextView* html;
  PixmapView* pixmap;
};
*/

/* container for the preview widget stack
class PreviewWidget : public QVBox, public QFilePreview
{
  Q_OBJECT
public:
  PreviewWidget( QWidget *parent );
  void previewUrl( const QUrl &url );
private:
  //QSpinBox *sizeSpinBox;
  Preview* preview;
};
*/

#endif
