/* ---------------------------------------------------------------------
 * Implementation of VgLogView                             vglogview.cpp
 * Links VgLog elements with qlistview elements
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (C) 2000-2008, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file COPYING for the full license details.
 */

#include "vglogview.h"
#include "vk_utils.h"        /* VK_DEBUG */
#include "vglogview_icons.h"

#include <qfileinfo.h>
#include <qstringlist.h>


/**********************************************************************/
/*
  setup static class maps
*/
ErrorItem::AcronymMap setupErrAcronymMap() {
   ErrorItem::AcronymMap amap;
   amap["CoreMemError"]        = "CRM";
   amap["InvalidFree"]         = "IVF";
   amap["MismatchedFree"]      = "MMF";
   amap["InvalidRead"]         = "IVR";
   amap["InvalidWrite"]        = "IVW";
   amap["InvalidJump"]         = "IVJ";
   amap["Overlap"]             = "OVL";
   amap["InvalidMemPool"]      = "IMP";
   amap["UninitCondition"]     = "UNC";
   amap["UninitValue"]         = "UNV";
   amap["SyscallParam"]        = "SCP";
   amap["ClientCheck"]         = "CCK";
   amap["Leak_DefinitelyLost"] = "LDL";
   amap["Leak_IndirectlyLost"] = "LIL";
   amap["Leak_PossiblyLost"]   = "LPL";
   amap["Leak_StillReachable"] = "LSR";
   return amap;
}
ErrorItem::AcronymMap ErrorItem::acronymMap = setupErrAcronymMap();


/* base class for SrcItem and OutputItem ------------------------------- */
VgOutputItem::VgOutputItem( QListView* parent, VgElement el ) 
   : QListViewItem( parent ), elem( el )
{ initialise(); }

VgOutputItem::VgOutputItem( QListViewItem* parent, VgElement el )
   : QListViewItem( parent ), elem( el )
{ initialise(); }

VgOutputItem::VgOutputItem( QListViewItem* parent, QListViewItem* after,
                            VgElement el )
   : QListViewItem( parent, after ), elem( el )
{ initialise(); }

void VgOutputItem::initialise() 
{
   isReadable = isWriteable = false;
}

void VgOutputItem::setText( QString str ) 
{ QListViewItem::setText( 0, str ); }

VgOutputItem* VgOutputItem::firstChild() 
{ return (VgOutputItem*)QListViewItem::firstChild(); }

VgOutputItem* VgOutputItem::nextSibling() 
{ return (VgOutputItem*)QListViewItem::nextSibling(); }

VgOutputItem* VgOutputItem::parent()
{ return (VgOutputItem*)QListViewItem::parent(); }


void VgOutputItem::paintCell( QPainter* p, const QColorGroup& cg,
                              int col, int width, int align ) 
{
   if ( ! (isReadable || isWriteable ) ) {
      QListViewItem::paintCell( p, cg, col, width, align );
   } else {
      QColorGroup cgrp( cg );
      cgrp.setColor( QColorGroup::Text, Qt::blue );
      QListViewItem::paintCell( p, cgrp, col, width, align );
   }
}

void VgOutputItem::setOpen( bool open ) 
{ QListViewItem::setOpen( open ); }



/**********************************************************************/
/* TopStatus: first item in listview
   as two text lines:
   status, client exe
   errcounts(num_errs), leak_errors(num_bytes++, num_blocks++)
*/
TopStatusItem::TopStatusItem( QListView* parent, VgStatus status,
                              VgElement exe, QString _protocol )
   : VgOutputItem( parent, exe ),
     num_errs(0), num_bytes(0), num_blocks(0), protocol(_protocol)
{
   QString exe_str = QFileInfo( exe.text() ).fileName();

   /* store stime for later use by updateStatus() */
   stime = status.getFirstElem( "time" ).text();

   status_tmplt = QString("Valgrind: %1 '%2'");
   status_str = status_tmplt
      .arg( status.getFirstElem( "state" ).text() )
      .arg( exe_str );

   /* errors, leaks */
   errcounts_tmplt = QString("Errors: %1    Leaked Bytes: %1 in %2 blocks");
   QString err_str = errcounts_tmplt
      .arg( num_errs )
      .arg( num_bytes )
      .arg( num_blocks );

   setText( status_str + "\n" + err_str );
   setMultiLinesEnabled( true );
}

