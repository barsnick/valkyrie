#!/bin/bash
#

VG_DIR=/home/de/Programs/valgrind/Inst
#VG_DIR=/home/cerion/Work/Valgrind/valgrind_trunk/Inst

#if [ ! -d "$QTDIR" ]; then
#    echo "error: QTDIR not set"
#    exit 1;
#fi

QTDIR=/usr/lib/qt-3.3

bakClean -r
rm -f ./bin/valkyrie
make clean
./configure --qt-dir=$QTDIR --vg-dir=$VG_DIR
make
#./bin/valkyrie
