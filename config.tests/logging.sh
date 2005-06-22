#!/bin/sh
#
# various bits and pieces that are used in every *.test script
#

# name of this script
sh_me=`basename $0`

# start up cleanly
rm -f conf$$ conf$$.exe conf$$.file
rm -rf conftest confdefs.h conftest.cpp conftest.err

# IFS: we need space, tab and new line, in precisely that order.
as_nl='
'
IFS=" 	$as_nl"

# pipe output to fd 1 so we can have result on same line as msg
exec 6>&1

# pipe error stuff to a log
exec 5>config.log

# where (and when) to direct output
case `echo "testing\c"; echo 1,2,3`,`echo -n testing; echo 1,2,3` in
  *c*,-n*) ECHO_N= ECHO_C='
' ECHO_T='	' ;;
  *c*,*  ) ECHO_N=-n ECHO_C= ECHO_T= ;;
  *)       ECHO_N= ECHO_C='\c' ECHO_T= ;;
esac


