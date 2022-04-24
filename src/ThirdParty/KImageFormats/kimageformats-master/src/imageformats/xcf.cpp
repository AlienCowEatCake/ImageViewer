/*
    xcf.cpp: A Qt 5 plug-in for reading GIMP XCF image files
    SPDX-FileCopyrightText: 2001 lignum Computing Inc. <allen@lignumcomputing.com>
    SPDX-FileCopyrightText: 2004 Melchior FRANZ <mfranz@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "xcf_p.h"

#include <QDebug>
#include <QIODevice>
#include <QImage>
#include <QLoggingCategory>
#include <QPainter>
#include <QStack>
#include <QVector>
#include <QtEndian>
#include <QColorSpace>

#include <stdlib.h>
#include <string.h>

#include "gimp_p.h"

Q_DECLARE_LOGGING_CATEGORY(XCFPLUGIN)
Q_LOGGING_CATEGORY(XCFPLUGIN, "kf.imageformats.plugins.xcf", QtWarningMsg)

const float INCHESPERMETER = (100.0f / 2.54f);

namespace
{
struct RandomTable {
    // From glibc
    static constexpr int rand_r(unsigned int *seed)
    {
        unsigned int next = *seed;
        int result = 0;

        next *= 1103515245;
        next += 12345;
        result = (unsigned int)(next / 65536) % 2048;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int)(next / 65536) % 1024;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int)(next / 65536) % 1024;

        *seed = next;

        return result;
    }

    constexpr RandomTable()
        : values{}
    {
        unsigned int next = RANDOM_SEED;

        for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
            values[i] = rand_r(&next);
        }

        for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
            int tmp{};
            int swap = i + rand_r(&next) % (RANDOM_TABLE_SIZE - i);
            tmp = values[i];
            values[i] = values[swap];
            values[swap] = tmp;
        }
    }

    int values[RANDOM_TABLE_SIZE]{};
};
} // namespace {

/*!
 * Each layer in an XCF file is stored as a matrix of
 * 64-pixel by 64-pixel images. The GIMP has a sophisticated
 * method of handling very large images as well as implementing
 * parallel processing on a tile-by-tile basis. Here, though,
 * we just read them in en-masse and store them in a matrix.
 */
typedef QVector<QVector<QImage>> Tiles;

class XCFImageFormat
{
    Q_GADGET
public:
    //! Properties which can be stored in an XCF file.
    enum PropType {
        PROP_END = 0,
        PROP_COLORMAP = 1,
        PROP_ACTIVE_LAYER = 2,
        PROP_ACTIVE_CHANNEL = 3,
        PROP_SELECTION = 4,
        PROP_FLOATING_SELECTION = 5,
        PROP_OPACITY = 6,
        PROP_MODE = 7,
        PROP_VISIBLE = 8,
        PROP_LINKED = 9,
        PROP_LOCK_ALPHA = 10,
        PROP_APPLY_MASK = 11,
        PROP_EDIT_MASK = 12,
        PROP_SHOW_MASK = 13,
        PROP_SHOW_MASKED = 14,
        PROP_OFFSETS = 15,
        PROP_COLOR = 16,
        PROP_COMPRESSION = 17,
        PROP_GUIDES = 18,
        PROP_RESOLUTION = 19,
        PROP_TATTOO = 20,
        PROP_PARASITES = 21,
        PROP_UNIT = 22,
        PROP_PATHS = 23,
        PROP_USER_UNIT = 24,
        PROP_VECTORS = 25,
        PROP_TEXT_LAYER_FLAGS = 26,
        PROP_OLD_SAMPLE_POINTS = 27,
        PROP_LOCK_CONTENT = 28,
        PROP_GROUP_ITEM = 29,
        PROP_ITEM_PATH = 30,
        PROP_GROUP_ITEM_FLAGS = 31,
        PROP_LOCK_POSITION = 32,
        PROP_FLOAT_OPACITY = 33,
        PROP_COLOR_TAG = 34,
        PROP_COMPOSITE_MODE = 35,
        PROP_COMPOSITE_SPACE = 36,
        PROP_BLEND_SPACE = 37,
        PROP_FLOAT_COLOR = 38,
        PROP_SAMPLE_POINTS = 39,
        MAX_SUPPORTED_PROPTYPE, // should always be at the end so its value is last + 1
    };
    Q_ENUM(PropType);

    //! Compression type used in layer tiles.
    enum XcfCompressionType {
        COMPRESS_INVALID = -1, /* our own */
        COMPRESS_NONE = 0,
        COMPRESS_RLE = 1,
        COMPRESS_ZLIB = 2, /* unused */
        COMPRESS_FRACTAL = 3, /* unused */
    };
    Q_ENUM(XcfCompressionType);

    enum LayerModeType {
        GIMP_LAYER_MODE_NORMAL_LEGACY,
        GIMP_LAYER_MODE_DISSOLVE,
        GIMP_LAYER_MODE_BEHIND_LEGACY,
        GIMP_LAYER_MODE_MULTIPLY_LEGACY,
        GIMP_LAYER_MODE_SCREEN_LEGACY,
        GIMP_LAYER_MODE_OVERLAY_LEGACY,
        GIMP_LAYER_MODE_DIFFERENCE_LEGACY,
        GIMP_LAYER_MODE_ADDITION_LEGACY,
        GIMP_LAYER_MODE_SUBTRACT_LEGACY,
        GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY,
        GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY,
        GIMP_LAYER_MODE_HSV_HUE_LEGACY,
        GIMP_LAYER_MODE_HSV_SATURATION_LEGACY,
        GIMP_LAYER_MODE_HSL_COLOR_LEGACY,
        GIMP_LAYER_MODE_HSV_VALUE_LEGACY,
        GIMP_LAYER_MODE_DIVIDE_LEGACY,
        GIMP_LAYER_MODE_DODGE_LEGACY,
        GIMP_LAYER_MODE_BURN_LEGACY,
        GIMP_LAYER_MODE_HARDLIGHT_LEGACY,
        GIMP_LAYER_MODE_SOFTLIGHT_LEGACY,
        GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY,
        GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY,
        GIMP_LAYER_MODE_COLOR_ERASE_LEGACY,
        GIMP_LAYER_MODE_OVERLAY,
        GIMP_LAYER_MODE_LCH_HUE,
        GIMP_LAYER_MODE_LCH_CHROMA,
        GIMP_LAYER_MODE_LCH_COLOR,
        GIMP_LAYER_MODE_LCH_LIGHTNESS,
        GIMP_LAYER_MODE_NORMAL,
        GIMP_LAYER_MODE_BEHIND,
        GIMP_LAYER_MODE_MULTIPLY,
        GIMP_LAYER_MODE_SCREEN,
        GIMP_LAYER_MODE_DIFFERENCE,
        GIMP_LAYER_MODE_ADDITION,
        GIMP_LAYER_MODE_SUBTRACT,
        GIMP_LAYER_MODE_DARKEN_ONLY,
        GIMP_LAYER_MODE_LIGHTEN_ONLY,
        GIMP_LAYER_MODE_HSV_HUE,
        GIMP_LAYER_MODE_HSV_SATURATION,
        GIMP_LAYER_MODE_HSL_COLOR,
        GIMP_LAYER_MODE_HSV_VALUE,
        GIMP_LAYER_MODE_DIVIDE,
        GIMP_LAYER_MODE_DODGE,
        GIMP_LAYER_MODE_BURN,
        GIMP_LAYER_MODE_HARDLIGHT,
        GIMP_LAYER_MODE_SOFTLIGHT,
        GIMP_LAYER_MODE_GRAIN_EXTRACT,
        GIMP_LAYER_MODE_GRAIN_MERGE,
        GIMP_LAYER_MODE_VIVID_LIGHT,
        GIMP_LAYER_MODE_PIN_LIGHT,
        GIMP_LAYER_MODE_LINEAR_LIGHT,
        GIMP_LAYER_MODE_HARD_MIX,
        GIMP_LAYER_MODE_EXCLUSION,
        GIMP_LAYER_MODE_LINEAR_BURN,
        GIMP_LAYER_MODE_LUMA_DARKEN_ONLY,
        GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY,
        GIMP_LAYER_MODE_LUMINANCE,
        GIMP_LAYER_MODE_COLOR_ERASE,
        GIMP_LAYER_MODE_ERASE,
        GIMP_LAYER_MODE_MERGE,
        GIMP_LAYER_MODE_SPLIT,
        GIMP_LAYER_MODE_PASS_THROUGH,
        GIMP_LAYER_MODE_COUNT,
    };
    Q_ENUM(LayerModeType);

    //! Type of individual layers in an XCF file.
    enum GimpImageType {
        RGB_GIMAGE,
        RGBA_GIMAGE,
        GRAY_GIMAGE,
        GRAYA_GIMAGE,
        INDEXED_GIMAGE,
        INDEXEDA_GIMAGE,
    };
    Q_ENUM(GimpImageType);

    //! Type of individual layers in an XCF file.
    enum GimpColorSpace {
        RgbLinearSpace,
        RgbPerceptualSpace,
        LabSpace,
    };

    //! Mode to use when compositing layer
    enum GimpCompositeMode {
        CompositeOver,
        CompositeUnion,
        CompositeClipBackdrop,
        CompositeClipLayer,
        CompositeIntersect,
    };

    XCFImageFormat();
    bool readXCF(QIODevice *device, QImage *image);

private:
    /*!
     * Each GIMP image is composed of one or more layers. A layer can
     * be one of any three basic types: RGB, grayscale or indexed. With an
     * optional alpha channel, there are six possible types altogether.
     *
     * Note: there is only ever one instance of this structure. The
     * layer info is discarded after it is merged into the final QImage.
     */
    class Layer
    {
    public:
        quint32 width; //!< Width of the layer
        quint32 height; //!< Height of the layer
        qint32 type; //!< Type of the layer (GimpImageType)
        char *name; //!< Name of the layer
        qint64 hierarchy_offset; //!< File position of Tile hierarchy
        qint64 mask_offset; //!< File position of mask image

        uint nrows; //!< Number of rows of tiles (y direction)
        uint ncols; //!< Number of columns of tiles (x direction)

        Tiles image_tiles; //!< The basic image
        //! For Grayscale and Indexed images, the alpha channel is stored
        //! separately (in this data structure, anyway).
        Tiles alpha_tiles;
        Tiles mask_tiles; //!< The layer mask (optional)

        //! Additional information about a layer mask.
        struct {
            quint32 opacity;
            float opacityFloat = 1.f;
            quint32 visible;
            quint32 show_masked;
            uchar red, green, blue;
            float redF, greenF, blueF; // Floats should override
            quint32 tattoo;
        } mask_channel;

        XcfCompressionType compression = COMPRESS_INVALID; //!< tile compression method (CompressionType)