/* finished */
void TopStatusItem::updateStatus( VgStatus status )
{
   QString etime = status.getFirstElem( "time" ).text();

   QString time_str;
   QTextStream time_strm( &time_str, IO_WriteOnly );

   int syear, smonth, sday, shours, smins, ssecs, smsecs;
   int eyear, emonth, eday, ehours, emins, esecs, emsecs;
   int ret;
   if (protocol == "1") {
      // Valgrind < v3.1 outputs standard date-time
      /* start date/time */
      ret = sscanf( stime.ascii(), "%d-%d-%d %d:%d:%d.%4d", 
                    &syear, &smonth, &sday,
                    &shours, &smins, &ssecs, &smsecs );
      if (ret == 7) {
         QDate sta_date;
         QTime sta_time( shours, smins, ssecs, smsecs );
         sta_date = QDate( syear, smonth, sday );
         time_strm << sta_date.toString( "MMM/d/yy " );
         time_strm << sta_time.toString( "hh:mm:ss.zzz" ); 

         /* end date/time */
         time_strm << " - ";
         ret = sscanf( etime.ascii(), "%d-%d-%d %d:%d:%d.%4d", 
                       &eyear, &emonth, &eday,
                       &ehours, &emins, &esecs, &emsecs );
         if (ret == 7) {
            /* end date/time */ 
            QDate end_date = QDate( eyear, emonth, eday );
            QTime end_time( ehours, emins, esecs, emsecs );
            // only output end_date if > start_date
            if ( end_date > sta_date )
               time_strm << end_date.toString( "MMM/d/yy " );
            time_strm << end_time.toString( "hh:mm:ss.zzz" );
           
            /* elapsed time */
            time_strm << " (";
            if ( end_date > sta_date ) {
               int days = sta_date.daysTo( end_date );
               time_strm << days << ((days == 1) ? "day " : "days ");
            }
            QTime time_a;    /* 00:00:00.000 */
            QTime elapsed_time =
               time_a.addMSecs( sta_time.msecsTo( end_time ) );
            time_strm << elapsed_time.toString( "hh:mm:ss.zzz" ) << ")";
         } else {
            VK_DEBUG("can't read end-time string\n");
         } 
      } else {
         VK_DEBUG("can't read start-time string\n");
      }
   }
   else if (protocol == "2") {
      // Valgrind >= v3.1 outputs a count only
      /* start count */
      ret = sscanf( stime.ascii(), "%d:%d:%d:%d.%4d", 
                    &sday, &shours, &smins, &ssecs, &smsecs );
      if (ret == 5) {
         ret = sscanf( etime.ascii(), "%d:%d:%d:%d.%4d", 
                       &eday, &ehours, &emins, &esecs, &emsecs );
         if (ret == 5 ) {
            /* elapsed time */
            int msecs = emsecs - smsecs;
            int secs  = esecs  - ssecs;
            int mins  = emins  - smins;
            int hours = ehours - shours;
            int days  = eday   - sday;
            if (msecs < 0) { secs--;  msecs += 1000; }
            if (secs  < 0) { mins--;  secs  += 60; }
            if (mins  < 0) { hours--; mins  += 60; }
            if (hours < 0) { days--;  hours += 24; }
            if (days  < 0) {
               VK_DEBUG("negative runtime!\n");
               msecs = secs = mins = hours = days = 0;
            }
            time_strm << "(wallclock runtime: ";
            if (days  > 0)
               time_strm << days  << ((days  == 1) ? "day "  : "days " );
            if (hours > 0 || days > 0)
               time_strm << hours << ((hours == 1) ? "hour " : "hours ");
            if (mins  > 0 || hours > 0 || days > 0)
               time_strm << mins  << ((mins  == 1) ? "min "  : "mins " );
            time_strm << secs  << "." << msecs << "secs)";
         } else {
            VK_DEBUG("can't read end-time string (protocol 2)\n");
         } 
      } else {
         VK_DEBUG("can't read start-time string\n");
      } 
   }
   else {
      VK_DEBUG("bad xml protocol version: '%s'\n", protocol.ascii() );
   }

   /* setup the first part: */
   status_str = status_tmplt
      .arg( status.getFirstElem( "state" ).text() )   // FINISHED
      .arg( QFileInfo( elem.text() ).fileName() );    // exe
   status_str += "  " + time_str;

   /* errors, leaks */
   QString err_str = errcounts_tmplt
      .arg( num_errs )
      .arg( num_bytes )
      .arg( num_blocks );

   setText( status_str + "\n" + err_str );
}

