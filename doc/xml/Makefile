# -*-Makefile-*-

# -------------------------------------------------------------------
# Makefile for building valkyrie's documentation
#/trunk/doc/*.html
#/trunk/doc/images/*.png
#/trunk/doc/vg-html/*.html
#/trunk/doc/xml/*.xml
#/trunk/doc/xml/lib/*.xsl, .css
# -------------------------------------------------------------------

top_dir   = ../../
doc_dir   = ../
build_dir = .
img_dir   = $(doc_dir)/images
html_dir  = $(doc_dir)
lib_dir   = $(build_dir)/lib
xml_dir   = $(build_dir)
# manual -> index.xml
MANUAL    = $(xml_dir)/index.xml
# list of 'about' pages for about dialog
ABOUT_FILES = about_vk.xml about_qt.xml support.xml $(top_dir)LICENSE.GPL

# validation stuff
XMLLINT = xmllint
LINT_FLAGS  = --noout --xinclude --noblanks --postvalid
VALID_FLAGS = --dtdvalid http://www.oasis-open.org/docbook/xml/4.2/docbookx.dtd 
XMLLINT_FLAGS = $(LINT_FLAGS) $(VALID_FLAGS)

# stylesheet processor
XSLTPROC = xsltproc
# phoenix: the dtd is installed, so don't fetch over the net
XSLTPROC_FLAGS = --nonet --xinclude 

# catalog
XML_CATALOG_FILES = /etc/xml/catalog

# stylesheet
XSL_HTML_CHUNK_STYLE  = $(lib_dir)/vk-html-chunk.xsl


default:
	@echo ""
	@echo "Choose a target from:"
	@echo "valid: validate all xml docs"
	@echo "html:  generate chunked html output for all docs"
	@echo "clean:  remove all generated html files"
	@echo ""

valid:
	$(XMLLINT) $(XMLLINT_FLAGS) $(xml_dir)/index.xml

all: html

html:
  # chunked html
	@echo "Generating chunked manual files..."
	export XML_CATALOG_FILES=$(XML_CATALOG_FILES)
	cp $(lib_dir)/valkyrie.css $(html_dir)
	$(XSLTPROC) $(XSLTPROC_FLAGS) -o $(html_dir) $(XSL_HTML_CHUNK_STYLE) $(MANUAL)
  # single html files for the 'about' dialog
	@echo "Generating single 'about' files..." ; \
  file_list='$(ABOUT_FILES)'; \
  for file in $$file_list; do \
	  echo "  processing file: $$file" ; \
	done


clean:
	rm -rf $(doc_dir)*.html
