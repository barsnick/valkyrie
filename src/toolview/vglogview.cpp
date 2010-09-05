/****************************************************************************
** VgLogView implementation
**  - links QDomElements with QTreeWidgetItems
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

#include "toolview/vglogview.h"
#include "utils/vk_utils.h"
#include "utils/vk_config.h"

#include <QFileInfo>
#include <QStringList>
#include <QTextStream>



// ============================================================
/*!
  static map (tagname->enum) + access functions
*/
static ElemTypeMap setupElemTypeMap()
{
   ElemTypeMap etmap;
   etmap["valgrindoutput"]   = VG_ELEM::ROOT;
   etmap["protocolversion"]  = VG_ELEM::PROTOCOL_VERSION;
   etmap["protocoltool"]     = VG_ELEM::PROTOCOL_TOOL;
   etmap["preamble"]         = VG_ELEM::PREAMBLE;
   etmap["pid"]              = VG_ELEM::PID;
   etmap["ppid"]             = VG_ELEM::PPID;
   etmap["tool"]             = VG_ELEM::TOOL;
   etmap["logfilequalifier"] = VG_ELEM::LOGQUAL;
   etmap["var"]              = VG_ELEM::VAR;
   etmap["value"]            = VG_ELEM::VALUE;
   etmap["usercomment"]      = VG_ELEM::COMMENT;
   etmap["args"]             = VG_ELEM::ARGS;
   etmap["vargv"]            = VG_ELEM::VARGV;
   etmap["argv"]             = VG_ELEM::ARGV;
   etmap["exe"]              = VG_ELEM::EXE;
   etmap["arg"]              = VG_ELEM::ARG;
   etmap["status"]           = VG_ELEM::STATUS;
   etmap["state"]            = VG_ELEM::STATE;
   etmap["time"]             = VG_ELEM::TIME;
   etmap["error"]            = VG_ELEM::ERROR;
   etmap["unique"]           = VG_ELEM::UNIQUE;
   etmap["tid"]              = VG_ELEM::TID;
   etmap["kind"]             = VG_ELEM::KIND;
   etmap["what"]             = VG_ELEM::WHAT;
   etmap["xwhat"]            = VG_ELEM::XWHAT;
   etmap["text"]             = VG_ELEM::TEXT;
   etmap["stack"]            = VG_ELEM::STACK;
   etmap["frame"]            = VG_ELEM::FRAME;
   etmap["ip"]               = VG_ELEM::IP;
   etmap["obj"]              = VG_ELEM::OBJ;
   etmap["fn"]               = VG_ELEM::FN;
   etmap["dir"]              = VG_ELEM::SRCDIR;
   etmap["file"]             = VG_ELEM::SRCFILE;
   etmap["line"]             = VG_ELEM::LINE;
   etmap["auxwhat"]          = VG_ELEM::AUXWHAT;
   etmap["xauxwhat"]         = VG_ELEM::XAUXWHAT;
   etmap["announcethread"]   = VG_ELEM::ANNOUNCETHREAD;
   etmap["hthreadid"]        = VG_ELEM::HTHREADID;
   etmap["errorcounts"]      = VG_ELEM::ERRORCOUNTS;
   etmap["pair"]             = VG_ELEM::PAIR;
   etmap["count"]            = VG_ELEM::COUNT;
   etmap["suppcounts"]       = VG_ELEM::SUPPCOUNTS;
   etmap["name"]             = VG_ELEM::NAME;
   etmap["leakedbytes"]      = VG_ELEM::LEAKEDBYTES;
   etmap["leakedblocks"]     = VG_ELEM::LEAKEDBLOCKS;
   etmap["suppression"]      = VG_ELEM::SUPPRESSION;
   etmap["sname"]            = VG_ELEM::SNAME;
   etmap["skind"]            = VG_ELEM::SKIND;
   etmap["skaux"]            = VG_ELEM::SKAUX;
   etmap["sframe"]           = VG_ELEM::SFRAME;
   etmap["rawtext"]          = VG_ELEM::RAWTEXT;
   return etmap;
}

ElemTypeMap VgOutputItem::elemtypeMap = setupElemTypeMap();

/*!
  static access function to static map data
*/
VG_ELEM::ElemType VgOutputItem::elemType( QString tagName )
{
   ElemTypeMap::Iterator it = elemtypeMap.find( tagName );

   if ( it == elemtypeMap.end() ) {
      VK_DEBUG( "Element not found: '%s'", qPrintable( tagName ) );
      return VG_ELEM::NUM_ELEMS;
   }
   return it.value();
}

