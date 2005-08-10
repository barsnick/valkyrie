/* ---------------------------------------------------------------------
 * Implementation of VKLogMerge                          vk_logmerge.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_logmerge.h"
#include "vklm_main.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <stdio.h>
#include <qdom.h>
#include <qvaluelist.h>
#include <qstringlist.h>

typedef QValueList<QDomElement> QDomElemQVList;


/*
  Searches tree from root for all instances of 'elem',
  returns nth (0=first) element in found list
  - if n == -1, returns last element
  - if no element found, returns a 'null' element
*/
QDomElement getElem( QDomElement root, QString elem, int n=0 )
{
  QDomNodeList nodes = root.elementsByTagName( elem );
  if (n == -1)
    n = nodes.count()-1;
  return nodes.item(n).toElement();
}

/*
  Retrieve list of (leakcheck)errors in log
*/
QDomElemQVList getErrors( QDomElement docRoot, bool leakcheck=false )
{
  QDomElemQVList errors;
  QDomNode n = docRoot.firstChild();

  bool found_first = false;
  if (!leakcheck) {   /* main errors */

    /* find first group of "error" elements */
    while( !n.isNull() ) {
      QDomElement e = n.toElement();
      if( !e.isNull() ) {
	if (found_first && e.tagName() != "error")
	  break;
	if (e.tagName() == "error") {
	  found_first = true;
	  errors.append( e );
	}
      }
      n = n.nextSibling();
    }
  } else {             /* leakcheck errors */

    /* find group of "error" elements _after_ suppcounts */
    while( !n.isNull() ) {
      QDomElement e = n.toElement();
      if( !e.isNull() ) {
	if (found_first) {
	  if (e.tagName() != "error")
	    break;
	  errors.append( e );
	}
	if (e.tagName() == "suppcounts")
	  found_first = true;
      }
      n = n.nextSibling();
    }    
  }
  return errors;
}



/*
  Heuristic function to test if frames are the 'same'
*/
bool matchingFrames( QDomElement frame1, QDomElement frame2 )
{
  QDomElement iptr1 = getElem( frame1, "ip"   );
  QDomElement objt1 = getElem( frame1, "obj"  );
  QDomElement func1 = getElem( frame1, "fn"   );
  QDomElement diry1 = getElem( frame1, "dir"  );
  QDomElement file1 = getElem( frame1, "file" );
  QDomElement line1 = getElem( frame1, "line" );
  
  QDomElement iptr2 = getElem( frame2, "ip"   );
  QDomElement objt2 = getElem( frame2, "obj"  );
  QDomElement func2 = getElem( frame2, "fn"   );
  QDomElement diry2 = getElem( frame2, "dir"  );
  QDomElement file2 = getElem( frame2, "file" );
  QDomElement line2 = getElem( frame2, "line" );
  
  /* a frame may be the 'same' as another even if it is
     missing some data - do the best comparison we can */
  
  /* only field guaranteed to be present is 'ip'.
     comparing 'ip' is dodgy, so we try our hardest not to. */
  
  /* test fields: 'dir', 'file', 'line' */
  if ( !diry1.isNull() && !diry2.isNull() &&
       !file1.isNull() && !file2.isNull() &&
       !line1.isNull() && !line2.isNull() ) {
    VKLM_DEBUG("frame test A");
    if (diry1.text() != diry2.text() ||
	file1.text() != file2.text() ||
	line1.text() != line2.text())
      return false;
    return true;
  }
  
  /* test fields: 'file', 'line' */
  if ( !file1.isNull() && !file2.isNull() &&
       !line1.isNull() && !line2.isNull() ) {
    VKLM_DEBUG("frame test B");
    if (file1.text() != file2.text() ||
	line1.text() != line2.text())
      return false;
    return true;
  }
  
  /* test fields: 'fn', 'ip' */
  if ( !func1.isNull() && !func2.isNull() ) {
    VKLM_DEBUG("frame test C");
    if (func1.text() != func2.text() ||
	iptr1.text() != iptr2.text())
      return false;
    return true;
  }
  
  VKLM_DEBUG("frame test D");
  /* test field: 'ip' */
  if (iptr1.text() != iptr2.text())
    return false;
  return true;
}


