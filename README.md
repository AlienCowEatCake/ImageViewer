# Image Viewer

## Description

Simple, cross-platform image viewer inspired by [GPicView](http://lxde.sourceforge.net/gpicview/).

Features:
* Just a viewer, not an editor or organizer;
* Support a large number of image formats;
* Support an embedded ICC Profiles;
* Support EXIF, XMP and other metadata;
* Lightweight and fast;
* Very suitable for default image viewer of desktop system;
* Simple and intuitive interface;
* Cross platform: GNU/Linux, macOS, Windows, Haiku and other systems;
* Minimal external lib dependency: only pure Qt and system libraries are used;
* Flexible and configurable;
* Desktop independent: doesn't require any specific desktop environment;
* Open source, licensed under GNU GPL v3 or later;

## Screenshots

![Ubuntu](img/Ubuntu.png)
![macOS](img/macOS.png)

## Building

### Requirements

Minimal configuration:
* Qt 4.4 or later
* GCC 3.4 or later

Full configuration:
* Qt 5.5 or later
* Modern GCC, Clang or MSVC compiler with C++11 support

### Building with bundled libraries example (Ubuntu 20.04)

```bash
sudo apt-get install git g++ make qt5-default libqt5svg5-dev
git clone https://github.com/AlienCowEatCake/ImageViewer.git
cd ImageViewer
mkdir build
cd build
qmake CONFIG+="release" -r ../ImageViewer.pro
make
cp -a src/ImageViewer/ImageViewer /path/to/install/
```

### Building with all system libraries example (Ubuntu 20.04)

```bash
sudo apt-get install git g++ make qt5-default libqt5svg5-dev zlib1g-dev \
    liblcms2-dev libexif-dev libexiv2-dev libjpeg-dev libmng-dev libpng-dev \
    libjbig-dev libtiff-dev libwebp-dev libfreetype6-dev libwmf-dev \
    libopenjp2-7-dev libgif-dev libraw-dev librsvg2-dev libheif-dev \
    libilmbase-dev libopenexr-dev libmagickcore-dev libmagickwand-dev \
    qt5-image-formats-plugins libqt5webkit5-dev qtwebengine5-dev \
    libjxr-dev
git clone https://github.com/AlienCowEatCake/ImageViewer.git
cd ImageViewer
mkdir build
cd build
qmake CONFIG+="release enable_pkgconfig" \
    CONFIG+="system_zlib disable_zstd disable_xzutils disable_libexpat" \
    CONFIG+="system_liblcms2 system_libexif system_exiv2 system_libjpeg" \
    CONFIG+="disable_libjasper system_libmng system_libpng system_jbigkit" \
    CONFIG+="system_libtiff system_libwebp disable_libbpg disable_freetype" \
    CONFIG+="system_libwmf system_openjpeg system_giflib system_libraw" \
    CONFIG+="system_librsvg disable_resvg disable_aom disable_libde265 " \
    CONFIG+="system_libheif system_openexr disable_libavif disable_lerc" \
    CONFIG+="disable_flif system_jxrlib disable_brotli disable_highway" \
    CONFIG+="disable_libjxl enable_magickcore system_magickwand" \
    CONFIG+="disable_graphicsmagick disable_graphicsmagickwand" \
    CONFIG+="disable_qtextended disable_stb disable_nanosvg" \
    CONFIG+="disable_qtimageformats enable_qtwebkit enable_qtwebengine" \
    INCLUDEPATH+="/usr/include/jxrlib" \
    -r ../ImageViewer.pro
make
cp -a src/ImageViewer/ImageViewer /path/to/install/
```

### Other building examples

See the [buildscripts/](buildscripts/) directory.

### Configuration options

**Languages Configuration:**
* C++11 options: `disable_cxx11`, `enable_cxx11` *(auto by default)*

**System Libraries Configuration:**
* pkg-config options: `disable_pkgconfig`, `enable_pkgconfig` *(auto by default)*
* ZLib options: `disable_zlib`, `system_zlib` *(bundled package by default)*
* Zstandard options: `disable_zstd`, `system_zstd` *(bundled package by default)*
* XZUtils options: `disable_xzutils`, `system_xzutils` *(bundled package by default)*
* brotli options: `disable_brotli`, `system_brotli` *(bundled package by default)*
* highway options: `disable_highway`, `system_highway` *(bundled package by default)*
* libexpat options: `disable_libexpat`, `system_libexpat` *(bundled package by default)*
* LCMS options: `disable_liblcms2`, `system_liblcms2` *(bundled package by default)*
* libexif options: `disable_libexif`, `system_libexif` *(bundled package by default)*
* exiv2 options: `disable_exiv2`, `system_exiv2` *(bundled package by default)*
* LibJPEG options: `disable_libjpeg`, `system_libjpeg` *(bundled package by default)*
* LibJasPer options: `disable_libjasper`, `system_libjasper` *(bundled package by default)*
* libmng options: `disable_libmng`, `system_libmng` *(bundled package by default)*
* libpng options: `disable_libpng`, `system_libpng` *(bundled package by default)*
* jbigkit options: `disable_jbigkit`, `system_jbigkit` *(bundled package by default)*
* LERC options: `disable_lerc`, `system_lerc` *(bundled package by default)*
* libtiff options: `disable_libtiff`, `system_libtiff` *(bundled package by default)*
* LibWebP options: `disable_libwebp`, `system_libwebp` *(bundled package by default)*
* libbpg options: `disable_libbpg`, `system_libbpg` *(bundled package by default)*
* FreeType options: `disable_freetype`, `system_freetype` *(bundled package by default)*
* libwmf options: `disable_libwmf`, `system_libwmf` *(bundled package by default)*
* OpenJPEG options: `disable_openjpeg`, `system_openjpeg` *(bundled package by default)*
* GIFLIB options: `disable_giflib`, `system_giflib` *(bundled package by default)*
* LibRaw options: `disable_libraw`, `system_libraw` *(bundled package by default)*
* libRSVG options: `disable_librsvg`, `system_librsvg` *(dynamic loading by default)*
* resvg options: `disable_resvg`, `enable_resvg`, `system_resvg` *(disabled by default)*
* aom options: `disable_aom`, `system_aom` *(bundled package by default)*
* libde265 options: `disable_libde265`, `system_libde265` *(bundled package by default)*
* libheif options: `disable_libheif`, `system_libheif` *(bundled package by default)*
* OpenEXR options: `disable_openexr`, `system_openexr` *(bundled package by default)*
* libavif options: `disable_libavif`, `system_libavif` *(bundled package by default)*
* FLIF options: `disable_flif`, `system_flif` *(bundled package by default)*
* jxrlib options: `disable_jxrlib`, `system_jxrlib` *(bundled package by default)*
* libjxl options: `disable_libjxl`, `system_libjxl` *(bundled package by default)*
* MagickCore options: `disable_magickcore`, `enable_magickcore` *(disabled by default)*
* MagickWand options: `disable_magickwand`, `enable_magickwand`, `system_magickwand` *(disabled by default)*
* GraphicsMagick options: `disable_graphicsmagick`, `enable_graphicsmagick` *(disabled by default)*
* GraphicsMagickWand options: `disable_graphicsmagickwand`, `enable_graphicsmagickwand`, `system_graphicsmagickwand` *(disabled by default)*

**Optional Third Party Components Configuration:**
* QtExtended options: `disable_qtextended` *(enabled by default for Qt < 6.0.0)*
* STB options: `disable_stb` *(enabled by default)*
* NanoSVG options: `disable_nanosvg` *(enabled by default)*
* QtImageFormats options: `disable_qtimageformats` *(enabled by default)*

**Optional Built-in Components Configuration:**
* DecoderQtSVG options: `disable_qtsvg` *(enabled by default)*
* DecoderQtWebKit options: `disable_qtwebkit`, `enable_qtwebkit` *(disabled by default)*
* DecoderQtWebEngine options: `disable_qtwebengine`, `enable_qtwebengine` *(disabled by default)*
* DecoderQMLWebEngine options: `disable_qmlwebengine`, `enable_qmlwebengine` *(disabled by default)*
* DecoderMSHTML options: `disable_mshtml` *(enabled by default for Windows)*
* DecoderWIC options: `disable_wic` *(enabled by default for Windows)*
* DecoderNSImage options: `disable_nsimage` *(enabled by default for macOS)*
* DecoderMacWebKit options: `disable_macwebkit` *(enabled by default for macOS)*
* MacToolBar options: `disable_mactoolbar` *(enabled by default for macOS)*
* MacTouchBar options: `disable_mactouchbar` *(enabled by default for macOS)*
* Print Support options: `disable_printsupport` *(enabled by default)*
* Updater options: `enable_update_checking` *(disabled by default)*