/*!
   non-static shortcut for elemType of this element
*/
VG_ELEM::ElemType VgOutputItem::elemType()
{
   return elemType( elem.tagName() );
}



// ============================================================
/*!
  base class for SrcItem and OutputItem
*/
VgOutputItem::VgOutputItem( QTreeWidget* parent, QDomElement el )
   : QTreeWidgetItem( parent ), elem( el )
{
   initialise();
}

VgOutputItem::VgOutputItem( QTreeWidgetItem* parent, QDomElement el )
   : QTreeWidgetItem( parent ), elem( el )
{
   initialise();
}

VgOutputItem::VgOutputItem( QTreeWidgetItem* parent, QTreeWidgetItem* after,
                            QDomElement el )
   : QTreeWidgetItem( parent, after ), elem( el )
{
   initialise();
}

void VgOutputItem::initialise()
{
   isReadable = isWriteable = false;
   isExpandable = false;
}

void VgOutputItem::setText( QString str )
{
   QTreeWidgetItem::setText( 0, str );
}

VgOutputItem* VgOutputItem::firstChild()
{
   return ( VgOutputItem* )QTreeWidgetItem::child( 0 );
}

VgOutputItem* VgOutputItem::parent()
{
   return ( VgOutputItem* )QTreeWidgetItem::parent();
}


/*!
  Setup children of this item.
  Derived items use this to load children from the model on demand.
  Rem to call this base class version at the end of derived methods,
  in order to finish initialisation of children.

  Ideally, we'd have overloaded QWidgetTreeItem::setExpanded(),
  but it's no longer virtual in Qt4 :-(
*/
void VgOutputItem::openChildren()
{
   if ( parent() == 0 ) {
      // Top-level item: no init needed.
      return;
   }

   // load children (overloaded by each item)
   setupChildren();

   // now we've loaded any child items from the model,
   // we can open the item without jitter.
   this->setExpanded( true );
}


/*!
   since we add children on demand, we can't use childCount()
*/
bool VgOutputItem::getIsExpandable()
{ return isExpandable; }

bool VgOutputItem::getIsReadable()
{ return isReadable; }

bool VgOutputItem::getIsWriteable()
{ return isWriteable; }

QDomElement VgOutputItem::getElement()
{ return elem; }





// ============================================================
/*!
  TopStatus: first item in listview
  as two text lines:
  status, client exe
  errcounts(num_errs), leak_errors(num_bytes++, num_blocks++)
*/
TopStatusItem::TopStatusItem( QTreeWidget* parent, QDomElement exe,
                              QDomElement status, QString toolstatus,
                              QString _protocol )
   : VgOutputItem( parent, exe ),
     toolstatus_str( toolstatus ), num_errs( 0 ),
     time_str(), protocol( _protocol )
{
   state_str  = status.firstChildElement( "state" ).text();
   start_time = status.firstChildElement( "time" ).text();

   status_tmplt = "Valgrind: %1 '%2'  %3\nErrors: %4%5";
   updateText();

   isExpandable = true;
}

void TopStatusItem::updateText()
{
   status_str = status_tmplt
                .arg( state_str )  // STARTED|FINISHED
                .arg( QFileInfo( elem.text() ).fileName() )       // exe
                .arg( time_str )                                  // time
                .arg( num_errs )
                .arg( toolstatus_str );

   setText( status_str );
}


