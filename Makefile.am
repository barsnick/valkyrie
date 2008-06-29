## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign 1.7 dist-bzip2
# subdir-objects
# std-options

AC_AUX_DIR = m4
AC_MACRO_DIR = m4

SUBDIRS = valkyrie vk_logmerge doc
# . tests


## Preprend @PERL@ because tests/vk_regtest isn't executable
regtest: check
	@PERL@ tests/vk_regtest vk_logmerge


EXTRA_DIST = \
	README_DEVELOPERS
