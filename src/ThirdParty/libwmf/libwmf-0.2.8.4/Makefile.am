if LIBWMF_BUILD_ALL
DIRHEAVY = . src include fonts doc
else
DIRHEAVY = . src include
endif

DIST_SUBDIRS = . src include fonts doc

SUBDIRS = $(DIRHEAVY)

bin_SCRIPTS = libwmf-config

WMFEXAMPLES = \
	examples/2doorvan.wmf \
	examples/Eg.wmf \
	examples/anim0002.wmf \
	examples/ant.wmf \
	examples/arrow01.wmf \
	examples/cell.wmf \
	examples/fjftest.wmf \
	examples/formula1.wmf \
	examples/formula2.wmf \
	examples/formula3.wmf \
	examples/formula4.wmf \
	examples/fulltest.wmf \
	examples/p0000001.wmf \
	examples/p0000016.wmf \
	examples/sample.wmf \
	examples/sample2.wmf \
	examples/text.wmf

EXTRA_DIST = AUTHORS BUILDING CREDITS NEWS README $(WMFEXAMPLES) \
	libwmf.spec \
	patches/acconfig.h \
	ac-helpers/pkg-config/pkg.m4