void TopStatusItem::updateErrorCounts( VgErrCounts ec )
{
   /* sum all counts in all pairs of errorcounts */
   num_errs = 0;
   QDomNodeList pairs = ec.childNodes();
   QDomElement e = pairs.item(0).toElement();
   for (; !e.isNull(); e = e.nextSibling().toElement() ) {
      VgElement pair = (VgElement&)e;
      QString count = pair.firstChild().toElement().text();
      num_errs += count.toInt();
   }

   QString err_str = errcounts_tmplt
      .arg( num_errs )
      .arg( num_bytes )
      .arg( num_blocks );

   setText( status_str + "\n" + err_str );
}

void TopStatusItem::updateLeakCounts(  VgError err )
{
   if ( !err.isLeak() )
      return;

   /* HACK ALERT!
      VALGRIND_DO_LEAK_CHECK gives repeated leaks...
      taking apart error::what to get record number
      - if '1' then reset counters
   */
   QString what = err.getFirstElem("what").text();
   QString lr_str = what.mid( what.find( "in loss record " ) );
   /* merged logs may not have a 'loss record' string... */
   if ( !lr_str.isEmpty() ) {
      /* valgrind can output multiple leak summaries
         - reset counts when we get a new summary */
      QString record = QStringList::split( " ", lr_str )[3];
      if (record == "1") {
         num_bytes = num_blocks = 0;
      }
   }
   num_bytes  += err.getFirstElem("leakedbytes").text().toUInt();
   num_blocks += err.getFirstElem("leakedblocks").text().toUInt();
  
   QString err_str = errcounts_tmplt
      .arg( num_errs )
      .arg( num_bytes )
      .arg( num_blocks );

   setText( status_str + "\n" + err_str );
}


/**********************************************************************/
/* InfoItem
   - info: as text line: tool, pid, ppid
   - logfilequal
   - var: as text line
   - value: as text line
   - usercomment:
   - comment: as text line
   - args
   - details: as text lines
*/
InfoItem::InfoItem( VgOutputItem* parent, VgElement root /*element:ROOT*/ )
   : VgOutputItem( parent, root )
{
   QString tool = elem.getFirstElem("tool").text();
   tool[0] = tool[0].upper();
   QString pid = elem.getFirstElem("pid").text();
   QString ppid = elem.getFirstElem("ppid").text();

   QString content =
      QString("%1 output for process id ==%2== (parent pid ==%3==)")
      .arg( tool )
      .arg( pid )
      .arg( ppid );

   setText( content );
   setExpandable( true );
}

void InfoItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* after = 0;
      VgElement logqual = elem.getFirstElem("logfilequalifier");
      /* may / may not have log-file-qualifier */
      if ( ! logqual.isNull() ) {
         after = new LogQualItem( this, logqual );
         after->setOpen( true );
      }

      VgElement comment = elem.getFirstElem("usercomment");
      /* may / may not have a user comment */
      if ( ! comment.isNull() ) {
         after = new CommentItem( this, after, comment );
         after->setOpen( true );
      }

      VgElement args = elem.getFirstElem("args");
      /* args */
      after = new ArgsItem( this, after, args );
      after->setOpen( true );
   }
   VgOutputItem::setOpen( open );
}


/**********************************************************************/
/* LogQualItem
 */
LogQualItem::LogQualItem( VgOutputItem* parent, VgElement logqual )
   : VgOutputItem( parent, logqual )
{
   setText( "logfilequalifier" );
}

void LogQualItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* last_item = 0;
      QDomElement e = elem.firstChild().toElement();
      for ( ; !e.isNull(); e = e.nextSibling().toElement() ) {
         VgElement elem = (VgElement&)e;
         last_item = new VgOutputItem( this, last_item, elem );
         last_item->setText( elem.text() );
      }
   }
   VgOutputItem::setOpen( open );
   setExpandable( true );
}


