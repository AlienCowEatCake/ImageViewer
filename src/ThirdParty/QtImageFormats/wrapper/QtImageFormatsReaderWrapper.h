#if !defined(QTIMAGEFORMATS_READER_WRAPPER_H_INCLUDED)
#define QTIMAGEFORMATS_READER_WRAPPER_H_INCLUDED

#include <QByteArray>

QT_BEGIN_NAMESPACE

class QIODevice;

class QtImageFormatsReaderWrapper
{
public:
    QtImageFormatsReaderWrapper();
    explicit QtImageFormatsReaderWrapper(QIODevice *device, const QByteArray &format = QByteArray());
    explicit QtImageFormatsReaderWrapper(const QString &fileName, const QByteArray &format = QByteArray());
    ~QtImageFormatsReaderWrapper();

private:
    struct Impl;
    Impl *m_impl;
};

QT_END_NAMESPACE

#endif // QTIMAGEFORMATS_READER_WRAPPER_H_INCLUDED
