/****************************************************************************
** Helgrind implementation
**  - Helgrind-specific options / flags / fns
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

#include "objects/helgrind_object.h"
#include "options/helgrind_options_page.h"
#include "toolview/helgrindview.h"


/*!
  class Helgrind
*/
Helgrind::Helgrind()
   : ToolObject( "helgrind", VGTOOL::ID_HELGRIND )
{
   setupOptions();
}


Helgrind::~Helgrind()
{
}


/*!
   Setup the options for this object.

   Note: These opts should be kept in exactly the same order as valgrind
   outputs them, as it makes keeping up-to-date a lot easier.
*/
void Helgrind::setupOptions()
{
   // nada
}


/*!
   check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages
*/
int Helgrind::checkOptArg( int /*optid*/, QString& /*argval*/ )
{
   // nothing to check.
   return PARSED_OK;
}


/*!
   Creates this tool's ToolView window
*/
ToolView* Helgrind::createToolView( QWidget* parent )
{
   return (ToolView*) new HelgrindView( parent );
}


/*!
  Creates option page for this tool.
*/
VkOptionsPage* Helgrind::createVkOptionsPage()
{
   return ( VkOptionsPage* )new HelgrindOptionsPage( this );
}


/*!
   outputs a message to the status bar.
*/
void Helgrind::statusMsg( QString msg )
{
   emit message( "Helgrind: " + msg );
}
