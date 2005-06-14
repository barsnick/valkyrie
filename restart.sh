#!/bin/bash
bakClean -r
rm ./bin/valkyrie
make clean
qmake -o Makefile valkyrie.pro
make && ./bin/valkyrie