/*
  Heuristic function to test if errors are the 'same'
*/
bool matchingErrors( QDomElement err1, QDomElement err2 )
{
#define MAX_FRAMES_COMPARE 4

  QDomElement kind1 = getElem( err1, "kind" );
  QDomElement kind2 = getElem( err2, "kind" );

  /* test #1: is the 'kind' the same */
  if (kind1.text() != kind2.text()) {
    VKLM_DEBUG("=> different error kind");
    return false;
  }

  /* - one stack per error */
  QDomElement stack1 = getElem( err1, "stack" );
  QDomElement stack2 = getElem( err2, "stack" );

  /* - one or more frames per stack */
  QDomNodeList framelist1 = stack1.elementsByTagName( "frame" );
  QDomNodeList framelist2 = stack2.elementsByTagName( "frame" );

  /* test #2: error stacks have same num frames,
     if either below our MAX_FRAMES_COMPARE */
  if ( (framelist1.count() < MAX_FRAMES_COMPARE ||
	framelist2.count() < MAX_FRAMES_COMPARE) &&
       framelist1.count() != framelist2.count()) {
    VKLM_DEBUG("=> different frame count");
    return false;
  }

  /* test #3: all stack frames the same,
     to MAX_FRAMES_COMPARE */
  unsigned int i;
  for (i=0; i<framelist1.count() && i<MAX_FRAMES_COMPARE; i++) {
    QDomElement frame1 = framelist1.item(i).toElement();
    QDomElement frame2 = framelist2.item(i).toElement();
    if ( ! matchingFrames( frame1, frame2 ) ) {
      VKLM_DEBUG("=> frames compare failed");
      return false;
    }
  }

  return true;
}



/*
  Get matching 'pair' element, based on elemType.
*/
QDomElement getMatchingPair( QDomElement pairs_root,
			     QDomElement matchElem_root,
			     QString elemType )
{
  QString matchStr1 = getElem( matchElem_root, elemType ).text();
//  VKLM_DEBUG("match1: %s", matchStr1.latin1());

  QDomNodeList pairs = pairs_root.elementsByTagName( "pair" );
  for (unsigned int i=0; i<pairs.count(); i++) {
    QDomElement pair = pairs.item(i).toElement();
    QString matchStr2 = getElem( pair, elemType ).text();
//    VKLM_DEBUG("match2: %s", matchStr2.latin1());
    if ( matchStr1 == matchStr2 )
      return pair;
 }
  return QDomElement();
}


/*
  Convert element value to integer.
  Returns 0 if conversion fails
*/
unsigned long elemToULong( QDomElement elem, bool* ok=0 )
{
  QDomText domText = elem.firstChild().toText();
  if (domText.isNull()) {
    *ok = false;
    return 0;
  }
  QString numStr = domText.data();
  return numStr.toULong(ok);
}


/*
  Update master's pair::count element
  master::pair::count += slave::pair::count
*/
bool updateCount( QDomElement mCount, QDomElement sCount )
{
  /* get slave count */
  bool ok;
  unsigned long sNum = elemToULong( sCount, &ok );
  if (!ok) {
    vklmPrint("error: converting string to int: '%s'",
	      sCount.text().latin1());
    return false;
  }
  
  /* get master count */
  unsigned long mNum = elemToULong( mCount, &ok );
  if (!ok) {
    vklmPrint("error: converting string to int: '%s'",
	      sCount.text().latin1());
    return false;
  }

  /* get the actual text node */
  QDomText mCountDomText = mCount.firstChild().toText();

  /* set the new number */
  mCountDomText.setData( QString::number( mNum + sNum ) );
  return true;
}


