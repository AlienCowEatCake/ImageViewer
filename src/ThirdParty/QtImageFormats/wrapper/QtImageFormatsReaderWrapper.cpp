#include "QtImageFormatsReaderWrapper.h"

#include <QIODevice>

#if defined (WRAPPER_USE_DDS_HANDLER)
#include "imageformats/dds/qddshandler.h"
#endif

QT_BEGIN_NAMESPACE

struct QtImageFormatsReaderWrapper::Impl
{

};

QtImageFormatsReaderWrapper::QtImageFormatsReaderWrapper()
{

}

QtImageFormatsReaderWrapper::QtImageFormatsReaderWrapper(QIODevice *device, const QByteArray &format)
{

}

QtImageFormatsReaderWrapper::QtImageFormatsReaderWrapper(const QString &fileName, const QByteArray &format)
{

}

QtImageFormatsReaderWrapper::~QtImageFormatsReaderWrapper()
{
    delete m_impl;
}


QT_END_NAMESPACE
