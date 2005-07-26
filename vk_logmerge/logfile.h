/* --------------------------------------------------------------------- 
 * Definition of class LogFile                                 logfile.h
 * Small class to parse an xml logfile into data structures,
 * nuke duplicates, and print the results to stdout or to file.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_LOGFILE_H
#define __VK_LOGFILE_H


#include <qstringlist.h>
#include <qobject.h>

#include "xml_parser.h"


/* definition of class LogFile ----------------------------------------- */
class LogFile : public QObject
{
  Q_OBJECT
public:
  LogFile( QString fname );
  ~LogFile();

  bool save( QString fname );
  bool merge( LogFile* slaveLog );

public slots:
  void loadItem( XmlOutput * );

private:
  bool compareFrames( Frame* mFrame, Frame* sFrame );

private:
  TopStatus* topStatus;
  Preamble*  preamble;
  Info*      info;
  ErrCounts* errCounts;
  SuppCounts* suppCounts;
  QPtrList<Error> errorList;      /* list of errors */
  QPtrList<Error> leakErrorList;  /* list of leak errors */
};


#endif
