/// @note This is modified QMovie code.

#include "QtImageFormatsMovie.h"

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

#define QMOVIE_INVALID_DELAY -1

QT_BEGIN_NAMESPACE

class QtImageFormatsFrameInfo
{
public:
    QPixmap pixmap;
    int delay;
    bool endMark;

    inline QtImageFormatsFrameInfo(bool endMark)
        : pixmap(QPixmap())
        , delay(QMOVIE_INVALID_DELAY)
        , endMark(endMark)
    {}

    inline QtImageFormatsFrameInfo()
        : pixmap(QPixmap())
        , delay(QMOVIE_INVALID_DELAY)
        , endMark(false)
    {}

    inline QtImageFormatsFrameInfo(const QPixmap &pixmap, int delay)
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

    static inline QtImageFormatsFrameInfo endMarker()
    {
        return QtImageFormatsFrameInfo(true);
    }
};

struct QtImageFormatsMovie::Impl
{
    Impl(QtImageFormatsMovie *movie);
    bool isDone();
    bool next();
    int speedAdjustedDelay(int delay) const;
    bool isValid() const;
    bool jumpToFrame(int frameNumber);
    int frameCount() const;
    bool jumpToNextFrame();
    QtImageFormatsFrameInfo infoForFrame(int frameNumber);
    void reset();

    inline void enterState(QMovie::MovieState newState)
    {
        movieState = newState;
        emit movie->stateChanged(newState);
    }

    // private slots
    void loadNextFrame();
    void loadNextFrame(bool starting);

    QtImageFormatsMovie *movie;

    QtImageFormatsImageReader *reader;
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
    QMap<int, QtImageFormatsFrameInfo> frameMap;
    QString absoluteFilePath;

    QTimer nextImageTimer;
};

QtImageFormatsMovie::Impl::Impl(QtImageFormatsMovie *movie)
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

void QtImageFormatsMovie::Impl::reset()
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

bool QtImageFormatsMovie::Impl::isDone()
{
    return (playCounter == 0);
}

int QtImageFormatsMovie::Impl::speedAdjustedDelay(int delay) const
{
    return int((qint64(delay) * qint64(100) ) / qint64(speed));
}

QtImageFormatsFrameInfo QtImageFormatsMovie::Impl::infoForFrame(int frameNumber)
{
    if(frameNumber < 0)
        return QtImageFormatsFrameInfo(); // Invalid

    if(haveReadAll && (frameNumber > greatestFrameNumber))
    {
        if(frameNumber == greatestFrameNumber+1)
            return QtImageFormatsFrameInfo::endMarker();
        return QtImageFormatsFrameInfo(); // Invalid
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
                        return QtImageFormatsFrameInfo(); // Invalid
                    QString fileName = reader->fileName();
                    QByteArray format = reader->format();
                    QIODevice *device = reader->device();
                    QColor bgColor = reader->backgroundColor();
                    QSize scaledSize = reader->scaledSize();
                    delete reader;
                    if(fileName.isEmpty())
                        reader = new QtImageFormatsImageReader(device, format);
                    else
                        reader = new QtImageFormatsImageReader(absoluteFilePath, format);
                    (void)reader->canRead(); // Provoke a device->open() call
                    reader->device()->seek(initialDevicePos);
                    reader->setBackgroundColor(bgColor);
                    reader->setScaledSize(scaledSize);
                }
                else
                {
                    return QtImageFormatsFrameInfo(); // Invalid
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
                return QtImageFormatsFrameInfo(); // Invalid
            }
            if(frameNumber > greatestFrameNumber)
                greatestFrameNumber = frameNumber;
            QPixmap aPixmap = QPixmap::fromImage(anImage);
            int aDelay = reader->nextImageDelay();
            return QtImageFormatsFrameInfo(aPixmap, aDelay);
        }
        else if(frameNumber != 0)
        {
            // We've read all frames now. Return an end marker
            haveReadAll = true;
            return QtImageFormatsFrameInfo::endMarker();
        }
        else
        {
            // No readable frames
            haveReadAll = true;
            return QtImageFormatsFrameInfo();
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
                    return QtImageFormatsFrameInfo(); // Invalid
                }
                greatestFrameNumber = i;
                QPixmap aPixmap = QPixmap::fromImage(anImage);
                int aDelay = reader->nextImageDelay();
                QtImageFormatsFrameInfo info(aPixmap, aDelay);
                // Cache it!
                frameMap.insert(i, info);
                if(i == frameNumber)
                    return info;
            }
            else
            {
                // We've read all frames now. Return an end marker
                haveReadAll = true;
                return QtImageFormatsFrameInfo::endMarker();
            }
        }
    }
    // Return info for requested (cached) frame
    return frameMap.value(frameNumber);
}

bool QtImageFormatsMovie::Impl::next()
{
    QTime time;
    time.start();
    QtImageFormatsFrameInfo info = infoForFrame(nextFrameNumber);
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
    nextDelay = speedAdjustedDelay(info.delay);
    // Adjust delay according to the time it took to read the frame
    int processingTime = time.elapsed();
    if(processingTime > nextDelay)
        nextDelay = 0;
    else
        nextDelay = nextDelay - processingTime;
    return true;
}

void QtImageFormatsMovie::Impl::loadNextFrame()
{
    loadNextFrame(false);
}

void QtImageFormatsMovie::Impl::loadNextFrame(bool starting)
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

        if(movieState == QMovie::Running)
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

bool QtImageFormatsMovie::Impl::isValid() const
{
    return (greatestFrameNumber >= 0) // have we seen valid data
        || reader->canRead(); // or does the reader see valid data
}

bool QtImageFormatsMovie::Impl::jumpToFrame(int frameNumber)
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

