/* --------------------------------------------------------------------- 
 * Implementation of class LogFile                           logfile.cpp
 * Small class to parse an xml logfile into data structures,
 * nuke duplicates, and print the results to stdout or to file.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, Donna Robinson <donna@valgrind.org>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */


#include "logfile.h"
#include "vk_utils.h"
#include "vk_msgbox.h"

#include <qfileinfo.h>


/* class Logfile ------------------------------------------------------- */
LogFile::~LogFile() 
{ 
  if ( topStatus )  { delete topStatus;  topStatus  = 0; }
  if ( preamble )   { delete preamble;   preamble   = 0; }
  if ( info )       { delete info;       info       = 0; }
  if ( errCounts )  { delete errCounts;  errCounts  = 0; }
  if ( suppCounts ) { delete suppCounts; suppCounts = 0; }

  errorList.setAutoDelete( true );
  errorList.clear(); 

  leakErrorList.setAutoDelete( true );
  leakErrorList.clear();
}


LogFile::LogFile( QString fname ) : QObject( 0, fname )
{ 
  topStatus  = 0;
  preamble   = 0;
  info       = 0;
  errCounts  = 0;
  suppCounts = 0;

  leakErrorList.setAutoDelete( true );
}


void LogFile::loadItem(XmlOutput* output ) 
{ 
  switch ( output->itemType ) {
    case XmlOutput::STATUS:
      topStatus = (TopStatus*)output;
      break;
    case XmlOutput::PREAMBLE: 
      preamble  = (Preamble*)output; 
      break;
    case XmlOutput::INFO:
      info = (Info*)output;
      break;
    case XmlOutput::ERROR:
      errorList.append( (Error*)output ); 
      break;
    case XmlOutput::LEAK_ERROR:
      leakErrorList.append( (Error*)output ); 
      break;
    case XmlOutput::ERR_COUNTS:
      errCounts = (ErrCounts*)output;
      break;
    case XmlOutput::SUPP_COUNTS:
      suppCounts = (SuppCounts*)output;
      break;
    case XmlOutput::STACK:
    case XmlOutput::FRAME:
    case XmlOutput::SRC:
    case XmlOutput::AUX:
      vk_assert_never_reached();
      break;
  }

}



bool LogFile::save( QString fname )
{
  QFile outFile( fname );
  if ( !outFile.open( IO_WriteOnly ) ) {
    vkError( 0, "I/O Error",
             "<p>Failed to open file '%s' for writing</p>",
             fname.latin1() );
    return false;
  }

  QTextStream stream( &outFile );
 
  /* top stuff */
  stream << "<?xml version=\"1.0\"?>\n\n";

  stream << "<valgrindoutput>\n\n";
  stream << "<protocolversion>" 
         << info->protocolVersion 
         << "</protocolversion>\n\n";

  /* preamble */
  stream << "<preamble>\n";
  for ( uint i=0; i<preamble->lines.count(); i++ )
    stream << "  <line>" << preamble->lines[i] << "</line>\n";
  stream << "</preamble>\n\n";

  /* pid, ppid, tool */
  stream << "<pid>"  << info->pid  << "</pid>\n";
  stream << "<ppid>" << info->ppid << "</ppid>\n";
  stream << "<tool>" << info->tool << "</tool>\n\n";

  /* argv stuff */
  stream << "<argv>\n";
  stream << "  <exe>" << info->infoList[0] << "</exe>\n";
  for ( uint i=1; i<info->infoList.count(); i++ )
    stream << "  <arg>" << info->infoList[i] << "</arg>\n";
  stream << "</argv>\n\n";

  /* starting status */
  stream << "<status>" << info->startStatus << "</status>\n\n";

  /* errors */
  Error* error;
  for ( error = errorList.first(); error; error = errorList.next() ) {
    error->print2File( stream );
  }

  /* error counts */
  if ( errCounts != 0 )
    errCounts->print2File( stream );

  /* ending status */
  stream << "<status>" << info->endStatus << "</status>\n\n";

  /* suppcounts */
  if ( suppCounts != 0 )
    suppCounts->print2File( stream );

  /* leak counts */
  for ( error = leakErrorList.first(); error; error = leakErrorList.next() )
    error->print2File( stream );

  stream << "</valgrindoutput>\n\n";
  outFile.close();

  return true;
}


bool LogFile::compareFrames( Frame* mFrame, Frame* sFrame )
{
  bool same;

  /* test fields: 'dir', 'file', 'line' */
  if ( sFrame->haveDir  && mFrame->haveDir  &&
       sFrame->haveFile && mFrame->haveFile &&
       sFrame->haveLine && mFrame->haveLine ) {
    same = ( sFrame->srcdir  == mFrame->srcdir  &&
             sFrame->srcfile == mFrame->srcfile && 
             sFrame->lineno  == mFrame->lineno );
    if ( same != true ) return same;
  }

  /* test fields: 'file', 'line' */
  if ( sFrame->haveFile && mFrame->haveFile &&
       sFrame->haveLine && mFrame->haveLine ) {
    same = ( sFrame->srcfile == mFrame->srcfile &&
             sFrame->lineno  == mFrame->lineno );
    if ( same != true ) return same;
  }

  /* test fields: 'fn', 'ip' */
  if ( sFrame->haveFunc && mFrame->haveFunc ) {
    same = ( sFrame->fn == mFrame->fn && sFrame->ip == mFrame->ip );
    if ( same != true ) return same;
  }
  
  /* test field: 'ip' */
  same = ( sFrame->ip == mFrame->ip );
  if ( same != true ) return same;

  return same;
}


