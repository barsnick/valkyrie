##------------------------------------------------------------
# Config file to create a Makefile
# Run with 'qmake -o Makefile valkyrie.pro'
# See http://doc.trolltech.com/3.0/qmake-manual-1.html


##------------------------------------------------------------
# Get qmake to make a list of all the headers and sources
# This list is included at the bottom of this file.
system( qmake -project -nopwd src/ icons/ -o valkyrie.sources ) {
  message( "Re-creating header/source list: valkyrie.sources" )
}


##------------------------------------------------------------
# What kind of Makefile to create (app | lib | subdirs | vcapp | vclib)
TEMPLATE = app


##------------------------------------------------------------
# Configuration (CONFIG specifies project configuration & compiler options)
CONFIG = qt x11 release


##------------------------------------------------------------
## Enable threading
exists( $(QTDIR)/lib/libqt-mt* ) {
  message( "Configuring multi-threaded application..." )
  CONFIG += thread
} else {
  message( "*** Failed configuration for multi-threaded application..." )
}

##------------------------------------------------------------
# Debugging (just comment this line out for no debugging)
CONFIG += debug warn_on


##------------------------------------------------------------
## compiler flags
QMAKE_CFLAGS_DEBUG = -O0 -g -Wundef -Wredundant-decls -Wunreachable-code -fno-inline
## Uncomment this when using the icc compiler
#QMAKE_CFLAGS_DEBUG += -wd981 -wd383
## Uncomment the foll. for additional really, truly strict.
## Using this shows millions of qt errors
# QMAKE_CFLAGS_DEBUG += -Winline
## Careful - this will cause make to die on errors
# QMAKE_CFLAGS_DEBUG += -Werror
## Using these spews Qt shadows 
# QMAKE_CFLAGS_DEBUG += -Wshadow -Wfloat-equal -Wcast-qual 
## Only use this for C code:
# QMAKE_CFLAGS_DEBUG += -Wmissing-declarations 

QMAKE_CXXFLAGS_DEBUG = $$QMAKE_CFLAGS_DEBUG



##------------------------------------------------------------
# Target name & destination; 
# and where to put the objects and moc files
TARGET      = valkyrie
DESTDIR     = ./bin
MOC_DIR     = ./.moc
OBJECTS_DIR = ./.obj


##------------------------------------------------------------
# Additional libs / includes
#
# the bfd library
#LIBS += -lbfd
# the cplus_demangle library
#LIBS += -liberty
# the glib 2.0 library
#LIBS += -lglib
# the mysql library
#LIBS += -L/usr/lib/mysql -lmysqlclient
# the cups library
#LIBS += -lcups
# the xml library
#LIBS += -lxml2 

#INCLUDEPATH += /opt/gnome/include/glib-2.0/ /opt/gnome/lib/glib-2.0/include
#INCLUDEPATH += /usr/include/mysql
#INCLUDEPATH += /usr/include/libxml2




##------------------------------------------------------------
# Include the list of sources/headers
include( valkyrie.sources )


##------------------------------------------------------------
message( "Creating Makefile" )
system( echo )

