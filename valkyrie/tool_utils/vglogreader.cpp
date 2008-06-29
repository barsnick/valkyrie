/* ---------------------------------------------------------------------
 * vglogreader: reads xml log into a VgLog                 vglogreader.h
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#include "vglogreader.h"
#include "vglog.h"

#include <assert.h>

/**********************************************************************/
/* VgLogReader */
VgLogReader::VgLogReader( VgLog* vglog )
   : vghandler(0), source(0)
{
   vghandler = new VgLogHandler( vglog );
   setContentHandler( vghandler );
   setErrorHandler( vghandler );
   //  setLexicalHandler( vghandler );
   //  setDeclHandler( vghandler );
   //  setDTDHandler( vghandler );
}

VgLogReader::~VgLogReader()
{
   if ( vghandler != 0 ) {
      delete vghandler;
      vghandler = 0;
   }
   if (source) {
      delete source;
      source = 0;
   }
   if (file.isOpen())
      file.close();
}

bool VgLogReader::parse( QString filepath, bool incremental/*=false*/ )
{
   if (source)
      delete source;
   if (file.isOpen())
      file.close();
   file.setName( filepath );
   source = new QXmlInputSource( file );
   return QXmlSimpleReader::parse( source, incremental );
}

bool VgLogReader::parseContinue()
{
   if (source)
      source->fetchData();
   return QXmlSimpleReader::parseContinue();
}


/**********************************************************************/
/* VgLogHandler */
VgLogHandler::VgLogHandler( VgLog* alog )
{
   vglog = alog;
   node = doc;
   m_finished = false;
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
                                 const QString& tag,
                                 const QXmlAttributes& )
{
   //  vkPrintErr("VgLogHandler::startElement: '%s'", tag.latin1());
   QDomNode n = doc.createElement( tag );
   node.appendChild( n );
   node = n;

   if (node == doc.documentElement()) {
      QDomProcessingInstruction xml_insn =
         doc.firstChild().toProcessingInstruction();
      if ( ! vglog->init( xml_insn, tag ) ) {
         //VK_DEBUG("Error: Failed log initialisation");
         return false;
      }
   }

   return true;
}

bool VgLogHandler::endElement( const QString&, const QString&,
                               const QString& /*tag*/ )
{
   //  vkPrintErr("VgLogHandler::endElement: %s", tag.latin1());
   // Should never have end element at doc level
   if ( node == doc ) {
      //VK_DEBUG("VgLogHandler::endElement(): Error: node == doc");
      return false;
   }

   QDomNode prnt = node.parentNode();

   /* if closing a top-level tag, append to vglog */
   if (prnt == doc.documentElement()) {
      if ( ! vglog->appendNode( node ) ) {
         //VK_DEBUG("Failed to append node");
         return false;
      }
   }
   node = prnt;

   if (node == doc) {
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
   if ( node == doc )
      return false;

   /* ignore text as child of doc_elem
      => valgrind non-xml output (shouldn't happen), or client output */
   if ( node == doc.documentElement() )
      return true;

   QString chars = ch.simplifyWhiteSpace();
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
   assert(vglog != 0);

   doc = QDomDocument();
   node = doc;
   m_fatalMsg = QString();
   m_finished = false;
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
   if (node != doc)
      return false;
   return true;
}

/* non-fatal error: just report it */
bool VgLogHandler::error( const QXmlParseException& exception )
{
//   vkPrintErr("VgLogHandler::error");
   QString err = exception.message() +
      " (line: " + QString::number(exception.lineNumber()) +
      ", col: " + QString::number(exception.columnNumber()) + ")";

   // printf("VgLogHandler::non-fatal error: %s", err.latin1());

   return true; /* try to continue. */
}

bool VgLogHandler::fatalError( const QXmlParseException& exception )
{
   //  vkPrintErr("fatalError");
   m_fatalMsg = exception.message() +
      " (line: " + QString::number(exception.lineNumber()) +
      ", col: " + QString::number(exception.columnNumber()) + ")";

   if (m_finished) {
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
