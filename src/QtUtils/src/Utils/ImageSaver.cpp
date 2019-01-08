/*
   Copyright (C) 2011-2017, Andrei V. Kurochkin <kurochkin.andrei.v@yandex.ru>
                 2011-2017, Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#include "ImageSaver.h"
#include <QImage>
#include <QStringList>
#include <QByteArray>
#include <QImageWriter>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

namespace {

QStringList getSupportedImageWriterFormats()
{
    static QStringList result;
    static bool initialized = false;
    if(!initialized)
    {
        const QList<QByteArray> formats = QImageWriter::supportedImageFormats();
        for(QList<QByteArray>::ConstIterator it = formats.constBegin(); it != formats.constEnd(); ++it)
            result.append(QString::fromLatin1(*it).toLower());
        initialized = true;
    }
    return result;
}

} // namespace

ImageSaver::ImageSaver(QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_defaultFilePath(QString::fromLatin1("Image"))
{}

ImageSaver::~ImageSaver()
{}

QString ImageSaver::defaultFilePath() const
{
    return m_defaultFilePath;
}

void ImageSaver::setDefaultFilePath(const QString &defaultFilePath)
{
    m_defaultFilePath = defaultFilePath;
}

bool ImageSaver::save(const QImage &image, const QString &preferredName)
{
    const QFileInfo preferredInfo(preferredName);
    const QString defaultExtension = QString::fromLatin1("png");
    QString preferredExtension;
    if(!preferredName.isEmpty())
        preferredExtension = preferredInfo.suffix();
    else if(!m_lastSavedFilePath.isEmpty())
        preferredExtension = QFileInfo(m_lastSavedFilePath).suffix();
    else if(!m_defaultFilePath.isEmpty())
        preferredExtension = QFileInfo(m_defaultFilePath).suffix();
    else
        preferredExtension = defaultExtension;

    // Белый список форматов, чтобы в предлагаемых форматах не было всяких ico, webp и прочих
    static const QStringList whiteList = QStringList()
            << QString::fromLatin1("bmp")
            << QString::fromLatin1("jpg")
            << QString::fromLatin1("jpeg")
            << QString::fromLatin1("png")
            << QString::fromLatin1("tif")
            << QString::fromLatin1("tiff");

    const QStringList supportedFormats = getSupportedImageWriterFormats();
    QString formatString, preferredFormatString, defaultFormatString;
    for(QStringList::ConstIterator it = supportedFormats.constBegin(); it != supportedFormats.constEnd(); ++it)
    {
        const QString &format = *it;
        if(!whiteList.contains(format, Qt::CaseInsensitive))
            continue;
        const QString currentFormatString = QString::fromLatin1("%1 %2 (*.%3)").arg(format.toUpper()).arg(tr("Images")).arg(format);
        formatString.append(QString::fromLatin1(";;%1").arg(currentFormatString));
        if(format == preferredExtension)
            preferredFormatString = currentFormatString;
        if(format == defaultExtension)
            defaultFormatString = currentFormatString;
    }
    const QString formatsAll = QString::fromLatin1("%1 (*.%2)").arg(tr("All Images")).arg(supportedFormats.join(QString::fromLatin1(" *.")));
    formatString.prepend(formatsAll);
    if(preferredFormatString.isEmpty())
        preferredFormatString = (defaultFormatString.isEmpty() ? formatsAll : defaultFormatString);

    QString supportedPreferredName = (preferredName.isEmpty() ? preferredName : preferredInfo.fileName());
    if(!supportedPreferredName.isEmpty() && !supportedFormats.contains(preferredExtension))
        supportedPreferredName = supportedPreferredName.left(supportedPreferredName.length() - preferredExtension.length()).append(defaultExtension);

    if(m_lastSavedFilePath.isEmpty())
        m_lastSavedFilePath = m_defaultFilePath;
    if(!m_lastSavedFilePath.isEmpty() && !supportedPreferredName.isEmpty())
        m_lastSavedFilePath = QFileInfo(m_lastSavedFilePath).absoluteDir().absoluteFilePath(supportedPreferredName);
    QString filename = QFileDialog::getSaveFileName(m_parent, tr("Save Image File"), m_lastSavedFilePath, formatString, &preferredFormatString);
    if(filename.length() == 0)
    {
        if(m_lastSavedFilePath == m_defaultFilePath)
            m_lastSavedFilePath.clear();
        return false;
    }

    const QString extension = QFileInfo(filename).suffix().toLower();
    if(!supportedFormats.contains(extension))
        filename.append(QString::fromLatin1(".")).append(defaultExtension);
    m_lastSavedFilePath = filename;

    bool saved = image.save(filename);
    if(!saved)
        QMessageBox::critical(m_parent, tr("Error"), tr("Error: Can't save file"), QMessageBox::Ok, QMessageBox::Ok);
    return saved;
}
