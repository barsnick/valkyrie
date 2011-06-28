#! /bin/sh
#
# Script updates the copyright year in every file in Valkyrie that contains
# a copyright notice.  Assumes they're all in the same format:
#
#     "Copyright (C) xxxx-yyyy"
#
# To use:
# - Just in case, run it on a _clean_ checkout only.
# - Run it from the base directory of a Valkyrie workspace.
# - And check the results look ok by diff'ing against the repository.
#
# Note that it will output the names of any code files that don't seem to
# have a copyright notice at all.

dt=`date +%Y`
COPYRIGHT_TPLT="^\*\* Copyright (C) \([0-9][0-9][0-9][0-9]\)-.*, OpenWorks LLP. All rights reserved.$"
COPYRIGHT_NEW="\*\* Copyright (C) \1-$dt, OpenWorks LLP. All rights reserved."

echo "Updating copyrights..."
find . -regex ".*\.\(cpp\|h\)" -not -regex ".*\/moc_.*" \
   -exec sed -i "s/$COPYRIGHT_TPLT/$COPYRIGHT_NEW/" {} +
if [ "$?" -ne "0" ]; then
	echo "Something went wrong!";
else
	echo "done."
fi

bad_files=`find . -regex ".*\.\(cpp\|h\)$" -not -regex ".*\/moc_.*" \
   | xargs grep -L "$COPYRIGHT_TPLT"`
if [ "X$bad_files" != "X" ]; then
	echo
	echo "Failed to update the following files (bad match or couldn't find):"
	echo "$bad_files"
	echo "Please update with the standard copyright notice"
fi

echo
echo "check the changes with:"
echo "svn diff | grep \"^\(\+\|-\)[^\+-]\" | less"
