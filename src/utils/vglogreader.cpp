/****************************************************************************
** VgLogReader implementation
**  - reads valgrind xml log into a VgLog
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

#include "utils/vglogreader.h"
#include "utils/vk_utils.h"


/**********************************************************************/
/*!
  VgLogReader
*/
VgLogReader::VgLogReader( VgLogView* lv )
   : vghandler( 0 )
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
   , readPos( 0 ), incremental( false )
#else
   , source( 0 )
#endif
{
   vghandler = new VgLogHandler( lv );
#if QT_VERSION < QT_VERSION_CHECK( 5, 15, 0 )
   setContentHandler( vghandler );
   setErrorHandler( vghandler );
   //  setLexicalHandler( vghandler );
   //  setDeclHandler( vghandler );
   //  setDTDHandler( vghandler );
#endif
}

VgLogReader::~VgLogReader()
{
   if ( vghandler != 0 ) {
      delete vghandler;
      vghandler = 0;
   }
   
#if QT_VERSION < QT_VERSION_CHECK( 5, 15, 0 )
   if ( source ) {
      delete source;
      source = 0;
   }
#endif
   
   if ( file.isOpen() ) {
      file.close();
   }
}

bool VgLogReader::parseFile( QString filepath, bool incremental/*=false*/ )
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
   if ( file.isOpen() ) {
      file.close();
   }
   
   file.setFileName( filepath );
   if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
      return false;
   }
   
   reader.clear();
   readPos = 0;
   this->incremental = incremental;
   
   if ( !readPendingData() ) {
      return false;
   }
   
   return parseChunk();
#else
   if ( source ) {
      delete source;
   }
   
   if ( file.isOpen() ) {
      file.close();
   }
   
   file.setFileName( filepath );
   source = new QXmlInputSource( &file );
   return QXmlSimpleReader::parse( source, incremental );
#endif
}

