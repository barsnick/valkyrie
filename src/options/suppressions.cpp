/****************************************************************************
** Suppressions implementation
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

#include "options/suppressions.h"
#include "options/vk_suppressions_dialog.h"
#include "utils/vk_config.h"
#include "utils/vk_messages.h"
#include "utils/vk_utils.h"

#include <QDateTime>
#include <QFile>



/*!
  class SuppRanges
  */
SuppRanges::SuppRanges()
{
   // init ref lists of allowable values
   
   // Tools
   kindTools.append("Memcheck");
   kindTools.append("DRD");
   kindTools.append("Exp-PtrCheck");
   kindTools.append("Helgrind");
   
   // Suppr types PER TOOL
   for (int i=0; i<kindTools.count(); i++)
      kindTypes.append( QStringList() );
   
   int i=0;
   QStringList* mc_types = &kindTypes[i++];
   mc_types->append("Value1"); // "Uninitialised-value error for value of 1, 2, 4, 8 or 16 bytes."
   mc_types->append("Value2"); 
   mc_types->append("Value4"); 
   mc_types->append("Value8"); 
   mc_types->append("Value16"); 
   mc_types->append("Cond");   // "Uninitialised CPU condition code."
   mc_types->append("Value0"); // "Old name for Cond - uninitialised CPU condition code"
   mc_types->append("Addr1");  // "Invalid address during a memory access of 1, 2, 4, 8 or 16 bytes."
   mc_types->append("Addr2");
   mc_types->append("Addr4");
   mc_types->append("Addr8");
   mc_types->append("Addr16");
   mc_types->append("Jump");   // "jump to an unaddressable location error."
   mc_types->append("Param");  // "invalid system call parameter error."
   mc_types->append("Free");   // "invalid or mismatching free."
   mc_types->append("Overlap");// "src / dst overlap in memcpy or similar function."
   mc_types->append("Leak");   // "memory leak."

   QStringList* hg_types = &kindTypes[i++];
   hg_types->append("Race");

#if 0 // TODO: support drd, ptr-check
   QStringList* drd_types = &kindTypes[i++];
   drd_types->append("CondErr"); 
   drd_types->append("ConflictingAccess"); 

   QStringList* exp_pchk_types = &kindTypes[i++];
   exp_pchk_types->append("Arith"); 
   exp_pchk_types->append("Heap"); 
   exp_pchk_types->append("SorG"); 
#endif
   
   // Frame types
   frameTypes.append("fun");  // "name of the function in which the error occurred"
   frameTypes.append("obj");  // "full path of the .so file or executable containing the error location"
}







/*!
  class Suppression
  */
Suppression::Suppression()
{}

// free format text, e.g. "from test_socket_ssl"
bool Suppression::setName( QString str )
{
   str = str.simplified();
   if ( str.isEmpty() )
      return false;
   
   m_name = str;
   return true;
}

// *** TODO: multiple tools e.g. "Memcheck,Addrcheck:Cond" ***
// e.g. "Memcheck:Cond"
bool Suppression::setKind( QString str )
{
   vk_assert( !m_name.isEmpty() );
   
   str = str.simplified();
   if ( str.isEmpty() )
      return false;
   
   QStringList list = str.split(":");
   if (list.count() != 2) {
      vkPrintErr("Bad Kind (%s) for this suppression (%s).",
                 qPrintable(str), qPrintable(m_name));
      return false;
   }
   
   QRegExp re( list[0], Qt::CaseInsensitive );
   int idx = SuppRanges::instance().getKindTools().indexOf( re );
   if ( idx == -1 ) {
      vkPrintErr("Bad Tool (%s) for this suppression (%s).",
                 qPrintable(list[0]), qPrintable(m_name));
      return false;
   }
   else {
      const QStringList kindTypes = SuppRanges::instance().getKindTypes()[idx];
      if ( !kindTypes.contains( list[1], Qt::CaseInsensitive ) ) {
         vkPrintErr("Bad SupprType (%s) for Tool (%s) for this suppression (%s).",
                    qPrintable(list[1]), qPrintable(list[0]), qPrintable(m_name));
         return false;
      }
   }
   m_kind = str;
   return true;
}

// free format text, e.g. "write(buf)"
bool Suppression::setKindAux( QString str )
{
   str = str.simplified();
   
   m_kind_aux = str;
   return true;
}

