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

#include "DecoderQtMovie.h"

#include <QMovie>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QFileInfo>
#include "Utils/ScopedPointer.h"

#include "DecoderAutoRegistrator.h"

namespace {

const int DECODER_QT_MOVIE_PRIORITY = 160;

DecoderAutoRegistrator registrator(new DecoderQtMovie);

} // namespace

QString DecoderQtMovie::name() const
{
    return QString::fromLatin1("DecoderQtMovie");
}

QList<DecoderFormatInfo> DecoderQtMovie::supportedFormats() const
{
    const QList<QByteArray> readerFormats = QList<QByteArray>() << QByteArray("gif");//QMovie::supportedFormats();
    QList<DecoderFormatInfo> result;
    for(QList<QByteArray>::ConstIterator it = readerFormats.constBegin(); it != readerFormats.constEnd(); ++it)
    {
        DecoderFormatInfo info;
        info.decoderPriority = DECODER_QT_MOVIE_PRIORITY;
        info.format = QString::fromLatin1(*it).toLower();
        result.append(info);
    }
    return result;
}

QGraphicsItem *DecoderQtMovie::loadImage(const QString &filename)
{
    const QFileInfo fileInfo(filename);
    if(!fileInfo.exists() || !fileInfo.isReadable())
        return NULL;
    QMovie *movie = new QMovie(filename);
    if(!movie->isValid())
    {
        movie->deleteLater();
        return NULL;
    }
    QLabel *movieLabel = new QLabel();
    movieLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    movieLabel->setMovie(movie);
    movie->start();
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
    proxy->setWidget(movieLabel);
    return proxy;
}