// finished
void TopStatusItem::updateStatus( QDomElement status )
{
   state_str = status.firstChildElement( "state" ).text();

   int sday, shours, smins, ssecs, smsecs;
   int eday, ehours, emins, esecs, emsecs;
   int ret;
   
   // Vg >= v3.1 (i.e. >= protocol 2) outputs:
   //  - "elapsed wallclock time since process start"
   bool ok;
   int proto = protocol.toInt( &ok );
   if ( !ok || proto < 2 ) {
      VK_DEBUG( "Unsupported Vg XML protocol version: '%s'\n",
                qPrintable( protocol ) );
      return;
   }

   // start count
   ret = sscanf( start_time.toAscii().constData(), "%d:%d:%d:%d.%4d",
                 &sday, &shours, &smins, &ssecs, &smsecs );

   if ( ret == 5 ) {
      QString end_time = status.firstChildElement( "time" ).text();
      ret = sscanf( end_time.toAscii().constData(), "%d:%d:%d:%d.%4d",
                    &eday, &ehours, &emins, &esecs, &emsecs );

      if ( ret == 5 ) {
         // elapsed time
         int msecs = emsecs - smsecs;
         int secs  = esecs  - ssecs;
         int mins  = emins  - smins;
         int hours = ehours - shours;
         int days  = eday   - sday;

         if ( msecs < 0 ) { secs--;  msecs += 1000; }
         if ( secs  < 0 ) { mins--;  secs  += 60;   }
         if ( mins  < 0 ) { hours--; mins  += 60;   }
         if ( hours < 0 ) { days--;  hours += 24;   }
         if ( days  < 0 ) {
            VK_DEBUG( "negative runtime!\n" );
            msecs = secs = mins = hours = days = 0;
         }

         // create our time string
         QTextStream time_strm( &time_str, QIODevice::WriteOnly );
         time_strm << "(wallclock runtime: ";
         if ( days  > 0 ) time_strm << days  << "d ";
         if ( hours > 0 ) time_strm << hours << "h ";
         if ( mins  > 0 ) time_strm << mins  << "m ";
         time_strm << secs << "." << msecs << "s)";

         updateText();
      }
      else {
         VK_DEBUG( "can't read end-time string\n" );
      }
   }
   else {
      VK_DEBUG( "can't read start-time string\n" );
   }
}

void TopStatusItem::updateFromErrorCounts( QDomElement errcnts )
{
   // sum all counts in all pairs of errorcounts
   num_errs = 0;
   QDomNodeList pairs = errcnts.childNodes();
   QDomElement e = pairs.item( 0 ).toElement();
   
   for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
      QString count = e.firstChildElement().text();
      num_errs += count.toInt();
   }

   updateText();
}




// ============================================================
/*!
  InfoItem
   - info: as text line: tool, pid, ppid
   - logfilequal
   - var: as text line
   - value: as text line
   - usercomment:
   - comment: as text line
   - args
   - details: as text lines
*/
InfoItem::InfoItem( VgOutputItem* parent, QDomElement root /*element:ROOT*/ )
   : VgOutputItem( parent, root )
{
   QString tool = elem.firstChildElement( "tool" ).text();
   tool[0] = tool[0].toUpper();
   QString pid = elem.firstChildElement( "pid" ).text();
   QString ppid = elem.firstChildElement( "ppid" ).text();
   
   QString content =
      QString( "%1 output for process id ==%2== (parent pid ==%3==)" )
      .arg( tool )
      .arg( pid )
      .arg( ppid );
      
   setText( content );
   
   isExpandable = true;
}

void InfoItem::setupChildren()
{
   if ( childCount() == 0 ) {
      VgOutputItem* last_item = 0;

      // handle any number of log-file-qualifiers
      QDomElement logqual = elem.firstChildElement( "logfilequalifier" );
      while ( !logqual.isNull() && logqual.tagName() == "logfilequalifier" ) {
         last_item = new LogQualItem( this, logqual );
         last_item->openChildren();
         logqual = logqual.nextSiblingElement();
      }
      
      // may / may not have a user comment
      QDomElement comment = elem.firstChildElement( "usercomment" );
      if ( ! comment.isNull() ) {
         last_item = new VgOutputItem( this, last_item, comment );
         last_item->setText( comment.text() );
      }
      
      // args
      QDomElement args = elem.firstChildElement( "args" );
      last_item = new ArgsItem( this, last_item, args );
      last_item->openChildren();
   }
}



// ============================================================
/*!
  LogQualItem
*/
LogQualItem::LogQualItem( VgOutputItem* parent, QDomElement logqual )
   : VgOutputItem( parent, logqual )
{
   setText( "logfilequalifier" );

   isExpandable = true;
}

void LogQualItem::setupChildren()
{
   if ( childCount() == 0 ) {
      VgOutputItem* last_item = 0;
      QDomElement var   = elem.firstChildElement();
      QDomElement value = var.nextSiblingElement();
#ifdef DEBUG_ON
      if ( var.tagName() != "var" ) {
         vkPrintErr( "LogQualItem::setupChildren(): unexpected tagName: %s",
                     qPrintable( var.tagName() ) );
      }
      if ( value.tagName() != "value" ) {
         vkPrintErr( "LogQualItem::setupChildren(): unexpected tagName: %s",
                     qPrintable( value.tagName() ) );
      }
#endif

      last_item = new VgOutputItem( this, last_item, var );
      last_item->setText( var.text() + ": '" + value.text() + "'" );
   }
}



