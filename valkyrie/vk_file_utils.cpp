/* ---------------------------------------------------------------------
 * Custom file dialog                                  vk_file_utils.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "vk_file_utils.h"

#include <qfile.h>


/* ------------------------------------------------------------------ */
bool FileCopy(const QString& in, const QString& out)
{
   const int bufSize = 16384; // 16Kb buffer
   char *buf = new char[bufSize];
   
   QFile fin(in);
   if( !fin.open(IO_ReadOnly) )
      return false;
   QFile fout(out);
   if( !fout.open(IO_WriteOnly) )
      return false;

   int len = fin.readBlock(buf, bufSize);
   while (len > 0) {
      if (fout.writeBlock(buf, len) == -1)
         return false;
      len = fin.readBlock(buf, len);
   }
   if (len == -1)
      return false;

   fin.close();
   fout.close();
   delete[] buf;

   return true;
}
