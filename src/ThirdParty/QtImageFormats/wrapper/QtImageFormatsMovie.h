/// @note This is modified QMovie code.

#if !defined(QTIMAGEFORMATS_MOVIE_H_INCLUDED)
#define QTIMAGEFORMATS_MOVIE_H_INCLUDED

#include <QMovie>
#include <QScopedPointer>
#include "QtImageFormatsImageReader.h"

QT_BEGIN_NAMESPACE

class QColor;
class QIODevice;
class QImage;
class QPixmap;
class QRect;
class QSize;

class QtImageFormatsMovie : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int speed READ speed WRITE setSpeed)
    Q_PROPERTY(QMovie::CacheMode cacheMode READ cacheMode WRITE setCacheMode)

public:
    explicit QtImageFormatsMovie(QObject *parent = Q_NULLPTR);
    explicit QtImageFormatsMovie(QIODevice *device, const QByteArray &format = QByteArray(), QObject *parent = Q_NULLPTR);
    explicit QtImageFormatsMovie(const QString &fileName, const QByteArray &format = QByteArray(), QObject *parent = Q_NULLPTR);
    ~QtImageFormatsMovie();

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
    Q_DISABLE_COPY(QtImageFormatsMovie)
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

QT_END_NAMESPACE

#endif // QTIMAGEFORMATS_MOVIE_H_INCLUDED