        bool active; //!< Is this layer the active layer?
        quint32 opacity = 255; //!< The opacity of the layer
        float opacityFloat = 1.f; //!< The opacity of the layer, but floating point (both are set)
        quint32 visible = 1; //!< Is the layer visible?
        quint32 linked; //!< Is this layer linked (geometrically)
        quint32 preserve_transparency; //!< Preserve alpha when drawing on layer?
        quint32 apply_mask = 9; //!< Apply the layer mask? Use 9 as "uninitilized". Spec says "If the property does not appear for a layer which has a layer
                                //!< mask, it defaults to true (1).
                                //   Robust readers should force this to false if the layer has no layer mask.
        quint32 edit_mask; //!< Is the layer mask the being edited?
        quint32 show_mask; //!< Show the layer mask rather than the image?
        qint32 x_offset = 0; //!< x offset of the layer relative to the image
        qint32 y_offset = 0; //!< y offset of the layer relative to the image
        quint32 mode = 0; //!< Combining mode of layer (LayerModeEffects)
        quint32 tattoo; //!< (unique identifier?)
        qint32 blendSpace = 0; //!< What colorspace to use when blending
        qint32 compositeSpace = 0; //!< What colorspace to use when compositing
        qint32 compositeMode = 0; //!< How to composite layer (union, clip, etc.)

        //! As each tile is read from the file, it is buffered here.
        uchar tile[TILE_WIDTH * TILE_HEIGHT * sizeof(QRgb)];

        //! The data from tile buffer is copied to the Tile by this
        //! method.  Depending on the type of the tile (RGB, Grayscale,
        //! Indexed) and use (image or mask), the bytes in the buffer are
        //! copied in different ways.
        void (*assignBytes)(Layer &layer, uint i, uint j);

        Layer(void)
            : name(nullptr)
        {
        }
        ~Layer(void)
        {
            delete[] name;
        }

        Layer(const Layer &) = delete;
        Layer &operator=(const Layer &) = delete;
    };

    /*!
     * The in-memory representation of the XCF Image. It contains a few
     * metadata items, but is mostly a container for the layer information.
     */
    class XCFImage
    {
    public:
        qint32 precision = 0;
        quint32 width; //!< width of the XCF image
        quint32 height; //!< height of the XCF image
        qint32 type; //!< type of the XCF image (GimpImageBaseType)

        quint8 compression = COMPRESS_RLE; //!< tile compression method (CompressionType)
        float x_resolution = -1; //!< x resolution in dots per inch
        float y_resolution = -1; //!< y resolution in dots per inch
        qint32 tattoo; //!< (unique identifier?)
        quint32 unit; //!< Units of The GIMP (inch, mm, pica, etc...)
        qint32 num_colors = 0; //!< number of colors in an indexed image
        QVector<QRgb> palette; //!< indexed image color palette

        int num_layers; //!< number of layers
        Layer layer; //!< most recently read layer

        bool initialized; //!< Is the QImage initialized?
        QImage image; //!< final QImage

        QHash<QString,QByteArray> parasites;    //!< parasites data

        XCFImage(void)
            : initialized(false)
        {
        }
    };

    static qint64 readOffsetPtr(QDataStream &stream)
    {
        if (stream.version() >= 11) {
            qint64 ret;
            stream >> ret;
            return ret;
        } else {
            quint32 ret;
            stream >> ret;
            return ret;
        }
    }

    //! In layer DISSOLVE mode, a random number is chosen to compare to a
    //! pixel's alpha. If the alpha is greater than the random number, the
    //! pixel is drawn. This table merely contains the random number seeds
    //! for each ROW of an image. Therefore, the random numbers chosen
    //! are consistent from run to run.
    static int random_table[RANDOM_TABLE_SIZE];
    static bool random_table_initialized;

    static constexpr RandomTable randomTable{};

    //! This table is used as a shared grayscale ramp to be set on grayscale
    //! images. This is because Qt does not differentiate between indexed and
    //! grayscale images.
    static QVector<QRgb> grayTable;

    //! This table provides the add_pixel saturation values (i.e. 250 + 250 = 255).
    // static int add_lut[256][256]; - this is so lame waste of 256k of memory
    static int add_lut(int, int);

    //! The bottom-most layer is copied into the final QImage by this
    //! routine.
    typedef void (*PixelCopyOperation)(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    //! Higher layers are merged into the final QImage by this routine.
    typedef void (*PixelMergeOperation)(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    static bool modeAffectsSourceAlpha(const quint32 type);

    bool loadImageProperties(QDataStream &xcf_io, XCFImage &image);
    bool loadProperty(QDataStream &xcf_io, PropType &type, QByteArray &bytes, quint32 &rawType);
    bool loadLayer(QDataStream &xcf_io, XCFImage &xcf_image);
    bool loadLayerProperties(QDataStream &xcf_io, Layer &layer);
    bool composeTiles(XCFImage &xcf_image);
    void setGrayPalette(QImage &image);
    void setPalette(XCFImage &xcf_image, QImage &image);
    void setImageParasites(const XCFImage &xcf_image, QImage &image);
    static void assignImageBytes(Layer &layer, uint i, uint j);
    bool loadHierarchy(QDataStream &xcf_io, Layer &layer);
    bool loadLevel(QDataStream &xcf_io, Layer &layer, qint32 bpp);
    static void assignMaskBytes(Layer &layer, uint i, uint j);
    bool loadMask(QDataStream &xcf_io, Layer &layer);
    bool loadChannelProperties(QDataStream &xcf_io, Layer &layer);
    bool initializeImage(XCFImage &xcf_image);
    bool loadTileRLE(QDataStream &xcf_io, uchar *tile, int size, int data_length, qint32 bpp);

    static void copyLayerToImage(XCFImage &xcf_image);
    static void copyRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    static void mergeLayerIntoImage(XCFImage &xcf_image);
    static void mergeRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeGrayAToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void mergeIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    static void initializeRandomTable();
    static void dissolveRGBPixels(QImage &image, int x, int y);
    static void dissolveAlphaPixels(QImage &image, int x, int y);
};

int XCFImageFormat::random_table[RANDOM_TABLE_SIZE];
bool XCFImageFormat::random_table_initialized;

constexpr RandomTable XCFImageFormat::randomTable;

QVector<QRgb> XCFImageFormat::grayTable;

bool XCFImageFormat::modeAffectsSourceAlpha(const quint32 type)
{
    switch (type) {
    case GIMP_LAYER_MODE_NORMAL_LEGACY:
    case GIMP_LAYER_MODE_DISSOLVE:
    case GIMP_LAYER_MODE_BEHIND_LEGACY:
        return true;

    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
    case GIMP_LAYER_MODE_HSL_COLOR_LEGACY:
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
    case GIMP_LAYER_MODE_DODGE_LEGACY:
    case GIMP_LAYER_MODE_BURN_LEGACY:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
        return false;

    case GIMP_LAYER_MODE_COLOR_ERASE_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_LCH_HUE:
    case GIMP_LAYER_MODE_LCH_CHROMA:
    case GIMP_LAYER_MODE_LCH_COLOR:
    case GIMP_LAYER_MODE_LCH_LIGHTNESS:
        return false;

    case GIMP_LAYER_MODE_NORMAL:
        return true;

    case GIMP_LAYER_MODE_BEHIND:
    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_HSV_HUE:
    case GIMP_LAYER_MODE_HSV_SATURATION:
    case GIMP_LAYER_MODE_HSL_COLOR:
    case GIMP_LAYER_MODE_HSV_VALUE:
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_MERGE:
    case GIMP_LAYER_MODE_VIVID_LIGHT:
    case GIMP_LAYER_MODE_PIN_LIGHT:
    case GIMP_LAYER_MODE_LINEAR_LIGHT:
    case GIMP_LAYER_MODE_HARD_MIX:
    case GIMP_LAYER_MODE_EXCLUSION:
    case GIMP_LAYER_MODE_LINEAR_BURN:
    case GIMP_LAYER_MODE_LUMA_DARKEN_ONLY:
    case GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LUMINANCE:
    case GIMP_LAYER_MODE_COLOR_ERASE:
    case GIMP_LAYER_MODE_ERASE:
    case GIMP_LAYER_MODE_MERGE:
    case GIMP_LAYER_MODE_SPLIT:
    case GIMP_LAYER_MODE_PASS_THROUGH:
        return false;

    default:
        qCWarning(XCFPLUGIN) << "Unhandled layer mode" << XCFImageFormat::LayerModeType(type);
        return false;
    }
}

//! Change a QRgb value's alpha only.
inline QRgb qRgba(const QRgb rgb, int a)
{
    return ((a & 0xff) << 24 | (rgb & RGB_MASK));
}

/*!
 * The constructor for the XCF image loader.
 */
XCFImageFormat::XCFImageFormat()
{
    static_assert(sizeof(QRgb) == 4, "the code assumes sizeof(QRgb) == 4, if that's not your case, help us fix it :)");
}

/*!
 * This initializes the tables used in the layer dissolving routines.
 */
void XCFImageFormat::initializeRandomTable()
{
    // From GIMP "paint_funcs.c" v1.2
    srand(RANDOM_SEED);

    for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
        random_table[i] = rand();
    }

    for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
        int tmp;
        int swap = i + rand() % (RANDOM_TABLE_SIZE - i);
        tmp = random_table[i];
        random_table[i] = random_table[swap];
        random_table[swap] = tmp;
    }
}

inline int XCFImageFormat::add_lut(int a, int b)
{
    return qMin(a + b, 255);
}

bool XCFImageFormat::readXCF(QIODevice *device, QImage *outImage)
{
    XCFImage xcf_image;
    QDataStream xcf_io(device);

    QByteArray tag(14, '\0');

    if (xcf_io.readRawData(tag.data(), tag.size()) != tag.size()) {
        qCDebug(XCFPLUGIN) << "XCF: read failure on header tag";
        return false;
    }
    if (!tag.startsWith("gimp xcf") || !tag.endsWith('\0')) {
        qCDebug(XCFPLUGIN) << "XCF: read called on non-XCF file";
        return false;
    }

    // Remove null terminator
    tag.chop(1);

    if (tag.right(4) == "file") {
        xcf_io.setVersion(0);
    } else {
        // Version 1 and onwards use the format "gimp xcf v###" instead of "gimp xcf file"
        bool ok;
        xcf_io.setVersion(tag.right(3).toInt(&ok));
        if (!ok) {
            qCDebug(XCFPLUGIN) << "Failed to parse version" << tag;
            return false;
        }
    }
    qCDebug(XCFPLUGIN) << "version" << xcf_io.version();

    if (xcf_io.version() > 11) {
        qCDebug(XCFPLUGIN) << "Unsupported version" << xcf_io.version();
        return false;
    }

    xcf_io >> xcf_image.width >> xcf_image.height >> xcf_image.type;

    if (xcf_io.version() >= 4) {
        xcf_io >> xcf_image.precision;
        qCDebug(XCFPLUGIN) << "Precision" << xcf_image.precision;
        if (xcf_io.version() < 7) {
            qCDebug(XCFPLUGIN) << "Conversion of image precision not supported";
        }
    }

    qCDebug(XCFPLUGIN) << tag << " " << xcf_image.width << " " << xcf_image.height << " " << xcf_image.type;
    if (!loadImageProperties(xcf_io, xcf_image)) {
        return false;
    }

    // The layers appear to be stored in top-to-bottom order. This is
    // the reverse of how a merged image must be computed. So, the layer
    // offsets are pushed onto a LIFO stack (thus, we don't have to load
    // all the data of all layers before beginning to construct the
    // merged image).

    QStack<qint64> layer_offsets;

    while (true) {
        const qint64 layer_offset = readOffsetPtr(xcf_io);

        if (layer_offset == 0) {
            break;
        }

        if (layer_offset < 0) {
            qCDebug(XCFPLUGIN) << "XCF: negative layer offset";
            return false;
        }

        layer_offsets.push(layer_offset);
    }

    xcf_image.num_layers = layer_offsets.size();

    if (layer_offsets.size() == 0) {
        qCDebug(XCFPLUGIN) << "XCF: no layers!";
        return false;
    }
    qCDebug(XCFPLUGIN) << xcf_image.num_layers << "layers";

    // Load each layer and add it to the image
    while (!layer_offsets.isEmpty()) {
        qint64 layer_offset = layer_offsets.pop();

        xcf_io.device()->seek(layer_offset);

        if (!loadLayer(xcf_io, xcf_image)) {
            return false;
        }
    }

    if (!xcf_image.initialized) {
        qCDebug(XCFPLUGIN) << "XCF: no visible layers!";
        return false;
    }

    // The image was created: now I can set metadata and ICC color profile inside it.
    setImageParasites(xcf_image, xcf_image.image);

    *outImage = xcf_image.image;
    return true;
}