// e.g. "obj:/usr/X11R6/lib*/libX11.so.6.2"
// e.g. "fun:*libc_write"
bool Suppression::addFrame( QString str )
{
   str = str.simplified();

   QStringList list = str.split(":");
   if (list.count() != 2) {
      vkPrintErr( "Bad Kind (%s) for suppression '%s'.",
                  qPrintable(str), qPrintable(m_name) );
      return false;
   }

   if ( !SuppRanges::instance().getFrameTypes().contains( list[0], Qt::CaseInsensitive ) ) {
      vkPrintErr( "Unsupported frame type (%s) for suppression '%s'.",
                  qPrintable(list[0]), qPrintable(m_name) );
      return false;
   }
   
   if ( list.at(1).isEmpty() ) {
      vkPrintErr( "Empty frame contents ('%s') for suppression '%s'.",
                  qPrintable(str), qPrintable(m_name) );
      return false;
   }
   
   m_frames.append( str );
   return true;
}


// suppression lines: name[1], kind[1], kindaux[0:1], frame[1:N]
bool Suppression::fromStringList( const QStringList& lines )
{
   int i=0;

   if ( lines.count() < 3 ) {
      vkPrintErr( "Expecting at least 3 lines: name, kind, frame" );
      return false;
   }

   // name, kind
   if ( !setName( lines[i++] ) ) return false;
   if ( !setKind( lines[i++] ) ) return false;

   // kaux (optional)
   QRegExp re( "Memcheck:Param", Qt::CaseInsensitive );
   if ( m_kind.contains( re ) &&
         !(lines.at(i).startsWith("obj:") ||
           lines.at(i).startsWith("fun:") ) ) {
      // found an aux line
      if ( !setKindAux( lines[i++] ) ) return false;
   }

   int nFrames = lines.count() - i;
   if ( nFrames < 1 ) {
      vkPrintErr( "No frames found: must have at least 1 frame." );
      return false;
   }
   if ( nFrames > MAX_SUPP_FRAMES ) {
      vkPrintErr( "Numer of frames (%d) greater than allowed (%d)",
                  nFrames, MAX_SUPP_FRAMES );
      return false;
   }
   
   // frames
   for (;i<lines.count(); i++) {
      if ( !addFrame( lines[i] ) ) return false;
   }

   return true;
}


QString Suppression::toString() const
{
   QString str = "{\n";
   str += "   " + m_name + "\n";
   str += "   " + m_kind + "\n";
   if ( !m_kind_aux.isEmpty() )
      str += "   " + m_kind_aux + "\n";
   for (int i=0; i<m_frames.count(); ++i) {
      str += "   " + m_frames.at(i) + "\n";
   }
   str += "}\n\n";

   return str;
}




/*!
  class SuppList
  */
bool SuppList::readSuppFile( QString& fname )
{
   m_fname = fname;
   
   QFile file( fname );
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      // TODO: error
      return false;
   }
   
   bool suppParseError = false;
   QTextStream in(&file);

   while (!in.atEnd()) {
      QString line = in.readLine().simplified();
      if ( line.contains(QRegExp("^\\{$")) ) {     // start of new supp
         QStringList suppLines;
         Suppression supp;

         while(!in.atEnd()) {
            line = in.readLine().simplified();
            if ( line.startsWith("#") || line.isEmpty() )
               continue;
            if ( line.contains(QRegExp("^\\}$")) ) // end of supp
               break;
            suppLines += line;
         }
         
         if ( supp.fromStringList( suppLines ) ) {
            m_supps.append( supp );
         }
         else {
            suppParseError = true;
            vkPrintErr( "Error reading supps file: %s\n", qPrintable(fname));
            // carry on with rest of input
         }
      }
   }

   if ( m_supps.count() == 0 ) {
      //TODO: tell user (INFO) no supps found in this file.
   }
   
   if ( suppParseError ) {
      
      int res = vkQuery( 0, "Confirm Continue", "&Ok;&Cancel",
                         "<p>Problems were found with a suppression file.<br/>"
                        "If you continue, the suppressions that were unsuccessfully "
                        "parsed will be deleted from the suppressions file.</p>"
                         "<p>Are you sure you want to continue?</p>" );

      if (res == MsgBox::vkYes) {
        // write parsed supps back to file.
        writeSuppFile();
     }
     else {
        return false;
     }
   }
   return true;
}