/**********************************************************************/
/* CommentItem
 */
CommentItem::CommentItem( VgOutputItem* parent, QListViewItem* after,
                          VgElement cmnt )
   : VgOutputItem( parent, after, cmnt )
{
   setText( "usercomment" );
}

void CommentItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* item = new VgOutputItem( this, elem );
      item->setText( elem.text() );
   }
   VgOutputItem::setOpen( open );
   setExpandable( true );
}


/**********************************************************************/
/* ArgsItem
 */
ArgsItem::ArgsItem( VgOutputItem* parent, QListViewItem* after,
                    VgElement args )
   : VgOutputItem( parent, after, args )
{
   setText( "args" );
   setExpandable( true );
}

void ArgsItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgElement vgInfo = elem.getFirstElem("vargv");
      VgElement exInfo = elem.getFirstElem("argv");

      VgOutputItem* last_item = 0;
      QDomElement e = vgInfo.firstChild().toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
         VgElement elem = (VgElement&)e;
         last_item = new VgOutputItem( this, last_item, elem );
         last_item->setText( e.text() );
      }
      e = exInfo.firstChild().toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
         VgElement elem = (VgElement&)e;
         last_item = new VgOutputItem( this, last_item, elem );
         last_item->setText( e.text() );
      }
   }
   VgOutputItem::setOpen( open );
}


/**********************************************************************/
/* PreambleItem
   - preamble
   - lines: as text lines
*/
PreambleItem::PreambleItem( VgOutputItem* parent,
                            QListViewItem* after,
                            VgPreamble preamble )
   : VgOutputItem( parent, after, preamble )
{
   setExpandable( true );
   setText( "Preamble" );
}
void PreambleItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* last_item = 0;
      QDomElement e = elem.firstChild().toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
         VgElement elem = (VgElement&)e;
         last_item = new VgOutputItem( this, last_item, elem );
         last_item->setText( e.text() );
      } 
   }
   VgOutputItem::setOpen( open );
}


/**********************************************************************/
/* ErrorItem
   - error
   - stack
   - frames
   - src: as clickable item
   - auxwhat: as text line
   - auxstack
   - (as stack)
*/
ErrorItem::ErrorItem( VgOutputItem* parent, QListViewItem* after,
                      VgError err )
   : VgOutputItem( parent, after, err )
{
   err_tmplt  = "%1 [%2]: %3";

   QString what = err.getFirstElem( "what" ).text();
   QString kind = err.getFirstElem( "kind" ).text();
   QString acnym = errorAcronym( kind );

   if ( err.isLeak() ) {
      setText( acnym + ": " + what );
   } else {
      setText( err_tmplt.arg(acnym).arg(1).arg(what) );
   }

   setExpandable( true );
}

void ErrorItem::updateCount( QString count )
{
   if (!isLeak() && !count.isEmpty()) {
      QString what = elem.getFirstElem( "what" ).text();
      QString kind = elem.getFirstElem( "kind" ).text();
      QString acnym = errorAcronym( kind );
      setText( err_tmplt.arg(acnym).arg(count).arg(what) );
   }
}

void ErrorItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* after = this;
    
      /* main stack */
      VgElement main_stack = elem.getFirstElem( "stack" );
      VgOutputItem* stack_item1 = new StackItem( this, after, main_stack );
      stack_item1->setOpen( true );
      after = stack_item1;

      /* aux what */
      VgElement aux_what = elem.getFirstElem( "auxwhat" );
      if ( ! aux_what.isNull() ) {
         VgOutputItem* aux_item = new VgOutputItem( this, after, aux_what );
         aux_item->setText( aux_what.text() );
         after = aux_item;
      }
      /* aux stack */
      QDomElement aux_stack = aux_what.nextSibling().toElement();
      if ( ! aux_stack.isNull() ) {
         VgElement auxstack = (VgElement&)aux_stack;
         new StackItem( this, after, auxstack );
      }

      /* J sez there may be more than two stacks in the future .. */
      vk_assert( aux_stack.nextSibling().isNull() );
   }
   VgOutputItem::setOpen( open );
}

/* returns an acronym for a given error::kind */
QString ErrorItem::errorAcronym( QString kind )
{
   AcronymMap::Iterator it = acronymMap.find( kind );
   if ( it == acronymMap.end() )
      return "???";
   return it.data(); 
}