/*!
 * An XCF file can contain an arbitrary number of properties associated
 * with the image (and layer and mask).
 * \param xcf_io the data stream connected to the XCF image
 * \param xcf_image XCF image data.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadImageProperties(QDataStream &xcf_io, XCFImage &xcf_image)
{
    while (true) {
        PropType type;
        QByteArray bytes;
        quint32 rawType;

        if (!loadProperty(xcf_io, type, bytes, rawType)) {
            qCDebug(XCFPLUGIN) << "XCF: error loading global image properties";
            return false;
        }

        QDataStream property(bytes);

        switch (type) {
        case PROP_END:
            return true;

        case PROP_COMPRESSION:
            property >> xcf_image.compression;
            break;

        case PROP_RESOLUTION:
            property.setFloatingPointPrecision(QDataStream::SinglePrecision);
            property >> xcf_image.x_resolution >> xcf_image.y_resolution;
            break;

        case PROP_TATTOO:
            property >> xcf_image.tattoo;
            break;

        case PROP_PARASITES:
            while (!property.atEnd()) {
                char *tag;
                quint32 size;

                property.readBytes(tag, size);

                quint32 flags;
                QByteArray data;
                property >> flags >> data;

                // WARNING: you cannot add metadata to QImage here because it can be null.
                // Adding a metadata to a QImage when it is null, does nothing (metas are lost).
                if(tag) // store metadata for future use
                    xcf_image.parasites.insert(QString::fromUtf8(tag), data);

                delete[] tag;
            }
            break;

        case PROP_UNIT:
            property >> xcf_image.unit;
            break;

        case PROP_PATHS: // This property is ignored.
            break;

        case PROP_USER_UNIT: // This property is ignored.
            break;

        case PROP_COLORMAP:
            property >> xcf_image.num_colors;
            if (xcf_image.num_colors < 0 || xcf_image.num_colors > 65535) {
                return false;
            }

            xcf_image.palette = QVector<QRgb>();
            xcf_image.palette.reserve(xcf_image.num_colors);

            for (int i = 0; i < xcf_image.num_colors; i++) {
                uchar r;
                uchar g;
                uchar b;
                property >> r >> g >> b;
                xcf_image.palette.push_back(qRgb(r, g, b));
            }
            break;

        default:
            qCDebug(XCFPLUGIN) << "XCF: unimplemented image property" << type << "(" << rawType << ")"
                               << ", size " << bytes.size();
            break;
        }
    }
}

/*!
 * Read a single property from the image file. The property type is returned
 * in type and the data is returned in bytes.
 * \param xcf the image file data stream.
 * \param type returns with the property type.
 * \param bytes returns with the property data.
 * \return true if there were no IO errors.  */
bool XCFImageFormat::loadProperty(QDataStream &xcf_io, PropType &type, QByteArray &bytes, quint32 &rawType)
{
    quint32 size;

    xcf_io >> rawType;
    if (rawType >= MAX_SUPPORTED_PROPTYPE) {
        type = MAX_SUPPORTED_PROPTYPE;
        // we don't support the property, but we still need to read from the device, assume it's like all the
        // non custom properties that is data_length + data
        xcf_io >> size;
        xcf_io.skipRawData(size);
        // return true because we don't really want to totally fail on an unsupported property since it may not be fatal
        return true;
    }

    type = PropType(rawType);

    char *data = nullptr;

    // The colormap property size is not the correct number of bytes:
    // The GIMP source xcf.c has size = 4 + ncolors, but it should be
    // 4 + 3 * ncolors

    if (type == PROP_COLORMAP) {
        xcf_io >> size;
        quint32 ncolors;
        xcf_io >> ncolors;

        size = 3 * ncolors + 4;

        if (size > 65535 || size < 4) {
            return false;
        }

        data = new char[size];

        // since we already read "ncolors" from the stream, we put that data back
        data[0] = 0;
        data[1] = 0;
        data[2] = ncolors >> 8;
        data[3] = ncolors & 255;

        // ... and read the remaining bytes from the stream
        xcf_io.readRawData(data + 4, size - 4);
    } else if (type == PROP_USER_UNIT) {
        // The USER UNIT property size is not correct. I'm not sure why, though.
        float factor;
        qint32 digits;

        xcf_io >> size >> factor >> digits;

        for (int i = 0; i < 5; i++) {
            char *unit_strings;

            xcf_io >> unit_strings;

            delete[] unit_strings;

            if (xcf_io.device()->atEnd()) {
                qCDebug(XCFPLUGIN) << "XCF: read failure on property " << type;
                return false;
            }
        }

        size = 0;
    } else {
        xcf_io >> size;
        if (size > 256000) {
            return false;
        }
        data = new char[size];
        const quint32 dataRead = xcf_io.readRawData(data, size);
        if (dataRead < size) {
            qCDebug(XCFPLUGIN) << "XCF: loadProperty read less data than expected" << size << dataRead;
            memset(&data[dataRead], 0, size - dataRead);
        }
    }

    if (size != 0 && data) {
        bytes = QByteArray(data, size);
    }

    delete[] data;

    return true;
}

/*!
 * Load a layer from the XCF file. The data stream must be positioned at
 * the beginning of the layer data.
 * \param xcf_io the image file data stream.
 * \param xcf_image contains the layer and the color table
 * (if the image is indexed).
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadLayer(QDataStream &xcf_io, XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);
    delete[] layer.name;

    xcf_io >> layer.width >> layer.height >> layer.type >> layer.name;

    // Don't want to keep passing this around, dumb XCF format
    layer.compression = XcfCompressionType(xcf_image.compression);

    if (!loadLayerProperties(xcf_io, layer)) {
        return false;
    }

    qCDebug(XCFPLUGIN) << "layer: \"" << layer.name << "\", size: " << layer.width << " x " << layer.height << ", type: " << GimpImageType(layer.type)
                       << ", mode: " << layer.mode << ", opacity: " << layer.opacity << ", visible: " << layer.visible << ", offset: " << layer.x_offset << ", "
                       << layer.y_offset;

    // Skip reading the rest of it if it is not visible. Typically, when
    // you export an image from the The GIMP it flattens (or merges) only
    // the visible layers into the output image.

    if (layer.visible == 0) {
        return true;
    }

    // If there are any more layers, merge them into the final QImage.

    layer.hierarchy_offset = readOffsetPtr(xcf_io);
    layer.mask_offset = readOffsetPtr(xcf_io);

    if (layer.hierarchy_offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative layer hierarchy_offset";
        return false;
    }

    if (layer.mask_offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative layer mask_offset";
        return false;
    }

    // Allocate the individual tile QImages based on the size and type
    // of this layer.

    if (!composeTiles(xcf_image)) {
        return false;
    }
    xcf_io.device()->seek(layer.hierarchy_offset);

    // As tiles are loaded, they are copied into the layers tiles by
    // this routine. (loadMask(), below, uses a slightly different
    // version of assignBytes().)

    layer.assignBytes = assignImageBytes;

    if (!loadHierarchy(xcf_io, layer)) {
        return false;
    }

    if (layer.mask_offset != 0) {
        // 9 means its not on the file. Spec says "If the property does not appear for a layer which has a layer mask, it defaults to true (1).
        if (layer.apply_mask == 9) {
            layer.apply_mask = 1;
        }

        xcf_io.device()->seek(layer.mask_offset);

        if (!loadMask(xcf_io, layer)) {
            return false;
        }
    } else {
        // Spec says "Robust readers should force this to false if the layer has no layer mask."
        layer.apply_mask = 0;
    }

    // Now we should have enough information to initialize the final
    // QImage. The first visible layer determines the attributes
    // of the QImage.

    if (!xcf_image.initialized) {
        if (!initializeImage(xcf_image)) {
            return false;
        }
        copyLayerToImage(xcf_image);
        xcf_image.initialized = true;
    } else {
        mergeLayerIntoImage(xcf_image);
    }

    return true;
}

/*!
 * An XCF file can contain an arbitrary number of properties associated
 * with a layer.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer layer to collect the properties.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadLayerProperties(QDataStream &xcf_io, Layer &layer)
{
    while (true) {
        PropType type;
        QByteArray bytes;
        quint32 rawType;

        if (!loadProperty(xcf_io, type, bytes, rawType)) {
            qCDebug(XCFPLUGIN) << "XCF: error loading layer properties";
            return false;
        }

        QDataStream property(bytes);

        switch (type) {
        case PROP_END:
            return true;

        case PROP_ACTIVE_LAYER:
            layer.active = true;
            break;

        case PROP_OPACITY:
            property >> layer.opacity;
            layer.opacity = std::min(layer.opacity, 255u);
            break;

        case PROP_FLOAT_OPACITY:
            // For some reason QDataStream isn't able to read the float (tried
            // setting the endianness manually)
            if (bytes.size() == 4) {
                layer.opacityFloat = qFromBigEndian(*reinterpret_cast<float *>(bytes.data()));
            } else {
                qCDebug(XCFPLUGIN) << "XCF: Invalid data size for float:" << bytes.size();
            }
            break;

        case PROP_VISIBLE:
            property >> layer.visible;
            break;

        case PROP_LINKED:
            property >> layer.linked;
            break;

        case PROP_LOCK_ALPHA:
            property >> layer.preserve_transparency;
            break;

        case PROP_APPLY_MASK:
            property >> layer.apply_mask;
            break;

        case PROP_EDIT_MASK:
            property >> layer.edit_mask;
            break;

        case PROP_SHOW_MASK:
            property >> layer.show_mask;
            break;

        case PROP_OFFSETS:
            property >> layer.x_offset >> layer.y_offset;
            break;

        case PROP_MODE:
            property >> layer.mode;
            if (layer.mode >= GIMP_LAYER_MODE_COUNT) {
                qCDebug(XCFPLUGIN) << "Found layer with unsupported mode" << LayerModeType(layer.mode) << "Defaulting to mode 0";
                layer.mode = 0;
            }
            break;

        case PROP_TATTOO:
            property >> layer.tattoo;
            break;

        case PROP_COMPOSITE_SPACE:
            property >> layer.compositeSpace;
            break;

        case PROP_COMPOSITE_MODE:
            property >> layer.compositeMode;
            break;

        case PROP_BLEND_SPACE:
            property >> layer.blendSpace;
            break;

        // Just for organization in the UI, doesn't influence rendering
        case PROP_COLOR_TAG:
            break;

        // We don't support editing, so for now just ignore locking
        case PROP_LOCK_CONTENT:
        case PROP_LOCK_POSITION:
            break;

        default:
            qCDebug(XCFPLUGIN) << "XCF: unimplemented layer property " << type << "(" << rawType << ")"
                               << ", size " << bytes.size();
            break;
        }
    }
}

/*!
 * Compute the number of tiles in the current layer and allocate
 * QImage structures for each of them.
 * \param xcf_image contains the current layer.
 */