bool VgLogReader::parseContinue()
{
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
   if ( !readPendingData() ) {
      return false;
   }
   
   return parseChunk();
#else
   if ( source ) {
      source->fetchData();
   }
   
   return QXmlSimpleReader::parseContinue();
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
bool VgLogReader::readPendingData()
{
   if ( !file.isOpen() ) {
      return false;
   }
   
   if ( !file.seek( readPos ) ) {
      return false;
   }
   
   QByteArray data = file.readAll();
   readPos = file.pos();
   
   if ( !data.isEmpty() ) {
      reader.addData( data );
   }
   
   return true;
}

bool VgLogReader::parseChunk()
{
   while ( !reader.atEnd() ) {
      switch ( reader.readNext() ) {
      case QXmlStreamReader::StartDocument:
         if ( !vghandler->startDocument() ) {
            return false;
         }
         break;
      case QXmlStreamReader::ProcessingInstruction:
         if ( !vghandler->processingInstruction(
                 reader.processingInstructionTarget().toString(),
                 reader.processingInstructionData().toString() ) ) {
            return false;
         }
         break;
      case QXmlStreamReader::StartElement:
         if ( !vghandler->startElement( QString(),
                                        reader.name().toString(),
                                        reader.qualifiedName().toString(),
                                        reader.attributes() ) ) {
            return false;
         }
         break;
      case QXmlStreamReader::EndElement:
         if ( !vghandler->endElement( QString(),
                                      reader.name().toString(),
                                      reader.qualifiedName().toString() ) ) {
            return false;
         }
         break;
      case QXmlStreamReader::Characters:
         if ( !reader.isWhitespace() &&
              !vghandler->characters( reader.text().toString() ) ) {
            return false;
         }
         break;
      case QXmlStreamReader::EndDocument:
         return vghandler->endDocument();
      case QXmlStreamReader::Invalid:
         if ( reader.error() == QXmlStreamReader::PrematureEndOfDocumentError &&
              incremental ) {
            return true;
         }
         
         return vghandler->fatalError( reader );
      default:
         break;
      }
   }
   
   return !reader.hasError();
}
#endif


/**********************************************************************/
/* VgLogHandler */
VgLogHandler::VgLogHandler( VgLogView* lv )
{
   logview = lv;
   node = doc;
   m_finished = false;
   m_started = false;
}

VgLogHandler::~VgLogHandler()
{ }

/* gets <?xml...> element */
bool VgLogHandler::processingInstruction( const QString& target, const QString& data )
{
   //  vkPrintErr("VgLogHandler::processingInstruction: %s, %s", target.latin1(), data.latin1());
   doc.appendChild( doc.createProcessingInstruction( target, data ) );
   node = doc;
   return true;
}

bool VgLogHandler::startElement( const QString&, const QString&,
                                 const QString& tag, const VgXmlAttributes& )
{
   //  vkPrintErr("VgLogHandler::startElement: '%s'", tag.latin1());
   QDomNode n = doc.createElement( tag );
   node.appendChild( n );
   node = n;
   
   if ( node == doc.documentElement() ) {
      QDomProcessingInstruction xml_insn =
         doc.firstChild().toProcessingInstruction();
      if ( ! logview->init( xml_insn, tag ) ) {
         //VK_DEBUG("Error: Failed log initialisation");
         return false;
      }
   }
   
   return true;
}

bool VgLogHandler::endElement( const QString&, const QString&,
                               const QString& /*tag*/ )
{
   // vkPrintErr("VgLogHandler::endElement: %s", qPrintable( tag ));
   // Should never have end element at doc level
   if ( node == doc ) {
      //VK_DEBUG("VgLogHandler::endElement(): Error: node == doc");
      return false;
   }
   
   QDomNode prnt = node.parentNode();
   
   /* if closing a top-level tag, append to vglog */
   if ( prnt == doc.documentElement() ) {
      QString errMsg;
      if ( ! logview->appendNode( node, errMsg ) ) {
         //VK_DEBUG("Failed to append node");
         m_fatalMsg = errMsg;
         return false;
      }
   }
   
   node = prnt;
   
   if ( node == doc ) {
      /* In case we get bad xml after the closing tag, mark as 'finished'
         This may happed, for example, as a result of doing fork() but
         not exec() under valgrind.  When the process forks, you wind up
         with 2 V's attached to the same logfile, which doesn't get
         sorted out until the child does exec().
      */
      m_finished = true;
   }
   
   return true;
}

bool VgLogHandler::characters( const QString&  ch )
{
   //  vkPrintErr("characters: '%s'", ch.latin1());
   // No text as child of some document
   if ( node == doc ) {
      return false;
   }
   
   /* ignore text as child of doc_elem
      => valgrind non-xml output (shouldn't happen), or client output */
   if ( node == doc.documentElement() ) {
      return true;
   }
   
   QString chars = ch.simplified();
   
   if ( !chars.isEmpty() ) {
      node.appendChild( doc.createTextNode( chars ) );
      //    vkPrintErr("chars: '%s'", chars.latin1());
   }
   
   return true;
}

/* Called by xml reader at start of parsing */
bool VgLogHandler::startDocument()
{
   //   vkPrintErr("VgLogHandler::startDocument()\n");
   vk_assert( logview != 0 );
   
   doc = QDomDocument();
   node = doc;
   m_fatalMsg = QString();
   m_finished = false;
   m_started = true;
   return true;
}

/* Called by xml reader after it has finished parsing
   Checks we have a complete document,
   i.e. endElement() has returned node ptr to root
*/
bool VgLogHandler::endDocument()
{
   //   vkPrintErr("VgLogHandler::endDocument()\n");
   m_finished = true;
   
   if ( node != doc ) {
      return false;
   }
   
   return true;
}

/* non-fatal error: just report it */
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
bool VgLogHandler::error( const QXmlStreamReader& reader )
#else
bool VgLogHandler::error( const QXmlParseException& exception )
#endif
{
   //   vkPrintErr("VgLogHandler::error");
   QString err =
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
      reader.errorString() +
      " (line: " + QString::number( reader.lineNumber() ) +
      ", col: " + QString::number( reader.columnNumber() ) + ")";
#else
      exception.message() +
      " (line: " + QString::number( exception.lineNumber() ) +
      ", col: " + QString::number( exception.columnNumber() ) + ")";
#endif
                 
   // printf("VgLogHandler::non-fatal error: %s", err.latin1());
   
   return true; /* try to continue. */
}

#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
bool VgLogHandler::fatalError( const QXmlStreamReader& reader )
#else
bool VgLogHandler::fatalError( const QXmlParseException& exception )
#endif
{
   //  vkPrintErr("fatalError");

   // msg previously set by logview: print everything.
   m_fatalMsg =
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
      reader.errorString() +
      " (line: " + QString::number( reader.lineNumber() ) +
      ", col: " + QString::number( reader.columnNumber() ) + ")" +
      "\n\n" + m_fatalMsg;
#else
      exception.message() +
      " (line: " + QString::number( exception.lineNumber() ) +
      ", col: " + QString::number( exception.columnNumber() ) + ")" +
      "\n\n" + m_fatalMsg;
#endif
                
   if ( m_finished ) {
      /* If we finished before we got the error, this is probably the
         result of Valgrind's fork-no-exec problem. */
      m_fatalMsg
      += "\nError after document closing tag.\nThis may be "
         "caused by the Valgrinded application doing fork() but "
         "not exec().  If so, ensure each fork() has a matching "
         "exec() call.";
   }
   
   return false; /* don't continue parsing */
}
