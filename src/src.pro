######################################################################
# Valkyrie qmake project file: build application
#
# Note: By default PREFIX="/usr/local". To change this simply do:
# qmake "PREFIX=<your_dir>"
######################################################################


VK_ROOT = ..

include( $${VK_ROOT}/vk_config.pri )

TARGET        = valkyrie
TEMPLATE      = app

MOC_DIR       = moc
OBJECTS_DIR   = obj
DESTDIR       = $${VK_ROOT}/bin


######################################################################
# Install directives

doc.files = \
    $${VK_ROOT}/doc/*.html \
    $${VK_ROOT}/doc/*.css \

doc_imgs.files = \
    $${VK_ROOT}/doc/images/*.png

INSTALLS  = target doc doc_imgs


######################################################################
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    help/help_about.cpp \
    help/help_context.cpp \
    help/help_handbook.cpp \
    help/help_urls.cpp \
    objects/helgrind_object.cpp \
    objects/memcheck_object.cpp \
    objects/tool_object.cpp \
    objects/valkyrie_object.cpp \
    objects/valgrind_object.cpp \
    objects/vk_objects.cpp \
    options/helgrind_options_page.cpp \
    options/memcheck_options_page.cpp \
    options/vk_option.cpp \
    options/vk_options_dialog.cpp \
    options/vk_options_page.cpp \
    options/vk_parse_cmdline.cpp \
    options/vk_popt.cpp \
    options/valgrind_options_page.cpp \
    options/valkyrie_options_page.cpp \
    options/widgets/opt_base_widget.cpp \
    options/widgets/opt_cb_widget.cpp \
    options/widgets/opt_ck_widget.cpp \
    options/widgets/opt_le_widget.cpp \
    options/widgets/opt_sp_widget.cpp \
    options/widgets/opt_lb_widget.cpp \
    toolview/helgrindview.cpp \
    toolview/helgrind_logview.cpp \
    toolview/memcheckview.cpp \
    toolview/memcheck_logview.cpp \
    toolview/toolview.cpp \
    toolview/vglogview.cpp \
    utils/vglogreader.cpp \
    utils/vk_config.cpp \
    utils/vk_logpoller.cpp \
    utils/vk_messages.cpp \
    utils/vk_utils.cpp

HEADERS += \
    mainwindow.h \
    help/help_about.h \
    help/help_context.h \
    help/help_handbook.h \
    help/help_urls.h \
    objects/helgrind_object.h \
    objects/memcheck_object.h \
    objects/tool_object.h \
    objects/valkyrie_object.h \
    objects/valgrind_object.h \
    objects/vk_objects.h \
    options/helgrind_options_page.h \
    options/memcheck_options_page.h \
    options/vk_option.h \
    options/vk_options_dialog.h \
    options/vk_options_page.h \
    options/vk_parse_cmdline.h \
    options/vk_popt.h \
    options/valgrind_options_page.h \
    options/valkyrie_options_page.h \
    options/widgets/opt_base_widget.h \
    options/widgets/opt_cb_widget.h \
    options/widgets/opt_ck_widget.h \
    options/widgets/opt_le_widget.h \
    options/widgets/opt_sp_widget.h \
    options/widgets/opt_lb_widget.h \
    toolview/helgrindview.h \
    toolview/helgrind_logview.h \
    toolview/memcheckview.h \
    toolview/memcheck_logview.h \
    toolview/toolview.h \
    toolview/vglogview.h \
    utils/vglogreader.h \
    utils/vk_config.h \
    utils/vk_logpoller.h \
    utils/vk_messages.h \
    utils/vk_utils.h

RESOURCES += $${VK_ROOT}/icons.qrc




######################################################################
# Generate a config.h with all the necessary variables
#
# Nicer would be to use something like the following, but how to get
# vkcfg as a depedency of _all_ others?
#vk_defines.target = vk_defines
#vk_defines.commands = <cmds>
#QMAKE_EXTRA_TARGETS += vk_defines
#PRE_TARGETDEPS += vk_defines

VK_DEFINES_H = vk_defines.h
system(rm -f $${VK_DEFINES_H})  # make sure build fails if cant generate.
system("echo '\
$${LITERAL_HASH}define VK_AUTHOR    \t \"OpenWorks GbR\"\n\
$${LITERAL_HASH}define VK_BUGREPORT \t \"info@open-works.co.uk\"\n\
$${LITERAL_HASH}define VK_COPYRIGHT \t \"(c) 2003-2010 OpenWorks GbR\"\n\
$${LITERAL_HASH}define VG_COPYRIGHT \t \"(c) 2000-2010 and GNU GPLd by Julian Seward et al.\"\n\
$${LITERAL_HASH}define VK_NAME      \t \"$$NAME\"\n\
$${LITERAL_HASH}define VK_VERSION   \t \"$$VERSION\"\n\
$${LITERAL_HASH}define VK_PACKAGE   \t \"$$PACKAGE\"\n\
$${LITERAL_HASH}define VK_DOC_PATH  \t \"$$doc.path\"\n\
$${LITERAL_HASH}define VK_CFG_DIR   \t \".valkyrie\"\n\
$${LITERAL_HASH}define VK_SUPPS_DIR \t \"suppressions/\"\n\
$${LITERAL_HASH}define VK_LOGS_DIRP \t \"/tmp/valkyrie_logs_\"\n\
' > $$VK_DEFINES_H")