/*
  Update master's error::'what' element
*/
bool updateWhat( QDomElement mErr, QDomElement sErr ) 
{
  unsigned long mLeakedBytesNum, mLeakedBlocksNum;
  mLeakedBytesNum  = elemToULong( getElem( mErr, "leakedbytes"  ) );
  mLeakedBlocksNum = elemToULong( getElem( mErr, "leakedblocks" ) );
  
  /* do our best to update the master's 'what' string 
     - J and I will 'have words' if he changes this ... */
  QDomElement mWhat = getElem( mErr, "what" );
  QString mWhatStr = mWhat.text();
  QStringList ms_what = QStringList::split( " ", mWhatStr );
  
  /* the first elem in the 'what' string is the no. of bytes */
  ms_what[0] = QString::number( mLeakedBytesNum );

  /* if the 2nd elem is 'bytes', then the 4th elem is blocks */
  if ( ms_what[1] == "bytes" ) {
    ms_what[3] = QString::number( mLeakedBlocksNum );

  } else {
    /* else life is a bit more difficult - hope for the best 
       [1] = direct, [3] = indirect, [7] = blocks */
    QString sWhatStr = getElem( sErr, "what" ).text();
    QStringList sl_what( QStringList::split( " ", sWhatStr ) );
    bool ok;
    
    /* 'direct': remove the leading '(' */
    unsigned long sl_num, ms_num;
    sl_num = sl_what[1].remove( 0, 1 ).toULong(&ok);
    if (ok)
      ms_num = ms_what[1].remove( 0, 1 ).toULong(&ok);
    if (!ok) {
      vklmPrint("error: failed to parse 'what' string");
      return false;
    }
    ms_what[1] = "(" + QString::number( ms_num + sl_num );
    
    /* indirect */
    sl_num = sl_what[3].toULong(&ok);
    if (ok)
      ms_num = ms_what[3].toULong(&ok);
    if (!ok) {
      vklmPrint("error: failed to parse 'what' string");
      return false;
    }
    
    ms_what[3] = QString::number( ms_num + sl_num );
    /* blocks */
    ms_what[7] = QString::number( mLeakedBlocksNum );
  }
  
  /* finally! update master's what string */
  QDomText mWhatDomText = mWhat.firstChild().toText();
  mWhatDomText.setData( ms_what.join( " " ) );
  return true;
}



/*
  Merge errors
*/
bool mergeErrors( QDomDocument& master_doc, QDomDocument& slave_doc )
{
  QDomElement sDocRoot = slave_doc.documentElement();
  QDomElement mDocRoot = master_doc.documentElement();

  QDomElemQVList sErrors = getErrors( sDocRoot );
  QDomElemQVList mErrors = getErrors( mDocRoot );
  QDomElement sErrCounts = getElem( sDocRoot, "errorcounts", -1 );
  QDomElement mErrCounts = getElem( mDocRoot, "errorcounts", -1 );
  
  VKLM_DEBUG( "--- update matches (n=%d) --- ", sErrors.count());

  /* --- find matches: update master err, delete slave err ---  */
  
  /* --- for each error in master ---  */
  QDomElemQVList::Iterator mIter;
  for ( mIter = mErrors.begin(); mIter != mErrors.end(); ++mIter ) {
    QDomElement mErr = *mIter;
    
    VKLM_DEBUG("master err: '%s'",
	       getElem( mErr, "unique" ).text().latin1());
    VKLM_DEBUG("");
    
    /* get master errorcount::pair for this error::unique */
    QDomElement mPair = getMatchingPair( mErrCounts, mErr, "unique" );
    if (mPair.isNull()) {
      VKLM_DEBUG("error: no matching master errorcount\n");
//      return false;
      continue;
    }
    
    /* --- for each error in slave --- */
    QDomElemQVList::Iterator sIter;
    for ( sIter = sErrors.begin(); sIter != sErrors.end(); ++sIter ) {
      QDomElement sErr = *sIter;
      
      VKLM_DEBUG("slave err: '%s'",
		 getElem( sErr, "unique" ).text().latin1());
      
      /* get slave errorcount::pair for this error::unique */
      QDomElement sPair = 
	getMatchingPair( sErrCounts, sErr, "unique" );
      
      if (sPair.isNull()) {
	VKLM_DEBUG("error: no matching slave errorcount");
//	return false;
	continue;
      }
      
      if ( matchingErrors(mErr, sErr) ) {
	VKLM_DEBUG("=> matched");
	/* --- master count += slave count --- */
	if ( ! updateCount( getElem( mPair, "count" ),
			    getElem( sPair, "count" ) ) ) {
	  VKLM_DEBUG("error: failed master errorcount update");
//	  return false;
	  continue;
	}
	
	/* --- remove error from slave list & xml --- */
	sIter = sErrors.remove( sIter );
	sDocRoot.removeChild( sErr );
	sErrCounts.removeChild( sPair );
	
	/* sIter now points to next error
	   - go back one, so for loop goes to next */
	sIter--;
      }
      VKLM_DEBUG("");
    }
    VKLM_DEBUG("\n");
  }

  VKLM_DEBUG( "--- append non-matches (n=%d) --- ", sErrors.count());

  /* --- if master has no errorcounts, but slave has remaining errors
     => create new errorcounts element --- */
  if ( mErrCounts.isNull() && sErrors.count() > 0 ) {
    VKLM_DEBUG("creating new master errcounts");
    
    /* create <errorcounts></errorcounts> */
    mErrCounts = master_doc.createElement( "errorcounts" );
    
    /* and insert before status_end */
    QDomElement mStatus = getElem( mDocRoot, "status", 1 );
    mDocRoot.insertBefore( mErrCounts, mStatus );
    
    /* set mErrCounts, so errors & counts know where to be appended */
    QDomElement mErrCounts = getElem( mDocRoot, "errorcounts", -1 );
  }
  
  /* --- append remaining slave errors to master --- */
  QDomElemQVList::Iterator sIter;
  for ( sIter = sErrors.begin(); sIter != sErrors.end(); ++sIter ) {
    QDomElement sErr = *sIter;
    VKLM_DEBUG("appending slave err: '%s'",
	       getElem( sErr, "unique" ).text().latin1());
    
    /* get slave errorcount::pair for this error::unique */
    QDomElement sPair = getMatchingPair( sErrCounts, sErr, "unique" );
    if (sPair.isNull()) {
      VKLM_DEBUG("error: no matching slave errorcount");
//	return false;
      continue;
    }
    
    /* --- append slave error to master --- */
    mDocRoot.insertBefore( sErr, mErrCounts );
    
    /* --- append slave errorcount to master --- */
    mErrCounts.appendChild( sPair );
  }
  VKLM_DEBUG("\n\n");

  return true;
}



