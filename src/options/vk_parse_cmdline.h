/****************************************************************************
** Parse command-line options
**  - Called from main() to parse both valkyrie and valgrind args
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

#ifndef __VK_PARSE_CMDLINE_H
#define __VK_PARSE_CMDLINE_H


/*!
  parseCmdArgs()
  Command-line args parsing, updates global config settings.
*/
extern bool parseCmdArgs( int argc, char** argv, Valkyrie* vk,
                          bool& show_help_and_exit );

#endif // #ifndef __VK_PARSE_CMDLINE_H