/**********************************************************************/
/* StackItem
 */
StackItem::StackItem( VgOutputItem* parent, QListViewItem* after, 
                      VgElement stck )
   : VgOutputItem( parent, after, stck )
{
   setText( "stack" );
   setExpandable( true );
}

void StackItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* after = this;
      QDomElement e = elem.firstChild().toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
         VgFrame frame = (VgFrame&)e;
         after = new FrameItem( this, after, frame );
         after->setOpen( false );
      }
   }
   VgOutputItem::setOpen( open );
}


/**********************************************************************/
/* FrameItem
 */
FrameItem::FrameItem( VgOutputItem* parent, QListViewItem* after,
                      VgFrame frm )
   : VgOutputItem( parent, after, frm )
{
   /* check what perms the user has w.r.t. this file */
   QDomElement srcdir  = frm.getFirstElem( "dir" );
   QDomElement srcfile = frm.getFirstElem( "file" );

   if ( !srcfile.isNull() ) {
      QString path;
      if ( !srcdir.isNull() )
         path = srcdir.text() + "/";
      path += srcfile.text();

      QFileInfo fi( path );
      if ( fi.exists() && fi.isFile() && !fi.isSymLink() ) {
         isReadable  = fi.isReadable();
         isWriteable = fi.isWritable();
      }
   }

   setText( frm.describe_IP() );
   setExpandable( isReadable || isWriteable );
}

void FrameItem::setOpen( bool open )
{
   if ( open && childCount() == 0 && isReadable ) {
      QDomElement srcdir  = elem.getFirstElem( "dir"  );
      QDomElement srcfile = elem.getFirstElem( "file" );
      QDomElement line    = elem.getFirstElem( "line" );

      if ( srcfile.isNull() )
         return;

      QString path;
      if ( !srcdir.isNull() )
         path = srcdir.text() + "/";
      path += srcfile.text();

      if ( !QFile::exists( path ) )
         return;

      VgElement srcline = (VgElement&)line;

      /* create the item for the src lines */
      SrcItem* src_item = new SrcItem( this, srcline, path );
      src_item->setOpen( true );
   }
   VgOutputItem::setOpen( open );
}

#include "vk_config.h"

/**********************************************************************/
/* SrcItem
 */
SrcItem::SrcItem( VgOutputItem* parent, VgElement line, QString path )
   : VgOutputItem( parent, line ) 
{ 
   /* --- setup text --- */
   int target_line = line.text().toInt();
   if (target_line < 0) target_line = 0;

   /* num lines to show above / below the target line */
   int n_lines = vkConfig->rdInt( "src-lines", "valkyrie" );

   /* figure out where to start showing src lines */
   int top_line = 1;
   if ( target_line > n_lines+1 )
      top_line = target_line - n_lines;
   int bot_line = target_line + n_lines;
   int current_line = 1;

   QFile file( path );
   if ( !file.open( IO_ReadOnly ) )
      return;


   /* TODO: faster to set file pos using QFile::at(offset) */

   QString src_lines;
   QTextStream stream( &file );
   while ( !stream.atEnd() && ( current_line <= bot_line ) ) {
      if ( current_line < top_line )
         stream.readLine();      /* skip lines to top_line */
      else
         src_lines += "  " + stream.readLine() + "\n";
      current_line++;
   }
   file.close();
   src_lines.truncate( src_lines.length()-1 ); /* remove last newline */

   /* --- setup item --- */
   isReadable  = parent->isReadable;
   isWriteable = parent->isWriteable;

   pix = 0;
   if ( isWriteable ) {
      setPixmap( write_xpm );
   } else if ( isReadable ) {
      setPixmap( read_xpm );
   }

   setMultiLinesEnabled( true );
   setText( src_lines );
}

SrcItem::~SrcItem() {
   if ( pix ) {
      delete pix;
      pix = 0;
   }
}

void SrcItem::paintCell( QPainter* p, const QColorGroup& cg,
                         int col, int width, int align ) 
{
   QColor bg( 240, 240, 240 );    // very pale gray
   QColorGroup cgrp( cg );        // copy the original
   cgrp.setColor( QColorGroup::Base, bg );
   QListViewItem::paintCell( p, cgrp, col, width, align );
}

