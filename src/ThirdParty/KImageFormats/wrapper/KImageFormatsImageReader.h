/// @note This is modified QImageReader code.

#if !defined(KIMAGEFORMATS_IMAGEREADER_H_INCLUDED)
#define KIMAGEFORMATS_IMAGEREADER_H_INCLUDED

#include <QImageReader>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

class QIODevice;

class KImageFormatsImageReader
{
public:
    KImageFormatsImageReader();
    explicit KImageFormatsImageReader(QIODevice *device, const QByteArray &format = QByteArray());
    explicit KImageFormatsImageReader(const QString &fileName, const QByteArray &format = QByteArray());
    ~KImageFormatsImageReader();

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setAutoDetectImageFormat(bool enabled);
    bool autoDetectImageFormat() const;

    void setDecideFormatFromContent(bool ignored);
    bool decideFormatFromContent() const;

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    QSize size() const;

    QImage::Format imageFormat() const;

    QStringList textKeys() const;
    QString text(const QString &key) const;

    void setClipRect(const QRect &rect);
    QRect clipRect() const;

    void setScaledSize(const QSize &size);
    QSize scaledSize() const;

    void setQuality(int quality);
    int quality() const;

    void setScaledClipRect(const QRect &rect);
    QRect scaledClipRect() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    bool supportsAnimation() const;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
    QImageIOHandler::Transformations transformation() const;
    void setAutoTransform(bool enabled);
    bool autoTransform() const;
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    void setGamma(float gamma);
    float gamma() const;
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
    QByteArray subType() const;
    QList<QByteArray> supportedSubTypes() const;
#endif

    bool canRead() const;
    QImage read();
    bool read(QImage *image);

    bool jumpToNextImage();
    bool jumpToImage(int imageNumber);
    int loopCount() const;
    int imageCount() const;
    int nextImageDelay() const;
    int currentImageNumber() const;
    QRect currentImageRect() const;

    QImageReader::ImageReaderError error() const;
    QString errorString() const;

    bool supportsOption(QImageIOHandler::ImageOption option) const;

    static QByteArray imageFormat(const QString &fileName);
    static QByteArray imageFormat(QIODevice *device);
    static QList<QByteArray> supportedImageFormats();
    static QList<QByteArray> supportedMimeTypes();

private:
    Q_DISABLE_COPY(KImageFormatsImageReader)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

QT_END_NAMESPACE

#endif // KIMAGEFORMATS_IMAGEREADER_H_INCLUDED
