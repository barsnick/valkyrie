/****************************************************************************
** VgLogView definition
**  - links QDomElements with QTreeWidgetItems
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2011, OpenWorks LLP. All rights reserved.
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

#ifndef __VK_VGLOGVIEW_H
#define __VK_VGLOGVIEW_H

#include <QColorGroup>
#include <QDateTime>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// QDom stuff
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QHash>
#include <QString>


// ============================================================
// Forward decls
class VgOutputItem;
class TopStatusItem;


// ============================================================
/*!
  VgLogView: abstract base class for tool-logviews

   - Representation of a Valgrind XML log.

   - Holds both model and view.
     As the the parser (vglogreader) parses a complete top-level
     element, it's passed to VgLogView to incrementally update
     both the model and view.

   - Takes a view* argument in constructor, and populates it at the same
     time as the underlying model.

   - Each view item gets a ref to its appropriate VgLog element, for
     setting the item text data, and providing access to any further
     element data.
     Note: this is NOT one-to-one!  Some elements are ignored, and some
        items represent multiple elements!

    - On-demand sub-item creation.
      Children of top-level items are created only when the user opens
      the branch. the QDomElement refs held by item are then queried
      to fill the item data.
*/
class VgLogView : public QObject
{
   Q_OBJECT
public:
   VgLogView( QTreeWidget* );
   ~VgLogView();

   bool init( QDomProcessingInstruction xml_insn, QString doc_tag );
   bool appendNode( QDomNode node, QString& errMsg );

//TODO: needed?
//   QString toString( int indent = 2 ); // xml output

protected:
   // keep track of our progress
   VgOutputItem*  lastItem;
   TopStatusItem* topStatus;

private:
   virtual QString toolName() = 0;
   virtual bool appendNodeTool( QDomElement elem, QString& errMsg ) = 0;
   virtual TopStatusItem* createTopStatus( QTreeWidget* view, QDomElement exe,
                                           QDomElement status, QString _protocol ) = 0;
   void updateErrorItems( QDomElement ec );
   QDomElement logRoot();

private:
   QDomDocument vglog;
   QTreeWidget* view;    // we don't own this: don't cleanup
};



// ============================================================
namespace VG_ELEM {
   // All valgrind tag types, for mapping of tags to enum values
   enum ElemType {
      ROOT, PROTOCOL_VERSION, PROTOCOL_TOOL, PREAMBLE, PID, PPID, TOOL,
      LOGQUAL, VAR, VALUE, COMMENT,
      ARGS, VARGV, ARGV, EXE, ARG,
      STATUS, STATE, TIME,
      ERROR, UNIQUE, TID, KIND, WHAT, XWHAT, TEXT, STACK,
      FRAME, IP, OBJ, FN, SRCDIR, SRCFILE, LINE, AUXWHAT, XAUXWHAT,
      ERRORCOUNTS, ANNOUNCETHREAD, HTHREADID, PAIR, COUNT,
      SUPPCOUNTS, NAME, LEAKEDBYTES, LEAKEDBLOCKS,
      SUPPRESSION, SNAME, SKIND, SKAUX, SFRAME, RAWTEXT,
      NUM_ELEMS
   };
}

// static map (tagname->enum) + access functions
typedef QHash<QString, VG_ELEM::ElemType> ElemTypeMap;



// ============================================================
/*!
   VgOutputItem: base class

   Items represent one (or more) branches/leaves of a Valgrind XML log.

   Top-level items are initialised with state and QDomElement references.
    - children are only initialised on demand, via openChildren(),
      for reasons of speed for large logs.

   Note: Items do not have a one-to-one relationship with XML elements:
   some XML log elements are ignored, some items represent multiple elements.
*/
class VgOutputItem : public QTreeWidgetItem
{
public:
   VgOutputItem( QTreeWidget* parent, QDomElement );
   VgOutputItem( QTreeWidgetItem* parent, QDomElement );
   VgOutputItem( QTreeWidgetItem* parent, QTreeWidgetItem* after, QDomElement );

   void setText( QString str );

   VgOutputItem* firstChild();
   VgOutputItem* parent();

   void openChildren();
   // all (non-root) items with children must reimplement this:
   virtual void setupChildren() {}