/*
  Merge suppcounts
*/
bool mergeSuppCounts( QDomElement& mDocRoot, QDomElement& sDocRoot )
{
  QDomElement mSuppCounts = getElem( mDocRoot, "suppcounts" );
  QDomElement sSuppCounts = getElem( sDocRoot, "suppcounts" );

  QDomNodeList mPairs = mSuppCounts.elementsByTagName( "pair" );
  QDomNodeList sPairs = sSuppCounts.elementsByTagName( "pair" );

  VKLM_DEBUG("--- update matches (n=%d) ---", sPairs.count());

  /* --- for each suppcount::pair in master --- */
  for (unsigned int mIdx=0; mIdx<mPairs.count(); mIdx++) {
    QDomElement mPair = mPairs.item(mIdx).toElement();
    QString     mStr  = getElem( mPair, "name" ).text();

    VKLM_DEBUG("master suppcount pair: '%s'", mStr.latin1());

    /* --- for each suppcount::pair in slave --- */
    for (unsigned int sIdx=0; sIdx<sPairs.count(); sIdx++) {
      QDomElement sPair = sPairs.item(sIdx).toElement();
      QString     sStr  = getElem( sPair, "name" ).text();
      VKLM_DEBUG("slave  suppcount pair: '%s'", sStr.latin1());

      if ( mStr == sStr ) { /* matching pair */
	VKLM_DEBUG("=> matched");
	/* --- master pair::count += slave pair::count --- */
	if ( ! updateCount( getElem( mPair, "count" ),
			    getElem( sPair, "count" ) ) ) {
	  VKLM_DEBUG("error: failed master suppcount update");
	  //	return false;
	  continue;
	}

	/* --- remove error from xml (auto removes from list) --- */
	sSuppCounts.removeChild( sPair );

	/* there can't be more than one match, so go to next mPair */
	break;
      }
      VKLM_DEBUG("");
    }
    VKLM_DEBUG("\n");
  }

  VKLM_DEBUG("--- append non-matches (n=%d) ---", sPairs.count());

  /* Note: guaranteed to have a suppcounts element, unlike errcounts */

  /* --- append remaining slave suppcount::pairs to master --- */
  for (unsigned int sIdx=0; sIdx<sPairs.count(); sIdx++) {
    QDomElement sPair = sPairs.item(sIdx).toElement();
    VKLM_DEBUG("appending slave suppcount pair: '%s'",
	       getElem( sPair, "name" ).text().latin1());
    mSuppCounts.appendChild( sPair );
  }
  VKLM_DEBUG("\n\n");

  return true;
}




