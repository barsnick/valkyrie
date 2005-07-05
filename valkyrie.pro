# Project file to create a Makefile
# Automatically generated by configure on Tue Jul  5 01:46:21 BST 2005

# the kind of Makefile to create
TEMPLATE = app
CONFIG   = qt x11 debug warn_on thread nocrosscompiler

# target name and destination, + where to put the moc and object files
TARGET      = valkyrie
DESTDIR     = ./bin
MOC_DIR     = ./.moc
OBJECTS_DIR = ./.obj

# compiler flags
QMAKE_CFLAGS   +=  -O0 -g -Wundef -Wredundant-decls -Wno-unreachable-code -Werror -fno-inline
QMAKE_CXXFLAGS +=  -O0 -g -Wundef -Wredundant-decls -Wno-unreachable-code -Werror -fno-inline
QMAKE_LFLAGS   += 

# additional libraries and includes
LIBS +=  -lmysqlclient
QMAKE_LIBDIR_FLAGS +=  -L/usr/lib
INCLUDEPATH +=  /usr/include/mysql

# paths to qt things
QMAKE_MOC   = /usr/lib/qt3/bin/moc
QMAKE_UIC   = /usr/lib/qt3/bin/uic -L /usr/lib/qt3/plugins
QMAKE_QMAKE = /usr/lib/qt3/bin/qmake
QMAKE_INCDIR_QT = /usr/lib/qt3/include
QMAKE_LIBDIR_QT = /usr/lib/qt3/lib
QMAKE_RTLDIR_QT = /usr/lib/qt3/lib
QMAKESPEC       = /usr/lib/qt3/mkspecs/linux-g++

# paths to valkyrie things
QMAKE_ABSOLUTE_SOURCE_ROOT = /home/de/Programs/valkyrie/trunk
QT_SOURCE_TREE    = /home/de/Programs/valkyrie/trunk
QT_BUILD_TREE     = /home/de/Programs/valkyrie/trunk
QT_INSTALL_PREFIX = /home/de/Programs/valkyrie/trunk
docs.path = /home/de/Programs/valkyrie/trunk/doc/
bins.path = /home/de/Programs/valkyrie/trunk/bin/

######################################################################
# Automatically generated by qmake (1.07a) Tue Jul 5 01:46:21 2005
######################################################################

TEMPLATE = app
DEPENDPATH += src \
              icons \
              src/core \
              src/help \
              src/options \
              src/tool_utils \
              src/tool_views
INCLUDEPATH += src \
               src/help \
               src/core \
               src/options \
               src/tool_views \
               icons \
               src/tool_utils

# Input
HEADERS += src/main_window.h \
           src/vk_config.h \
           src/vk_include.h \
           src/vk_utils.h \
           src/workspace.h \
           icons/msgbox_icons.h \
           icons/tb_handbook_icons.h \
           icons/tb_mainwin_icons.h \
           icons/tb_memcheck_icons.h \
           src/core/cachegrind_object.h \
           src/core/massif_object.h \
           src/core/memcheck_object.h \
           src/core/valgrind_object.h \
           src/core/valkyrie_object.h \
           src/core/vk_objects.h \
           src/help/context_help.h \
           src/help/hand_book.h \
           src/help/help_about.h \
           src/help/html_urls.h \
           src/help/vk_msgbox.h \
           src/options/cachegrind_options_page.h \
           src/options/intspinbox.h \
           src/options/massif_options_page.h \
           src/options/memcheck_options_page.h \
           src/options/options_page.h \
           src/options/options_widgets.h \
           src/options/options_window.h \
           src/options/valgrind_options_page.h \
           src/options/valkyrie_options_page.h \
           src/options/vk_option.h \
           src/options/vk_popt.h \
           src/options/vk_popt_option.h \
           src/tool_utils/async_process.h \
           src/tool_utils/logfile.h \
           src/tool_utils/xml_parser.h \
           src/tool_views/cachegrind_view.h \
           src/tool_views/massif_view.h \
           src/tool_views/memcheck_view.h \
           src/tool_views/tool_view.h
SOURCES += src/main.cpp \
           src/main_window.cpp \
           src/vk_config.cpp \
           src/vk_utils.cpp \
           src/workspace.cpp \
           src/core/cachegrind_object.cpp \
           src/core/massif_object.cpp \
           src/core/memcheck_object.cpp \
           src/core/valgrind_object.cpp \
           src/core/valkyrie_object.cpp \
           src/core/vk_objects.cpp \
           src/help/context_help.cpp \
           src/help/hand_book.cpp \
           src/help/help_about.cpp \
           src/help/html_urls.cpp \
           src/help/vk_msgbox.cpp \
           src/options/cachegrind_options_page.cpp \
           src/options/intspinbox.cpp \
           src/options/massif_options_page.cpp \
           src/options/memcheck_options_page.cpp \
           src/options/options_page.cpp \
           src/options/options_widgets.cpp \
           src/options/options_window.cpp \
           src/options/parse_cmd_args.cpp \
           src/options/valgrind_options_page.cpp \
           src/options/valkyrie_options_page.cpp \
           src/options/vk_option.cpp \
           src/options/vk_popt.c \
           src/tool_utils/async_process.cpp \
           src/tool_utils/logfile.cpp \
           src/tool_utils/xml_parser.cpp \
           src/tool_views/cachegrind_view.cpp \
           src/tool_views/massif_view.cpp \
           src/tool_views/memcheck_view.cpp \
           src/tool_views/tool_view.cpp