   // useful static data + functions for mapping tagname -> enum
   static ElemTypeMap elemtypeMap;
   static VG_ELEM::ElemType elemType( QString tagName );
   VG_ELEM::ElemType elemType();

   // getters
   bool getIsExpandable();
   bool getIsReadable();
   bool getIsWriteable();
   QDomElement getElement();

protected:
   bool isReadable, isWriteable;
   QDomElement elem;               // associated element
   bool isExpandable;

private:
   void initialise();
};




// ============================================================
class TopStatusItem : public VgOutputItem
{
public:
   TopStatusItem( QTreeWidget* parent, QDomElement exe,
                              QDomElement status, QString toolstatus,
                              QString _protocol );
   void updateStatus( QDomElement status );
   void updateFromErrorCounts( QDomElement ec );

   // all tool TopStatusItems must implement this:
   virtual void updateToolStatus( QDomElement ) = 0;

protected:
   void updateText();

protected:
   QString toolstatus_str;
   int num_errs;

private:
   QString state_str, start_time, time_str;
   QString protocol;
   QString status_tmplt, status_str;
};



// ============================================================
class InfoItem : public VgOutputItem
{
public:
   InfoItem( VgOutputItem* parent, QDomElement root );

   void setupChildren();
};


// ============================================================
class LogQualItem : public VgOutputItem
{
public:
   LogQualItem( VgOutputItem* parent, QDomElement logqual );

   void setupChildren();
};


// ============================================================
class ArgsItem : public VgOutputItem
{
public:
   ArgsItem( VgOutputItem* parent, QTreeWidgetItem* after,
             QDomElement vgargs );

   void setupChildren();
};


// ============================================================
class PreambleItem : public VgOutputItem
{
public:
   PreambleItem( VgOutputItem* parent, QTreeWidgetItem* after,
                 QDomElement preamble );

   void setupChildren();
};




// ============================================================
// ErrorItem: abstract base class
class ErrorItem : public VgOutputItem
{
public:
   typedef QMap<QString, QString> AcronymMap;

   ErrorItem( VgOutputItem* parent, QTreeWidgetItem* after,
              QDomElement err, ErrorItem::AcronymMap map );//, QString acnym );
   void updateCount( QString count );

   void showFullSrcPath( bool show );
   bool isFullSrcPathShown();
   QString getSuppressionStr();

   void setupChildren();

protected:
   QString getErrorAcronym( ErrorItem::AcronymMap map, QString kind );

private:
   QString err_tmplt;
   bool fullSrcPathShown;
   QString str_supp;
};



// ============================================================
class StackItem : public VgOutputItem
{
public:
   StackItem( VgOutputItem* parent, QTreeWidgetItem* after,
              QDomElement stck );

   void setupChildren();
};


// ============================================================
class FrameItem : public VgOutputItem
{
public:
   FrameItem( VgOutputItem* parent, QTreeWidgetItem* after,
              QDomElement frm );

   QString describe_IP( bool withPath = false );

   void setupChildren();
};


// ============================================================
class SrcItem : public VgOutputItem
{
public:
   SrcItem( VgOutputItem* parent, QDomElement line, QString path );
   // leaf item: no children to setup.
};


// ============================================================
class SuppCountsItem : public VgOutputItem
{
public:
   SuppCountsItem( VgOutputItem* parent, QTreeWidgetItem* after,
                   QDomElement sc );

   void setupChildren();
};




// ============================================================
/* Notes re xml weaknesses
   -----------------------
   * leak errors need their what string taken apart and put in xml fields:
   - record x of N: need 'x', to detect new record group
   - remove all numbers, just have "memory is definitely lost", or something.

   * indent everything after <valgrindoutput> by 2 spaces more

   * nitpick:
   < <?xml version="1.0"?>
   ---
   > <?xml version = '1.0'?>

   * nitpick:
   < <logfilequalifier> <var>VAR</var> <value>$VAR</value> </logfilequalifier>
   ---
   > <logfilequalifier>
   >   <var>VAR</var>
   >   <value>$VAR</value>
   > </logfilequalifier>
*/

#endif // #ifndef __VK_VGLOGVIEW_H
