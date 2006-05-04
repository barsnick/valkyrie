## Makefile.am -- Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I m4

QT_CXXFLAGS = @QT_CXXFLAGS@
QT_LIBS = @QT_LIBS@
QT_MOC = @QT_MOC@
QT_UIC = @QT_UIC@
QT_DIR = @QT_DIR@


########################################################################
# Build valkyrie
#
bin_PROGRAMS = valkyrie

# QT MOC rule
%.moc.cpp: %.h
	$(QT_MOC) -o $@ $<

# All header filed to be MOC'd
valkyrie_MOC = \
	main_window.moc.cpp \
	vk_file_utils.moc.cpp \
	core/memcheck_object.moc.cpp \
	core/tool_object.moc.cpp \
	core/valkyrie_object.moc.cpp \
	core/vk_objects.moc.cpp \
	help/context_help.moc.cpp \
	help/hand_book.moc.cpp \
	help/help_about.moc.cpp \
	help/vk_messages.moc.cpp \
	options/cachegrind_options_page.moc.cpp \
	options/intspinbox.moc.cpp \
	options/massif_options_page.moc.cpp \
	options/memcheck_options_page.moc.cpp \
	options/options_page.moc.cpp \
	options/options_widgets.moc.cpp \
	options/options_window.moc.cpp \
	options/valgrind_options_page.moc.cpp \
	options/valkyrie_options_page.moc.cpp \
	tool_utils/vk_logpoller.moc.cpp \
	tool_utils/vk_process.moc.cpp \
	tool_views/cachegrind_view.moc.cpp \
	tool_views/massif_view.moc.cpp \
	tool_views/memcheck_view.moc.cpp \
	tool_views/tool_view.moc.cpp

# All sources, plus MOCs
valkyrie_SOURCES = \
	main.cpp \
	main_window.cpp \
	vk_config.cpp \
	vk_file_utils.cpp \
	vk_utils.cpp \
	core/cachegrind_object.cpp \
	core/massif_object.cpp \
	core/memcheck_object.cpp \
	core/tool_object.cpp \
	core/valgrind_object.cpp \
	core/valkyrie_object.cpp \
	core/vk_objects.cpp \
	help/context_help.cpp \
	help/hand_book.cpp \
	help/help_about.cpp \
	help/html_urls.cpp \
	help/vk_messages.cpp \
	options/cachegrind_options_page.cpp \
	options/intspinbox.cpp \
	options/massif_options_page.cpp \
	options/memcheck_options_page.cpp \
	options/options_page.cpp \
	options/options_widgets.cpp \
	options/options_window.cpp \
	options/parse_cmd_args.cpp \
	options/valgrind_options_page.cpp \
	options/valkyrie_options_page.cpp \
	options/vk_option.cpp \
	options/vk_popt.cpp \
	tool_utils/async_process.cpp \
	tool_utils/vglog.cpp \
	tool_utils/vglogreader.cpp \
	tool_utils/vk_logpoller.cpp \
	tool_utils/vk_process.cpp \
	tool_views/cachegrind_view.cpp \
	tool_views/massif_view.cpp \
	tool_views/memcheck_view.cpp \
	tool_views/tool_view.cpp \
	tool_views/vglogview.cpp \
	$(valkyrie_MOC)

BUILT_SOURCES = $(valkyrie_MOC)

noinst_HEADERS = \
	main_window.h \
	vk_config.h \
	vk_file_utils.h \
	vk_include.h \
	vk_utils.h \
	../icons/filedlg_icons.h \
	../icons/memcheck_icons.h \
	../icons/msgbox_icons.h \
	../icons/tb_handbook_icons.h \
	../icons/tb_mainwin_icons.h \
	../icons/valkyrie_xpm.h \
	../icons/vglogview_icons.h \
	core/cachegrind_object.h \
	core/massif_object.h \
	core/memcheck_object.h \
	core/tool_object.h \
	core/valgrind_object.h \
	core/valkyrie_object.h \
	core/vk_objects.h \
	help/context_help.h \
	help/hand_book.h \
	help/help_about.h \
	help/html_urls.h \
	help/vk_messages.h \
	options/cachegrind_options_page.h \
	options/intspinbox.h \
	options/massif_options_page.h \
	options/memcheck_options_page.h \
	options/options_page.h \
	options/options_widgets.h \
	options/options_window.h \
	options/valgrind_options_page.h \
	options/valkyrie_options_page.h \
	options/vk_option.h \
	options/vk_popt.h \
	options/vk_popt_option.h \
	tool_utils/async_process.h \
	tool_utils/vglog.h \
	tool_utils/vglogreader.h \
	tool_utils/vk_logpoller.h \
	tool_utils/vk_process.h \
	tool_views/cachegrind_view.h \
	tool_views/massif_view.h \
	tool_views/memcheck_view.h \
	tool_views/tool_view.h \
	tool_views/vglogview.h


# Note: for UIC stuff, refer to autoqt docs

# Compile flags
valkyrie_CXXFLAGS = -pipe -O0 -g -W -Wall -Wundef -Wredundant-decls -Wno-unreachable-code -fno-inline -D_REENTRANT -Ihelp -Icore -Ioptions -Itool_views -I../icons -Itool_utils -I/usr/X11R6/include -I$(QTDIR)/mkspecs/linux-g++ $(QT_CXXFLAGS)

# Link flags
valkyrie_LDADD := $(QT_LIBS) -lfreetype -lm -lpthread


CLEANFILES = $(BUILT_SOURCES)
