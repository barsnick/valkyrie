/* ---------------------------------------------------------------------
 * main(): vk_logmerge program entry point                 vklm_main.cpp
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2005, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include <qfileinfo.h>

#include "vk_logmerge.h"


QString program_name;

void usage()
{
  QString fname = QFileInfo(program_name).fileName();
  fprintf(stderr, "%s, a valgrind log file merger.  Version 0.9.0, 25-July-2005.\n\n", fname.latin1());
  fprintf(stderr, "usage: %s [-h] -f log_list [-o outfile]\n", fname.latin1());
  fprintf(stderr, "\
  -h\t\tprint this message\n\
  -f log_list\tfile of list of log files to be merged (one per line)\n\
  -o outfile\tmerged output log file\n");
  fprintf(stderr, "\nIf no -o option given, outputs to stdout\n\n");
}

bool valid_arg( QString& arg )
{
  if (!(arg.startsWith("-") && arg.length() > 2)) {
    usage();
    return false;
  }
  return true;
}


int main ( int argc, char* argv[] )
{
  VKLogMerge logmerge;
  QString log_list, fname_out;

  program_name = argv[0];

  /* parse command-line args ---------------------------------- */
  for (int opt_idx=1; opt_idx < argc; opt_idx++) {
    QString opt = argv[opt_idx];
    if (!(opt.startsWith("-") && opt.length() == 2)) {
      usage();
      return 1;
    }
    char opt_c = opt[1].latin1();

    bool short_opt = false;
    /* no-arg options */
    switch (opt_c) {
    case 'h':
      usage();
      short_opt = true;
      return 0;
    default:
      break; /* fall thru... */
    }
    if (short_opt) continue;

    /* all our other options have arguments: */
    opt_idx++;
    if (opt_idx >= argc) {
      usage();
      return 1;
    }
    QString arg = argv[opt_idx];

    switch (opt_c) {
    case 'f':
      log_list = arg;
      break;
    case 'o':
      fname_out = arg;
      break;
    default:
      usage();
      return 1;
    }  
  }

  /* validate args -------------------------------------------- */
  if (log_list.isEmpty()) {
    usage();
    return 1;
  }

  /* merge ---------------------------------------------------- */
  bool ok = logmerge.mergeLogFiles( log_list, fname_out );

  return ok ? 0 : 1;
}
