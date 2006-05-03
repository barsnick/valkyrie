#!/bin/sh

run ()
{
    echo "running: $*"
    eval $*

    if test $? != 0 ; then
        echo "error: while running '$*'"
        exit 1
    fi
}

run aclocal -I ./m4
run autoheader
# run automake -a
run automake --gnu --add-missing
run autoconf