bool SuppList::writeSuppFile()
{
   if ( !initSuppsFile( m_fname ) ) {
      return false;
   }

   QFile file( m_fname );
   if (!file.open( QIODevice::WriteOnly |
                   QIODevice::Text |
                   QIODevice::Append )) {  // append to init header
      // TODO: error
      return false;
   }
   
   QTextStream out(&file);
   
   for (int i=0; i<m_supps.count(); ++i) {
      out << m_supps.at(i).toString();
   }
  
   file.close();
   return true;
}



const QStringList SuppList::suppNames()
{
   QStringList list;
   foreach(Suppression supp, m_supps) {
      list.append( supp.getName() );
   }
   return list;
}


void SuppList::clear()
{
   m_supps.clear();
   m_fname = QString();
}



// truncate and initialise supp file
bool SuppList::initSuppsFile( const QString& fname )
{
   QFile file( fname );
   if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
   {
      QTextStream stream( &file );
      stream << "# Valgrind suppressions file\n"
             << "# Created by: Valkyrie (" + VkCfg::appVersion() + ")\n"
             << "# Date: " + QDateTime::currentDateTimeUtc().toString() << "\n"
             << "#\n"
             << "# WARNING: other '# comments' will be discarded by Valkyrie.\n"
             << "#\n"
             << "# Format of the objects in this file (in regex style) is:\n"
             << "#\n"
             << "# {\n"
             << "#    TEXT                    -supp name\n"
             << "#    TOOL::KIND              -Valgrind tools, supp kind, eg 'Memcheck:Param'\n"
             << "#    KAUX?                   -(optional) aux for some kinds, eg 'write(buf)'\n"
             << "#    ((obj|fun):<TEXT>\\n)+   -(min one) list of call-chain frames, one per line\n"
             << "# }\n"
             << "#\n"
             << "# Note: Multiple tools per kind e.g. TOOL(,TOOL)*::KIND' is not (yet) supported.\n"
             << "#\n"
             << "# Note: For Memcheck, the the optional aux info is:\n"
             << "#       if (KIND == 'Param'): KAUX = system call param e.g. 'write(buf)'\n" << endl;
      
      file.close();
      return true;
   }
   else {
      return false;
   }
}



/*!
  New suppression
  Returns false if user cancelled edit, else true
*/
bool SuppList::newSupp()
{
   return editSupp( -1 );
}

/*!
  Edit suppression
   idx == -1: new supp (i.e. not yet managed by us)
   supp:       empty for !new supp
               empty|filled for new supp
  Returns false if user cancelled edit, else true
*/
bool SuppList::editSupp( int idx, Suppression supp )
{
   bool isNew = (idx == -1);
   if ( !isNew ) {                            // if editing an existing supp:
      vk_assert( supp.getName().isEmpty() );  //  - it can't be empty
      supp = m_supps.at( idx );               //  - load supp from model at idx
   }

   VkSuppressionsDialog dlg;

   // If supp has content, load it
   if ( !supp.getName().isEmpty() ) {
      dlg.setSupp( supp );
   }

   // run dialogbox
   if ( dlg.exec() == QDialog::Accepted ) {
      supp = dlg.getUpdatedSupp();
      
      // update model and rewrite suppfile
      if ( isNew )
         m_supps.append( supp );
      else {
         m_supps.replace( idx, supp );
      }
      
      if (!writeSuppFile()) {
         //TODO: error
         vkPrintErr("Error: failure during log save");
      }
      return true;
   }
   else {
      // QDialog::Rejected:
      return false;
   }
}

/*!
  Delete suppression
  Returns false if user cancelled delete, else true
*/
bool SuppList::deleteSupp( int idx )
{
   int res = vkQuery( 0, "Confirm Delete", "&Delete;&Cancel",
                      "<p>The suppression will be deleted from the file.</p>"
                      "<p>Are you sure you want to do this ?</p>" );

   if ( res == MsgBox::vkYes ) {            // Delete
      // remove from our list, and write our list over the file.
      m_supps.removeAt( idx );
      if (!writeSuppFile()) {
         //TODO: error
         vkPrintErr("Error: failure during log save");
      }
      return true;
   }
   else {                                   // Cancel
      // MsgBox::vkNo
      return false;
   }
}

