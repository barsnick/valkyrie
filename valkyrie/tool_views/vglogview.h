/* ---------------------------------------------------------------------
 * Definition of VgLogView                                   vglogview.h
 * Links VgLog elements with qlistview elements
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_VGLOGVIEW_H
#define __VK_VGLOGVIEW_H

#include "vglog.h"

#include <qlistview.h>
#include <qpixmap.h>
#include <qdatetime.h>


/**********************************************************************/
/* VgOutputItem: base class
   item classes set up their own state and text in their construtors,
   and item->setOpen() sets up item's children.
   - only spends time populating an item when seen
   - no item opened before setup (inc. children's) finished.
   only a couple of sub-classes currently implement different
   looks/functionality, but more future proof to have them separated.
*/
class VgOutputItem : public QListViewItem
{
public:
   VgOutputItem( QListView* parent, VgElement );
   VgOutputItem( QListViewItem* parent, VgElement );
   VgOutputItem( QListViewItem* parent, QListViewItem* after, VgElement );

   void initialise();
   void setText( QString str );

   /* some make-life-simpler stuff */
   VgOutputItem* firstChild();
   VgOutputItem* nextSibling();
   VgOutputItem* parent();
   void paintCell( QPainter* p, const QColorGroup& cg,
                   int col, int width, int align );
   void setOpen( bool open );

   VgElement::ElemType elemType() { return elem.elemType(); }

public:
   bool isReadable, isWriteable;
   VgElement elem;               /* associated element */
};

/**********************************************************************/
class TopStatusItem : public VgOutputItem
{
public:
   TopStatusItem( QListView* parent, VgStatus status,
                  VgElement args, QString protocol );
   void updateStatus( VgStatus s );
   void updateErrorCounts( VgErrCounts ec );
   void updateLeakCounts( VgError leak );

private:
   int num_errs, num_bytes, num_blocks;
   QString status_tmplt, status_str, errcounts_tmplt;
   QString stime;
   QString protocol;
};

/**********************************************************************/
class InfoItem : public VgOutputItem
{
public:
   InfoItem( VgOutputItem* parent, VgElement root );
   void setOpen( bool open );
};

/**********************************************************************/
class LogQualItem : public VgOutputItem
{
public:
   LogQualItem( VgOutputItem* parent, VgElement logqual );
   void setOpen( bool open );
};

/**********************************************************************/
class CommentItem : public VgOutputItem
{
public:
   CommentItem( VgOutputItem* parent, QListViewItem* after,
                VgElement cmnt );
   void setOpen( bool open );
};

/**********************************************************************/
class ArgsItem : public VgOutputItem
{
public:
   ArgsItem( VgOutputItem* parent, QListViewItem* after,
             VgElement vgargs );
   void setOpen( bool open );
};

/**********************************************************************/
class PreambleItem : public VgOutputItem
{
public:
   PreambleItem( VgOutputItem* parent, QListViewItem* after,
                 VgPreamble preamble );
   void setOpen( bool open );
};

/**********************************************************************/
class ErrorItem : public VgOutputItem
{
public:
   ErrorItem( VgOutputItem* parent, QListViewItem* after,
              VgError err );
   void setOpen( bool open );
   void updateCount( QString count );
   bool isLeak() { return error.isLeak(); }
   VgError error;

   /* mapping of error::kind to 3-letter acronyms */
   typedef QMap<QString, QString> AcronymMap;
   static AcronymMap acronymMap;

private:
   QString errorAcronym( QString kind );
   QString err_tmplt;
};

/**********************************************************************/
class StackItem : public VgOutputItem
{
public:
   StackItem( VgOutputItem* parent, QListViewItem* after,
              VgElement stck );
   void setOpen( bool open );
};

/**********************************************************************/
class FrameItem : public VgOutputItem
{
public:
   FrameItem( VgOutputItem* parent, QListViewItem* after,
              VgFrame frm );
   void setOpen( bool open );
};

/**********************************************************************/
/* class SrcItem (error::stack::frame::dir/file/line)
   - pale gray background colour.
   - 'read'/'write' pixmap to denote user perms for src file.
   - click item => source file opened in an editor, at lineno. */
class SrcItem : public VgOutputItem
{
public:
   SrcItem( VgOutputItem* parent, VgElement line, QString path );
   ~SrcItem();

   void setPixmap( const char* pix_xpm[] );
   const QPixmap* pixmap( int i ) const;
   void paintCell( QPainter* p, const QColorGroup& cg,
                   int col, int width, int align );

private:
   QPixmap* pix;
};

/**********************************************************************/
class SuppCountsItem : public VgOutputItem
{
public:
   SuppCountsItem( VgOutputItem* parent, QListViewItem* after,
                   VgSuppCounts sc );
   void setOpen( bool open );
};




/**********************************************************************/
/* VgLogView: inherits VgLog
   - takes qlistview in constructor, and populates it at the same 
   time the underlying vglog is populated
   - associates each listview item with a VgLog element,
   which is used to set the text of the item, and is intended for
   future use of accessing/editing the underlying log
   (e.g. generating a suppression by clicking on an error item)
*/
class VgLogView : public VgLog
{
public:
   VgLogView( QListView* lv );
   ~VgLogView();

   bool appendNode( QDomNode node );

private:
   void updateErrorItems( VgErrCounts ec );

private:
   QListView* lview;          /* we don't own this: don't delete it */
   VgOutputItem* lastChild;
   TopStatusItem* topStatus;
};


#endif // #ifndef __VK_VGLOGVIEW_H
