CFLAGS = @CFLAGS@ @WMF_CFLAGS@

LDFLAGS = @LDFLAGS@

DEFS = @DEFS@ @WMF_DEFS@ \
	-DPACKAGE=\"@PACKAGE@\" \
	-DVERSION=\"@VERSION@\"

if LIBWMF_OPT_SYS_GD
GDHDRS = 
else
GDHDRS = -I$(top_srcdir)/src/extra/gd
endif

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	$(GDHDRS) \
	-I$(top_builddir)

noinst_LTLIBRARIES = libipa.la

libipa_la_SOURCES = eps.c fig.c svg.c xgd.c x.c ipa.c plot.c foreign.c

libipa_la_LDFLAGS = 

IPAHEADERS = \
	ipa/bmp.h \
	ipa.h

EPSHEADERS = \
	eps/bmp.h \
	eps/device.h \
	eps/draw.h \
	eps/region.h \
	eps.h

FIGHEADERS = \
	fig/bmp.h \
	fig/color.h \
	fig/device.h \
	fig/draw.h \
	fig/font.h \
	fig/region.h \
	fig.h

SVGHEADERS = \
	svg/bmp.h \
	svg/device.h \
	svg/draw.h \
	svg/region.h \
	svg.h

GDHEADERS = \
	xgd/bmp.h \
	xgd/device.h \
	xgd/draw.h \
	xgd/font.h \
	xgd/region.h \
	xgd.h

XHEADERS = \
	x/bmp.h \
	x/color.h \
	x/device.h \
	x/draw.h \
	x/font.h \
	x/region.h \
	x.h

PLOTHEADERS = \
	plot/bmp.h \
	plot/device.h \
	plot/draw.h \
	plot/region.h \
	plot.h

EXTRA_DIST = template.c template.h $(IPAHEADERS) $(EPSHEADERS) $(FIGHEADERS) $(SVGHEADERS) $(GDHEADERS) $(XHEADERS) $(PLOTHEADERS)