// ============================================================
/*!
  ArgsItem
*/
ArgsItem::ArgsItem( VgOutputItem* parent, QTreeWidgetItem* after,
                    QDomElement args )
   : VgOutputItem( parent, after, args )
{
   setText( "args" );
   isExpandable = true;
}

void ArgsItem::setupChildren()
{
   if ( childCount() == 0 ) {
      QDomElement vgInfo = elem.firstChildElement( "vargv" );
      QDomElement exInfo = elem.firstChildElement( "argv" );
      
      VgOutputItem* last_item = 0;
      QDomElement e = vgInfo.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
#ifdef DEBUG_ON
         if ( e.tagName() != "exe" && e.tagName() != "arg" ) {
            vkPrintErr( "ArgsItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( e.tagName() ) );
         }
#endif
         last_item = new VgOutputItem( this, last_item, e );
         last_item->setText( e.text() );
      }
      
      e = exInfo.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
#ifdef DEBUG_ON
         if ( e.tagName() != "exe" && e.tagName() != "arg" ) {
            vkPrintErr( "ArgsItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( e.tagName() ) );
         }
#endif
         last_item = new VgOutputItem( this, last_item, e );
         last_item->setText( e.text() );
      }
   }
}



// ============================================================
/*!
  PreambleItem
   - preamble
   - lines: as text lines
*/
PreambleItem::PreambleItem( VgOutputItem* parent,
                            QTreeWidgetItem* after,
                            QDomElement preamble )
   : VgOutputItem( parent, after, preamble )
{
   setText( "Preamble" );
   isExpandable = true;
}

void PreambleItem::setupChildren()
{
   if ( childCount() == 0 ) {
      VgOutputItem* last_item = 0;
      QDomElement e = elem.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
#ifdef DEBUG_ON
         if ( e.tagName() != "line" ) {
            vkPrintErr( "PreambleItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( e.tagName() ) );
         }
#endif
         last_item = new VgOutputItem( this, last_item, e );
         last_item->setText( e.text() );
      }
   }
}



// ============================================================
/*!
  ErrorItem
*/
ErrorItem::ErrorItem( VgOutputItem* parent, QTreeWidgetItem* after,
                      QDomElement err, ErrorItem::AcronymMap acnymMap )//, QString acnym )
   : VgOutputItem( parent, after, err )
{
   fullSrcPathShown = false;
   isExpandable = true;

   // unclear what we can expect re what/xwhat.
   //  - give 'what' preference over 'xwhat'.
   QString description;
   QDomElement what = elem.firstChildElement( "what" );
   QDomElement xwhat = elem.firstChildElement( "xwhat" );
   if ( !what.isNull() ) {
      description = what.text();
   }
   else {
      QDomElement text = xwhat.firstChildElement( "text" );
      description = text.text();
   }

   QString kind = elem.firstChildElement( "kind" ).text();
   QString acnym = getErrorAcronym( acnymMap, kind );

   err_tmplt  = acnym + " [%1]: " + description;
   updateCount( "1" );
}

void ErrorItem::updateCount( QString count )
{
//TODO: perhaps only print [count] if >1 ?
   setText( err_tmplt.arg( count ) );
}

