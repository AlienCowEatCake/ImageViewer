/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DecoderMacWebKit.h"

#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>

#include <QtGlobal>
#include <QGraphicsProxyWidget>
#include <QFileInfo>
#include <QUrl>
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
#include <QMacCocoaViewContainer>
#endif

#include <QDebug>

#include "DecoderAutoRegistrator.h"

//#if defined (QT_DEBUG)
//#define DECODER_MAC_WEBKIT_PRIORITY -1
//#else
#define DECODER_MAC_WEBKIT_PRIORITY 200
//#endif

namespace {

DecoderAutoRegistrator registrator(new DecoderMacWebKit);

} // namespace

QString DecoderMacWebKit::name() const
{
    return QString::fromLatin1("DecoderMacWebKit");
}

QList<DecoderFormatInfo> DecoderMacWebKit::supportedFormats() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    const QList<QByteArray> svgFormats = QList<QByteArray>()
            << "svg"
            << "svgz";
    QList<DecoderFormatInfo> result;
    for(QList<QByteArray>::ConstIterator it = svgFormats.constBegin(); it != svgFormats.constEnd(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_MAC_WEBKIT_PRIORITY;
        info.format = QString::fromLatin1(*it).toLower();
        result.append(info);
    }
    return result;
#else
    return QList<DecoderFormatInfo>();
#endif
}

QGraphicsItem *DecoderMacWebKit::loadImage(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;

    QGraphicsProxyWidget *result = NULL;

//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    WebView *view = [[WebView alloc] initWithFrame: NSMakeRect(0, 0, 0, 0)];

    NSURLRequest *request = [NSURLRequest requestWithURL: QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toNSURL()];
    [[view mainFrame] loadRequest: request];

    QMacCocoaViewContainer *container = new QMacCocoaViewContainer(view);

    container->setWindowFlags(Qt::Window);
    container->show();

//    result = new QGraphicsProxyWidget();
//    result->setWidget(container);

//    [pool release];

    return result;
}
