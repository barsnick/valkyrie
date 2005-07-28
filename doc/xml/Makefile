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
html_dir  = ..
xml_dir   = .
img_dir   = $(html_dir)/images
lib_dir   = $(xml_dir)/lib

# manual -> index.xml
MANUAL    = $(xml_dir)/index.xml
# list of 'about' pages for about dialog
ABOUT_FILES = about_qt about_vk support
# $(top_dir)LICENSE.GPL

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

# stylesheets
XSL_HTML_CHUNKED = $(lib_dir)/vk-html-chunk.xsl
XSL_HTML_SINGLE  = $(lib_dir)/vk-html-single.xsl


default:
	@echo ""
	@echo "Choose a target from:"
	@echo "valid:      validate all xml docs"
	@echo "manual:     generate chunked html for the manual"
	@echo "standalone: generate the 'about' html files"
	@echo "clean:      remove all generated html files"
	@echo ""

all: clean manual standalone


valid:
	$(XMLLINT) $(XMLLINT_FLAGS) $(xml_dir)/index.xml


# single html files for the 'about' dialog
standalone:
	@echo "Generating standalone 'about' files..." ; \
  file_list='$(ABOUT_FILES)'; \
  for file in $$file_list; do \
	  echo "  processing file: $$file.xml" ; \
    $(XSLTPROC) $(XSLTPROC_FLAGS) -o $(html_dir)/$$file.html $(XSL_HTML_SINGLE) $$file.xml ; \
	done

# chunked html
manual:
	@echo "Generating chunked manual files..."
	export XML_CATALOG_FILES=$(XML_CATALOG_FILES)
	cp $(lib_dir)/valkyrie.css $(html_dir)
	$(XSLTPROC) $(XSLTPROC_FLAGS) -o $(html_dir)/ $(XSL_HTML_CHUNKED) $(MANUAL)


clean:
	rm -rf $(html_dir)/*.html