void ErrorItem::setupChildren()
{
   if ( childCount() == 0 ) {
      VgOutputItem* last_item = this;  // for listview ordering.

      // iterate over all child dom elements
      QDomElement e = elem.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
         VG_ELEM::ElemType elemtype = VgOutputItem::elemType( e.tagName() );

         switch ( elemtype ) {
         case VG_ELEM::UNIQUE:
         case VG_ELEM::KIND:
            // ignore.
            break;

         case VG_ELEM::TID: {
            last_item = new VgOutputItem( this, last_item, e );
            last_item->setText( "Thread Id: " + e.text() );
            break;
         }

         case VG_ELEM::WHAT:
         case VG_ELEM::AUXWHAT: {
            //Note: (xml-output.txt, 1Mar2008): Some errors may have two <auxwhat>
            // blocks, rather than just one, resulting from DATASYMS branch merge.
            VgOutputItem* item = new VgOutputItem( this, last_item, e );
            item->setText( e.text() );

            QFont fnt = item->font( 0 );
            fnt.setWeight( QFont::DemiBold );
            fnt.setItalic( true );
            item->setFont( 0, fnt );

            last_item = item;
            break;
         }

         case VG_ELEM::XWHAT:
         case VG_ELEM::XAUXWHAT: {
            QDomElement zwhat = e;

            // All XWHAT/XAUXWHAT's have a text element: print it
            QDomElement text = zwhat.firstChildElement();
      #ifdef DEBUG_ON
            if ( text.tagName() != "text" ) {
               vkPrintErr( "ErrorItemHG::setupChildrenPerTool(): unexpected"
                           " tagName within %s: %s", qPrintable( zwhat.tagName() ),
                           qPrintable( text.tagName() ) );
            }
      #endif
            last_item = new VgOutputItem( this, last_item, text );
            last_item->setText( text.text() );

            QFont fnt = last_item->font( 0 );
            fnt.setWeight( QFont::DemiBold );
            fnt.setItalic( true );
            last_item->setFont( 0, fnt );

            // Ignore further XWHAT/XAUXWHAT children elements here:
            // they are used elsewhere, e.g. for updating TopStatus.
            break;
         }

         case VG_ELEM::STACK: {
            VgOutputItem* stack = new StackItem( this, last_item, e );
            stack->openChildren();
            last_item = stack;
            break;
         }

         case VG_ELEM::SUPPRESSION: {
            // don't display this here.
            // TODO: right click on ErrorItem => copy suppression (if available)
            break;
         }

         default:
            vkPrintErr( "ErrorItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( e.tagName() ) );
            break;
         }
      }
   }
}

/*!
  returns an acronym for a given error::kind
*/
QString ErrorItem::getErrorAcronym( ErrorItem::AcronymMap map,
                                    QString kind )
{
   AcronymMap::Iterator it = map.find( kind );
   if ( it == map.end() ) {
      return "???";
   }
   return it.value();
}


/*!
  Shows src paths for all frames under this error
*/
void ErrorItem::showFullSrcPath( bool show )
{
   // (maybe) multiple stacks
   for ( int i=0; i<childCount(); ++i ) {
      StackItem* stack = (StackItem*)child( i );
      if ( stack->elemType() == VG_ELEM::STACK ) {
         // multiple frames
         for ( int i=0; i<stack->childCount(); ++i ) {
            VgOutputItem* item = (VgOutputItem*)stack->child( i );
            if ( item->elemType() == VG_ELEM::FRAME ) {
               QString text = ((FrameItem*)item)->describe_IP( show );
               item->setText( text );
            }
         }
      }
   }

   fullSrcPathShown = show;
}

/*!
  getter: isFullSrcPathShown()
*/
bool ErrorItem::isFullSrcPathShown()
{
   return fullSrcPathShown;
}



// ============================================================
/*!
  StackItem
*/
StackItem::StackItem( VgOutputItem* parent, QTreeWidgetItem* after,
                      QDomElement stck )
   : VgOutputItem( parent, after, stck )
{
   setText( "stack" );

   isExpandable = true;
}

void StackItem::setupChildren()
{
   if ( childCount() == 0 ) {
      VgOutputItem* last_item = this;
      QDomElement e = elem.firstChildElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
#ifdef DEBUG_ON
         if ( e.tagName() != "frame" ) {
            vkPrintErr( "StackItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( e.tagName() ) );
         }
#endif
         last_item = new FrameItem( this, last_item, e );
         // don't open children: just set them up.
         last_item->setupChildren();
      }
   }
}


// ============================================================
/*!
  FrameItem
*/
FrameItem::FrameItem( VgOutputItem* parent, QTreeWidgetItem* after,
                      QDomElement frm )
   : VgOutputItem( parent, after, frm )
{
   // check what perms the user has w.r.t. this file
   QDomElement srcdir  = frm.firstChildElement( "dir" );
   QDomElement srcfile = frm.firstChildElement( "file" );
   
   if ( !srcfile.isNull() ) {
      QString path;
      
      if ( !srcdir.isNull() ) {
         path = srcdir.text() + "/";
      }
      
      path += srcfile.text();
      
      QFileInfo fi( path );
      
      if ( fi.exists() && fi.isFile() /* && !fi.isSymLink() */) {
         isReadable  = fi.isReadable();
         isWriteable = fi.isWritable();
      }
   }
   
   setText( describe_IP( false ) );

   isExpandable = isReadable;

   if ( isExpandable ) {
      QColor col( "blue" );
      QBrush brush( col );
      setForeground( 0, brush );
   } else {
      QColor col( "darkred" );
      QBrush brush( col );
      setForeground( 0, brush );
   }
}


