/****************************************************************************
** VgLogReader definition
**  - reads valgrind xml log into a VgLog
** --------------------------------------------------------------------------
**
** Copyright (C) 2000-2010, OpenWorks LLP. All rights reserved.
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

#ifndef __VGLOGREADER_H
#define __VGLOGREADER_H

#include "toolview/vglogview.h"

#include <QFile>
#include <QString>
#include <QDomDocument>

#if 1
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QXmlParseException>
#else
// For debugging xml parser
#include <vkxml.h>
#define QXmlSimpleReader VkXmlSimpleReader
#define QXmlInputSource VkXmlInputSource
#define QXmlDefaultHandler VkXmlDefaultHandler
#define QXmlAttributes VkXmlAttributes
#define QXmlParseException VkXmlParseException
#endif


// ============================================================
/*
  Simple xml handler class for valgrind logs:
  - creates node tree from input
  - hands off complete top-level branches to VgLog
  (e.g. preamble, error etc)
*/
class VgLogHandler : public QXmlDefaultHandler
{
public:
   VgLogHandler( VgLogView* lv );
   ~VgLogHandler();
   
   // content handler
   bool processingInstruction( const QString& target,
                               const QString& data );
   bool startElement( const QString& nsURI,
                      const QString& localName,
                      const QString& qName,
                      const QXmlAttributes& atts );
   bool endElement( const QString& nsURI,
                    const QString& localName,
                    const QString& qName );
   bool characters( const QString& ch );
   bool startDocument();
   bool endDocument();
   
   // reimplement error handlers
   bool error( const QXmlParseException& exception );
   bool fatalError( const QXmlParseException& exception );
   
   /* only set if fatal error */
   QString fatalMsg() {
      return m_fatalMsg;
   }
   /* may have reached end of log even with fatal error */
   bool finished() {
      return m_finished;
   }
   /* let us find out when parsing has been started */
   bool started() {
      return m_started;
   }
   
private:
   QDomDocument doc;
   VgLogView* logview;
   QDomNode node;
   
   QString m_fatalMsg;
   bool m_finished;
   bool m_started;
};



// ============================================================
/*
  Simple subclass of QXmlSimpleReader,
  to setup VgLogHandler for this reader
*/
class VgLogReader : public QXmlSimpleReader
{
public:
   VgLogReader( VgLogView* lv );
   ~VgLogReader();
   
   bool parse( QString filepath, bool incremental = false );
   bool parseContinue();
   
   VgLogHandler* handler() {
      return vghandler;
   }
   
private:
   VgLogHandler* vghandler;
   QXmlInputSource* source;
   QFile file;
};

#endif // #ifndef __VGLOGREADER_H