bool XCFImageFormat::composeTiles(XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);

    layer.nrows = (layer.height + TILE_HEIGHT - 1) / TILE_HEIGHT;
    layer.ncols = (layer.width + TILE_WIDTH - 1) / TILE_WIDTH;

    qCDebug(XCFPLUGIN) << "IMAGE: height=" << xcf_image.height << ", width=" << xcf_image.width;
    qCDebug(XCFPLUGIN) << "LAYER: height=" << layer.height << ", width=" << layer.width;
    qCDebug(XCFPLUGIN) << "LAYER: rows=" << layer.nrows << ", columns=" << layer.ncols;

    // SANITY CHECK: Catch corrupted XCF image file where the width or height
    // of a tile is reported are bogus. See Bug# 234030.
    if (layer.width > 32767 || layer.height > 32767 || (sizeof(void *) == 4 && layer.width * layer.height > 16384 * 16384)) {
        return false;
    }

    layer.image_tiles.resize(layer.nrows);

    if (layer.type == GRAYA_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
        layer.alpha_tiles.resize(layer.nrows);
    }

    if (layer.mask_offset != 0) {
        layer.mask_tiles.resize(layer.nrows);
    }

    for (uint j = 0; j < layer.nrows; j++) {
        layer.image_tiles[j].resize(layer.ncols);

        if (layer.type == GRAYA_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
            layer.alpha_tiles[j].resize(layer.ncols);
        }

        if (layer.mask_offset != 0) {
            layer.mask_tiles[j].resize(layer.ncols);
        }
    }

    for (uint j = 0; j < layer.nrows; j++) {
        for (uint i = 0; i < layer.ncols; i++) {
            uint tile_width = (i + 1) * TILE_WIDTH <= layer.width ? TILE_WIDTH : layer.width - i * TILE_WIDTH;

            uint tile_height = (j + 1) * TILE_HEIGHT <= layer.height ? TILE_HEIGHT : layer.height - j * TILE_HEIGHT;

            // Try to create the most appropriate QImage (each GIMP layer
            // type is treated slightly differently)

            switch (layer.type) {
            case RGB_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_RGB32);
                layer.image_tiles[j][i].setColorCount(0);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                break;

            case RGBA_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_ARGB32);
                layer.image_tiles[j][i].setColorCount(0);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                break;

            case GRAY_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.image_tiles[j][i].setColorCount(256);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.image_tiles[j][i]);
                break;

            case GRAYA_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.image_tiles[j][i].setColorCount(256);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.image_tiles[j][i]);

                layer.alpha_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.alpha_tiles[j][i].setColorCount(256);
                if (layer.alpha_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.alpha_tiles[j][i]);
                break;

            case INDEXED_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.image_tiles[j][i].setColorCount(xcf_image.num_colors);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                setPalette(xcf_image, layer.image_tiles[j][i]);
                break;

            case INDEXEDA_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.image_tiles[j][i].setColorCount(xcf_image.num_colors);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                setPalette(xcf_image, layer.image_tiles[j][i]);

                layer.alpha_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.alpha_tiles[j][i].setColorCount(256);
                if (layer.alpha_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.alpha_tiles[j][i]);
            }

            if (layer.mask_offset != 0) {
                layer.mask_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.mask_tiles[j][i].setColorCount(256);
                if (layer.mask_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.mask_tiles[j][i]);
            }
        }
    }
    return true;
}

/*!
 * Apply a grayscale palette to the QImage. Note that Qt does not distinguish
 * between grayscale and indexed images. A grayscale image is just
 * an indexed image with a 256-color, grayscale palette.
 * \param image image to set to a grayscale palette.
 */
void XCFImageFormat::setGrayPalette(QImage &image)
{
    if (grayTable.isEmpty()) {
        grayTable.resize(256);

        for (int i = 0; i < 256; i++) {
            grayTable[i] = qRgb(i, i, i);
        }
    }

    image.setColorTable(grayTable);
}

/*!
 * Copy the indexed palette from the XCF image into the QImage.
 * \param xcf_image XCF image containing the palette read from the data stream.
 * \param image image to apply the palette to.
 */
void XCFImageFormat::setPalette(XCFImage &xcf_image, QImage &image)
{
    Q_ASSERT(xcf_image.num_colors == xcf_image.palette.size());

    image.setColorTable(xcf_image.palette);
}

/*!
 * Copy the parasites info to QImage.
 * \param xcf_image XCF image containing the parasites read from the data stream.
 * \param image image to apply the parasites data.
 * \note Some comment taken from https://gitlab.gnome.org/GNOME/gimp/-/blob/master/devel-docs/parasites.txt
 */
void XCFImageFormat::setImageParasites(const XCFImage &xcf_image, QImage &image)
{
    auto&& p = xcf_image.parasites;
    auto keys = p.keys();
    for (auto&& key : qAsConst(keys)) {
        auto value = p.value(key);
        if(value.isEmpty())
            continue;

        // "icc-profile" (IMAGE, PERSISTENT | UNDOABLE)
        //     This contains an ICC profile describing the color space the
        //     image was produced in. TIFF images stored in PhotoShop do
        //     oftentimes contain embedded profiles. An experimental color
        //     manager exists to use this parasite, and it will be used
        //     for interchange between TIFF and PNG (identical profiles)
        if (key == QStringLiteral("icc-profile")) {
            auto cs = QColorSpace::fromIccProfile(value);
            if(cs.isValid())
                image.setColorSpace(cs);
            continue;
        }

        // "gimp-comment" (IMAGE, PERSISTENT)
        //    Standard GIF-style image comments.  This parasite should be
        //    human-readable text in UTF-8 encoding.  A trailing \0 might
        //    be included and is not part of the comment.  Note that image
        //    comments may also be present in the "gimp-metadata" parasite.
        if (key == QStringLiteral("gimp-comment")) {
            value.replace('\0', QByteArray());
            image.setText(QStringLiteral("Comment"), QString::fromUtf8(value));
            continue;
        }

        // "gimp-image-metadata"
        //     Saved by GIMP 2.10.30 but it is not mentioned in the specification.
        //     It is an XML block with the properties set using GIMP.
        if (key == QStringLiteral("gimp-image-metadata")) {
            // NOTE: I arbitrary defined the metadata "XML:org.gimp.xml" because it seems
            //       a GIMP proprietary XML format (no xmlns defined)
            value.replace('\0', QByteArray());
            image.setText(QStringLiteral("XML:org.gimp.xml"), QString::fromUtf8(value));
            continue;
        }

#if 0   // Unable to generate it using latest GIMP version
        // "gimp-metadata" (IMAGE, PERSISTENT)
        //     The metadata associated with the image, serialized as one XMP
        //     packet.  This metadata includes the contents of any XMP, EXIF
        //     and IPTC blocks from the original image, as well as
        //     user-specified values such as image comment, copyright,
        //     license, etc.
        if (key == QStringLiteral("gimp-metadata")) {
            // NOTE: "XML:com.adobe.xmp" is the meta set by Qt reader when an
            //       XMP packet is found (e.g. when reading a PNG saved by Photoshop).
            //       I reused the same key because some programs could search for it.
            value.replace('\0', QByteArray());
            image.setText(QStringLiteral("XML:com.adobe.xmp"), QString::fromUtf8(value));
            continue;
        }
#endif

    }
}


/*!
 * Copy the bytes from the tile buffer into the image tile QImage, taking into
 * account all the myriad different modes.
 * \param layer layer containing the tile buffer and the image tile matrix.
 * \param i column index of current tile.
 * \param j row index of current tile.
 */
void XCFImageFormat::assignImageBytes(Layer &layer, uint i, uint j)
{
    QImage &image = layer.image_tiles[j][i];
    const uchar *tile = layer.tile;
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();
    uchar *bits = image.bits();

    switch (layer.type) {
    case RGB_GIMAGE:
        for (int y = 0; y < height; y++) {
            QRgb *dataPtr = (QRgb *)(bits + y * bytesPerLine);
            for (int x = 0; x < width; x++) {
                *dataPtr++ = qRgb(tile[0], tile[1], tile[2]);
                tile += sizeof(QRgb);
            }
        }
        break;

    case RGBA_GIMAGE:
        for (int y = 0; y < height; y++) {
            QRgb *dataPtr = (QRgb *)(bits + y * bytesPerLine);
            for (int x = 0; x < width; x++) {
                *dataPtr++ = qRgba(tile[0], tile[1], tile[2], tile[3]);
                tile += sizeof(QRgb);
            }
        }
        break;

    case GRAY_GIMAGE:
    case INDEXED_GIMAGE:
        for (int y = 0; y < height; y++) {
            uchar *dataPtr = bits + y * bytesPerLine;
            for (int x = 0; x < width; x++) {
                *dataPtr++ = tile[0];
                tile += sizeof(QRgb);
            }
        }
        break;

    case GRAYA_GIMAGE:
    case INDEXEDA_GIMAGE:
        for (int y = 0; y < height; y++) {
            uchar *dataPtr = bits + y * bytesPerLine;
            uchar *alphaPtr = layer.alpha_tiles[j][i].scanLine(y);
            for (int x = 0; x < width; x++) {
                // The "if" here should not be necessary, but apparently there
                // are some cases where the image can contain larger indices
                // than there are colors in the palette. (A bug in The GIMP?)

                if (tile[0] < image.colorCount()) {
                    *dataPtr = tile[0];
                }

                *alphaPtr = tile[1];
                dataPtr += 1;
                alphaPtr += 1;
                tile += sizeof(QRgb);
            }
        }
        break;
    }
}

