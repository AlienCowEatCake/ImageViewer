AC_INIT([libwmf],[0.2.8],[wvware-devel@sourceforge.net])
AC_CONFIG_SRCDIR([src/player.c])

LIBWMF_PACKAGE=libwmf
AC_SUBST(LIBWMF_PACKAGE)

dnl This next section is courtesy gtk+
dnl 
# Making releases:
#   WMF_MICRO_VERSION += 1;
#   WMF_INTERFACE_AGE += 1;
#   WMF_BINARY_AGE += 1;
# if any functions have been added, set WMF_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set WMF_BINARY_AGE and WMF_INTERFACE_AGE to 0.
#
WMF_MAJOR_VERSION=0
WMF_MINOR_VERSION=2
WMF_MICRO_VERSION=8

WMF_INTERFACE_AGE=0
WMF_BINARY_AGE=1

WMFLITE_INTERFACE_AGE=1
WMFLITE_BINARY_AGE=1

dnl spec file substitutions
LIBWMF_VERSION=$WMF_MAJOR_VERSION.$WMF_MINOR_VERSION.$WMF_MICRO_VERSION
LIBWMF_RELEASE=1
AC_SUBST(LIBWMF_VERSION)
AC_SUBST(LIBWMF_RELEASE)

# libtool versioning
LT_RELEASE=$WMF_MAJOR_VERSION.$WMF_MINOR_VERSION
AC_SUBST(LT_RELEASE)

LT_CURRENT=`expr $WMF_MICRO_VERSION - $WMF_INTERFACE_AGE`
LT_REVISION=$WMF_INTERFACE_AGE
LT_AGE=`expr $WMF_BINARY_AGE - $WMF_INTERFACE_AGE`
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

LT_LITE_CURRENT=`expr $WMF_MICRO_VERSION - $WMFLITE_INTERFACE_AGE`
LT_LITE_REVISION=$WMFLITE_INTERFACE_AGE
LT_LITE_AGE=`expr $WMFLITE_BINARY_AGE - $WMFLITE_INTERFACE_AGE`
AC_SUBST(LT_LITE_CURRENT)
AC_SUBST(LT_LITE_REVISION)
AC_SUBST(LT_LITE_AGE)

# For automake.
PACKAGE=$LIBWMF_PACKAGE
VERSION=$LIBWMF_VERSION

dnl Initialize automake stuff
AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

# Specify a configuration file
AM_CONFIG_HEADER(wmfconfig.h)

AH_TOP([
#ifndef LIBWMF_CONFIG_H
#define LIBWMF_CONFIG_H
])

# CFLAGS to be listed by libwmf-config should be added to this:
WMF_CONFIG_CFLAGS=

# Other CFLAGS, not to be listed by libwmf-config, should be added to this:
WMF_CFLAGS=

# LDFLAGS to be listed by libwmf-config should be added to this:
WMF_LIBFLAGS=

# WMF_DEFS is for flags such as -DDEBUG which are to applied only to
# libwmf source not extra packages such as gd - more global defs should be
# added to WMF_CFLAGS
AC_ARG_ENABLE(debug,[  --enable-debug          set DEBUG option to libwmf [default=no]],[
	if [ test "x$enableval" != "xno" ]; then
		WMF_DEFS="-DDEBUG=1"
	else
		WMF_DEFS=
	fi
],[	WMF_DEFS=
])

AC_ARG_ENABLE(ansi,[  --enable-ansi           set -ansi option to libwmf [default=no]],[
	if [ test "x$enableval" != "xno" ]; then
		WMF_DEFS="$WMF_DEFS -ansi"
	fi
])

AC_ARG_ENABLE(pedantic,[  --enable-pedantic       set -pedantic option to libwmf [default=no]],[
	if [ test "x$enableval" != "xno" ]; then
		WMF_DEFS="$WMF_DEFS -pedantic"
	fi
])

LIBWMF_BUILDSTYLE=heavy
libwmf_build_all=yes
AC_ARG_ENABLE(heavy,[  --disable-heavy         only build libwmflite [default=build all]],[
	if [ test "x$enableval" = "xno" ]; then
		LIBWMF_BUILDSTYLE=lite
		libwmf_build_all=no
	fi
])
AC_SUBST(LIBWMF_BUILDSTYLE)
AM_CONDITIONAL(LIBWMF_BUILD_ALL,[ test "x$libwmf_build_all" = "xyes" ])

# gd does not read config.h so need to pass it options on the command line:
GD_DEFS=

dnl Initialize libtool
# Tests for Windows
AC_CYGWIN
AC_EXEEXT
AC_OBJEXT
AC_MINGW32
# End Tests for Windows