void SrcItem::setPixmap( const char* pix_xpm[] ) 
{
   pix = new QPixmap( pix_xpm );
   setup();
   widthChanged( 0 );
   invalidateHeight();
   repaint();
}

const QPixmap* SrcItem::pixmap( int i ) const 
{ return ( i ) ? 0 : pix; }


/**********************************************************************/
/* SuppCountsItem
   - suppcounts
   - pairs: as text line
*/
SuppCountsItem::SuppCountsItem( VgOutputItem* parent,
                                QListViewItem* after,
                                VgSuppCounts sc )
   : VgOutputItem( parent, after, sc )
{
   setExpandable( true );
   setText( "Suppressed errors" );
}

void SuppCountsItem::setOpen( bool open )
{
   if ( open && childCount() == 0 ) {
      VgOutputItem* child_item = 0;
      QDomNodeList pairs = elem.childNodes();
      QDomElement e = pairs.item(0).toElement();
      for (; !e.isNull(); e = e.nextSibling().toElement() ) {
         VgElement pair = (VgElement&)e;
         QString count = pair.firstChild().toElement().text();
         QString name  = pair.lastChild().toElement().text();
         QString supp_str = QString("%1:  " + name).arg( count, 4 );

         child_item = new VgOutputItem( this, child_item, pair );
         child_item->setText( supp_str );
      }
   }
   VgOutputItem::setOpen( open );
}




/**********************************************************************/
/*
  VgLogView: inherits VgLog
*/
VgLogView::VgLogView( QListView* lv )
   : lview(lv), lastChild( 0 )
{ }

VgLogView::~VgLogView()
{ }

/* Reimplements VgLog::appendNode() to populate log and listview */
bool VgLogView::appendNode( QDomNode node )
{
   /* populate log */
   if ( ! VgLog::appendNode( node ) )
      return false;

   QDomElement e  = node.toElement();
   VgElement elem = (VgElement&)e;

   /* populate listview */

   switch ( elem.elemType() ) {
   case VgElement::STATUS: {
      VgStatus status = (VgStatus&)elem;
      VgStatus::StateType state = status.state();
      if (state == VgStatus::RUNNING) {
         VgElement exe =
            args().getFirstElem( "argv" ).getFirstElem( "exe" );
         topStatus = new TopStatusItem( lview, status, exe, protocol().text() );
         topStatus->setOpen( true );
         lastChild = new InfoItem( topStatus, docroot() );
         lastChild = new PreambleItem( topStatus, lastChild, preamble() );
      } else {
         /* update topStatus */
         topStatus->updateStatus( status );
      }
      break;
   }
   case VgElement::ERROR: {
      VgError err = (VgError&)elem;
      lastChild = new ErrorItem( topStatus, lastChild, err );
      if (err.isLeak()) {
         /* update topStatus */
         topStatus->updateLeakCounts( err );
      }
      break;
   }
   case VgElement::SUPPCOUNTS: {
      VgSuppCounts sc = (VgSuppCounts&)elem;
      lastChild = new SuppCountsItem( topStatus, lastChild, sc );
      break;
   }
   case VgElement::ERRORCOUNTS: {
      if (elem.childNodes().count() == 0)  // ignore empty errorcounts
         break;
      VgErrCounts ec = (VgErrCounts&)elem;
      /* update topStatus */
      topStatus->updateErrorCounts( ec );
      /* update all non-leak errors */
      updateErrorItems( ec );
      break;
   }
   default:
      break;
   }

   return true;
}


/* iterate over all errors in the listview, looking for a match on
   error->unique with ecounts->pairList->unique.  if we find a match,
   update the error's num_times value */
void VgLogView::updateErrorItems( VgErrCounts ec )
{
   VgOutputItem* item = (VgOutputItem*)topStatus->firstChild();
   while ( item ) {
      if ( item->elem.elemType() == VgElement::ERROR ) {
         ErrorItem* err_item = (ErrorItem*)item;
         VgError err = *((VgError*)&err_item->elem);
         if ( ! err.isLeak() ) {
            QString count = ec.getCount( err ).text();
            err_item->updateCount( count );
         }
      }
      item = item->nextSibling();
   }
}

