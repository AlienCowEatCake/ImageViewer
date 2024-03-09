.SUFFIXES: .xml .html .txt .adoc .1 .7

.xml.html:
	xmlto xhtml-nochunks $<

.xml.1:
	xmlto man $<

.xml.7:
	xmlto man $<

.xml.txt:
	xmlto txt $<

.adoc.html:
	asciidoc $<

all: allhtml manpages

# The distinction between XMLMAN and XMLINTERNAL is because
# some pages shouldn't be installed as part of a binary package;
# they're just for test-pattern generators.
XMLMAN1 = \
	gif2rgb.xml \
	gifbuild.xml \
	gifclrmp.xml \
	giffilter.xml \
	giffix.xml \
	gifsponge.xml \
	giftext.xml \
	giftool.xml
XMLMAN7 = \
	giflib.xml
XMLINTERNAL = \
	gifbg.xml \
	gifcolor.xml \
	gifecho.xml \
	gifinto.xml \
	gifhisto.xml \
	gifwedge.xml
XMLDOC = intro.xml gif_lib.xml
XMLALL = $(XMLMAN1) $(XMLMAN7) $(XMLINTERNAL) $(XMLDOC)

# Logo image file for HTML docs
giflib-logo.gif: ../pic/gifgrid.gif
	convert $^ -resize 50x50 $@

# Philosophical choice: the website gets the internal manual pages
allhtml: $(XMLALL:.xml=.html) giflib-logo.gif

manpages: $(XMLMAN1:.xml=.1) $(XMLMAN7:.xml=.7) $(XMLINTERNAL:.xml=.1)

# Prepare the website directory to deliver an update.
# ImageMagick and asciidoc are required.
website: allhtml
	rm -fr staging; mkdir staging; 
	cp -r $(XMLALL:.xml=.html) gifstandard whatsinagif giflib-logo.gif staging
	cp index.html.in staging/index.html
	asciidoc - <../history.adoc >staging/history.html
