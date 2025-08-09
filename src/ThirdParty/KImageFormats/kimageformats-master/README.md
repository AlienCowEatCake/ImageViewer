# KImageFormats

Plugins to allow [`QImage`](https://doc.qt.io/qt-6/qimage.html) to support
extra file formats.

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
- Interchange Format Files (iff, ilbm, lbm)
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
- DirectDraw Surface (dds)
- Encapsulated PostScript (eps)
- High Efficiency Image File Format (heif)
- JPEG 2000 (jp2, j2k, jpf)
- JPEG XL (jxl)
- JPEG XR (jxr)
- OpenEXR (exr)
- Personal Computer Exchange (pcx)
- Quite OK Image format (qoi)
- SGI images (rgb, rgba, sgi, bw)
- Softimage PIC (pic)
- Targa (tga): supports more formats than Qt's version

## Contributing

See the [`QImageIOPlugin`](https://doc.qt.io/qt-6/qimageioplugin.html)
documentation for information on how to write a new plugin.

The main difference between this framework and the image formats of Qt is
the license. As such, if you write an image format plugin and you are
willing to sign the Qt Project contributor agreement, it may be better to
submit the plugin directly to the Qt Project.

To be accepted, contributions must:
- Contain the test images needed to verify that the changes work correctly.
- Pass the tests successfully.

For more info about tests, see also [Autotests README](autotests/README.md).

## Duplicated Plugins

### The TGA plugin

The TGA plugin supports more formats than Qt's own TGA plugin;
specifically, the one provided here supports indexed, greyscale and RLE
images (types 1-3 and 9-11), while Qt's plugin only supports type 2
(RGB) files.

The code for this cannot be contributed upstream directly because of
licensing.  If anyone were willing to write fresh code to improve Qt's
TGA plugin, it would allow the TGA plugin in this framework to be
removed.

### The DDS plugin

The DDS plugin is a fork from Qt 5.6 with bug fixes and improvements.

The plugin was forked because Qt Project no longer supports its DDS plugin.

### The JP2 plugin

The JP2 plugin is based on the popular and wide used OpenJPEG library.

The Qt project has a no longer supported JPEG 2000 plugin based on Jasper.

## License

This framework is licensed under the
[LGPLv2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html#SEC1).

The CMake code in this framework is licensed under the
[BSD license](http://opensource.org/licenses/BSD-3-Clause).

## Plugin status

The current implementation of a plugin may not be complete or may have
limitations of various kinds. Typically the limitations are on maximum size
and color depth.

The various plugins are also limited by the formats natively supported by Qt.
For example, native support for CMYK images is only available since Qt 6.8.

### HDR images

HDR images are supported via floating point image formats from DDS, EXR, HDR,
JXL, JXR, PFM and PSD plugins.
It is important to note that in the past these plugins stripped away HDR
information, returning SDR images.

HDR images return R, G and B values ​​outside the range 0.0 - 1.0.
While Qt painters handles HDR data correctly, some older programs may display
strange artifacts if they do not use a tone mapping operator (or at least a
clamp). This is not a plugin issue.

### Metadata

Metadata support is available in formats that include it via 
`QImage::setText()` and `QImage::text()`. To ensure consistent metadata 
functionality, the following keys have been adopted.

About the image:
- `Altitude`: Floating-point number indicating the GPS altitude in meters 
  above sea level (e.g. 35.4).
- `Author`: Person who created the image.
- `Comment`: Additional image information in human-readable form, for 
  example a verbal description of the image.
- `Copyright`: Copyright notice of the person or organization that claims 
  the copyright to the image.
- `CreationDate`: When the image was created or captured. Date and time in 
  ISO 8601 format without milliseconds (e.g. 2024-03-23T15:30:43). This value
  should be kept unchanged when present.
- `Description`: A string that describes the subject of the image.
- `Direction`: Floating-point number indicating the direction of the image
  when it was captured in degrees (e.g. 123.3).
- `DocumentName`: The name of the document from which this image was 
  scanned.
- `HostComputer`: The computer and/or operating system in use at the time 
  of image creation.
- `Latitude`: Floating-point number indicating the latitude in degrees 
  north of the equator (e.g. 27.717).
- `Longitude`: Floating-point number indicating the longitude in degrees 
  east of Greenwich (e.g. 85.317).
- `ModificationDate`: Last modification date and time in ISO 8601 format 
  without milliseconds (e.g. 2024-03-23T15:30:43). This value should be 
  updated every time the image is saved.
- `Owner`: Name of the owner of the image.
- `Software`: Name and version number of the software package(s) used to 
  create the image.
- `Title`: The title of the image.

About the camera:
- `Manufacturer`: The manufacturer of the recording equipment.
- `Model`: The model name or model number of the recording equipment.
- `SerialNumber`: The serial number of the recording equipment.

About the lens:
- `LensManufacturer`: The manufacturer of the interchangeable lens that was 
  used.
- `LensModel`: The model name or model number of the lens that was used.
- `LensSerialNumber`: The serial number of the interchangeable lens that was 
  used.

Complex metadata (requires a parser):
- `XML:org.gimp.xml`: XML metadata generated by GIMP and present only in XCF 
  files.
- `XML:com.adobe.xmp`: [Extensible Metadata Platform (XMP)](https://developer.adobe.com/xmp/docs/)
  is the metadata standard used by Adobe applications and is supported by all 
  common image formats. **Note that XMP metadata is read and written by 
  plugins as is.** Since it may contain information present in other metadata 
  (e.g. `Description`), it is the user's responsibility to ensure consistency 
  between all metadata and XMP metadata when writing an image.

Supported metadata may vary from one plugin to another. Please note that only
the most common metadata are supported and some plugins may return keys not 
listed here.

### EXIF Metadata

[EXIF (Exchangeable Image File Format)](https://en.wikipedia.org/wiki/Exif) 
metadata is a standard for embedding information within the image file itself.

Unlike the metadata described above, EXIF ​​metadata is used internally by some 
plugins to standardize image handling. For example, the JXL plugin uses them 
to **set/get the image resolution and metadata**. They are also needed to 
make the image properties appear in the file details of some file managers 
(e.g. Dolphin).

When reading, EXIF meta​​data is converted into simple metadata (e.g. 
`Description`) and inserted into the image if and only if it is not already 
present.

On writing, the image metadata is converted to EXIF ​​and saved appropriately.
Note that, if not present in the image to be saved, the following metadata 
are created automatically:

- `Software`: Created using `applicationName` and `applicationVersion` methods
  of [`QCoreApplication`](https://doc.qt.io/qt-6/qcoreapplication.html).
- `CreationDate`: Set to current time and date.
- `ModificationDate`: Set to current time and date.

### ICC profile support

ICC profile support is implemented in all formats that handle them using
[`QColorSpace`](https://doc.qt.io/qt-6/qcolorspace.html). When saving, some 
plugins convert the image using color profiles according to format 
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
- DDS: n/a
- EXR: 300,000 x 300,000 pixels
- EPS: n/a
- HDR: n/a (large image)
- HEIF: n/a
- IFF: 65,535 x 65,535 pixels
- JP2: 300,000 x 300,000 pixels, in any case no larger than 2 gigapixels
- JXL: 262,144 x 262,144 pixels, in any case no larger than 256 megapixels
- JXR: n/a, in any case no larger than 4 GB
- KRA: same size as Qt's PNG plugin
- ORA: same size as Qt's PNG plugin
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

JP2, JXL, JXR, PSD and SCT plugins natively support 4-channel CMYK images when
compiled with Qt 6.8+.

### The DDS plugin

**This plugin can be disabled by setting `KIMAGEFORMATS_DDS` to `OFF` 
in your cmake options.**

The following defines can be defined in cmake to modify the behavior of the 
plugin:
- `DDS_DISABLE_STRIDE_ALIGNMENT`: disable the stride aligment based on DDS 
  pitch: it is known that some writers do not set it correctly.

### The HEIF plugin

**This plugin is disabled by default. It can be enabled by settings
`KIMAGEFORMATS_HEIF` to `ON` in your cmake options.**

The plugin is disabled due to issues with the heif library on certain
distributions. In particular, it is necessary that the HEIF library has
support for HEVC codec. If HEVC codec is not available the plugin
will compile but will fail the tests.

### The EXR plugin

The following defines can be defined in cmake to modify the behavior of the 
plugin:
- `EXR_CONVERT_TO_SRGB`: the linear data is converted to sRGB on read to 
  accommodate programs that do not support color profiles.
- `EXR_DISABLE_XMP_ATTRIBUTE`: disables the stores XMP values in a non-standard 
  attribute named "xmp". Note that Gimp reads the "xmp" attribute and Darktable 
  writes it as well.

### The EPS plugin

The plugin uses `Ghostscript` to convert the raster image. When reading it
converts the EPS to PPM and uses the Qt PPM plugin to read the image.
When writing it uses [`QPrinter`](https://doc.qt.io/qt-6/qprinter.html) to
create a temporary PDF file which is then converted to EPS. Therefore, if
`Ghostscript` is not installed, the plugin will not work.

### The HDR plugin

The following defines can be defined in cmake to modify the behavior of the 
plugin:
- `HDR_HALF_QUALITY`: on read, a 16-bit float image is returned instead of a 
  32-bit float one.


### The IFF plugin

Interchange File Format is a chunk-based format. Since the original 1985
version, various extensions have been created over time.

The plugin supports the following image data:
- FORM ILBM (Interleaved Bitmap): Electronic Arts’ IFF standard for
  Interchange File Format (EA IFF 1985). ILBM is a format to handle raster
  images, specifically an InterLeaved bitplane BitMap image with color map.
  It supports from 1 to 8-bit indexed images with HAM, Halfbride, and normal
  encoding. It also supports interleaved 24-bit RGB and 32-bit RGBA 
  extension without color map.
- FORM ILBM 64: ILBM extension to support 48-bit RGB and 64-bit RGBA encoding.
- FORM ACBM (Amiga Contiguous BitMap): It supports uncompressed ACBMs by 
  converting them to ILBMs at runtime.
- FORM RGBN / RGB8: It supports 13-bit and 25-bit RGB images with compression 
  type 4.
- FORM PBM: PBM is a chunky version of IFF pictures. It supports 8-bit images 
  with color map only.
- FOR4 CIMG (Maya Image File Format): It supports 24/48-bit RGB and 32/64-bit 
  RGBA images.

The plugin does not load images with non-standard SHAM/CTBL chunks due to the
lack of clear specifications.


### The JP2 plugin

**This plugin can be disabled by setting `KIMAGEFORMATS_JP2` to `OFF` 
in your cmake options.**

JP2 plugin has the following limitations due to the lack of support by OpenJPEG:
- Metadata are not supported.
- Image resolution is not supported.
- To write ICC profiles you need OpenJPEG V2.5.4 or higher

### The JXL plugin

**The current version of the plugin limits the image size to 256 megapixels
according to feature level 5 of the JXL stream encoding.**

The following defines can be defined in cmake to modify the behavior of the
plugin:
- `JXL_HDR_PRESERVATION_DISABLED`: disable floating point images (both read 
  and write) by converting them to UINT16 images. Any HDR data is lost. Note 
  that FP images are always disabled when compiling with libJXL less than v0.9.
- `JXL_DECODE_BOXES_DISABLED`: disable reading of metadata (e.g. XMP).

### The JXR plugin

**This plugin is disabled by default. It can be enabled by settings
`KIMAGEFORMATS_JXR` to `ON` in your cmake options.**

The following defines can be defined in cmake to modify the behavior of the 
plugin:
- `JXR_DENY_FLOAT_IMAGE`: disables the use of float images and consequently 
  any HDR data will be lost.
- `JXR_DISABLE_DEPTH_CONVERSION`: remove the neeeds of additional memory by 
  disabling the conversion between different color depths (e.g. RGBA64bpp to 
  RGBA32bpp) at the cost of reduced compatibility.
- `JXR_DISABLE_BGRA_HACK`: Windows displays and opens JXR files correctly out 
  of the box. Unfortunately it doesn't seem to open (P)RGBA @32bpp files as 
  it only wants (P)BGRA32bpp files (a format not supported by Qt). Only for 
  this format an hack is activated to guarantee total compatibility of the 
  plugin with Windows.
- `JXR_ENABLE_ADVANCED_METADATA`: enable metadata support (e.g. XMP). Some 
  distributions use an incomplete JXR library that does not allow reading 
  metadata, causing compilation errors.

### The KRA plugin

The KRA format is a ZIP archive containing image data. In particular, the
rendered image in PNG format is saved in the root: the plugin reads this 
image.

### The ORA plugin

The ORA format is a ZIP archive containing image data. In particular, the
rendered image in PNG format is saved in the root: the plugin reads this 
image.

### The PSD plugin

PSD support has the following limitations:
- Only images saved by Photoshop using compatibility mode enabled (Photoshop 
  default) can be decoded.
- Multichannel images are treated as CMYK if they have 2 or more channels.
- Multichannel images are treated as Grayscale if they have 1 channel.
- Duotone images are treated as grayscale images.
- Extra channels other than alpha are discarded.

The following defines can be defined in cmake to modify the behavior of the 
plugin:
- `PSD_FAST_LAB_CONVERSION`: the LAB image is converted to linear sRGB instead
  of sRGB which significantly increases performance.
- `PSD_NATIVE_CMYK_SUPPORT_DISABLED`: disable native support for CMYK images 
  when compiled with Qt 6.8+

### The RAW plugin

Loading RAW images always requires a conversion. To allow the user to
choose how to convert the image, it was chosen to use the quality parameter
to act on the converter. The quality parameter can be used with values ​​from 
0 to 100 (0 = fast, 100 = maximum quality) or by setting flags to 
selectively change the conversion (see also [raw_p.h](./src/imageformats/raw_p.h)).

The default setting tries to balance quality and conversion speed.

### The XCF plugin

XCF support has the following limitations:
- XCF format up to [version 12](https://testing.developer.gimp.org/core/standards/xcf/#version-history) 
  (no support for GIMP 3).
- The returned image is always 8-bit.
- Cannot read zlib compressed files.
- The rendered image may be slightly different (colors/transparencies) than 
  in GIMP.
