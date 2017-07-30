if LIBWMF_BUILD_ALL
LIBHEAVY = libwmf.la
DIRHEAVY = extra ipa . convert
DEFHEAVY = -DLIBWMF_INCLUDE_TRIO_H=1
else
LIBHEAVY = 
DIRHEAVY = .
DEFHEAVY = 
endif

if LIBWMF_GDK_PIXBUF
GDK_PIXBUF_PLUGIN = io-wmf.la
GDK_PIXBUF_DEFS = -DGDK_PIXBUF_ENABLE_BACKEND=1
else
GDK_PIXBUF_PLUGIN = 
GDK_PIXBUF_DEFS = 
endif

DIST_SUBDIRS = extra ipa . convert

SUBDIRS = $(DIRHEAVY)

CFLAGS = @CFLAGS@ @WMF_CFLAGS@ @GDK_PIXBUF_CFLAGS@

DEFS = @DEFS@ @WMF_DEFS@ $(DEFHEAVY) $(GDK_PIXBUF_DEFS) \
	-DPACKAGE=\"@PACKAGE@\" \
	-DVERSION=\"@VERSION@\" \
	-DWMF_FONTDIR=\"@WMF_FONTDIR@\" \
	-DWMF_GS_FONTDIR=\"@WMF_GS_FONTDIR@\" \
	-DWMF_SYS_FONTMAP=\"@WMF_SYS_FONTMAP@\" \
	-DWMF_XTRA_FONTMAP=\"@WMF_XTRA_FONTMAP@\" \
	-DWMF_GS_FONTMAP=\"@WMF_GS_FONTMAP@\"

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src -I$(top_builddir)

lib_LTLIBRARIES = libwmflite.la $(LIBHEAVY)

libwmflite_la_SOURCES = api.c bbuf.c meta.c player.c recorder.c

libwmflite_la_LDFLAGS = $(SYS_LIBM) \
	-no-undefined \
	-version-info $(LT_LITE_CURRENT):$(LT_LITE_REVISION):$(LT_LITE_AGE) \
	-release $(LT_RELEASE) -export-dynamic

libwmf_la_SOURCES = font.c stream.c wmf.c xml.c

if LIBWMF_OPT_TRIO
LIBTRIO = extra/trio/libtrio.la
else
LIBTRIO = 
endif

if LIBWMF_OPT_SYS_GD
LIBGD = 
else
LIBGD = extra/gd/libgd.la
endif

libwmf_la_LIBADD = ipa/libipa.la libwmflite.la $(LIBGD) $(LIBTRIO) @WMF_LIBFLAGS@

libwmf_la_LDFLAGS = \
	-no-undefined \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-release $(LT_RELEASE) -export-dynamic

loaderdir = $(libdir)/$(GDK_PIXBUF_DIR)

loader_LTLIBRARIES = $(GDK_PIXBUF_PLUGIN)

io_wmf_la_SOURCES = io-wmf.c

io_wmf_la_LIBADD = libwmf.la libwmflite.la @GDK_PIXBUF_LIBS@

io_wmf_la_LDFLAGS = -no-undefined -module -avoid-version

PLAYERHEADERS = \
	player/clip.h \
	player/color.h \
	player/coord.h \
	player/dc.h \
	player/defaults.h \
	player/meta.h \
	player/record.h \
	player/region.h

EXTRA_DIST = wmfdefs.h metadefs.h player.h $(PLAYERHEADERS) recorder.h \
	api.h bbuf.h font.h stream.h xml.h
