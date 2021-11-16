# URL: https://www.freetype.org/
# License: FreeType License or GNU GPL v2 - https://www.freetype.org/license.html

TEMPLATE = lib
CONFIG += staticlib
TARGET = tp_freetype

QT -= core gui

CONFIG -= warn_on
CONFIG += exceptions_off rtti_off warn_off

THIRDPARTY_FREETYPE_PATH = $${PWD}/freetype-2.8.1
THIRDPARTY_FREETYPE_CONFIG_PATH = $${PWD}/config

include(../CommonSettings.pri)
include(../libpng/libpng.pri)
include(../zlib/zlib.pri)

INCLUDEPATH = $${THIRDPARTY_FREETYPE_CONFIG_PATH} $${THIRDPARTY_FREETYPE_PATH}/include $${INCLUDEPATH}

DEFINES += FT_PREFIX

DEFINES += FT2_BUILD_LIBRARY
DEFINES += FT_CONFIG_OPTION_SYSTEM_ZLIB
DEFINES += FT_CONFIG_OPTION_USE_PNG
DEFINES += TT_CONFIG_OPTION_SUBPIXEL_HINTING
DEFINES += FT_CONFIG_OPTION_NO_ASSEMBLER

SOURCES += \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/autofit.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbase.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbitmap.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbbox.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftdebug.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftglyph.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftfntfmt.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftinit.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftlcdfil.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftmm.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftsynth.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftsystem.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/fttype1.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/bdf/bdf.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcache.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cff.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/type1cid.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/ftgzip.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/lzw/ftlzw.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvalid.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcf.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfr.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/psaux.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshinter.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/psnames/psmodule.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/raster.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/sfnt.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/smooth/smooth.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/truetype.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/type1.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/type42.c \
    $${THIRDPARTY_FREETYPE_PATH}/src/winfonts/winfnt.c

HEADERS += \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afangles.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afblue.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afcjk.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afcover.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afdummy.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/aferrors.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afglobal.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afhints.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afindic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/aflatin.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/aflatin2.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afloader.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afmodule.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afranges.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afscript.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afshaper.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afstyles.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/aftypes.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afwarp.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/autofit/afwrtsys.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/basepic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/ftbase.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/base/md5.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/bdf/bdf.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/bdf/bdfdrivr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/bdf/bdferror.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftccache.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftccback.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcerror.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcglyph.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcimage.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcmanag.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcmru.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cache/ftcsbits.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2arrst.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2blues.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2error.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2fixed.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2font.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2ft.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2glue.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2hints.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2intrp.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2read.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2stack.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cf2types.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffcmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffdrivr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cfferrs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffgload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffparse.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cffpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cfftoken.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cff/cfftypes.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/ciderrs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/cidgload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/cidload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/cidobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/cidparse.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/cidriver.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/cid/cidtoken.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxvalid.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxvcommn.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxverror.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxvfeat.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxvmod.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxvmort.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gxvalid/gxvmorx.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/ftzconf.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/infblock.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/infcodes.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/inffixed.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/inftrees.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/infutil.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/zlib.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/gzip/zutil.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/lzw/ftzopen.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvalid.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvcommn.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otverror.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvgpos.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/otvalid/otvmod.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcf.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcfdrivr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcferror.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcfread.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pcf/pcfutil.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrcmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrdrivr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrerror.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrgload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrsbit.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pfr/pfrtypes.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/afmparse.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/psauxerr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/psauxmod.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/psconv.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/psobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/t1cmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psaux/t1decode.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshalgo.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshglob.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshmod.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshnterr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/pshinter/pshrec.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psnames/psmodule.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psnames/psnamerr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psnames/pspic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/psnames/pstables.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/ftmisc.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/ftraster.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/ftrend1.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/rasterrs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/raster/rastpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/pngshim.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/sfdriver.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/sferrors.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/sfntpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/sfobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttbdf.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttcmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttcmapc.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttkern.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttmtx.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttpost.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/sfnt/ttsbit.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/smooth/ftgrays.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/smooth/ftsmerrs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/smooth/ftsmooth.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/smooth/ftspic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttdriver.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/tterrors.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttgload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttgxvar.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttinterp.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttpload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/truetype/ttsubpix.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1afm.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1driver.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1errors.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1gload.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1load.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1objs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1parse.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type1/t1tokens.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/t42drivr.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/t42error.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/t42objs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/t42parse.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/type42/t42types.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/winfonts/fnterrs.h \
    $${THIRDPARTY_FREETYPE_PATH}/src/winfonts/winfnt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/config/ftconfig.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/config/ftheader.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/config/ftmodule.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/config/ftoption.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/config/ftstdlib.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/freetype.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftadvanc.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftautoh.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftbbox.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftbdf.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftbitmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftbzip2.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftcache.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftcffdrv.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftchapters.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftcid.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/fterrdef.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/fterrors.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftfntfmt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftgasp.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftglyph.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftgxval.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftgzip.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftimage.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftincrem.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftlcdfil.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftlist.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftlzw.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftmac.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftmm.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftmodapi.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftmoderr.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftotval.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftoutln.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftpcfdrv.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftpfr.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftrender.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftsizes.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftsnames.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftstroke.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftsynth.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftsystem.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/fttrigon.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftttdrv.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/fttypes.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ftwinfnt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/autohint.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftcalc.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftdebug.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftdriver.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftgloadr.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/fthash.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftmemory.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftobjs.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftpic.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftrfork.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftserv.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftstream.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/fttrace.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/ftvalid.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/internal.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/psaux.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/pshints.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svbdf.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svcid.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svfntfmt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svgldict.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svgxval.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svkern.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svmetric.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svmm.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svotval.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svpfr.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svpostnm.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svprop.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svpscmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svpsinfo.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svsfnt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svttcmap.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svtteng.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svttglyf.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/services/svwinfnt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/sfnt.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/t1types.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/internal/tttypes.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/t1tables.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ttnameid.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/tttables.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/tttags.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/freetype/ttunpat.h \
    $${THIRDPARTY_FREETYPE_PATH}/include/ft2build.h \
    $${THIRDPARTY_FREETYPE_CONFIG_PATH}/ftprefix.h

TR_EXCLUDE += $${THIRDPARTY_FREETYPE_PATH}/* $${THIRDPARTY_FREETYPE_CONFIG_PATH}/*