/*!
 * The GIMP stores images in a "mipmap"-like hierarchy. As far as the QImage
 * is concerned, however, only the top level (i.e., the full resolution image)
 * is used.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer the layer to collect the image.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadHierarchy(QDataStream &xcf_io, Layer &layer)
{
    qint32 width;
    qint32 height;
    quint32 bpp;

    xcf_io >> width >> height >> bpp;
    const qint64 offset = readOffsetPtr(xcf_io);

    qCDebug(XCFPLUGIN) << "width" << width << "height" << height << "bpp" << bpp << "offset" << offset;

    if (offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative hierarchy offset";
        return false;
    }

    const bool isMask = layer.assignBytes == assignMaskBytes;

    // make sure bpp is correct and complain if it is not
    switch (layer.type) {
    case RGB_GIMAGE:
        if (bpp != 3) {
            qCDebug(XCFPLUGIN) << "Found layer of type RGB but with bpp != 3" << bpp;

            if (!isMask) {
                return false;
            }
        }
        break;
    case RGBA_GIMAGE:
        if (bpp != 4) {
            qCDebug(XCFPLUGIN) << "Found layer of type RGBA but with bpp != 4, got" << bpp << "bpp";

            if (!isMask) {
                return false;
            }
        }
        break;
    case GRAY_GIMAGE:
        if (bpp != 1) {
            qCDebug(XCFPLUGIN) << "Found layer of type Gray but with bpp != 1" << bpp;
            return false;
        }
        break;
    case GRAYA_GIMAGE:
        if (bpp != 2) {
            qCDebug(XCFPLUGIN) << "Found layer of type Gray+Alpha but with bpp != 2" << bpp;

            if (!isMask) {
                return false;
            }
        }
        break;
    case INDEXED_GIMAGE:
        if (bpp != 1) {
            qCDebug(XCFPLUGIN) << "Found layer of type Indexed but with bpp != 1" << bpp;
            return false;
        }
        break;
    case INDEXEDA_GIMAGE:
        if (bpp != 2) {
            qCDebug(XCFPLUGIN) << "Found layer of type Indexed+Alpha but with bpp != 2" << bpp;

            if (!isMask) {
                return false;
            }
        }
        break;
    }

    if (bpp > 4) {
        qCDebug(XCFPLUGIN) << "bpp is" << bpp << "We don't support layers with bpp > 4";
        return false;
    }

    // GIMP stores images in a "mipmap"-like format (multiple levels of
    // increasingly lower resolution). Only the top level is used here,
    // however.

    quint32 junk;
    do {
        xcf_io >> junk;

        if (xcf_io.device()->atEnd()) {
            qCDebug(XCFPLUGIN) << "XCF: read failure on layer " << layer.name << " level offsets";
            return false;
        }
    } while (junk != 0);

    qint64 saved_pos = xcf_io.device()->pos();

    xcf_io.device()->seek(offset);
    if (!loadLevel(xcf_io, layer, bpp)) {
        return false;
    }

    xcf_io.device()->seek(saved_pos);
    return true;
}

/*!
 * Load one level of the image hierarchy (but only the top level is ever used).
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer the layer to collect the image.
 * \param bpp the number of bytes in a pixel.
 * \return true if there were no I/O errors.
 * \sa loadTileRLE().
 */
bool XCFImageFormat::loadLevel(QDataStream &xcf_io, Layer &layer, qint32 bpp)
{
    qint32 width;
    qint32 height;

    xcf_io >> width >> height;
    qint64 offset = readOffsetPtr(xcf_io);

    if (offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative level offset";
        return false;
    }

    if (offset == 0) {
        // offset 0 with rowsxcols != 0 is probably an error since it means we have tiles
        // without data but just clear the bits for now instead of returning false
        for (uint j = 0; j < layer.nrows; j++) {
            for (uint i = 0; i < layer.ncols; i++) {
                layer.image_tiles[j][i].fill(Qt::transparent);
                if (layer.type == GRAYA_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
                    layer.alpha_tiles[j][i].fill(Qt::transparent);
                }
            }
        }
        return true;
    }

    for (uint j = 0; j < layer.nrows; j++) {
        for (uint i = 0; i < layer.ncols; i++) {
            if (offset == 0) {
                qCDebug(XCFPLUGIN) << "XCF: incorrect number of tiles in layer " << layer.name;
                return false;
            }

            qint64 saved_pos = xcf_io.device()->pos();
            qint64 offset2 = readOffsetPtr(xcf_io);

            if (offset2 < 0) {
                qCDebug(XCFPLUGIN) << "XCF: negative level offset";
                return false;
            }

            // Evidently, RLE can occasionally expand a tile instead of compressing it!
            if (offset2 == 0) {
                offset2 = offset + (uint)(TILE_WIDTH * TILE_HEIGHT * 4 * 1.5);
            }

            xcf_io.device()->seek(offset);

            switch (layer.compression) {
            case COMPRESS_NONE: {
                if (xcf_io.version() > 11) {
                    qCDebug(XCFPLUGIN) << "Component reading not supported yet";
                    return false;
                }
                const int data_size = bpp * TILE_WIDTH * TILE_HEIGHT;
                if (data_size > int(sizeof(layer.tile))) {
                    qCDebug(XCFPLUGIN) << "Tile data too big, we can only fit" << sizeof(layer.tile) << "but need" << data_size;
                    return false;
                }
                int dataRead = xcf_io.readRawData(reinterpret_cast<char *>(layer.tile), data_size);
                if (dataRead < data_size) {
                    qCDebug(XCFPLUGIN) << "short read, expected" << data_size << "got" << dataRead;
                    return false;
                }
                break;
            }
            case COMPRESS_RLE: {
                int size = layer.image_tiles[j][i].width() * layer.image_tiles[j][i].height();
                if (!loadTileRLE(xcf_io, layer.tile, size, offset2 - offset, bpp)) {
                    return true;
                }
                break;
            }
            default:
                qCDebug(XCFPLUGIN) << "Unhandled compression" << layer.compression;
                return false;
            }

            // The bytes in the layer tile are juggled differently depending on
            // the target QImage. The caller has set layer.assignBytes to the
            // appropriate routine.

            layer.assignBytes(layer, i, j);

            xcf_io.device()->seek(saved_pos);
            offset = readOffsetPtr(xcf_io);

            if (offset < 0) {
                qCDebug(XCFPLUGIN) << "XCF: negative level offset";
                return false;
            }
        }
    }

    return true;
}

/*!
 * A layer can have a one channel image which is used as a mask.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer the layer to collect the mask image.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadMask(QDataStream &xcf_io, Layer &layer)
{
    qint32 width;
    qint32 height;
    char *name;

    xcf_io >> width >> height >> name;

    delete[] name;

    if (!loadChannelProperties(xcf_io, layer)) {
        return false;
    }

    const qint64 hierarchy_offset = readOffsetPtr(xcf_io);

    if (hierarchy_offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative mask hierarchy_offset";
        return false;
    }

    xcf_io.device()->seek(hierarchy_offset);
    layer.assignBytes = assignMaskBytes;

    if (!loadHierarchy(xcf_io, layer)) {
        return false;
    }

    return true;
}

/*!
 * This is the routine for which all the other code is simply
 * infrastructure. Read the image bytes out of the file and
 * store them in the tile buffer. This is passed a full 32-bit deep
 * buffer, even if bpp is smaller. The caller can figure out what to
 * do with the bytes.
 *
 * The tile is stored in "channels", i.e. the red component of all
 * pixels, then the green component of all pixels, then blue then
 * alpha, or, for indexed images, the color indices of all pixels then
 * the alpha of all pixels.
 *
 * The data is compressed with "run length encoding". Some simple data
 * integrity checks are made.
 *
 * \param xcf_io the data stream connected to the XCF image.
 * \param tile the buffer to expand the RLE into.
 * \param image_size number of bytes expected to be in the image tile.
 * \param data_length number of bytes expected in the RLE.
 * \param bpp number of bytes per pixel.
 * \return true if there were no I/O errors and no obvious corruption of
 * the RLE data.
 */
bool XCFImageFormat::loadTileRLE(QDataStream &xcf_io, uchar *tile, int image_size, int data_length, qint32 bpp)
{
    uchar *data;

    uchar *xcfdata;
    uchar *xcfodata;
    uchar *xcfdatalimit;

    if (data_length < 0 || data_length > int(TILE_WIDTH * TILE_HEIGHT * 4 * 1.5)) {
        qCDebug(XCFPLUGIN) << "XCF: invalid tile data length" << data_length;
        return false;
    }

    xcfdata = xcfodata = new uchar[data_length];

    const int dataRead = xcf_io.readRawData((char *)xcfdata, data_length);
    if (dataRead <= 0) {
        delete[] xcfodata;
        qCDebug(XCFPLUGIN) << "XCF: read failure on tile" << dataRead;
        return false;
    }

    if (dataRead < data_length) {
        memset(&xcfdata[dataRead], 0, data_length - dataRead);
    }

    if (!xcf_io.device()->isOpen()) {
        delete[] xcfodata;
        qCDebug(XCFPLUGIN) << "XCF: read failure on tile";
        return false;
    }

    xcfdatalimit = &xcfodata[data_length - 1];

    for (int i = 0; i < bpp; ++i) {
        data = tile + i;

        int count = 0;
        int size = image_size;

        while (size > 0) {
            if (xcfdata > xcfdatalimit) {
                goto bogus_rle;
            }

            uchar val = *xcfdata++;
            uint length = val;

            if (length >= 128) {
                length = 255 - (length - 1);
                if (length == 128) {
                    if (xcfdata >= xcfdatalimit) {
                        goto bogus_rle;
                    }

                    length = (*xcfdata << 8) + xcfdata[1];

                    xcfdata += 2;
                }

                count += length;
                size -= length;

                if (size < 0) {
                    goto bogus_rle;
                }

                if (&xcfdata[length - 1] > xcfdatalimit) {
                    goto bogus_rle;
                }

                while (length-- > 0) {
                    *data = *xcfdata++;
                    data += sizeof(QRgb);
                }
            } else {
                length += 1;
                if (length == 128) {
                    if (xcfdata >= xcfdatalimit) {
                        goto bogus_rle;
                    }

                    length = (*xcfdata << 8) + xcfdata[1];
                    xcfdata += 2;
                }

                count += length;
                size -= length;

                if (size < 0) {
                    goto bogus_rle;
                }

                if (xcfdata > xcfdatalimit) {
                    goto bogus_rle;
                }

                val = *xcfdata++;

                while (length-- > 0) {
                    *data = val;
                    data += sizeof(QRgb);
                }
            }
        }
    }

    delete[] xcfodata;
    return true;

bogus_rle:

    qCDebug(XCFPLUGIN) << "The run length encoding could not be decoded properly";
    delete[] xcfodata;
    return false;
}

