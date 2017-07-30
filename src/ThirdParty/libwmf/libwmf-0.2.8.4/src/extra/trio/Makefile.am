CFLAGS = @CFLAGS@ @WMF_CFLAGS@

LDFLAGS = @LDFLAGS@

DEFS = @DEFS@ @WMF_DEFS@

INCLUDES = -I$(top_builddir) -I$(srcdir)

if LIBWMF_OPT_TRIO
LIBTRIO = libtrio.la
else
LIBTRIO = 
endif

noinst_LTLIBRARIES = $(LIBTRIO)

libtrio_la_SOURCES = trio.c strio.c

libtrio_la_LDFLAGS = 

EXTRA_DIST = trio.h triop.h strio.h
