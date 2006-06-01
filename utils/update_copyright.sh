#!/bin/bash
# update_copyright.sh
# CAB: 01Jun06

########################################################################
# Check & update copyright in all relevant files within current dir.
#
# First check given all $FILES have $RE_COPYRIGHT
# - if some fail, exit with filenames
# - else continue on to update all copyrights to current year


# All relevant files:
# This actually ends up being .cpp|.h files, but you never know for the future...
FILES=`find . -type f ! -perm +111 \
 | grep -v update_copyright.sh \
 | grep -v '\.svn' | grep -v '\.moc\.' | grep -v '\.o$' | grep -v '\.Po' \
 | grep -v Inst | grep -v icons | grep -v doc | grep -v tests \
 | grep -v Makefile | grep -v config | grep -v autom4te | grep -v stamp-h1 | grep -v m4 \
 | grep -v COPYING | grep -v NEWS | grep -v INSTALL | grep -v AUTHORS | grep -v README \
 | grep -v nightly \
 | grep -v vk_popt.\*`

echo "Checking files for copyright notice..."

# Expected copyright notice:
RE_COPYRIGHT=" \* This file is part of Valkyrie, a front-end for Valgrind
 \* Copyright \(c\) [1|2][9|0][0-9][0-9]-[0-9]*, OpenWorks LLP <info\@open-works.co.uk>
 \* This program is released under the terms of the GNU GPL v.2
 \* See the file LICENSE.GPL for the full license details."

# All files that don't contain $RE_COPYRIGHT
# gotta be a nicer way of doing this...
NO_CR_FILES=`echo $FILES \
 | xargs perl -0777ne 'print "$ARGV " if !$seen{$ARGV}++;' -e "print ! m/$RE_COPYRIGHT/; print \"\n\"" \
 | grep " 1$" | sed -e 's/ 1//'`

if test x"$NO_CR_FILES" != x; then
  echo " - go fix the copyright on these files (or filter them out):"
  for f in $NO_CR_FILES; do echo $f; done
  exit;
else
  echo " - no problems found (but should check none inadvertently filtered out!)"
fi




########################################################################
# Update all copyright dates to current year
echo "Updating copyright year to current year..."
# current year
CURR_YEAR=`date +%Y`

CR_LINE="^ \* Copyright (c) \([0-9]*\)-[0-9]*, OpenWorks LLP <info@open-works.co.uk>$"
CR_LINE_NEW=" \* Copyright (c) \1-$CURR_YEAR, OpenWorks LLP <info@open-works.co.uk>"

for file in $FILES; do
    echo " - $file"
    sed -e "s/$CR_LINE/$CR_LINE_NEW/" <$file >$file.tmp && mv $file.tmp $file;
done

echo "done"