/*!
 * An XCF file can contain an arbitrary number of properties associated
 * with a channel. Note that this routine only reads mask channel properties.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer layer containing the mask channel to collect the properties.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadChannelProperties(QDataStream &xcf_io, Layer &layer)
{
    while (true) {
        PropType type;
        QByteArray bytes;
        quint32 rawType;

        if (!loadProperty(xcf_io, type, bytes, rawType)) {
            qCDebug(XCFPLUGIN) << "XCF: error loading channel properties";
            return false;
        }

        QDataStream property(bytes);

        switch (type) {
        case PROP_END:
            return true;

        case PROP_OPACITY:
            property >> layer.mask_channel.opacity;
            layer.mask_channel.opacity = std::min(layer.mask_channel.opacity, 255u);
            break;

        case PROP_FLOAT_OPACITY:
            // For some reason QDataStream isn't able to read the float (tried
            // setting the endianness manually)
            if (bytes.size() == 4) {
                layer.mask_channel.opacityFloat = qFromBigEndian(*reinterpret_cast<float *>(bytes.data()));
            } else {
                qCDebug(XCFPLUGIN) << "XCF: Invalid data size for float:" << bytes.size();
            }
            break;

        case PROP_VISIBLE:
            property >> layer.mask_channel.visible;
            break;

        case PROP_SHOW_MASKED:
            property >> layer.mask_channel.show_masked;
            break;

        case PROP_COLOR:
            property >> layer.mask_channel.red >> layer.mask_channel.green >> layer.mask_channel.blue;
            break;

        case PROP_FLOAT_COLOR:
            property >> layer.mask_channel.redF >> layer.mask_channel.greenF >> layer.mask_channel.blueF;
            break;

        case PROP_TATTOO:
            property >> layer.mask_channel.tattoo;
            break;

        // Only used in edit mode
        case PROP_LINKED:
            break;

        // Just for organization in the UI, doesn't influence rendering
        case PROP_COLOR_TAG:
            break;

        // We don't support editing, so for now just ignore locking
        case PROP_LOCK_CONTENT:
        case PROP_LOCK_POSITION:
            break;

        default:
            qCDebug(XCFPLUGIN) << "XCF: unimplemented channel property " << type << "(" << rawType << ")"
                               << ", size " << bytes.size();
            break;
        }
    }
}

/*!
 * Copy the bytes from the tile buffer into the mask tile QImage.
 * \param layer layer containing the tile buffer and the mask tile matrix.
 * \param i column index of current tile.
 * \param j row index of current tile.
 */
void XCFImageFormat::assignMaskBytes(Layer &layer, uint i, uint j)
{
    QImage &image = layer.mask_tiles[j][i];
    uchar *tile = layer.tile;
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();
    uchar *bits = image.bits();

    for (int y = 0; y < height; y++) {
        uchar *dataPtr = bits + y * bytesPerLine;
        for (int x = 0; x < width; x++) {
            *dataPtr++ = tile[0];
            tile += sizeof(QRgb);
        }
    }
}

/*!
 * Construct the QImage which will eventually be returned to the QImage
 * loader.
 *
 * There are a couple of situations which require that the QImage is not
 * exactly the same as The GIMP's representation. The full table is:
 * \verbatim
 *  Grayscale  opaque      :  8 bpp indexed
 *  Grayscale  translucent : 32 bpp + alpha
 *  Indexed    opaque      :  1 bpp if num_colors <= 2
 *                         :  8 bpp indexed otherwise
 *  Indexed    translucent :  8 bpp indexed + alpha if num_colors < 256
 *                         : 32 bpp + alpha otherwise
 *  RGB        opaque      : 32 bpp
 *  RGBA       translucent : 32 bpp + alpha
 * \endverbatim
 * Whether the image is translucent or not is determined by the bottom layer's
 * alpha channel. However, even if the bottom layer lacks an alpha channel,
 * it can still have an opacity < 1. In this case, the QImage is promoted
 * to 32-bit. (Note this is different from the output from the GIMP image
 * exporter, which seems to ignore this attribute.)
 *
 * Independently, higher layers can be translucent, but the background of
 * the image will not show through if the bottom layer is opaque.
 *
 * For indexed images, translucency is an all or nothing effect.
 * \param xcf_image contains image info and bottom-most layer.
 */
bool XCFImageFormat::initializeImage(XCFImage &xcf_image)
{
    // (Aliases to make the code look a little better.)
    Layer &layer(xcf_image.layer);
    QImage &image(xcf_image.image);

    switch (layer.type) {
    case RGB_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY) {
            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_RGB32);
            if (image.isNull()) {
                return false;
            }
            image.fill(qRgb(255, 255, 255));
            break;
        } // else, fall through to 32-bit representation
        Q_FALLTHROUGH();
    case RGBA_GIMAGE:
        image = QImage(xcf_image.width, xcf_image.height, QImage::Format_ARGB32);
        if (image.isNull()) {
            return false;
        }
        image.fill(qRgba(255, 255, 255, 0));
        break;

    case GRAY_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY) {
            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_Indexed8);
            image.setColorCount(256);
            if (image.isNull()) {
                return false;
            }
            setGrayPalette(image);
            image.fill(255);
            break;
        } // else, fall through to 32-bit representation
        Q_FALLTHROUGH();
    case GRAYA_GIMAGE:
        image = QImage(xcf_image.width, xcf_image.height, QImage::Format_ARGB32);
        if (image.isNull()) {
            return false;
        }
        image.fill(qRgba(255, 255, 255, 0));
        break;

    case INDEXED_GIMAGE:
        // As noted in the table above, there are quite a few combinations
        // which are possible with indexed images, depending on the
        // presence of transparency (note: not translucency, which is not
        // supported by The GIMP for indexed images) and the number of
        // individual colors.

        // Note: Qt treats a bitmap with a Black and White color palette
        // as a mask, so only the "on" bits are drawn, regardless of the
        // order color table entries. Otherwise (i.e., at least one of the
        // color table entries is not black or white), it obeys the one-
        // or two-color palette. Have to ask about this...

        if (xcf_image.num_colors <= 2) {
            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_MonoLSB);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        } else if (xcf_image.num_colors <= 256) {
            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_Indexed8);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        }
        break;

    case INDEXEDA_GIMAGE:
        if (xcf_image.num_colors == 1) {
            // Plenty(!) of room to add a transparent color
            xcf_image.num_colors++;
            xcf_image.palette.resize(xcf_image.num_colors);
            xcf_image.palette[1] = xcf_image.palette[0];
            xcf_image.palette[0] = qRgba(255, 255, 255, 0);

            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_MonoLSB);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        } else if (xcf_image.num_colors < 256) {
            // Plenty of room to add a transparent color
            xcf_image.num_colors++;
            xcf_image.palette.resize(xcf_image.num_colors);
            for (int c = xcf_image.num_colors - 1; c >= 1; c--) {
                xcf_image.palette[c] = xcf_image.palette[c - 1];
            }

            xcf_image.palette[0] = qRgba(255, 255, 255, 0);
            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_Indexed8);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        } else {
            // No room for a transparent color, so this has to be promoted to
            // true color. (There is no equivalent PNG representation output
            // from The GIMP as of v1.2.)
            image = QImage(xcf_image.width, xcf_image.height, QImage::Format_ARGB32);
            if (image.isNull()) {
                return false;
            }
            image.fill(qRgba(255, 255, 255, 0));
        }
        break;
    }

    if (xcf_image.x_resolution > 0 && xcf_image.y_resolution > 0) {
        const float dpmx = xcf_image.x_resolution * INCHESPERMETER;
        if (dpmx > std::numeric_limits<int>::max()) {
            return false;
        }
        const float dpmy = xcf_image.y_resolution * INCHESPERMETER;
        if (dpmy > std::numeric_limits<int>::max()) {
            return false;
        }
        image.setDotsPerMeterX((int)dpmx);
        image.setDotsPerMeterY((int)dpmy);
    }
    return true;
}

/*!
 * Copy a layer into an image, taking account of the manifold modes. The
 * contents of the image are replaced.
 * \param xcf_image contains the layer and image to be replaced.
 */
void XCFImageFormat::copyLayerToImage(XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);
    QImage &image(xcf_image.image);
    PixelCopyOperation copy = nullptr;

    switch (layer.type) {
    case RGB_GIMAGE:
    case RGBA_GIMAGE:
        copy = copyRGBToRGB;
        break;
    case GRAY_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY) {
            copy = copyGrayToGray;
        } else {
            copy = copyGrayToRGB;
        }
        break;
    case GRAYA_GIMAGE:
        copy = copyGrayAToRGB;
        break;
    case INDEXED_GIMAGE:
        copy = copyIndexedToIndexed;
        break;
    case INDEXEDA_GIMAGE:
        if (xcf_image.image.depth() <= 8) {
            copy = copyIndexedAToIndexed;
        } else {
            copy = copyIndexedAToRGB;
        }
    }

    if (!copy) {
        return;
    }

    // For each tile...

    for (uint j = 0; j < layer.nrows; j++) {
        uint y = j * TILE_HEIGHT;

        for (uint i = 0; i < layer.ncols; i++) {
            uint x = i * TILE_WIDTH;

            // This seems the best place to apply the dissolve because it
            // depends on the global position of each tile's
            // pixels. Apparently it's the only mode which can apply to a
            // single layer.

            if (layer.mode == DISSOLVE_MODE) {
                if (!random_table_initialized) {
                    initializeRandomTable();
                    random_table_initialized = true;
                }
                if (layer.type == RGBA_GIMAGE) {
                    dissolveRGBPixels(layer.image_tiles[j][i], x, y);
                }

                else if (layer.type == GRAYA_GIMAGE) {
                    dissolveAlphaPixels(layer.alpha_tiles[j][i], x, y);
                }
            }

            // Shortcut for common case
            if (copy == copyRGBToRGB && layer.apply_mask != 1) {
                QPainter painter(&image);
                painter.setOpacity(layer.opacity / 255.0);
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.drawImage(x + layer.x_offset, y + layer.y_offset, layer.image_tiles[j][i]);
                continue;
            }

            for (int l = 0; l < layer.image_tiles[j][i].height(); l++) {
                for (int k = 0; k < layer.image_tiles[j][i].width(); k++) {
                    int m = x + k + layer.x_offset;
                    int n = y + l + layer.y_offset;

                    if (m < 0 || m >= image.width() || n < 0 || n >= image.height()) {
                        continue;
                    }

                    (*copy)(layer, i, j, k, l, image, m, n);
                }
            }
        }
    }
}

/*!
 * Copy an RGB pixel from the layer to the RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.opacity;

    if (layer.type == RGBA_GIMAGE) {
        src_a = INT_MULT(src_a, qAlpha(src));
    }

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Copy a Gray pixel from the layer to the Gray image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
}

/*!
 * Copy a Gray pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.opacity;
    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Copy a GrayA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Copy an Indexed pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
}

/*!
 * Copy an IndexedA pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    uchar src = layer.image_tiles[j][i].pixelIndex(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    if (src_a > 127) {
        src++;
    } else {
        src = 0;
    }

    image.setPixel(m, n, src);
}

/*!
 * Copy an IndexedA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)
    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    // This is what appears in the GIMP window
    if (src_a <= 127) {
        src_a = 0;
    } else {
        src_a = OPAQUE_OPACITY;
    }

    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Merge a layer into an image, taking account of the manifold modes.
 * \param xcf_image contains the layer and image to merge.
 */