AC_MSG_CHECKING([for Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    SYS_LIBM=""
    ;;
  *)
    os_win32=no
    SYS_LIBM="-lm"
    ;;
esac
AC_MSG_RESULT([$os_win32])

AC_SUBST(SYS_LIBM)

dnl Checks for programs.

AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Configure libtool
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
#end libtool stuff

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADER(unistd.h,[
	AC_DEFINE(HAVE_UNISTD_H,1,[define if you have unistd.h])
	GD_DEFS="$GD_DEFS -DHAVE_UNISTD_H"
])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_TYPE_OFF_T
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS(strstr)

dnl Check for trio functions.
require_trio=no

if test $LIBWMF_BUILDSTYLE = heavy; then

AC_CHECK_FUNCS(printf fprintf vfprintf sprintf,,require_trio=yes)
AC_CHECK_FUNCS(vprintf vsprintf)
AC_CHECK_FUNCS(sscanf,,require_trio=yes)
AC_CHECK_FUNCS(scanf vscanf fscanf vsscanf)

dnl Special case (1): vsnprintf
AC_CHECK_FUNCS(vsnprintf,,check_for_vsnprintf=yes)

if [ test "x$check_for_vsnprintf" = "xyes" ]; then
	AC_CHECK_FUNC(_vsnprintf,[
		AC_DEFINE(HAVE_VSNPRINTF,1,[define if you have vsnprintf])
		AC_DEFINE(vsnprintf,_vsnprintf,[define to your implementation of vsnprintf])
	],[	dnl Hmm. On the off-chance, check for vsnprintf in libdb.
		dnl This should, hopefully, solve the problem with Tru64 4.x
		dnl which is incompatible with trio (the latter's fault).
		AC_CHECK_LIB(db,vsnprintf,[
			AC_DEFINE(HAVE_VSNPRINTF,1,[define if you have vsnprintf])
			WMF_LIBFLAGS="$WMF_LIBFLAGS -ldb"
		],[	require_trio=yes
		])
	])
fi

dnl Special case (2): snprintf
AC_CHECK_FUNCS(snprintf,,check_for_snprintf=yes)

if [ test "x$check_for_snprintf" = "xyes" ]; then
	AC_CHECK_FUNC(_snprintf,[
		AC_DEFINE(HAVE_SNPRINTF,1,[define if you have snprintf])
		AC_DEFINE(snprintf,_snprintf,[define to your implementation of snprintf])
	])
fi

dnl Special case (3): vfscanf is defined as a macro for Darwin / Mac OS X
AC_CHECK_FUNCS(vfscanf,,check_for_vfscanf=yes)

if [ test "x$check_for_vfscanf" = "xyes" ]; then
	AC_MSG_CHECKING(for vfscanf in stdio.h)
	AC_EGREP_HEADER(vfscanf,stdio.h,[
		AC_MSG_RESULT(yes)
		AC_DEFINE(HAVE_VFSCANF,1,[define if you have vfscanf])
	],[	AC_MSG_RESULT(no)
	])
fi

fi # $LIBWMF_BUILDSTYLE = heavy

AM_CONDITIONAL(LIBWMF_OPT_TRIO,[ test "x$require_trio" = "xyes" ])

dnl - but always define this; the re-definitions are selective anyway...
AC_DEFINE(TRIO_REPLACE_STDIO,1,[If any *printf or *scanf functions are missing, cover them with trio functions])

dnl Checks for libraries, etc.

AC_ARG_WITH(layers,[  --with-layers           whether to build layers [default is yes]],[
	if [ test "x$withval" = "xyes" ]; then
		libwmf_layers=yes
	elif [ test "x$withval" = "xno" ]; then
		libwmf_layers=no
	elif [ test "x$withval" = "xmodules" ]; then
		AC_MSG_ERROR([* * * Sorry, modules not yet supported! * * *])
		libwmf_layers=modules
	else
		AC_MSG_WARN([* * * Bad option to --with-layers! * * *])
		libwmf_layers=yes
	fi
],[	libwmf_layers=yes
])

if test $LIBWMF_BUILDSTYLE = lite; then
	libwmf_layers=no
fi # $LIBWMF_BUILDSTYLE = lite

if [ test $libwmf_layers = no ]; then
	AC_DEFINE(WITHOUT_LAYERS,1,[defining this will disable egs, fig, svg & foreign layers])
fi

AM_CONDITIONAL(LIBWMF_OPT_MODULES,[ test $libwmf_layers = modules ])
AM_CONDITIONAL(LIBWMF_OPT_LAYERS,[ test $libwmf_layers != no ])

libwmf_x=no
libwmf_xml=no
libwmf_gd_jpeg=no
libwmf_plot=no

if [ test $libwmf_layers != no ]; then
	AC_PATH_X
	AC_PATH_XTRA
else
	X_CFLAGS="-DX_DISPLAY_MISSING=1"
	no_x=yes
fi

dnl (1) X        {-lX11}      [(!)-DX_DISPLAY_MISSING]

WMF_X_CONFIG_CFLAGS=""
WMF_X_CFLAGS=""
WMF_X_LDFLAGS=""

WMF_X_CFLAGS="$X_CFLAGS"
if [ test "x$no_x" != "xyes" ]; then
	WMF_X_CONFIG_CFLAGS="$X_CFLAGS"
	WMF_X_LDFLAGS="$X_LIBS -lX11"
	libwmf_x=yes
fi

AM_CONDITIONAL(LIBWMF_OPT_X,[ test "x$no_x" != "xyes" ])

dnl (2) expat/libxml2 {-lexpat/-lxml2}

WMF_XML_DIR=""
WMF_XML_CFLAGS=""
WMF_XML_LDFLAGS=""

libwmf_xml=unknown

AC_ARG_WITH(expat,[  --with-expat=DIR        use expat in DIR],[
	if test "x$withval" = "xno"; then
		libwmf_xml=libxml2
	elif test "x$withval" = "xyes"; then
		libwmf_xml=expat
	else
		libwmf_xml=expat
		WMF_XML_DIR="$withval"
	fi
])

AC_ARG_WITH(libxml2,[  --with-libxml2=DIR      use libxml2 in DIR],[
	if test "x$withval" = "xno"; then
		if test $libwmf_xml = libxml2; then
			libwmf_xml=none
		else
			libwmf_xml=expat
		fi
	elif test "x$withval" = "xyes"; then
		if test $libwmf_xml = expat; then
			AC_MSG_ERROR([* * * Can't specify both expat and libxml2! * * *])
		else
			libwmf_xml=libxml2
		fi
	else
		if test $libwmf_xml = expat; then
			AC_MSG_ERROR([* * * Can't specify both expat and libxml2! * * *])
		else
			libwmf_xml=libxml2
			WMF_XML_DIR="$withval"
		fi
	fi
])

if test $LIBWMF_BUILDSTYLE = lite; then
	libwmf_xml=none
fi # $LIBWMF_BUILDSTYLE = lite

if test $libwmf_xml = expat -o $libwmf_xml = unknown; then
	if test "x$WMF_XML_DIR" != "x"; then
		_cppflags=$CPPFLAGS
		_ldflags=$LDFLAGS
		CPPFLAGS="$CPPFLAGS -I$WMF_XML_DIR/include"
		LDFLAGS="$LDFLAGS -L$WMF_XML_DIR/lib"
		WMF_XML_CFLAGS="-I$WMF_XML_DIR/include"
		WMF_XML_LDFLAGS="-L$WMF_XML_DIR/lib -lexpat"
	else
		WMF_XML_CFLAGS=""
		WMF_XML_LDFLAGS="-lexpat"
	fi
	AC_CHECK_HEADER(expat.h,[
		AC_CHECK_LIB(expat,XML_ParserFree,[
			libwmf_xml=expat
		],[	if test $libwmf_xml = expat; then
				AC_MSG_ERROR([* * * unable to find expat * * *])
			fi
		])
	],[	if test $libwmf_xml = expat; then
			AC_MSG_ERROR([* * * unable to find expat * * *])
		fi
	])
	if test "x$WMF_XML_DIR" != "x"; then
		CPPFLAGS=$_cppflags
		LDFLAGS=$_ldflags
	fi
fi

if test $libwmf_xml = libxml2 -o $libwmf_xml = unknown; then
	if [ test -n "$WMF_XML_DIR" ]; then
		AC_PATH_PROG(XML2_CONFIG,xml2-config,,$WMF_XML_DIR/bin $PATH)
	else
		AC_PATH_PROG(XML2_CONFIG,xml2-config)
	fi

	if test "x$XML2_CONFIG" != "x"; then
		WMF_XML_CFLAGS=`$XML2_CONFIG --cflags`
		WMF_XML_LDFLAGS=`$XML2_CONFIG --libs`
		libwmf_xml=libxml2
	elif test $libwmf_xml = libxml2; then
		AC_MSG_ERROR([* * * unable to find xml2-config; see ftp://xmlsoft.org/ * * *])
	fi
fi

if test $libwmf_xml = expat; then
	AC_DEFINE(HAVE_EXPAT,1,[define if you have expat])
elif test $libwmf_xml = libxml2; then
	AC_DEFINE(HAVE_LIBXML2,1,[define if you have libxml2])
else
	libwmf_xml=none
	WMF_XML_CFLAGS=""
	WMF_XML_LDFLAGS=""
fi

dnl (3) freetype {-lfreetype} [-DHAVE_LIBFREETYPE]

WMF_FT_CONFIG_CFLAGS=""
WMF_FT_CFLAGS=""
WMF_FT_LDFLAGS=""

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

have_freetype=no

AC_ARG_WITH(freetype,[  --with-freetype=DIR     use freetype2 in DIR],[
	if [ test "x$withval" != "xno" ]; then
		if [ test "x$withval" != "xyes" ]; then
			FREETYPE_DIR=$withval
		fi
	else
		AC_MSG_ERROR(* * * libwmf requires freetype2 * * *)
	fi
])

if [ test -n "$FREETYPE_DIR" ]; then
	AC_PATH_PROG(FREETYPE_CONFIG,freetype-config, ,[$FREETYPE_DIR/bin:$PATH])
else
	AC_PATH_PROG(FREETYPE_CONFIG,freetype-config)
fi

if [ test -n "$FREETYPE_CONFIG" ]; then
	if [ test -n "$FREETYPE_DIR" ]; then
		freetype_cflags="`$FREETYPE_CONFIG --cflags` -I$FREETYPE_DIR/include"
		freetype_libs=`$FREETYPE_CONFIG --libs`
	else
		freetype_cflags=`$FREETYPE_CONFIG --cflags`
		freetype_libs=`$FREETYPE_CONFIG --libs`
	fi
else
	if [ test -n "$FREETYPE_DIR" ]; then
		freetype_cflags="-I$FREETYPE_DIR/include/freetype2 -I$FREETYPE_DIR/include"
		freetype_libs="-L$FREETYPE_DIR/lib -lfreetype"
	else
		freetype_cflags=""
		freetype_libs="-lfreetype"
	fi
fi

CPPFLAGS="$freetype_cflags $CPPFLAGS"
LDFLAGS="$LDFLAGS $freetype_libs"

AC_CHECK_LIB(freetype,FT_Init_FreeType,[
	WMF_FT_LDFLAGS="$freetype_libs"
],[	AC_MSG_ERROR([* * * freetype(2) is required * * *])
])
AC_CHECK_HEADER(ft2build.h,[
	WMF_FT_CFLAGS="$freetype_cflags"
	WMF_FT_CONFIG_CFLAGS="$freetype_cflags"
],[	AC_MSG_ERROR([* * * freetype(2) is required * * *])
])

GD_DEFS="$GD_DEFS -DHAVE_LIBFREETYPE"

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

dnl (4) zlib     {-lz}

WMF_Z_CONFIG_CFLAGS=""
WMF_Z_CFLAGS=""
WMF_Z_LDFLAGS=""

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

AC_ARG_WITH(zlib,[  --with-zlib=DIR         use libz in DIR],[
	if [ test "x$withval" != "xno" ]; then
		if [ test "x$withval" != "xyes" ]; then
			ZLIB_DIR=$withval
			CPPFLAGS="$CPPFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
		fi
	else
		AC_MSG_ERROR(* * * libwmf requires zlib * * *)
	fi
])

if test $LIBWMF_BUILDSTYLE = heavy; then

dnl WMF_Z_CFLAGS are required later on when testing for png, etc.

AC_CHECK_HEADER(zlib.h,[
	AC_CHECK_LIB(z,gzputs,[
		if [ test "x$ZLIB_DIR" != "x" ]; then
			WMF_Z_CONFIG_CFLAGS="-I$ZLIB_DIR/include"
			WMF_Z_CFLAGS="-I$ZLIB_DIR/include"
			WMF_Z_LDFLAGS="-L$ZLIB_DIR/lib -lz"
		else
			WMF_Z_LDFLAGS="-lz"
		fi
	],[	AC_CHECK_LIB(gz,gzputs,[
			if [ test "x$ZLIB_DIR" != "x" ]; then
				WMF_Z_CONFIG_CFLAGS="-I$ZLIB_DIR/include"
				WMF_Z_CFLAGS="-I$ZLIB_DIR/include"
				WMF_Z_LDFLAGS="-L$ZLIB_DIR/lib -lgz"
			else
				WMF_Z_LDFLAGS="-lgz"
			fi
		],[	AC_MSG_ERROR(* * * unable to find libz which is required by libwmf * * *)
		])
	])
],[	AC_MSG_ERROR(* * * unable to find "zlib.h" which is required by libwmf * * *)
])

fi # $LIBWMF_BUILDSTYLE = heavy

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

dnl (5) png      {-lpng}      [-DHAVE_LIBPNG]

WMF_PNG_CFLAGS=""
WMF_PNG_LDFLAGS=""

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

AC_ARG_WITH(png,[  --with-png=DIR          use libpng in DIR],[
	if [ test "x$withval" != "xno" ]; then
		if [ test "x$withval" != "xyes" ]; then
			PNG_DIR=$withval
			CPPFLAGS="$CPPFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
		fi
	else
		AC_MSG_ERROR(* * * libwmf requires libpng * * *)
	fi
])

if test $LIBWMF_BUILDSTYLE = heavy; then

dnl "png.h" includes "zlib.h"
CPPFLAGS="$CPPFLAGS $WMF_Z_CFLAGS"

AC_CHECK_HEADER(png.h,[
	AC_CHECK_LIB(png,png_write_image,[
		if [ test "x$PNG_DIR" != "x" ]; then
			WMF_PNG_CFLAGS="-I$PNG_DIR/include"
			WMF_PNG_LDFLAGS="-L$PNG_DIR/lib -lpng"
		else
			WMF_PNG_LDFLAGS="-lpng"
		fi
	],[	AC_MSG_ERROR(* * * unable to find libpng which is required by libwmf * * *)
	],-lz -lm)
],[	AC_MSG_ERROR(* * * unable to find "png.h" which is required by libwmf * * *)
])

AC_DEFINE(HAVE_LIBPNG,1,[define if you have libpng])
GD_DEFS="$GD_DEFS -DHAVE_LIBPNG"

fi # $LIBWMF_BUILDSTYLE = heavy

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

dnl (6) jpeg     {-ljpeg}     [-DHAVE_LIBJPEG] (optional)

WMF_JPEG_CFLAGS=""
WMF_JPEG_LDFLAGS=""

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

AC_ARG_WITH(jpeg,[  --with-jpeg=DIR         use libjpeg in DIR],[
	if [ test "x$withval" != "xno" ]; then
		if [ test "x$withval" != "xyes" ]; then
			JPEG_DIR=$withval
			CPPFLAGS="$CPPFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
		fi
		search_for_jpeg=yes
	else
		search_for_jpeg=no
	fi
],[	search_for_jpeg=yes
])

if test $LIBWMF_BUILDSTYLE = lite; then
	search_for_jpeg=no
fi # $LIBWMF_BUILDSTYLE = lite

if [ test "$search_for_jpeg" != "no" ]; then
	AC_CHECK_HEADER(jpeglib.h,[
		AC_CHECK_LIB(jpeg,jpeg_CreateCompress,[
			if [ test "x$JPEG_DIR" != "x" ]; then
				WMF_JPEG_CFLAGS="-I$JPEG_DIR/include"
				WMF_JPEG_LDFLAGS="-L$JPEG_DIR/lib -ljpeg"
			else
				WMF_JPEG_LDFLAGS="-ljpeg"
			fi
			AC_DEFINE(HAVE_LIBJPEG,1,[define if you have libjpeg])
			GD_DEFS="$GD_DEFS -DHAVE_LIBJPEG"
			libwmf_gd_jpeg=yes
		])
	])
fi

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

dnl (7) gd       {-lgd}

dnl Chances are that even if gd is present, it won't have been patched for compatibility
dnl with libwmf, but worth checking anyway...

WMF_GD_CFLAGS=""
WMF_GD_LDFLAGS=""

libwmf_gd="wmf"

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

AC_ARG_WITH(sys-gd,[  --with-sys-gd=DIR       use system GD in DIR (if compatible)],[
	if [ test "x$withval" != "xno" ]; then
		if [ test "x$withval" != "xyes" ]; then
			GD_DIR=$withval
			CPPFLAGS="$CPPFLAGS -I$withval/include"
			LDFLAGS="$LDFLAGS -L$withval/lib"
		fi
		search_for_gd=yes
	else
		search_for_gd=no
	fi
],[	search_for_gd=yes
])

AC_ARG_ENABLE(gd,[  --disable-gd            don't build GD, don't look for it, don't require it],[
	if [ test "x$enableval" = "xno" ]; then
		search_for_gd=no
		libwmf_gd="none"
	fi
],[	if [ test $libwmf_layers = no ]; then
		search_for_gd=no
		libwmf_gd="none"
	fi
])

if [ test "$search_for_gd" != "no" ]; then
	other_libs=""

	dnl Do I need extra libs with Xpm?
	dnl 
	if [ test "x$libwmf_x" = "xyes" ]; then
		CPPFLAGS="$CPPFLAGS $WMF_X_CFLAGS"
		LDFLAGS="$LDFLAGS $WMF_X_LDFLAGS"
		AC_CHECK_LIB(Xpm,main,[
			other_libs="$other_libs -lXpm"
			WMF_GD_LDFLAGS="-lXpm"
		])
	fi

	if [ test "x$libwmf_gd_jpeg" = "xyes" ]; then
		CPPFLAGS="$CPPFLAGS $WMF_JPEG_CFLAGS"
		LDFLAGS="$LDFLAGS $WMF_JPEG_LDFLAGS"
		other_libs="$other_libs -ljpeg"
	fi

	CPPFLAGS="$CPPFLAGS $WMF_FT_CFLAGS $WMF_PNG_CFLAGS $WMF_Z_CFLAGS"
	LDFLAGS="$LDFLAGS $WMF_FT_LDFLAGS $WMF_PNG_LDFLAGS $WMF_Z_LDFLAGS"
	other_libs="$other_libs -lfreetype -lpng -lz -lm"

	AC_CHECK_HEADER(gd.h,[
		AC_CHECK_LIB(gd,gdClipSetAdd,[
			AC_CHECK_LIB(gd,gdft_draw_bitmap,[
				libwmf_gd="sys"
			],,$other_libs)
		],,$other_libs)
	])
fi

if [ test "x$libwmf_gd" != "xnone" ]; then
	AC_DEFINE(HAVE_GD,1,[define if you have libgd])
	build_gd_layer=yes
else
	build_gd_layer=no
fi

if [ test "x$libwmf_gd" = "xsys" ]; then
	if [ test "x$GD_DIR" != "x" ]; then
		WMF_GD_CFLAGS="-I$GD_DIR/include"
		WMF_GD_LDFLAGS="-L$GD_DIR/lib -lgd $WMF_GD_LDFLAGS"
	else
		WMF_GD_LDFLAGS="-lgd $WMF_GD_LDFLAGS"
	fi
elif [ test "x$libwmf_gd" = "xnone" ]; then
	dnl Slight hack here to disable GD build:
	libwmf_gd="sys"
else
	WMF_GD_LDFLAGS=""
fi

AM_CONDITIONAL(LIBWMF_OPT_GD,[ test "x$build_gd_layer" = "xyes" ])
AM_CONDITIONAL(LIBWMF_OPT_SYS_GD,[ test "x$libwmf_gd" = "xsys" ])

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

dnl (8) plot     {-lplot}

WMF_PLOT_CFLAGS=""
WMF_PLOT_LDFLAGS=""

_cppflags=$CPPFLAGS
_ldflags=$LDFLAGS

# AC_ARG_WITH(plot,[  --with-plot=DIR         use GNU plotutils (>= 2.4.0) in DIR (default no)],[
#	if [ test "x$withval" != "xno" ]; then
#		AC_MSG_ERROR([* * * plotutils is not supported, sorry * * *])
#		if [ test "x$withval" != "xyes" ]; then
#			PLOT_DIR=$withval
#			CPPFLAGS="$CPPFLAGS -I$withval/include"
#			LDFLAGS="$LDFLAGS -L$withval/lib"
#		fi
#		search_for_plot=yes
#	else
#		search_for_plot=no
#	fi
#],[	search_for_plot=no
#])

search_for_plot=no

if [ test "$search_for_plot" != "no" ]; then
	AC_CHECK_HEADER(plot.h,[
		AC_CHECK_LIB(plot,pl_libplot_ver,[
			AC_LIBPLOT_LIBS
			if [ test $ac_can_link_libplot != no ]; then
				if [ test "x$PLOT_DIR" != "x" ]; then
					WMF_PLOT_CFLAGS="-I$PLOT_DIR/include"
					WMF_PLOT_LDFLAGS="-L$PLOT_DIR/lib $LIBPLOT_LIBS"
				else
					WMF_PLOT_LDFLAGS="$LIBPLOT_LIBS"
				fi
				AC_DEFINE(HAVE_LIBPLOT,1,[define if you have libplot])
				libwmf_plot=maybe
			else
				AC_MSG_ERROR(* * * sorry - unable to link against libplot * * *)
			fi
		],[	AC_MSG_ERROR(GNU plotutils >= 2.4.0 required)
		])
	])
fi

AM_CONDITIONAL(LIBWMF_OPT_PLOT,[ test "x$libwmf_plot" != "xno" ])

CPPFLAGS=$_cppflags
LDFLAGS=$_ldflags

dnl Dom's gdk-pixbuf plugin

wmf_gdk_pixbuf=no

GDK_PIXBUF_CFLAGS=""
GDK_PIXBUF_LIBS=""
GDK_PIXBUF_DIR=""

if test $LIBWMF_BUILDSTYLE != lite; then
	PKG_CHECK_MODULES(GDK_PIXBUF,gdk-pixbuf-2.0 >= 2.1.2,[
		GTK_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
		GDK_PIXBUF_DIR="gtk-2.0/$GTK_VERSION/loaders"
		wmf_gdk_pixbuf=yes
	],[	wmf_gdk_pixbuf=no
	])
fi
AM_CONDITIONAL(LIBWMF_GDK_PIXBUF,[test $wmf_gdk_pixbuf = yes])

AC_SUBST(GDK_PIXBUF_CFLAGS)
AC_SUBST(GDK_PIXBUF_LIBS)
AC_SUBST(GDK_PIXBUF_DIR)

dnl Other options...

AC_ARG_WITH(docdir,[  --with-docdir=DIR       install documentation in DIR],[
	WMF_DOCDIR=$withval
],[	if test "x$prefix" = "xNONE"; then
		WMF_DOCDIR=$ac_default_prefix/share/doc/$PACKAGE
	else
		WMF_DOCDIR=$prefix/share/doc/$PACKAGE
	fi
])

AC_ARG_WITH(fontdir,[  --with-fontdir=DIR      install Type1 fonts in DIR],[
	WMF_FONTDIR=$withval
],[	if test "x$prefix" = "xNONE"; then
		WMF_FONTDIR=$ac_default_prefix/share/$PACKAGE/fonts
	else
		WMF_FONTDIR=$prefix/share/$PACKAGE/fonts
	fi
])

AC_ARG_WITH(sysfontmap,[  --with-sysfontmap=FILE  [default is /usr/share/fonts/fontmap]],[
	WMF_SYS_FONTMAP=$withval
],[	WMF_SYS_FONTMAP=/usr/share/fonts/fontmap
])

AC_ARG_WITH(xtrafontmap,[  --with-xtrafontmap=FILE non-system XML fontmap],[
	WMF_XTRA_FONTMAP=$withval
],[	WMF_XTRA_FONTMAP=$WMF_FONTDIR/fontmap
])

AC_ARG_WITH(gsfontmap,[  --with-gsfontmap=FILE   ghostscript fontmap],[
	WMF_GS_FONTMAP=$withval
],[	dnl 
	dnl Test first for Debian Font Manager's ghostscript Fontmap.
	dnl Next test for GnuWin32 ghostscript Fontmap.
	dnl Next test for standard ghostscript Fontmap.
	dnl If not found, default to RedHat location.
	dnl 
	if [ test -r /var/lib/defoma/gs.d/dirs/fonts/Fontmap.GS ]; then
		WMF_GS_FONTMAP=/var/lib/defoma/gs.d/dirs/fonts/Fontmap.GS
	elif [ test -r /var/lib/defoma/gs.d/dirs/fonts/Fontmap ]; then
		WMF_GS_FONTMAP=/var/lib/defoma/gs.d/dirs/fonts/Fontmap
	elif [ test -r 'c:/progra~1/gs/gs/lib/fontmap.gs' ]; then
		WMF_GS_FONTMAP='c:/progra~1/gs/gs/lib/fontmap.gs'
	elif [ test -r 'c:/progra~1/gs/gs/lib/fontmap' ]; then
		WMF_GS_FONTMAP='c:/progra~1/gs/gs/lib/fontmap'
	elif [ test -r 'c:/gs/fonts/fontmap.gs' ]; then
		WMF_GS_FONTMAP='c:/gs/fonts/fontmap.gs'
	elif [ test -r 'c:/gs/fonts/fontmap' ]; then
		WMF_GS_FONTMAP='c:/gs/fonts/fontmap'
	else
		WMF_GS_FONTMAP=/usr/share/ghostscript/5.50/Fontmap
		if test -d /usr/share/ghostscript; then
			for i in `ls /usr/share/ghostscript`; do
				if test -r "/usr/share/ghostscript/$i/Fontmap.GS"; then
					WMF_GS_FONTMAP="/usr/share/ghostscript/$i/Fontmap.GS"
				elif test -r "/usr/share/ghostscript/$i/Fontmap"; then
					WMF_GS_FONTMAP="/usr/share/ghostscript/$i/Fontmap"
				elif test -r "/usr/share/ghostscript/$i/lib/Fontmap.GS"; then
					WMF_GS_FONTMAP="/usr/share/ghostscript/$i/lib/Fontmap.GS"
				elif test -r "/usr/share/ghostscript/$i/lib/Fontmap"; then
					WMF_GS_FONTMAP="/usr/share/ghostscript/$i/lib/Fontmap"
				fi
			done
		fi
	fi
])

AC_ARG_WITH(gsfontdir,[  --with-gsfontdir=DIR    directory for ghostscript fonts],[
	WMF_GS_FONTDIR=$withval
],[	dnl 
	dnl Test first for Debian Font Manager's ghostscript font directory.
	dnl Next test for GnuWin32 ghostscript font directory.
	dnl Next test for standard ghostscript font directory.
	dnl If not found, default to RedHat location.
	dnl 
	if [ test -d /var/lib/defoma/gs.d/dirs/fonts ]; then
		WMF_GS_FONTDIR=/var/lib/defoma/gs.d/dirs/fonts
	elif [ test -d 'c:/progra~1/gs/gs/lib/fonts' ]; then
		WMF_GS_FONTDIR='c:/progra~1/gs/gs/lib/fonts'
	elif [ test -d 'c:/gs/fonts' ]; then
		WMF_GS_FONTDIR='c:/gs/fonts'
	else
		WMF_GS_FONTDIR=/usr/share/fonts/default/Type1
	fi
])

dnl Output...

WMF_CFLAGS="$WMF_FT_CFLAGS $WMF_X_CFLAGS $WMF_PLOT_CFLAGS $WMF_XML_CFLAGS"
WMF_CFLAGS="$WMF_CFLAGS $WMF_GD_CFLAGS $WMF_JPEG_CFLAGS $WMF_PNG_CFLAGS $WMF_Z_CFLAGS"

AC_SUBST(WMF_CFLAGS)

WMF_CONFIG_CFLAGS="$WMF_FT_CONFIG_CFLAGS $WMF_Z_CONFIG_CFLAGS $WMF_X_CONFIG_CFLAGS"

AC_SUBST(WMF_CONFIG_CFLAGS)

WMF_LIBFLAGS="$WMF_PLOT_LDFLAGS $WMF_GD_LDFLAGS $WMF_FT_LDFLAGS $WMF_X_LDFLAGS $WMF_XML_LDFLAGS"
WMF_LIBFLAGS="$WMF_LIBFLAGS $WMF_JPEG_LDFLAGS $WMF_PNG_LDFLAGS $WMF_Z_LDFLAGS $SYS_LIBM"

AC_SUBST(WMF_LIBFLAGS)

AC_SUBST(WMF_DOCDIR)
AC_SUBST(WMF_FONTDIR)
AC_SUBST(WMF_SYS_FONTMAP)
AC_SUBST(WMF_XTRA_FONTMAP)
AC_SUBST(WMF_GS_FONTMAP)
AC_SUBST(WMF_GS_FONTDIR)

AC_SUBST(WMF_DEFS)
AC_SUBST(GD_DEFS)

AH_BOTTOM([

#endif /* ! LIBWMF_CONFIG_H */
])

AC_CONFIG_FILES([
Makefile
doc/Makefile
doc/caolan/Makefile
doc/caolan/pics/Makefile
doc/html/Makefile
fonts/Makefile
fonts/libwmf-fontmap
fonts/fontmap
include/Makefile
include/libwmf/Makefile
src/Makefile
src/extra/Makefile
src/extra/trio/Makefile
src/extra/gd/Makefile
src/ipa/Makefile
src/convert/Makefile
libwmf-config
libwmf.spec
])
AC_OUTPUT

echo ""
echo "	Support for X:			$libwmf_x"
echo "	Support for GD:			$libwmf_gd"
echo "	Support for JPEG though GD:	$libwmf_gd_jpeg"
echo "	Support for XML:		$libwmf_xml"
echo ""
echo "	Documentation directory:	$WMF_DOCDIR"
echo "	Font directory:			$WMF_FONTDIR"
echo "	System XML fontmap file:	$WMF_SYS_FONTMAP"
echo "	Non-system XML fontmap file:	$WMF_XTRA_FONTMAP"
echo "	ghostscript fontmap file:	$WMF_GS_FONTMAP"
echo "	ghostscript font directory:	$WMF_GS_FONTDIR"
echo ""
echo "	building gdk-pixbuf plug-in?	$wmf_gdk_pixbuf"
echo ""
