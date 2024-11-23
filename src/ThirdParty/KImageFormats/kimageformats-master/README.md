# KImageFormats

Plugins to allow `QImage` to support extra file formats.

## Introduction

This framework provides additional image format plugins for QtGui.  As
such it is not required for the compilation of any other software, but
may be a runtime requirement for Qt-based software to support certain
image formats.

## Formats

The following image formats have read-only support:

- Animated Windows cursors (ani)
- Camera RAW images (arw, cr2, cr3, dcs, dng, ...)
- Gimp (xcf)
- Krita (kra)
- OpenRaster (ora)
- Pixar raster (pxr)
- Portable FloatMap/HalfMap (pfm, phm)
- Photoshop documents (psd, psb, pdd, psdt)
- Radiance HDR (hdr)
- Scitex CT (sct)
- Sun Raster (im1, im8, im24, im32, ras, sun)

The following image formats have read and write support:

- AV1 Image File Format (avif)
- Encapsulated PostScript (eps)
- High Efficiency Image File Format (heif)
- JPEG XL (jxl)
- JPEG XR (jxr)
- OpenEXR (exr)
- Personal Computer Exchange (pcx)
- Quite OK Image format (qoi)
- SGI images (rgb, rgba, sgi, bw)
- Softimage PIC (pic)
- Targa (tga): supports more formats than Qt's version

## Contributing

