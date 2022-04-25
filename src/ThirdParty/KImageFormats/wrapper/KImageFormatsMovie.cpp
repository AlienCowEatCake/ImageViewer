/// @note This is modified QMovie code.

#include "KImageFormatsMovie.h"

#include <QtGlobal>
#include <QImage>
#include <QPixmap>
#include <QRect>
#include <QDateTime>
#include <QTimer>
#include <QPair>
#include <QMap>
#include <QList>
#include <QBuffer>
#include <QDir>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#include <QElapsedTimer>
#endif

#define QMOVIE_INVALID_DELAY -1

QT_BEGIN_NAMESPACE

class KImageFormatsFrameInfo
{
public:
    QPixmap pixmap;
    int delay;
    bool endMark;

    inline KImageFormatsFrameInfo(bool endMark)
        : pixmap(QPixmap())
        , delay(QMOVIE_INVALID_DELAY)
        , endMark(endMark)
    {}

    inline KImageFormatsFrameInfo()
        : pixmap(QPixmap())
        , delay(QMOVIE_INVALID_DELAY)
        , endMark(false)
    {}

    inline KImageFormatsFrameInfo(const QPixmap &pixmap, int delay)
        : pixmap(pixmap)
        , delay(delay)
        , endMark(false)
    {}

    inline bool isValid()
    {
        return endMark || !(pixmap.isNull() && (delay == QMOVIE_INVALID_DELAY));
    }

    inline bool isEndMarker()
    {
        return endMark;
    }

    static inline KImageFormatsFrameInfo endMarker()
    {
        return KImageFormatsFrameInfo(true);
    }
};

struct KImageFormatsMovie::Impl
{
    Impl(KImageFormatsMovie *movie);
    bool isDone();
    bool next();
    int speedAdjustedDelay(int delay) const;
    bool isValid() const;
    bool jumpToFrame(int frameNumber);
    int frameCount() const;
    bool jumpToNextFrame();
    KImageFormatsFrameInfo infoForFrame(int frameNumber);
    void reset();

    inline void enterState(QMovie::MovieState newState)
    {
        movieState = newState;
        emit movie->stateChanged(newState);
    }

    // private slots
    void loadNextFrame();
    void loadNextFrame(bool starting);

    KImageFormatsMovie *movie;

    KImageFormatsImageReader *reader;
    int speed;
    QMovie::MovieState movieState;
    QRect frameRect;
    QPixmap currentPixmap;
    int currentFrameNumber;
    int nextFrameNumber;
    int greatestFrameNumber;
    int nextDelay;
    int playCounter;
    qint64 initialDevicePos;
    QMovie::CacheMode cacheMode;
    bool haveReadAll;
    bool isFirstIteration;
    QMap<int, KImageFormatsFrameInfo> frameMap;
    QString absoluteFilePath;

    QTimer nextImageTimer;
};

KImageFormatsMovie::Impl::Impl(KImageFormatsMovie *movie)
    : movie(movie)
    , reader(Q_NULLPTR)
    , speed(100)
    , movieState(QMovie::NotRunning)
    , currentFrameNumber(-1)
    , nextFrameNumber(0)
    , greatestFrameNumber(-1)
    , nextDelay(0)
    , playCounter(-1)
    , cacheMode(QMovie::CacheNone)
    , haveReadAll(false)
    , isFirstIteration(true)
{
    nextImageTimer.setSingleShot(true);
}

void KImageFormatsMovie::Impl::reset()
{
    nextImageTimer.stop();
    if(reader->device())
        initialDevicePos = reader->device()->pos();
    currentFrameNumber = -1;
    nextFrameNumber = 0;
    greatestFrameNumber = -1;
    nextDelay = 0;
    playCounter = -1;
    haveReadAll = false;
    isFirstIteration = true;
    frameMap.clear();
}

bool KImageFormatsMovie::Impl::isDone()
{
    return (playCounter == 0);
}

int KImageFormatsMovie::Impl::speedAdjustedDelay(int delay) const
{
    return int((qint64(delay) * qint64(100) ) / qint64(speed));
}

