/* --------------------------------------------------------------------- 
 * Definition of class VGLog                                     vglog.h
 * Small class to parse a valgrind xml logfile into data structures,
 * nuke duplicates, and print the results to stdout or to file.
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __VK_VGLOG_H
#define __VK_VGLOG_H


#include <qstringlist.h>
#include <qobject.h>

#include "xml_parser.h"


/* definition of class VGLog ----------------------------------------- */
class VGLog : public QObject
{
  Q_OBJECT
public:
  VGLog( QString fname );
  ~VGLog();

  bool save( QString fname );
  bool merge( VGLog* slaveLog );

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
