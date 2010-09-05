#!/bin/bash

cfg=`dirname $_`/astyle.conf

function usage() {
  echo "Usage: `basename $0` [-d|-x]
         -d : dry run: list files found to beautify
         -x : go for it"
  exit 1
}


if [ $# -ne 1 ]; then
  usage
fi
if [ $1 != "-d" ] && [ $1 != "-x" ]; then
  usage
fi 


# find all source files, within our restricted directories.
dirs=`find . -regex ".*\.\(h\|cpp\)" | grep "\.\/\(help\/\|objects\/\|options\/\|toolview\/\|utils\/\|[^\/]\+$\)"`

if test $1 = '-d'; then
  /bin/ls -l $dirs
else 
  if [ ! -e $cfg ]; then
    echo "error: can't find astyle.conf"
    exit 1
  fi

  echo $dirs | xargs astyle --options=$cfg
fi