KImageFormatsFrameInfo KImageFormatsMovie::Impl::infoForFrame(int frameNumber)
{
    if(frameNumber < 0)
        return KImageFormatsFrameInfo(); // Invalid

    if(haveReadAll && (frameNumber > greatestFrameNumber))
    {
        if(frameNumber == greatestFrameNumber+1)
            return KImageFormatsFrameInfo::endMarker();
        return KImageFormatsFrameInfo(); // Invalid
    }

    if(cacheMode == QMovie::CacheNone)
    {
        if(frameNumber != currentFrameNumber+1)
        {
            // Non-sequential frame access
            if(!reader->jumpToImage(frameNumber))
            {
                if(frameNumber == 0)
                {
                    // Special case: Attempt to "rewind" so we can loop
                    // ### This could be implemented as QImageReader::rewind()
                    if(reader->device()->isSequential())
                        return KImageFormatsFrameInfo(); // Invalid
                    QString fileName = reader->fileName();
                    QByteArray format = reader->format();
                    QIODevice *device = reader->device();
                    QColor bgColor = reader->backgroundColor();
                    QSize scaledSize = reader->scaledSize();
                    delete reader;
                    if(fileName.isEmpty())
                        reader = new KImageFormatsImageReader(device, format);
                    else
                        reader = new KImageFormatsImageReader(absoluteFilePath, format);
                    if(!reader->canRead()) // Provoke a device->open() call
                        emit movie->error(reader->error());
                    reader->device()->seek(initialDevicePos);
                    reader->setBackgroundColor(bgColor);
                    reader->setScaledSize(scaledSize);
                }
                else
                {
                    return KImageFormatsFrameInfo(); // Invalid
                }
            }
        }
        if(reader->canRead())
        {
            // reader says we can read. Attempt to actually read image
            QImage anImage = reader->read();
            if(anImage.isNull())
            {
                // Reading image failed.
                return KImageFormatsFrameInfo(); // Invalid
            }
            if(frameNumber > greatestFrameNumber)
                greatestFrameNumber = frameNumber;
            QPixmap aPixmap = QPixmap::fromImage(anImage);
            int aDelay = reader->nextImageDelay();
            return KImageFormatsFrameInfo(aPixmap, aDelay);
        }
        else if(frameNumber != 0)
        {
            // We've read all frames now. Return an end marker
            haveReadAll = true;
            return KImageFormatsFrameInfo::endMarker();
        }
        else
        {
            // No readable frames
            haveReadAll = true;
            return KImageFormatsFrameInfo();
        }
    }

    // CacheMode == CacheAll
    if(frameNumber > greatestFrameNumber)
    {
        // Frame hasn't been read from file yet. Try to do it
        for(int i = greatestFrameNumber + 1; i <= frameNumber; ++i)
        {
            if (reader->canRead())
            {
                // reader says we can read. Attempt to actually read image
                QImage anImage = reader->read();
                if(anImage.isNull())
                {
                    // Reading image failed.
                    return KImageFormatsFrameInfo(); // Invalid
                }
                greatestFrameNumber = i;
                QPixmap aPixmap = QPixmap::fromImage(anImage);
                int aDelay = reader->nextImageDelay();
                KImageFormatsFrameInfo info(aPixmap, aDelay);
                // Cache it!
                frameMap.insert(i, info);
                if(i == frameNumber)
                    return info;
            }
            else
            {
                // We've read all frames now. Return an end marker
                haveReadAll = true;
                return KImageFormatsFrameInfo::endMarker();
            }
        }
    }
    // Return info for requested (cached) frame
    return frameMap.value(frameNumber);
}

bool KImageFormatsMovie::Impl::next()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QElapsedTimer time;
#else
    QTime time;
#endif
    time.start();
    KImageFormatsFrameInfo info = infoForFrame(nextFrameNumber);
    if(!info.isValid())
        return false;
    if(info.isEndMarker())
    {
        // We reached the end of the animation.
        if(isFirstIteration)
        {
            if(nextFrameNumber == 0)
            {
                // No frames could be read at all (error).
                return false;
            }
            // End of first iteration. Initialize play counter
            playCounter = reader->loopCount();
            isFirstIteration = false;
        }
        // Loop as appropriate
        if(playCounter != 0)
        {
            if(playCounter != -1) // Infinite?
                playCounter--;    // Nope
            nextFrameNumber = 0;
            return next();
        }
        // Loop no more. Done
        return false;
    }
    // Image and delay OK, update internal state
    currentFrameNumber = nextFrameNumber++;
    QSize scaledSize = reader->scaledSize();
    if(scaledSize.isValid() && (scaledSize != info.pixmap.size()))
        currentPixmap = QPixmap::fromImage(info.pixmap.toImage().scaled(scaledSize));
    else
        currentPixmap = info.pixmap;

    if(!speed)
        return true;

    nextDelay = speedAdjustedDelay(info.delay);
    // Adjust delay according to the time it took to read the frame
    int processingTime = time.elapsed();
    if(processingTime > nextDelay)
        nextDelay = 0;
    else
        nextDelay = nextDelay - processingTime;
    return true;
}

