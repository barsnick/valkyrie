######################################################################
# Valkyrie top-level qmake project file
#
# Note: By default PREFIX="/usr/local". To change this simply do:
# qmake "PREFIX=<your_dir>" && make && make install
######################################################################

include( qwtconfig.pri )

TEMPLATE = subdirs
SUBDIRS  = src


