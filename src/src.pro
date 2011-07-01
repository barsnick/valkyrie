######################################################################
# Valkyrie qmake project file: build application
#
# Note: By default PREFIX="/usr/local". To change this simply do:
# qmake "PREFIX=/path/to/install/tree"
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


#TODO: This seems to work, but surely can't be right... what to do?
#QtCreator wants this for debugging info
#SOURCES += /usr/share/qtcreator/gdbmacros/gdbmacros.cpp


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
    options/suppressions.cpp \
    options/vk_option.cpp \
    options/vk_options_dialog.cpp \
    options/vk_options_page.cpp \
    options/vk_parse_cmdline.cpp \
    options/vk_popt.cpp \
    options/vk_suppressions_dialog.cpp \
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
    utils/vk_utils.cpp \
    utils/vknewprojectdialog.cpp

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
    options/suppressions.h \
    options/vk_option.h \
    options/vk_options_dialog.h \
    options/vk_options_page.h \
    options/vk_parse_cmdline.h \
    options/vk_popt.h \
    options/valgrind_options_page.h \
    options/valkyrie_options_page.h \
    options/vk_suppressions_dialog.h \
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
    utils/vk_defines.h \
    utils/vk_logpoller.h \
    utils/vk_messages.h \
    utils/vk_utils.h \
    utils/vknewprojectdialog.h

RESOURCES += $${VK_ROOT}/icons.qrc




######################################################################
# Generate a config.h with all the necessary variables
#
# Nicer would be to use something like the following, but how to get
# vk_defines as a depedency of _all_ others?
#vk_defines.target = vk_defines
#vk_defines.commands = <cmds>
#QMAKE_EXTRA_TARGETS += vk_defines
#PRE_TARGETDEPS += vk_defines

VK_DEFINES_H = utils/vk_defines.h
system(rm -f $${VK_DEFINES_H})  # make sure build fails if can't generate.
system("echo '$${LITERAL_HASH}define VK_NAME     \"$$NAME\"'      > $$VK_DEFINES_H")
system("echo '$${LITERAL_HASH}define VK_VERSION  \"$$VERSION\"'  >> $$VK_DEFINES_H")
system("echo '$${LITERAL_HASH}define VK_PACKAGE  \"$$PACKAGE\"'  >> $$VK_DEFINES_H")
system("echo '$${LITERAL_HASH}define VK_DOC_PATH \"$$doc.path\"' >> $$VK_DEFINES_H")