void XCFImageFormat::mergeLayerIntoImage(XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);
    QImage &image(xcf_image.image);

    PixelMergeOperation merge = nullptr;

    if (!layer.opacity) {
        return; // don't bother doing anything
    }
    XCFImageFormat::GimpColorSpace blendSpace = XCFImageFormat::RgbLinearSpace;
    switch (layer.blendSpace) {
    case 0:
        layer.blendSpace = XCFImageFormat::RgbLinearSpace;
        break;
    case 1:
        layer.blendSpace = XCFImageFormat::RgbPerceptualSpace;
        break;
    case 2:
        layer.blendSpace = XCFImageFormat::LabSpace;
        break;
    default:
        if (layer.blendSpace < 0) {
            qCDebug(XCFPLUGIN) << "Auto blendspace not handled" << layer.blendSpace;
        } else {
            qCDebug(XCFPLUGIN) << "Unknown blend space" << layer.blendSpace;
        }
        break;
    }

    if (blendSpace != XCFImageFormat::RgbLinearSpace) {
        qCDebug(XCFPLUGIN) << "Unimplemented blend color space" << blendSpace;
    }

    if (layer.compositeSpace < 0) {
        qCDebug(XCFPLUGIN) << "Auto composite space not handled" << layer.compositeSpace;
    }
    XCFImageFormat::GimpColorSpace compositeSpace = XCFImageFormat::RgbLinearSpace;
    switch (qAbs(layer.compositeSpace)) {
    case 0:
        layer.compositeSpace = XCFImageFormat::RgbLinearSpace;
        break;
    case 1:
        layer.compositeSpace = XCFImageFormat::RgbPerceptualSpace;
        break;
    case 2:
        layer.compositeSpace = XCFImageFormat::LabSpace;
        break;
    default:
        qCDebug(XCFPLUGIN) << "Unknown composite space" << layer.compositeSpace;
        break;
    }

    if (compositeSpace != XCFImageFormat::RgbLinearSpace) {
        qCDebug(XCFPLUGIN) << "Unimplemented composite color space" << compositeSpace;
    }

    if (layer.compositeMode < 0) {
        qCDebug(XCFPLUGIN) << "Auto composite mode not handled" << layer.compositeMode;
    }

    XCFImageFormat::GimpCompositeMode compositeMode = XCFImageFormat::CompositeOver;
    switch (qAbs(layer.compositeMode)) {
    case 0:
        compositeMode = XCFImageFormat::CompositeOver;
        break;
    case 1:
        compositeMode = XCFImageFormat::CompositeUnion;
        break;
    case 2:
        compositeMode = XCFImageFormat::CompositeClipBackdrop;
        break;
    case 3:
        compositeMode = XCFImageFormat::CompositeClipLayer;
        break;
    case 4:
        compositeMode = XCFImageFormat::CompositeIntersect;
        break;
    default:
        qCDebug(XCFPLUGIN) << "Unknown composite mode" << layer.compositeMode;
        break;
    }
    if (compositeMode != XCFImageFormat::CompositeOver) {
        qCDebug(XCFPLUGIN) << "Unhandled composite mode" << compositeMode;
    }

    switch (layer.type) {
    case RGB_GIMAGE:
    case RGBA_GIMAGE:
        merge = mergeRGBToRGB;
        break;
    case GRAY_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY) {
            merge = mergeGrayToGray;
        } else {
            merge = mergeGrayToRGB;
        }
        break;
    case GRAYA_GIMAGE:
        if (xcf_image.image.depth() <= 8) {
            merge = mergeGrayAToGray;
        } else {
            merge = mergeGrayAToRGB;
        }
        break;
    case INDEXED_GIMAGE:
        merge = mergeIndexedToIndexed;
        break;
    case INDEXEDA_GIMAGE:
        if (xcf_image.image.depth() <= 8) {
            merge = mergeIndexedAToIndexed;
        } else {
            merge = mergeIndexedAToRGB;
        }
    }

    if (!merge) {
        return;
    }

    for (uint j = 0; j < layer.nrows; j++) {
        uint y = j * TILE_HEIGHT;

        for (uint i = 0; i < layer.ncols; i++) {
            uint x = i * TILE_WIDTH;

            // This seems the best place to apply the dissolve because it
            // depends on the global position of each tile's
            // pixels. Apparently it's the only mode which can apply to a
            // single layer.

            if (layer.mode == DISSOLVE_MODE) {
                if (!random_table_initialized) {
                    initializeRandomTable();
                    random_table_initialized = true;
                }
                if (layer.type == RGBA_GIMAGE) {
                    dissolveRGBPixels(layer.image_tiles[j][i], x, y);
                }

                else if (layer.type == GRAYA_GIMAGE) {
                    dissolveAlphaPixels(layer.alpha_tiles[j][i], x, y);
                }
            }

            // Shortcut for common case
            if (merge == mergeRGBToRGB && layer.apply_mask != 1 && layer.mode == NORMAL_MODE) {
                QPainter painter(&image);
                painter.setOpacity(layer.opacity / 255.0);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.drawImage(x + layer.x_offset, y + layer.y_offset, layer.image_tiles[j][i]);
                continue;
            }

            for (int l = 0; l < layer.image_tiles[j][i].height(); l++) {
                for (int k = 0; k < layer.image_tiles[j][i].width(); k++) {
                    int m = x + k + layer.x_offset;
                    int n = y + l + layer.y_offset;

                    if (m < 0 || m >= image.width() || n < 0 || n >= image.height()) {
                        continue;
                    }

                    (*merge)(layer, i, j, k, l, image, m, n);
                }
            }
        }
    }
}