See the [`QImageIOPlugin`](https://doc.qt.io/qt-6/qimageioplugin.html) documentation for information on how to write a
new plugin.

The main difference between this framework and the qimageformats module
of Qt is the license.  As such, if you write an imageformat plugin and
you are willing to sign the Qt Project contributor agreement, it may be
better to submit the plugin directly to the Qt Project.

## Duplicated Plugins

The TGA plugin supports more formats than Qt's own TGA plugin;
specifically, the one provided here supports indexed, greyscale and RLE
images (types 1-3 and 9-11), while Qt's plugin only supports type 2
(RGB) files.

The code for this cannot be contributed upstream directly because of
licensing.  If anyone were willing to write fresh code to improve Qt's
TGA plugin, it would allow the TGA plugin in this framework to be
removed.

## License

This framework is licensed under the
[LGPLv2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html#SEC1).

The CMake code in this framework is licensed under the
[BSD license](http://opensource.org/licenses/BSD-3-Clause).

## Plugin status

The current implementation of a plugin may not be complete or may have limitations
of various kinds. Typically the limitations are on maximum size and color depth.

The various plugins are also limited by the formats natively supported by Qt. 
For example, native support for CMYK images is only available since Qt 6.8.

### HDR images

HDR images are supported via floating point image formats from EXR, HDR, JXL,
JXR, PFM and PSD plugins.
It is important to note that in the past these plugins stripped away HDR 
information, returning SDR images.

HDR images return R, G and B values ​​outside the range 0.0 - 1.0.
While Qt painters handles HDR data correctly, some older programs may display 
strange artifacts if they do not use a tone mapping operator (or at least a 
clamp). This is not a plugin issue.

### Metadata

Metadata support is implemented in all formats that support it. In particular,
in addition to the classic `"Description"`, `"Author"`, `"Copyright"`, etc... where 
possible, XMP data is supported via the `"XML:com.adobe.xmp"` key.

Please note that only the most common metadata is supported.

### ICC profile support

ICC support is fully implemented in all formats that support it. When saving, 
some formats convert the image using color profiles according to
specifications. In particular, HDR formats almost always convert to linear
RGB.

### Maximum image size

Where possible, plugins support large images. By convention, many of the
large image plugins are limited to a maximum of 300,000 x 300,000 pixels.
Anyway, all plugins are also limited by the 
`QImageIOReader::allocationLimit()`. Below are the maximum sizes for each
plugin ('n/a' means no limit, i.e. the limit depends on the format encoding).

- ANI: n/a
- AVIF: 32,768 x 32,768 pixels, in any case no larger than 256 megapixels
- EXR: 300,000 x 300,000 pixels
- HDR: n/a (large image)
- HEIF: n/a
- JXL: 262,144 x 262,144 pixels, in any case no larger than 256 megapixels
- JXR: n/a, in any case no larger than 4 GB
- PCX: 65,535 x 65,535 pixels
- PFM: n/a (large image)
- PIC: 65,535 x 65,535 pixels
- PSD: 300,000 x 300,000 pixels
- PXR: 65,535 x 65,535 pixels
- QOI: 300,000 x 300,000 pixels
- RAS: n/a (large image)
- RAW: n/a (depends on the RAW format loaded)
- RGB: 65,535 x 65,535 pixels
- SCT: 300,000 x 300,000 pixels
- TGA: 65,535 x 65,535 pixels
- XCF: 300,000 x 300,000 pixels

### Sequential and random access devices

All plugins work fine on random access devices while only some work on
sequential access devices.
Some plugins, such as PSD, allow reading RGB images on sequential access 
devices, but cannot do the same for Lab files.

**Important: some plugins use `QIODevice` transactions and/or 
`QIODevice::ungetChar()`. Therefore, the device used to read the image must not
have any active transactions.**

### Memory usage

Qt has added many image formats over time. In older plugins, to support new
formats, `QImage` conversion functions have been used, causing memory
consumption proportional to the size of the image to be saved.
Normally this is not a source of problems because the affected plugins
are limited to maximum images of 2GiB or less.

On plugins for formats that support large images, progressive conversion has
been used or the maximum size of the image that can be saved has been limited.

### Non-RGB formats

PSD plugin loads CMYK, Lab and Multichannel images and converts them to RGB 
without using the ICC profile.

JXR, PSD and SCT plugins natively support 4-channel CMYK images when compiled 
with Qt 6.8+.

### The HEIF plugin

**This plugin is disabled by default. It can be enabled with the 
`KIMAGEFORMATS_HEIF` build option in the cmake file.**

### The EXR plugin

The following defines can be defined in cmake to modify the behavior of the plugin:
- `EXR_CONVERT_TO_SRGB`: the linear data is converted to sRGB on read to accommodate programs that do not support color profiles.
- `EXR_DISABLE_XMP_ATTRIBUTE`: disables the stores XMP values in a non-standard attribute named "xmp". Note that Gimp reads the "xmp" attribute and Darktable writes it as well.

### The HDR plugin

The following defines can be defined in cmake to modify the behavior of the plugin:
- `HDR_HALF_QUALITY`: on read, a 16-bit float image is returned instead of a 32-bit float one.

### The JXL plugin

**The current version of the plugin limits the image size to 256 megapixels
according to feature level 5 of the JXL stream encoding.**

The following defines can be defined in cmake to modify the behavior of the plugin:
- `JXL_HDR_PRESERVATION_DISABLED`: disable floating point images (both read and write) by converting them to UINT16 images. Any HDR data is lost. Note that FP images are always disabled when compiling with libJXL less than v0.9.
- `JXL_DECODE_BOXES_DISABLED`: disable reading of metadata (e.g. XMP).

### The JXR plugin

**This plugin is disabled by default. It can be enabled with the 
`KIMAGEFORMATS_JXR` build option in the cmake file.**

The following defines can be defined in cmake to modify the behavior of the plugin:
- `JXR_DENY_FLOAT_IMAGE`: disables the use of float images and consequently any HDR data will be lost.
- `JXR_DISABLE_DEPTH_CONVERSION`: remove the neeeds of additional memory by disabling the conversion between different color depths (e.g. RGBA64bpp to RGBA32bpp) at the cost of reduced compatibility.
- `JXR_DISABLE_BGRA_HACK`: Windows displays and opens JXR files correctly out of the box. Unfortunately it doesn't seem to open (P)RGBA @32bpp files as it only wants (P)BGRA32bpp files (a format not supported by Qt). Only for this format an hack is activated to guarantee total compatibility of the plugin with Windows.
- `JXR_ENABLE_ADVANCED_METADATA`: enable metadata support (e.g. XMP). Some distributions use an incomplete JXR library that does not allow reading metadata, causing compilation errors.

### The PSD plugin

PSD support has the following limitations:
- Only images saved by Photoshop using compatibility mode enabled (Photoshop default) can be decoded.
- Multichannel images are treated as CMY/CMYK and are only loaded if they have 3 or more channels.
- Duotone images are treated as grayscale images.
- Extra channels other than alpha are discarded.

The following defines can be defined in cmake to modify the behavior of the plugin:
- `PSD_FAST_LAB_CONVERSION`: the LAB image is converted to linear sRGB instead of sRGB which significantly increases performance.
- `PSD_NATIVE_CMYK_SUPPORT_DISABLED`: disable native support for CMYK images when compiled with Qt 6.8+

### The RAW plugin

Loading RAW images always requires a conversion. To allow the user to
choose how to convert the image, it was chosen to use the quality parameter
to act on the converter. The quality parameter can be used with values ​​from 
0 to 100 (0 = fast, 100 = maximum quality) or by setting flags to 
selectively change the conversion (see also [raw_p.h](./src/imageformats/raw_p.h)).

The default setting tries to balance quality and conversion speed.

### The XCF plugin

XCF support has the following limitations:
- XCF format up to [version 12](https://testing.developer.gimp.org/core/standards/xcf/#version-history) (no support for GIMP 3).
- The returned image is always 8-bit.
- Cannot read zlib compressed files.
- The rendered image may be slightly different (colors/transparencies) than in GIMP.