/*
  Merge leak errors
*/
bool mergeLeakErrors( QDomElement& mDocRoot, QDomElement& sDocRoot )
{
  QDomElemQVList sLeakErrors = getErrors( sDocRoot, true/*leak*/ );
  QDomElemQVList mLeakErrors = getErrors( mDocRoot, true/*leak*/ );

  VKLM_DEBUG("--- update matches (n=%d) ---", sLeakErrors.count());

  /* --- for each leak_error in master ---  */
  QDomElemQVList::Iterator mIter;
  for ( mIter = mLeakErrors.begin();
	mIter != mLeakErrors.end(); ++mIter ) {
    QDomElement mErr = *mIter;

    VKLM_DEBUG("master leak_err: '%s'",
	       getElem( mErr, "unique" ).text().latin1());

    /* --- for each leak_error in slave ---  */
    QDomElemQVList::Iterator sIter;
    for ( sIter = sLeakErrors.begin();
	  sIter != sLeakErrors.end(); ++sIter ) {
      QDomElement sErr = *sIter;

      VKLM_DEBUG("slave leak_err: '%s'",
		 getElem( sErr, "unique" ).text().latin1());

      if ( matchingErrors(mErr, sErr) ) {
	VKLM_DEBUG("=> matched");

	/* --- update master leakedBytes, leakedBlocks, what --- */

	if ( ! updateCount( getElem( mErr, "leakedbytes"  ),
			    getElem( sErr, "leakedbytes"  ) ) ) {
	  VKLM_DEBUG("error: failed master leakedbytes update");
//	    return false;
	  continue;
	}
	if ( ! updateCount( getElem( mErr, "leakedblocks" ),
		     getElem( sErr, "leakedblocks" ) ) ) {
	  VKLM_DEBUG("error: failed master leakedblocks update");
//	    return false;
	  continue;
	}
	if ( ! updateWhat( mErr, sErr ) ) {
	  VKLM_DEBUG("error: failed to update master 'what'");
//	    return false;
	  continue;
	}

	/* --- remove error from slave list & xml --- */
	sIter = sLeakErrors.remove( sIter );
	sDocRoot.removeChild( sErr );

	/* sIter now points to next error
	   - go back one, so for loop goes to next */
	sIter--;
      }
      VKLM_DEBUG("");
    }
    VKLM_DEBUG("\n");
  }

  VKLM_DEBUG("--- append non-matches (n=%d) ---", sLeakErrors.count());

  /* --- append remaining slave leak_errors to master --- */
  QDomElemQVList::Iterator sIter;
  for ( sIter = sLeakErrors.begin();
	sIter != sLeakErrors.end(); ++sIter ) {
    VKLM_DEBUG("appending slave leak_err: '%s'",
	       getElem( *sIter, "unique" ).text().latin1());
    
    mDocRoot.appendChild( *sIter );
  }
  return true;
}


/*
  Merge Valgrind Logs: master_doc <= slave_doc
*/
bool mergeVgLogs( QDomDocument& master_doc, QDomDocument& slave_doc )
{
  QDomElement sDocRoot = slave_doc.documentElement();
  QDomElement mDocRoot = master_doc.documentElement();

  /* check the same tool was used */
  QDomElement sTool = getElem( sDocRoot, "tool" );
  QDomElement mTool = getElem( mDocRoot, "tool" );
  if (sTool.text() != mTool.text()) {
    vklmPrint("error: different tool used for this log");
    return false;
  }

  /* check the same binary was used */
  QDomElement sExe = getElem( getElem( sDocRoot, "argv" ), "exe" );
  QDomElement mExe = getElem( getElem( mDocRoot, "argv" ), "exe" );
  if (sExe.text() != mExe.text()) {
    vklmPrint("error: different executable used for this log");
    return false;
  }

  /* merge errors */
  VKLM_DEBUG("=== MERGE ERRORS ===\n");
  if (getErrors( sDocRoot ).count() != 0) {
    mergeErrors( master_doc, slave_doc );
  } else {
    VKLM_DEBUG("no errors to merge");
  }
 
  /* merge suppcounts */
  VKLM_DEBUG("=== MERGE SUPPCOUNTS ===\n");
  QDomElement sSuppCounts = getElem( sDocRoot, "suppcounts" );
  if ( sSuppCounts.elementsByTagName( "pair" ).count() != 0) {
    mergeSuppCounts( mDocRoot, sDocRoot );
  } else {
    VKLM_DEBUG("no suppcounts to merge");
  }

  /* merge leak_errors */
  VKLM_DEBUG("=== MERGE LEAKCOUNTS ===\n");
  if (getErrors( sDocRoot, true/*leak*/ ).count() != 0) {
    mergeLeakErrors( mDocRoot, sDocRoot );
  } else {
    VKLM_DEBUG("no leak errors to merge");
  }
  return true;
}
