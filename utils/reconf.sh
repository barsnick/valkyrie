#!/bin/bash

make distclean
rm -fr ./Inst
./autogen.sh &&
  ./configure --prefix=`pwd`/Inst &&
  make install --quiet