/*!
 * Merge an RGB pixel from the layer to the RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    QRgb dst = image.pixel(m, n);

    uchar src_r = qRed(src);
    uchar src_g = qGreen(src);
    uchar src_b = qBlue(src);
    uchar src_a = qAlpha(src);

    uchar dst_r = qRed(dst);
    uchar dst_g = qGreen(dst);
    uchar dst_b = qBlue(dst);
    uchar dst_a = qAlpha(dst);

    if (!src_a) {
        return; // nothing to merge
    }

    switch (layer.mode) {
    case GIMP_LAYER_MODE_NORMAL:
    case GIMP_LAYER_MODE_NORMAL_LEGACY:
        break;
    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
        src_r = INT_MULT(src_r, dst_r);
        src_g = INT_MULT(src_g, dst_g);
        src_b = INT_MULT(src_b, dst_b);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
        src_r = qMin((dst_r * 256) / (1 + src_r), 255);
        src_g = qMin((dst_g * 256) / (1 + src_g), 255);
        src_b = qMin((dst_b * 256) / (1 + src_b), 255);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
        src_r = 255 - INT_MULT(255 - dst_r, 255 - src_r);
        src_g = 255 - INT_MULT(255 - dst_g, 255 - src_g);
        src_b = 255 - INT_MULT(255 - dst_b, 255 - src_b);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
        src_r = INT_MULT(dst_r, dst_r + INT_MULT(2 * src_r, 255 - dst_r));
        src_g = INT_MULT(dst_g, dst_g + INT_MULT(2 * src_g, 255 - dst_g));
        src_b = INT_MULT(dst_b, dst_b + INT_MULT(2 * src_b, 255 - dst_b));
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
        src_r = dst_r > src_r ? dst_r - src_r : src_r - dst_r;
        src_g = dst_g > src_g ? dst_g - src_g : src_g - dst_g;
        src_b = dst_b > src_b ? dst_b - src_b : src_b - dst_b;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
        src_r = add_lut(dst_r, src_r);
        src_g = add_lut(dst_g, src_g);
        src_b = add_lut(dst_b, src_b);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
        src_r = dst_r > src_r ? dst_r - src_r : 0;
        src_g = dst_g > src_g ? dst_g - src_g : 0;
        src_b = dst_b > src_b ? dst_b - src_b : 0;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
        src_r = dst_r < src_r ? dst_r : src_r;
        src_g = dst_g < src_g ? dst_g : src_g;
        src_b = dst_b < src_b ? dst_b : src_b;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
        src_r = dst_r < src_r ? src_r : dst_r;
        src_g = dst_g < src_g ? src_g : dst_g;
        src_b = dst_b < src_b ? src_b : dst_b;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHSV(src_r, src_g, src_b);
        RGBTOHSV(new_r, new_g, new_b);

        new_r = src_r;

        HSVTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHSV(src_r, src_g, src_b);
        RGBTOHSV(new_r, new_g, new_b);

        new_g = src_g;

        HSVTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHSV(src_r, src_g, src_b);
        RGBTOHSV(new_r, new_g, new_b);

        new_b = src_b;

        HSVTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HSL_COLOR_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHLS(src_r, src_g, src_b);
        RGBTOHLS(new_r, new_g, new_b);

        new_r = src_r;
        new_b = src_b;

        HLSTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_DODGE_LEGACY: {
        uint tmp;

        tmp = dst_r << 8;
        tmp /= 256 - src_r;
        src_r = (uchar)qMin(tmp, 255u);

        tmp = dst_g << 8;
        tmp /= 256 - src_g;
        src_g = (uchar)qMin(tmp, 255u);

        tmp = dst_b << 8;
        tmp /= 256 - src_b;
        src_b = (uchar)qMin(tmp, 255u);

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_BURN_LEGACY: {
        uint tmp;

        tmp = (255 - dst_r) << 8;
        tmp /= src_r + 1;
        src_r = (uchar)qMin(tmp, 255u);
        src_r = 255 - src_r;

        tmp = (255 - dst_g) << 8;
        tmp /= src_g + 1;
        src_g = (uchar)qMin(tmp, 255u);
        src_g = 255 - src_g;

        tmp = (255 - dst_b) << 8;
        tmp /= src_b + 1;
        src_b = (uchar)qMin(tmp, 255u);
        src_b = 255 - src_b;

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY: {
        uint tmp;
        if (src_r > 128) {
            tmp = ((int)255 - dst_r) * ((int)255 - ((src_r - 128) << 1));
            src_r = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst_r * ((int)src_r << 1);
            src_r = (uchar)qMin(tmp >> 8, 255u);
        }

        if (src_g > 128) {
            tmp = ((int)255 - dst_g) * ((int)255 - ((src_g - 128) << 1));
            src_g = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst_g * ((int)src_g << 1);
            src_g = (uchar)qMin(tmp >> 8, 255u);
        }

        if (src_b > 128) {
            tmp = ((int)255 - dst_b) * ((int)255 - ((src_b - 128) << 1));
            src_b = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst_b * ((int)src_b << 1);
            src_b = (uchar)qMin(tmp >> 8, 255u);
        }
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY: {
        uint tmpS;
        uint tmpM;

        tmpM = INT_MULT(dst_r, src_r);
        tmpS = 255 - INT_MULT((255 - dst_r), (255 - src_r));
        src_r = INT_MULT((255 - dst_r), tmpM) + INT_MULT(dst_r, tmpS);

        tmpM = INT_MULT(dst_g, src_g);
        tmpS = 255 - INT_MULT((255 - dst_g), (255 - src_g));
        src_g = INT_MULT((255 - dst_g), tmpM) + INT_MULT(dst_g, tmpS);

        tmpM = INT_MULT(dst_b, src_b);
        tmpS = 255 - INT_MULT((255 - dst_b), (255 - src_b));
        src_b = INT_MULT((255 - dst_b), tmpM) + INT_MULT(dst_b, tmpS);

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY: {
        int tmp;

        tmp = dst_r - src_r + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_r = (uchar)tmp;

        tmp = dst_g - src_g + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_g = (uchar)tmp;

        tmp = dst_b - src_b + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_b = (uchar)tmp;

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY: {
        int tmp;

        tmp = dst_r + src_r - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_r = (uchar)tmp;

        tmp = dst_g + src_g - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_g = (uchar)tmp;

        tmp = dst_b + src_b - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_b = (uchar)tmp;

        src_a = qMin(src_a, dst_a);
    } break;
    default:
        break;
    }

    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    uchar new_r;
    uchar new_g;
    uchar new_b;
    uchar new_a;
    new_a = dst_a + INT_MULT(OPAQUE_OPACITY - dst_a, src_a);

    const float src_ratio = new_a == 0 ? 1.0 : (float)src_a / new_a;
    float dst_ratio = 1.0 - src_ratio;

    new_r = (uchar)(src_ratio * src_r + dst_ratio * dst_r + EPSILON);
    new_g = (uchar)(src_ratio * src_g + dst_ratio * dst_g + EPSILON);
    new_b = (uchar)(src_ratio * src_b + dst_ratio * dst_b + EPSILON);

    if (!modeAffectsSourceAlpha(layer.mode)) {
        new_a = dst_a;
    }

    image.setPixel(m, n, qRgba(new_r, new_g, new_b, new_a));
}

/*!
 * Merge a Gray pixel from the layer to the Gray image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
}

/*!
 * Merge a GrayA pixel from the layer to the Gray image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeGrayAToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = qGray(layer.image_tiles[j][i].pixel(k, l));
    int dst = image.pixelIndex(m, n);

    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);

    if (!src_a) {
        return; // nothing to merge
    }

    switch (layer.mode) {
    case MULTIPLY_MODE: {
        src = INT_MULT(src, dst);
    } break;
    case DIVIDE_MODE: {
        src = qMin((dst * 256) / (1 + src), 255);
    } break;
    case SCREEN_MODE: {
        src = 255 - INT_MULT(255 - dst, 255 - src);
    } break;
    case OVERLAY_MODE: {
        src = INT_MULT(dst, dst + INT_MULT(2 * src, 255 - dst));
    } break;
    case DIFFERENCE_MODE: {
        src = dst > src ? dst - src : src - dst;
    } break;
    case ADDITION_MODE: {
        src = add_lut(dst, src);
    } break;
    case SUBTRACT_MODE: {
        src = dst > src ? dst - src : 0;
    } break;
    case DARKEN_ONLY_MODE: {
        src = dst < src ? dst : src;
    } break;
    case LIGHTEN_ONLY_MODE: {
        src = dst < src ? src : dst;
    } break;
    case DODGE_MODE: {
        uint tmp = dst << 8;
        tmp /= 256 - src;
        src = (uchar)qMin(tmp, 255u);
    } break;
    case BURN_MODE: {
        uint tmp = (255 - dst) << 8;
        tmp /= src + 1;
        src = (uchar)qMin(tmp, 255u);
        src = 255 - src;
    } break;
    case HARDLIGHT_MODE: {
        uint tmp;
        if (src > 128) {
            tmp = ((int)255 - dst) * ((int)255 - ((src - 128) << 1));
            src = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst * ((int)src << 1);
            src = (uchar)qMin(tmp >> 8, 255u);
        }
    } break;
    case SOFTLIGHT_MODE: {
        uint tmpS;
        uint tmpM;

        tmpM = INT_MULT(dst, src);
        tmpS = 255 - INT_MULT((255 - dst), (255 - src));
        src = INT_MULT((255 - dst), tmpM) + INT_MULT(dst, tmpS);

    } break;
    case GRAIN_EXTRACT_MODE: {
        int tmp;

        tmp = dst - src + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
    } break;
    case GRAIN_MERGE_MODE: {
        int tmp;

        tmp = dst + src - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
    } break;
    }

    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    uchar new_a = OPAQUE_OPACITY;

    const float src_ratio = new_a == 0 ? 1.0 : (float)src_a / new_a;
    float dst_ratio = 1.0 - src_ratio;

    uchar new_g = (uchar)(src_ratio * src + dst_ratio * dst + EPSILON);

    image.setPixel(m, n, new_g);
}

/*!
 * Merge a Gray pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.opacity;
    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Merge a GrayA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = qGray(layer.image_tiles[j][i].pixel(k, l));
    int dst = qGray(image.pixel(m, n));

    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    uchar dst_a = qAlpha(image.pixel(m, n));

    if (!src_a) {
        return; // nothing to merge
    }

    switch (layer.mode) {
    case MULTIPLY_MODE: {
        src = INT_MULT(src, dst);
        src_a = qMin(src_a, dst_a);
    } break;
    case DIVIDE_MODE: {
        src = qMin((dst * 256) / (1 + src), 255);
        src_a = qMin(src_a, dst_a);
    } break;
    case SCREEN_MODE: {
        src = 255 - INT_MULT(255 - dst, 255 - src);
        src_a = qMin(src_a, dst_a);
    } break;
    case OVERLAY_MODE: {
        src = INT_MULT(dst, dst + INT_MULT(2 * src, 255 - dst));
        src_a = qMin(src_a, dst_a);
    } break;
    case DIFFERENCE_MODE: {
        src = dst > src ? dst - src : src - dst;
        src_a = qMin(src_a, dst_a);
    } break;
    case ADDITION_MODE: {
        src = add_lut(dst, src);
        src_a = qMin(src_a, dst_a);
    } break;
    case SUBTRACT_MODE: {
        src = dst > src ? dst - src : 0;
        src_a = qMin(src_a, dst_a);
    } break;
    case DARKEN_ONLY_MODE: {
        src = dst < src ? dst : src;
        src_a = qMin(src_a, dst_a);
    } break;
    case LIGHTEN_ONLY_MODE: {
        src = dst < src ? src : dst;
        src_a = qMin(src_a, dst_a);
    } break;
    case DODGE_MODE: {
        uint tmp = dst << 8;
        tmp /= 256 - src;
        src = (uchar)qMin(tmp, 255u);
        src_a = qMin(src_a, dst_a);
    } break;
    case BURN_MODE: {
        uint tmp = (255 - dst) << 8;
        tmp /= src + 1;
        src = (uchar)qMin(tmp, 255u);
        src = 255 - src;
        src_a = qMin(src_a, dst_a);
    } break;
    case HARDLIGHT_MODE: {
        uint tmp;
        if (src > 128) {
            tmp = ((int)255 - dst) * ((int)255 - ((src - 128) << 1));
            src = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst * ((int)src << 1);
            src = (uchar)qMin(tmp >> 8, 255u);
        }
        src_a = qMin(src_a, dst_a);
    } break;
    case SOFTLIGHT_MODE: {
        uint tmpS;
        uint tmpM;

        tmpM = INT_MULT(dst, src);
        tmpS = 255 - INT_MULT((255 - dst), (255 - src));
        src = INT_MULT((255 - dst), tmpM) + INT_MULT(dst, tmpS);

        src_a = qMin(src_a, dst_a);
    } break;
    case GRAIN_EXTRACT_MODE: {
        int tmp;

        tmp = dst - src + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
        src_a = qMin(src_a, dst_a);
    } break;
    case GRAIN_MERGE_MODE: {
        int tmp;

        tmp = dst + src - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
        src_a = qMin(src_a, dst_a);
    } break;
    }

    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)
    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    uchar new_a = dst_a + INT_MULT(OPAQUE_OPACITY - dst_a, src_a);

    const float src_ratio = new_a == 0 ? 1.0 : (float)src_a / new_a;
    float dst_ratio = 1.0 - src_ratio;

    uchar new_g = (uchar)(src_ratio * src + dst_ratio * dst + EPSILON);

    if (!modeAffectsSourceAlpha(layer.mode)) {
        new_a = dst_a;
    }

    image.setPixel(m, n, qRgba(new_g, new_g, new_g, new_a));
}

/*!
 * Merge an Indexed pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
}

/*!
 * Merge an IndexedA pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    uchar src = layer.image_tiles[j][i].pixelIndex(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    if (src_a > 127) {
        src++;
        image.setPixel(m, n, src);
    }
}

/*!
 * Merge an IndexedA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::mergeIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)
    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    // This is what appears in the GIMP window
    if (src_a <= 127) {
        src_a = 0;
    } else {
        src_a = OPAQUE_OPACITY;
    }

    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Dissolving pixels: pick a random number between 0 and 255. If the pixel's
 * alpha is less than that, make it transparent.
 * \param image the image tile to dissolve.
 * \param x the global x position of the tile.
 * \param y the global y position of the tile.
 */
void XCFImageFormat::dissolveRGBPixels(QImage &image, int x, int y)
{
    // The apparently spurious rand() calls are to wind the random
    // numbers up to the same point for each tile.

    for (int l = 0; l < image.height(); l++) {
        unsigned int next = randomTable.values[(l + y) % RANDOM_TABLE_SIZE];

        for (int k = 0; k < x; k++) {
            RandomTable::rand_r(&next);
        }

        for (int k = 0; k < image.width(); k++) {
            int rand_val = RandomTable::rand_r(&next) & 0xff;
            QRgb pixel = image.pixel(k, l);

            if (rand_val > qAlpha(pixel)) {
                image.setPixel(k, l, qRgba(pixel, 0));
            }
        }
    }
}

/*!
 * Dissolving pixels: pick a random number between 0 and 255. If the pixel's
 * alpha is less than that, make it transparent. This routine works for
 * the GRAYA and INDEXEDA image types where the pixel alpha's are stored
 * separately from the pixel themselves.
 * \param image the alpha tile to dissolve.
 * \param x the global x position of the tile.
 * \param y the global y position of the tile.
 */
void XCFImageFormat::dissolveAlphaPixels(QImage &image, int x, int y)
{
    // The apparently spurious rand() calls are to wind the random
    // numbers up to the same point for each tile.

    for (int l = 0; l < image.height(); l++) {
        unsigned int next = randomTable.values[(l + y) % RANDOM_TABLE_SIZE];

        for (int k = 0; k < x; k++) {
            RandomTable::rand_r(&next);
        }

        for (int k = 0; k < image.width(); k++) {
            int rand_val = RandomTable::rand_r(&next) & 0xff;
            uchar alpha = image.pixelIndex(k, l);

            if (rand_val > alpha) {
                image.setPixel(k, l, 0);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

XCFHandler::XCFHandler()
{
}

bool XCFHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("xcf");
        return true;
    }
    return false;
}

bool XCFHandler::read(QImage *image)
{
    XCFImageFormat xcfif;
    return xcfif.readXCF(device(), image);
}

bool XCFHandler::write(const QImage &)
{
    return false;
}

bool XCFHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCDebug(XCFPLUGIN) << "XCFHandler::canRead() called with no device";
        return false;
    }

    qint64 oldPos = device->pos();

    char head[8];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0) {
                device->ungetChar(head[readBytes-- - 1]);
            }
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0) {
            device->ungetChar(head[readBytes-- - 1]);
        }
    } else {
        device->seek(oldPos);
    }

    return qstrncmp(head, "gimp xcf", 8) == 0;
}

QImageIOPlugin::Capabilities XCFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "xcf") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && XCFHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *XCFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new XCFHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

// Just so I can get enum values printed
#include "xcf.moc"