void FrameItem::setupChildren()
{
   if ( childCount() == 0 && isExpandable ) {
      QDomElement srcdir  = elem.firstChildElement( "dir" );
      QDomElement srcfile = elem.firstChildElement( "file" );
      QDomElement line    = elem.firstChildElement( "line" );
      
      if ( srcfile.isNull() ) {
         return;
      }
      
      QString path;
      if ( !srcdir.isNull() ) {
         path = srcdir.text() + "/";
      }
      path += srcfile.text();
      if ( !QFile::exists( path ) ) {
         vkPrintErr( "FrameItem::setupChildren(): can't find source: %s, %s",
                     qPrintable( srcdir.text() ), qPrintable( srcfile.text() ) );
         return;
      }
      
      // create the item for the src lines
      new SrcItem( this, line, path );
   }
}


/*!
  ref: coregrind/m_debuginfo/symtab.c :: VG_(describe_IP)
*/
QString FrameItem::describe_IP( bool withPath/*=false*/ )
{
   QDomNodeList frame_details = elem.childNodes();
   vk_assert( frame_details.count() >= 1 );  /* only ip guaranteed */
   QDomElement ip     = elem.firstChildElement( "ip" );
   QDomElement obj    = elem.firstChildElement( "obj" );
   QDomElement fn     = elem.firstChildElement( "fn" );
   QDomElement dir    = elem.firstChildElement( "dir" );
   QDomElement srcloc = elem.firstChildElement( "file" );
   QDomElement line   = elem.firstChildElement( "line" );

   bool  know_fnname  = !fn.isNull();
   bool  know_objname = !obj.isNull();
   bool  know_srcloc  = !srcloc.isNull() && !line.isNull();
   bool  know_dirinfo = !dir.isNull();

   QString str = ip.text() + ": ";

   if ( know_fnname ) {
      str += fn.text();

      if ( !know_srcloc && know_objname ) {
         str += " (in " + obj.text() + ")";
      }
   }
   else if ( know_objname && !know_srcloc ) {
      str += "(within " + obj.text() + ")";
   }
   else {
      str += "???";
   }

   if ( know_srcloc ) {
      QString path;

      if ( withPath && know_dirinfo ) {
         path = dir.text() + "/";
      }

      path += srcloc.text();
      str += " (" + path + ":" + line.text() + ")";
   }

   return str;
}



// ============================================================
/*!
  class SrcItem (error::stack::frame::dir/file/line)
   - given a valid source file path, display a chunk of the
     offending file at the given lineno.
   - double-click item => source file opened in an editor, at lineno.
*/
SrcItem::SrcItem( VgOutputItem* parent, QDomElement line, QString path )
   : VgOutputItem( parent, line )
{
   // --- setup text ---
   int target_line = line.text().toInt();
   
   if ( target_line < 0 ) {
      target_line = 0;
   }
   
   // num lines to show above / below the target line
   bool ok = false;
   int n_lines = vkConfig->value( "valkyrie/src-lines" ).toInt( &ok );
   if ( !ok ) {
      vkPrintErr( "SrcItem::cons: failed to retrieve/convert 'src-lines' from config." );
   }

   // figure out where to start showing src lines
   int top_line = 1;
   if ( target_line > n_lines + 1 ) {
      top_line = target_line - n_lines;
   }
   int bot_line = target_line + n_lines;
   int current_line = 1;
   
   QFile file( path );
   if ( !file.open( QIODevice::ReadOnly ) ) {
      return;
   }   
   
   // TODO: faster to set file pos using QFile::at(offset)
   QString src_lines;
   QTextStream stream( &file );
   
   while ( !stream.atEnd() && ( current_line <= bot_line ) ) {
      if ( current_line < top_line ) {
         stream.readLine();   // skip lines to top_line
      }
      else {
         src_lines += "  " + stream.readLine() + "\n";
      }
      
      current_line++;
   }
   
   file.close();
   src_lines.truncate( src_lines.length() - 1 ); // remove last newline
   
   // --- setup item ---
   isReadable  = parent->getIsReadable();
   isWriteable = parent->getIsWriteable();
   
   // if we got this far, the source is at least readable.
   vk_assert( isReadable == true );

   if ( isWriteable ) {  // read & write
      setIcon( 0, QPixmap( QString::fromUtf8( ":/vk_icons/icons/vglogview_readwrite.xpm" ) ) );
   }
   else {                // readonly
      setIcon( 0, QPixmap( QString::fromUtf8( ":/vk_icons/icons/vglogview_readonly.xpm" ) ) );
   }
   
   setText( src_lines );

   // pale gray background colour.
   QColor col( "lightgrey" );
   QBrush brush( col );
   setBackground( 0, brush );
}




