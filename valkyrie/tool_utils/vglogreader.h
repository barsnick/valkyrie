/* ---------------------------------------------------------------------
 * vglogreader: reads xml log into a VgLog                 vglogreader.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_XMLPARSER_H
#define __VK_XMLPARSER_H

#include <vglog.h>

#include <qdom.h>
#include <qlistview.h>
#include <qvaluelist.h>

#if 1
#include <qxml.h>
#else
/* For debugging xml parser */
#include <vkxml.h>
#define QXmlSimpleReader VkXmlSimpleReader
#define QXmlInputSource VkXmlInputSource
#define QXmlDefaultHandler VkXmlDefaultHandler
#define QXmlAttributes VkXmlAttributes
#define QXmlParseException VkXmlParseException
#endif


/**********************************************************************/
/*
  Simple subclass of QXmlSimpleReader,
  to setup VgLogHandler for this reader
*/
class VgLogHandler;
class VgLogReader : public QXmlSimpleReader
{
public:
   VgLogReader( VgLog* vglog );
   ~VgLogReader();

   bool parse( QString filepath, bool incremental=false );
   bool parseContinue();

   VgLogHandler* handler() { return vghandler; }

private:
   VgLogHandler* vghandler;
   QXmlInputSource* source;
   QFile file;
};



/**********************************************************************/
/*
  Simple xml handler class for valgrind logs:
  - creates node tree from input
  - hands off complete top-level branches to VgLog
  (e.g. preamble, error etc)
*/
class VgLogHandler : public QXmlDefaultHandler
{
public:
   VgLogHandler( VgLog* l );
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

   // error handler
   bool fatalError( const QXmlParseException& exception );

   QString errorString();

   bool finished;

private:
   QDomDocument doc;
   VgLog* vglog;
   QDomNode node;

   QString m_errorMsg;
};

#endif // #ifndef __VK_XMLPARSER_H
