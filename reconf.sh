#!/bin/bash
#

#VALG_EXEC=/home/cerion/Work/Valgrind/valgrind_trunk/Inst/bin/valgrind
#VALG_SUPP=/home/cerion/Work/Valgrind/valgrind_trunk/Inst/lib/valgrind

#VALG_EXEC=/home/sewardj/Vg3LINE/trunk/Inst/bin/valgrind
#VALG_SUPP=/home/sewardj/Vg3LINE/trunk/Inst/lib/valgrind

#if [ ! -d "$QTDIR" ]; then
#    echo "error: QTDIR not set"
#    exit 1;
#fi

#OPTS="--shared --thread=yes --qt-dir=$QTDIR"

bakClean -r
rm -f ./bin/valkyrie
make clean
#./configure $OPTS --vg-exec=$VALG_EXEC --vg-supp=$VALG_SUPP
./configure --prefix=/home/de/Programs/valkyrie/trunk/Inst --qt-dir=/usr/lib/qt-3.3 --vg-exec=/home/de/Programs/valgrind/Inst/bin --vg-supp=/home/de/Programs/valgrind/Inst/lib/valgrind
make
#./bin/valkyrie