// ============================================================
/*!
  SuppCountsItem
   - suppcounts
   - pairs: as text line
*/
SuppCountsItem::SuppCountsItem( VgOutputItem* parent,
                                QTreeWidgetItem* after,
                                QDomElement sc )
   : VgOutputItem( parent, after, sc )
{
   setText( "Suppressed errors" );

   isExpandable = true;
}

void SuppCountsItem::setupChildren()
{
   if ( childCount() == 0 ) {
      VgOutputItem* child_item = 0;
      QDomNodeList pairs = elem.childNodes();

      QDomElement e = pairs.item( 0 ).toElement();
      for ( ; !e.isNull(); e = e.nextSiblingElement() ) {
#ifdef DEBUG_ON
         if ( e.tagName() != "pair" ) {
            vkPrintErr( "SuppCountsItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( e.tagName() ) );
         }
#endif
         QDomElement pair = e;
         QDomElement count = pair.firstChildElement();
         QDomElement name = count.nextSiblingElement();
#ifdef DEBUG_ON
         if ( count.tagName() != "count" ) {
            vkPrintErr( "SuppCountsItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( count.tagName() ) );
         }
         if ( name.tagName() != "name" ) {
            vkPrintErr( "SuppCountsItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( name.tagName() ) );
         }
         QDomElement elem = name.nextSiblingElement();
         if ( ! elem.isNull() ) {
            vkPrintErr( "SuppCountsItem::setupChildren(): unexpected tagName: %s",
                        qPrintable( elem.tagName() ) );
         }
#endif
         QString count_str = count.text();
         QString name_str  = name.text();
         QString supp_str = QString( "%1:  " + name_str ).arg( count_str, 4 );
         
         child_item = new VgOutputItem( this, child_item, pair );
         child_item->setText( supp_str );
      }
   }
}





// ============================================================
/*!
  VgLogView
*/
VgLogView::VgLogView( QTreeWidget* v )
   : lastItem( 0 ), view( v )
{}

VgLogView::~VgLogView()
{ }


/*!
  initialise our log
*/
bool VgLogView::init( QDomProcessingInstruction xml_insn, QString doc_tag )
{
   if ( xml_insn.isNull() ) {
      vkPrintErr( "VgLogView::init(): xml_insn isNull" );
      return false;
   }

   if ( doc_tag.isEmpty() ) {
      vkPrintErr( "VgLogView::init(): doc_tag isEmpty" );
      return false;
   }

   QString init_str;
   QTextStream strm( &init_str, QIODevice::WriteOnly );
   int indent = 2;
   xml_insn.save( strm, indent );
   strm << "<" << doc_tag << "/>";

   vglog.setContent( init_str );
   return true;
}



/*!
  Populate our model (QDomDocument) and the view (QListWidget)
   - top-level xml elements are pushed to us from the parser
   - node is reparented to the QDomDocument log

  Tool-logviews can do stuff with the QDomElement, a-la "Template Method",
  by implementing appendNodeTool().
   - rem to set lastItem to created items, so items get added in order
*/
bool VgLogView::appendNode( QDomNode node, QString& errMsg )
{
   errMsg = "";

   // Test node validity, and attempt to populate QDomDocument model first
   if ( vglog.isNull() ) {
      errMsg = "Program error: VgLog not initialised";
      vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
      return false;
   }

   if ( logRoot().isNull() ) {
      errMsg = "XML document root is null";
      vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
      return false;
   }

   QDomElement elem = node.toElement();
   if ( elem.isNull() ) {
      errMsg = "XML Node not an element (" + node.firstChild().nodeValue() + ")";
      vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
      return false;
   }

   // reparent node
   QDomNode n = logRoot().appendChild( node );
   if ( n.isNull() ) {
      errMsg = "Program error: Failed to reparent node: (" + elem.tagName() + ")";
      vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
      return false;
   }

   // check elem is a top-level xml chunk
   VG_ELEM::ElemType elemtype = VgOutputItem::elemType( elem.tagName() );
   if ( elemtype == VG_ELEM::NUM_ELEMS ) {
      errMsg = "Unrecognised tagname: (" + elem.tagName() + ")";
      vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
      return false;
   }


   // --------------------
   // ok so far...
   // now populate view with top-level items, from our model (QDomElements)
   //  - children of these view items are only populated on-demand

   switch ( elemtype ) {
   case VG_ELEM::PROTOCOL_VERSION: {
      if ( elem.text() != "4" ) {
         errMsg = "Unsupported XML protocol version: (" + elem.text() + ")";
         vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
         return false;
      }
      break;
   }

   case VG_ELEM::PROTOCOL_TOOL: {
      QString tool = this->toolName();
      if ( elem.text() != tool ) {
         errMsg = "Wrong tool (" + tool + ") for XML stream (" + elem.text() + ")";
         vkPrintErr( "%s", qPrintable( "VgLogView::appendNode(): " + errMsg ) );
         return false;
      }
      break;
   }

   case VG_ELEM::STATUS: {
      QDomElement status = elem;
      if ( status.firstChildElement().text() == "RUNNING" ) {
         QDomElement exe = logRoot().firstChildElement( "args" )
                           .firstChildElement( "argv" ).firstChildElement( "exe" );

         QDomElement protocol = logRoot().firstChildElement( "protocolversion" );

         topStatus = createTopStatus( view, exe, status, protocol.text() );
         topStatus->setExpanded( true );

         lastItem = new InfoItem( topStatus, logRoot() );
         lastItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );

         QDomElement preamble = logRoot().firstChildElement( "preamble" );
         lastItem = new PreambleItem( topStatus, lastItem, preamble );
      }
      else {
         // update topStatus
         topStatus->updateStatus( status );
      }
      break;
   }

   case VG_ELEM::ERRORCOUNTS: {
      if ( elem.childNodes().count() == 0 ) { // ignore empty errorcounts
         break;
      }

      // update topStatus
      topStatus->updateFromErrorCounts( elem );
      
      // update all non-leak errors
      updateErrorItems( elem );
      break;
   }

   case VG_ELEM::SUPPCOUNTS: {
      lastItem = new SuppCountsItem( topStatus, lastItem, elem );
      break;
   }

   default:
      // may not have dealt with element yet, don't panic!
      break;
   }


   // --------------------
   // Allow tools to do stuff with elem, a-la "Template Method".
   if ( ! appendNodeTool( elem, errMsg ) ) {
      return false;
   }


   // --------------------
   // Set properties for all new items
   if ( lastItem ) {
      lastItem->setChildIndicatorPolicy( QTreeWidgetItem::ShowIndicator );
   }

   return true;
}


/*!
   document element: <valgrindoutput/>
*/
QDomElement VgLogView::logRoot()
{
   return vglog.documentElement();
}


/*!
  iterate over all errors in the listview, looking for a match on
  error->unique with ecounts->pairList->unique.  if we find a match,
  update the error's num_times value
*/
void VgLogView::updateErrorItems( QDomElement ec )
{
   for ( int i=0; i<topStatus->childCount(); ++i ) {
      VgOutputItem* vgItem = (VgOutputItem*)topStatus->child( i );

      if ( vgItem->elemType() != VG_ELEM::ERROR ) {
         continue;
      }

      ErrorItem* vgItemError = ( ErrorItem* )vgItem;

      QString count = "1"; // can't have less than 1 for a reported error
      QString err_unique = vgItemError->getElement().firstChildElement().text();

      // search errorcount pairs for err_unique
      QDomNodeList pairs = ec.childNodes();
      for ( int i = 0; i < pairs.count(); i++ ) {
         QDomNode pair = pairs.item( i );
         QDomNodeList pair_details = pair.childNodes();
         QString unique = pair_details.item( 1 ).toElement().text();

         if ( err_unique == unique ) {
            count = pair_details.item( 0 ).toElement().text();
            break;
         }
      }

      vgItemError->updateCount( count );
   }
}









//TODO: needed?
#if 0
/*
  xml string
  - with \n between each top-level chunk
  - reimplementation of QDomDocument::toString()
*/
QString VgLogView::toString( int )
{
   if ( log.isNull() ) {
      return "";
   }

   QString str;
   QTextStream s( &str, QIODevice::WriteOnly );

   s << log.firstChild() << "\n";
   QString doc_elem = logRoot().nodeName();
   s  << "<" << doc_elem << ">\n\n";
   QDomNode n = logRoot().firstChild();

   for ( ; !n.isNull(); n = n.nextSibling() ) {
      n.save( s, 2 );
      s << "\n";
   }

   s  << "</" << doc_elem << ">\n";
   return str;
}
#endif
