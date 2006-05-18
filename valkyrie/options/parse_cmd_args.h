/* --------------------------------------------------------------------- 
 * Parse command-line options                           parse_cmd_args.h
 * Called from main()
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#ifndef __PARSE_CMD_ARGS_H
#define __PARSE_CMD_ARGS_H

#include "valkyrie_object.h"

/* command-line args parsing ---------------------------------------- */
extern int parseCmdArgs( int argc, char** argv, Valkyrie* vk );

#endif // #ifndef __PARSE_CMD_ARGS_H