void KImageFormatsMovie::Impl::loadNextFrame()
{
    loadNextFrame(false);
}

void KImageFormatsMovie::Impl::loadNextFrame(bool starting)
{
    if(next())
    {
        if(starting && movieState == QMovie::NotRunning)
        {
            enterState(QMovie::Running);
            emit movie->started();
        }

        if(frameRect.size() != currentPixmap.rect().size())
        {
            frameRect = currentPixmap.rect();
            emit movie->resized(frameRect.size());
        }

        emit movie->updated(frameRect);
        emit movie->frameChanged(currentFrameNumber);

        if(speed && movieState == QMovie::Running)
            nextImageTimer.start(nextDelay);
    }
    else
    {
        // Could not read another frame
        if(!isDone())
            emit movie->error(reader->error());

        // Graceful finish
        if(movieState != QMovie::Paused)
        {
            nextFrameNumber = 0;
            isFirstIteration = true;
            playCounter = -1;
            enterState(QMovie::NotRunning);
            emit movie->finished();
        }
    }
}

bool KImageFormatsMovie::Impl::isValid() const
{
    if(greatestFrameNumber >= 0)
        return true; // have we seen valid data
    bool canRead = reader->canRead();
    if(!canRead)
    {
        // let the consumer know it's broken
        //
        // ### the const_cast here is ugly, but 'const' of this method is
        // technically wrong right now, since it may cause the underlying device
        // to open.
        emit const_cast<KImageFormatsMovie*>(movie)->error(reader->error());
    }
    return canRead;
}

bool KImageFormatsMovie::Impl::jumpToFrame(int frameNumber)
{
    if(frameNumber < 0)
        return false;
    if(currentFrameNumber == frameNumber)
        return true;
    nextFrameNumber = frameNumber;
    if (movieState == QMovie::Running)
        nextImageTimer.stop();
    loadNextFrame();
    return (nextFrameNumber == currentFrameNumber+1);
}

int KImageFormatsMovie::Impl::frameCount() const
{
    int result;
    if((result = reader->imageCount()) != 0)
        return result;
    if(haveReadAll)
        return greatestFrameNumber+1;
    return 0; // Don't know
}

bool KImageFormatsMovie::Impl::jumpToNextFrame()
{
    return jumpToFrame(currentFrameNumber+1);
}

