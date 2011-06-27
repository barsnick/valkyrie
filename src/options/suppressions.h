/****************************************************************************
** Suppressions definition
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

#ifndef SUPPRESSIONS_H
#define SUPPRESSIONS_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>

// TODO: what's reasonable? what does Vg allow?
#define MAX_SUPP_FRAMES 20



// ============================================================
// Singleton class holding some handy reference data
class SuppRanges
{
public:
   static SuppRanges *instance()
   {
      if (!s_instance)
         s_instance = new SuppRanges;
      return s_instance;
   }
   
   QStringList kindTools;        // Memcheck|...
   QList<QStringList> kindTypes; // list for every tool
   QStringList frameTypes;       // obj|fun

private:
   SuppRanges();
   static SuppRanges *s_instance;
};




// ============================================================
class Suppression
{
public:
   Suppression();

   bool fromStringList( const QStringList& lines );
   QString toString() const;

   // Setters
   bool setName( QString str );
   bool setKind( QString str );
   bool setKindAux( QString str );
   bool addFrame( QString str );

   // Getters
   QString getName() const { return m_name; }
   QString getKind() const { return m_kind; }
   QString getKAux() const { return m_kind_aux; }
   QStringList getFrames() const { return m_frames; }
   
private:
   QString m_name;           // name of the suppression
   QString m_kind;           // kind, eg "Memcheck:Param"
   QString m_kind_aux;       // (optional) aux kind, eg  "write(buf)"
   QStringList m_frames;     // (one or more) frames
};


// ============================================================
class SuppList
{
public:
   SuppList() {};
   
   bool readSuppFile( QString& filename );
   bool writeSuppFile();
   const QStringList suppNames();
   void clear();
   bool initSuppsFile( const QString& fname );
   bool newSupp();
   bool editSupp( int idx, Suppression supp = Suppression() );
   bool deleteSupp( int idx );

private:   
   QList<Suppression> m_supps;
   QString m_fname;
};


#endif // SUPPRESSIONS_H
