CFLAGS = @CFLAGS@ @WMF_CFLAGS@

LDFLAGS = -L$(top_builddir)/src/.libs @WMF_LIBFLAGS@ @LDFLAGS@

DEFS = @DEFS@ @WMF_DEFS@ \
	-DPACKAGE=\"@PACKAGE@\" \
	-DVERSION=\"@VERSION@\"

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_builddir)

if LIBWMF_OPT_LAYERS
WMF2LAYERS = wmf2eps wmf2fig wmf2svg
else
WMF2LAYERS = 
endif

if LIBWMF_OPT_GD
WMF2GD = wmf2gd
else
WMF2GD = 
endif

if LIBWMF_OPT_X
WMF2X = wmf2x
else
WMF2X = 
endif

if LIBWMF_OPT_PLOT
WMF2PLOT = wmf2plot
else
WMF2PLOT = 
endif

bin_PROGRAMS = $(WMF2LAYERS) $(WMF2GD) $(WMF2X) $(WMF2PLOT)

wmf2eps_LDADD = ../libwmf.la ../libwmflite.la

wmf2fig_LDADD = ../libwmf.la ../libwmflite.la

wmf2svg_LDADD = ../libwmf.la ../libwmflite.la

wmf2gd_LDADD = ../libwmf.la ../libwmflite.la

wmf2x_LDADD = ../libwmf.la ../libwmflite.la

wmf2plot_LDADD = ../libwmf.la ../libwmflite.la