int QtImageFormatsMovie::Impl::frameCount() const
{
    int result;
    if((result = reader->imageCount()) != 0)
        return result;
    if(haveReadAll)
        return greatestFrameNumber+1;
    return 0; // Don't know
}

bool QtImageFormatsMovie::Impl::jumpToNextFrame()
{
    return jumpToFrame(currentFrameNumber+1);
}

QtImageFormatsMovie::QtImageFormatsMovie(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    m_impl->reader = new QtImageFormatsImageReader;
    connect(&m_impl->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
}

QtImageFormatsMovie::QtImageFormatsMovie(QIODevice *device, const QByteArray &format, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    m_impl->reader = new QtImageFormatsImageReader(device, format);
    m_impl->initialDevicePos = device->pos();
    connect(&m_impl->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
}

QtImageFormatsMovie::QtImageFormatsMovie(const QString &fileName, const QByteArray &format, QObject *parent)
    : QObject(parent)
    , m_impl(new Impl(this))
{
    m_impl->absoluteFilePath = QDir(fileName).absolutePath();
    m_impl->reader = new QtImageFormatsImageReader(fileName, format);
    if(m_impl->reader->device())
        m_impl->initialDevicePos = m_impl->reader->device()->pos();
    connect(&m_impl->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
}

QtImageFormatsMovie::~QtImageFormatsMovie()
{
    delete m_impl->reader;
}

QList<QByteArray> QtImageFormatsMovie::supportedFormats()
{
    QList<QByteArray> list = QtImageFormatsImageReader::supportedImageFormats();
    QBuffer buffer;
    buffer.open(QIODevice::ReadOnly);
    for(QList<QByteArray>::Iterator it = list.begin(); it != list.end();)
    {
        if(QtImageFormatsImageReader(&buffer, *it).supportsAnimation())
            ++it;
        else
            it = list.erase(it);
    }
    return list;
}

void QtImageFormatsMovie::setDevice(QIODevice *device)
{
    m_impl->reader->setDevice(device);
    m_impl->reset();
}

QIODevice *QtImageFormatsMovie::device() const
{
    return m_impl->reader->device();
}

void QtImageFormatsMovie::setFileName(const QString &fileName)
{
    m_impl->absoluteFilePath = QDir(fileName).absolutePath();
    m_impl->reader->setFileName(fileName);
    m_impl->reset();
}

QString QtImageFormatsMovie::fileName() const
{
    return m_impl->reader->fileName();
}

void QtImageFormatsMovie::setFormat(const QByteArray &format)
{
    m_impl->reader->setFormat(format);
}

QByteArray QtImageFormatsMovie::format() const
{
    return m_impl->reader->format();
}

void QtImageFormatsMovie::setBackgroundColor(const QColor &color)
{
    m_impl->reader->setBackgroundColor(color);
}

QColor QtImageFormatsMovie::backgroundColor() const
{
    return m_impl->reader->backgroundColor();
}

QMovie::MovieState QtImageFormatsMovie::state() const
{
    return m_impl->movieState;
}

QRect QtImageFormatsMovie::frameRect() const
{
    return m_impl->frameRect;
}

QImage QtImageFormatsMovie::currentImage() const
{
    return m_impl->currentPixmap.toImage();
}

QPixmap QtImageFormatsMovie::currentPixmap() const
{
    return m_impl->currentPixmap;
}

bool QtImageFormatsMovie::isValid() const
{
    return m_impl->isValid();
}

bool QtImageFormatsMovie::jumpToFrame(int frameNumber)
{
    return m_impl->jumpToFrame(frameNumber);
}

int QtImageFormatsMovie::loopCount() const
{
    return m_impl->reader->loopCount();
}

int QtImageFormatsMovie::frameCount() const
{
    return m_impl->frameCount();
}

int QtImageFormatsMovie::nextFrameDelay() const
{
    return m_impl->nextDelay;
}

int QtImageFormatsMovie::currentFrameNumber() const
{
    return m_impl->currentFrameNumber;
}

int QtImageFormatsMovie::speed() const
{
    return m_impl->speed;
}

QSize QtImageFormatsMovie::scaledSize()
{
    return m_impl->reader->scaledSize();
}

void QtImageFormatsMovie::setScaledSize(const QSize &size)
{
    m_impl->reader->setScaledSize(size);
}

QMovie::CacheMode QtImageFormatsMovie::cacheMode() const
{
    return m_impl->cacheMode;
}

void QtImageFormatsMovie::setCacheMode(QMovie::CacheMode mode)
{
    m_impl->cacheMode = mode;
}

void QtImageFormatsMovie::start()
{
    if(m_impl->movieState == QMovie::NotRunning)
        m_impl->loadNextFrame(true);
    else if(m_impl->movieState == QMovie::Paused)
        setPaused(false);
}

bool QtImageFormatsMovie::jumpToNextFrame()
{
    return m_impl->jumpToNextFrame();
}

void QtImageFormatsMovie::setPaused(bool paused)
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

void QtImageFormatsMovie::stop()
{
    if(m_impl->movieState == QMovie::NotRunning)
        return;
    m_impl->enterState(QMovie::NotRunning);
    m_impl->nextImageTimer.stop();
    m_impl->nextFrameNumber = 0;
}

void QtImageFormatsMovie::setSpeed(int percentSpeed)
{
    m_impl->speed = percentSpeed;
}

void QtImageFormatsMovie::loadNextFrame()
{
    m_impl->loadNextFrame();
}

void QtImageFormatsMovie::loadNextFrame(bool starting)
{
    m_impl->loadNextFrame(starting);
}

QT_END_NAMESPACE