KImageFormatsMovie::KImageFormatsMovie(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    m_impl->reader = new KImageFormatsImageReader;
    connect(&m_impl->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
}

KImageFormatsMovie::KImageFormatsMovie(QIODevice *device, const QByteArray &format, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    m_impl->reader = new KImageFormatsImageReader(device, format);
    m_impl->initialDevicePos = device->pos();
    connect(&m_impl->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
}

KImageFormatsMovie::KImageFormatsMovie(const QString &fileName, const QByteArray &format, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    m_impl->absoluteFilePath = QDir(fileName).absolutePath();
    m_impl->reader = new KImageFormatsImageReader(fileName, format);
    if(m_impl->reader->device())
        m_impl->initialDevicePos = m_impl->reader->device()->pos();
    connect(&m_impl->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
}

KImageFormatsMovie::~KImageFormatsMovie()
{
    delete m_impl->reader;
}

QList<QByteArray> KImageFormatsMovie::supportedFormats()
{
    QList<QByteArray> list = KImageFormatsImageReader::supportedImageFormats();
    QBuffer buffer;
    buffer.open(QIODevice::ReadOnly);
    for(QList<QByteArray>::Iterator it = list.begin(); it != list.end();)
    {
        if(KImageFormatsImageReader(&buffer, *it).supportsOption(QImageIOHandler::Animation))
            ++it;
        else
            it = list.erase(it);
    }
    return list;
}

void KImageFormatsMovie::setDevice(QIODevice *device)
{
    m_impl->reader->setDevice(device);
    m_impl->reset();
}

QIODevice *KImageFormatsMovie::device() const
{
    return m_impl->reader->device();
}

void KImageFormatsMovie::setFileName(const QString &fileName)
{
    m_impl->absoluteFilePath = QDir(fileName).absolutePath();
    m_impl->reader->setFileName(fileName);
    m_impl->reset();
}

QString KImageFormatsMovie::fileName() const
{
    return m_impl->reader->fileName();
}

void KImageFormatsMovie::setFormat(const QByteArray &format)
{
    m_impl->reader->setFormat(format);
}

QByteArray KImageFormatsMovie::format() const
{
    return m_impl->reader->format();
}

void KImageFormatsMovie::setBackgroundColor(const QColor &color)
{
    m_impl->reader->setBackgroundColor(color);
}

QColor KImageFormatsMovie::backgroundColor() const
{
    return m_impl->reader->backgroundColor();
}

QMovie::MovieState KImageFormatsMovie::state() const
{
    return m_impl->movieState;
}

QRect KImageFormatsMovie::frameRect() const
{
    return m_impl->frameRect;
}

QImage KImageFormatsMovie::currentImage() const
{
    return m_impl->currentPixmap.toImage();
}

QPixmap KImageFormatsMovie::currentPixmap() const
{
    return m_impl->currentPixmap;
}

bool KImageFormatsMovie::isValid() const
{
    return m_impl->isValid();
}

QImageReader::ImageReaderError KImageFormatsMovie::lastError() const
{
    return m_impl->reader->error();
}

QString KImageFormatsMovie::lastErrorString() const
{
    return m_impl->reader->errorString();
}

bool KImageFormatsMovie::jumpToFrame(int frameNumber)
{
    return m_impl->jumpToFrame(frameNumber);
}

int KImageFormatsMovie::loopCount() const
{
    return m_impl->reader->loopCount();
}

int KImageFormatsMovie::frameCount() const
{
    return m_impl->frameCount();
}

int KImageFormatsMovie::nextFrameDelay() const
{
    return m_impl->nextDelay;
}

int KImageFormatsMovie::currentFrameNumber() const
{
    return m_impl->currentFrameNumber;
}

int KImageFormatsMovie::speed() const
{
    return m_impl->speed;
}

QSize KImageFormatsMovie::scaledSize()
{
    return m_impl->reader->scaledSize();
}

void KImageFormatsMovie::setScaledSize(const QSize &size)
{
    m_impl->reader->setScaledSize(size);
}

QMovie::CacheMode KImageFormatsMovie::cacheMode() const
{
    return m_impl->cacheMode;
}

void KImageFormatsMovie::setCacheMode(QMovie::CacheMode mode)
{
    m_impl->cacheMode = mode;
}

void KImageFormatsMovie::start()
{
    if(m_impl->movieState == QMovie::NotRunning)
        m_impl->loadNextFrame(true);
    else if(m_impl->movieState == QMovie::Paused)
        setPaused(false);
}

bool KImageFormatsMovie::jumpToNextFrame()
{
    return m_impl->jumpToNextFrame();
}

void KImageFormatsMovie::setPaused(bool paused)
{
    if(paused)
    {
        if(m_impl->movieState == QMovie::NotRunning)
            return;
        m_impl->enterState(QMovie::Paused);
        m_impl->nextImageTimer.stop();
    }
    else
    {
        if(m_impl->movieState == QMovie::Running)
            return;
        m_impl->enterState(QMovie::Running);
        m_impl->nextImageTimer.start(nextFrameDelay());
    }
}

void KImageFormatsMovie::stop()
{
    if(m_impl->movieState == QMovie::NotRunning)
        return;
    m_impl->enterState(QMovie::NotRunning);
    m_impl->nextImageTimer.stop();
    m_impl->nextFrameNumber = 0;
}

void KImageFormatsMovie::setSpeed(int percentSpeed)
{
    if(!m_impl->speed && m_impl->movieState == QMovie::Running)
        m_impl->nextImageTimer.start(nextFrameDelay());
    m_impl->speed = percentSpeed;
}

void KImageFormatsMovie::loadNextFrame()
{
    m_impl->loadNextFrame();
}

void KImageFormatsMovie::loadNextFrame(bool starting)
{
    m_impl->loadNextFrame(starting);
}

QT_END_NAMESPACE
