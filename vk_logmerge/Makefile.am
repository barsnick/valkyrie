## Makefile.am -- Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

QT_CXXFLAGS = @QT_CXXFLAGS@
QT_LIBS = @QT_LIBS@
QT_MOC = @QT_MOC@
QT_UIC = @QT_UIC@
QT_DIR = @QT_DIR@

SUBDIRS = tests

########################################################################
# Build vk_logmerge
#
bin_PROGRAMS = vk_logmerge

# QT MOC rule
%.moc.cpp: %.h
	$(QT_MOC) -o $@ $<

# All header filed to be MOC'd
vk_logmerge_MOC = 

# All sources, plus MOCs
vk_logmerge_SOURCES = \
	vklm_main.cpp \
	../valkyrie/tool_utils/vglog.cpp \
	../valkyrie/tool_utils/vglogreader.cpp \
	$(vk_logmerge_MOC) \
	vklm_main.h \
	../valkyrie/tool_utils/vglog.h \
	../valkyrie/tool_utils/vglogreader.h

# make the moc's first, so dependencies work out.
BUILT_SOURCES = $(vk_logmerge_MOC)

# Note: if need UIC stuff, see autoqt docs


# Compile flags
vk_logmerge_CXXFLAGS = -pipe -O0 -g -W -Wall -Wno-non-virtual-dtor -Wundef -Wredundant-decls -Wno-unreachable-code -fno-inline -D_REENTRANT -I$(srcdir)/../valkyrie/tool_utils -I/usr/X11R6/include -I$(QTDIR)/mkspecs/linux-g++ $(QT_CXXFLAGS)

# Link flags
vk_logmerge_LDADD := $(QT_LIBS) -lfreetype -lm -lpthread

CLEANFILES = $(BUILT_SOURCES)