/* the objective is to discard duplicate errors found in the slave's
   error list.  
   - we first iterate over the two lists of errors, comparing the
     master and slave error 'kind'; if a match is found, we have a
     possible duplicate candidate.
   - then the two sets of stack frames are compared; if this returns
     true, we consider this error to be a duplicate, and it is
     discarded from the slave's error list.
   - finally, we append whatever is left in the slave error list onto
     the master error list. */
void LogFile::merge( LogFile* slaveLog )
{
  /* list of slaveLog errors for deletion*/
  QPtrList<Error> deleteList;

  /* check the same tool was used */
  if ( info->tool != slaveLog->info->tool )
    printf("Error: different tool used to generate output\n");

  /* check the same binary was used */
  if ( info->exe != slaveLog->info->exe )
    printf("Error: different executable used to generate output\n");

  /* check we do actually have some errors to merge */
  if ( errorList.isEmpty() || slaveLog->errorList.isEmpty() ) 
    printf("Error: no errors in master/slave to merge\n");

  /* start the tests */
  bool same;
  for ( Error* slaveError = slaveLog->errorList.first(); 
        slaveError; slaveError = slaveLog->errorList.next() ) {

    for ( Error* masterError = errorList.first(); 
          masterError; masterError = errorList.next() ) {
      same = false;

      /* test #1: is the 'kind' the same */
      if ( masterError->kind == slaveError->kind ) {

        /* test #2: compare each stack frame of the two errors.
           possible fields: 'ip', 'obj', 'fn', 'dir', 'file', 'line',
           but the only field guaranteed to be present is 'ip'.
           comparing 'ip' is dodgy, so we try our hardest not to. */
        Stack* masterStack = masterError->stackList.first();
        Stack* slaveStack  = slaveError->stackList.first();
        while ( masterStack != 0 && slaveStack != 0 ) {

          Frame* masterFrame = masterStack->frameList.first();
          Frame* slaveFrame  = slaveStack->frameList.first();
          while ( slaveFrame != 0 && masterFrame != 0 ) {

            same = compareFrames( masterFrame, slaveFrame );
            if ( same == false ) break;

            masterFrame = masterStack->frameList.next();
            slaveFrame  = slaveStack->frameList.next();
          }
          if ( same == false ) break; 

          masterStack = masterError->stackList.next();
          slaveStack  = slaveError->stackList.next();
        }  /* end of stack frame comparisons */

      }    /* end 'kind == 'kind' */

      /* if we have found a twin, keep a ptr to it.  we don't need to
         compare this slaveError with any more masterErrors, so skip
         to the next slaveError */
      if ( same == true ) {
        deleteList.append( slaveError );
        break;
      }

    }    /* end of for masterLog->errorList() */
  }      /* end of for slaveLog->errorList() */

  /* remove all the found duplicates from slaveLog->errorList, at the
     same time incrementing masterLog->errorCounts w.r.t. the slaveLog
     error-to-be-deleted.  
     also remove the relevant slaveLog->errorCounts pair from the
     slaveLog->errorCounts list. */
  for ( Error* del_error = deleteList.first(); 
        del_error; del_error = deleteList.next() ) {
    int index = slaveLog->errorList.findRef( del_error );
    if ( index == -1 ) 
      VK_DEBUG("Error: couldn't find error in slaveLog list");
    Error* sl_error = slaveLog->errorList.take( index );
    if ( sl_error == 0 ) {
      VK_DEBUG("Error: couldn't take slaveLog_error out of slaveLog list");
    } else {
      /* increment the masterLog->errorCounts w.r.t. this slave_error */
      int count = slaveLog->errCounts->findUnique( sl_error->unique );
      /* update the masterLog's errorcount record */
      errCounts->updateCount( count, sl_error->unique );
      /* and remove the slaveLog->errorCounts pair from its list */
      slaveLog->errCounts->remove( count, sl_error->unique );
    }
  }

  /* having 'taken' (ie. removed but not deleted) all the duplicates
     from the slaveLog error list, delete them now for real */
  deleteList.setAutoDelete( true );
  deleteList.clear();
  deleteList.setAutoDelete( false );

  /* append whatever is left in slaveLog->errorList onto
     masterLog->errorList */
  for ( Error* sl_error = slaveLog->errorList.first(); 
        sl_error; sl_error = slaveLog->errorList.next() ) {
    errorList.append( sl_error );
  }

  /* append whatever is left in slaveLog->errCounts onto
     masterLog->errCounts */
  errCounts->appendList( slaveLog->errCounts );

  /* update the masterLog's suppCounts: if we find a matching 'pair',
     merely increment the masterLog's 'count' value; otherwise, append
     the not-found 'pair' onto the masterLog's suppcounts list */
  suppCounts->updateList( slaveLog->suppCounts );

  /* now remove all errors from the slaveLog errorList so they don't
     get deleted twice when the slaveLog is deleted */
  slaveLog->errorList.clear();

  /* last, but not least, update masterLog->leakErrorList
     w.r.t. slaveLog->leakErrorList */
  QPtrList<Error> sl_leakList;
  QPtrList<Error> ms_leakList;
  for ( Error* slaveError = slaveLog->leakErrorList.first(); 
        slaveError; slaveError = slaveLog->leakErrorList.next() ) {

    for ( Error* masterError = leakErrorList.first(); 
          masterError; masterError = leakErrorList.next() ) {
      same = false;

      /* test #1: is the 'kind' the same */
      if ( masterError->kind == slaveError->kind ) {

        /* test #2: compare each stack frame of the two errors.
           Possible fields: 'ip', 'obj', 'fn', 'dir', 'file', 'line',
           but the only field guaranteed to be present is 'ip'.
           Comparing 'ip' is dodgy, so we try our hardest not to. */
        Stack* masterStack = masterError->stackList.first();
        Stack* slaveStack  = slaveError->stackList.first();
        while ( masterStack != 0 && slaveStack != 0 ) {

          Frame* masterFrame = masterStack->frameList.first();
          Frame* slaveFrame  = slaveStack->frameList.first();
          while ( masterFrame != 0 && slaveFrame != 0 ) {

            same = compareFrames( masterFrame, slaveFrame );
            if ( same == false ) break;

            masterFrame = masterStack->frameList.next();
            slaveFrame  = slaveStack->frameList.next();
          }
          if ( same == false ) break; 

          masterStack = masterError->stackList.next();
          slaveStack  = slaveError->stackList.next();
        }   /* end of stack frame comparisons */
      }     /* end 'kind == 'kind' */

      /* if we have found twins, keep ptrs to them.  we don't need to
         compare this slaveError with any more masterErrors, so skip
         to the next slaveError */
      if ( same == true ) {
        sl_leakList.append( slaveError );
        ms_leakList.append( masterError );
        break;
      }

    }  /* end of for masterLog->leakErrorList() */
  }    /* end of for slaveLog->leakErrorList() */


  /* remove all the found duplicates from slaveLog->leakErrorList, at
     the same time updating 'what', and incrementing 'leakedBytes' and
     'leakedBlocks' in masterLog->leakErrorList w.r.t. the slaveLog
     leakError-to-be-deleted (good grief, charlie brown ...) */
  for ( unsigned int i=0; i<sl_leakList.count(); i++ ) {

    Error* rem_error = sl_leakList.at(i);
    /* remove this error from the slave's leakError list */
    int index = slaveLog->leakErrorList.findRef( rem_error );
    Error* sl_error = slaveLog->leakErrorList.take( index );

    /* since we inserted both the ptrs to leak errors at the same
       time, we can just index into the master's list to find the
       slave's twin */
    Error* ms_error = ms_leakList.at(i);

    /* update the master's bytes and blocks counts */
    ms_error->leakedBytes  += sl_error->leakedBytes;
    ms_error->leakedBlocks += sl_error->leakedBlocks;

    /* do our best to update the master's 'what' string 
       - J and I will 'have words' if he changes this ... */
    QStringList ms_what( QStringList::split( " ", ms_error->what ) );
    /* the first elem in the 'what' string is the no. of bytes */
    ms_what[0] = QString::number( ms_error->leakedBytes );
    /* if the 2nd elem is 'bytes', then the 4th elem is blocks */
    if ( ms_what[1] == "bytes" ) 
      ms_what[3] = QString::number( ms_error->leakedBlocks );
    else {
      /* else life is a bit more difficult - hope for the best 
         [1] = direct, [3] = indirect, [7] = blocks */
      QStringList sl_what( QStringList::split( " ", sl_error->what ) );
      /* 'direct': remove the leading '(' */
      sl_what[1] = sl_what[1].remove( 0, 1 );
      int sl_num = sl_what[1].toInt();
      ms_what[1] = ms_what[1].remove( 0, 1 );
      int ms_num = ms_what[1].toInt();
      ms_what[1] = "(" + QString::number( ms_num + sl_num );
      /* indirect */
      sl_num = sl_what[3].toInt();
      ms_num = ms_what[3].toInt();
      ms_what[3] = QString::number( ms_num + sl_num );
      /* blocks */
      ms_what[7] = QString::number( ms_error->leakedBlocks );
    }
    ms_error->what = ms_what.join( " " );
  }

  /* clear the two lists - we are done.  don't bother deleting the
     contents of the slave's leakErrorLists as this will be done when
     the slaveLog itself is deleted. */
  sl_leakList.clear();
  ms_leakList.clear();
}

