/// @note This is modified QMovie code.

#if !defined(KIMAGEFORMATS_MOVIE_H_INCLUDED)
#define KIMAGEFORMATS_MOVIE_H_INCLUDED

#include <QMovie>
#include <QScopedPointer>
#include "KImageFormatsImageReader.h"

QT_BEGIN_NAMESPACE

class QColor;
class QIODevice;
class QImage;
class QPixmap;
class QRect;
class QSize;

class KImageFormatsMovie : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int speed READ speed WRITE setSpeed)
    Q_PROPERTY(QMovie::CacheMode cacheMode READ cacheMode WRITE setCacheMode)

public:
    explicit KImageFormatsMovie(QObject *parent = Q_NULLPTR);
    explicit KImageFormatsMovie(QIODevice *device, const QByteArray &format = QByteArray(), QObject *parent = Q_NULLPTR);
    explicit KImageFormatsMovie(const QString &fileName, const QByteArray &format = QByteArray(), QObject *parent = Q_NULLPTR);
    ~KImageFormatsMovie();

    static QList<QByteArray> supportedFormats();

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    QMovie::MovieState state() const;

    QRect frameRect() const;
    QImage currentImage() const;
    QPixmap currentPixmap() const;

    bool isValid() const;
    QImageReader::ImageReaderError lastError() const;
    QString lastErrorString() const;

    bool jumpToFrame(int frameNumber);
    int loopCount() const;
    int frameCount() const;
    int nextFrameDelay() const;
    int currentFrameNumber() const;

    int speed() const;

    QSize scaledSize();
    void setScaledSize(const QSize &size);

    QMovie::CacheMode cacheMode() const;
    void setCacheMode(QMovie::CacheMode mode);

Q_SIGNALS:
    void started();
    void resized(const QSize &size);
    void updated(const QRect &rect);
    void stateChanged(QMovie::MovieState state);
    void error(QImageReader::ImageReaderError error);
    void finished();
    void frameChanged(int frameNumber);

public Q_SLOTS:
    void start();
    bool jumpToNextFrame();
    void setPaused(bool paused);
    void stop();
    void setSpeed(int percentSpeed);

private Q_SLOTS:
    void loadNextFrame();
    void loadNextFrame(bool starting);

private:
    Q_DISABLE_COPY(KImageFormatsMovie)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

QT_END_NAMESPACE

#endif // KIMAGEFORMATS_MOVIE_H_INCLUDED
