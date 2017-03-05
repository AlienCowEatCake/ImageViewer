/*
   Copyright (C) 2011-2017,
        Andrei V. Kurochkin     <kurochkin.andrei.v@yandex.ru>
        Peter S. Zhigalov       <peter.zhigalov@gmail.com>

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
#include <QString>
#include <QImage>
#include <QStringList>
#include <QByteArray>
#include <QImageWriter>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

ImageSaver::ImageSaver(QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
    , m_defaultName(QString::fromLatin1("Image"))
{}

ImageSaver::~ImageSaver()
{}

QString ImageSaver::defaultName() const
{
    return m_defaultName;
}

void ImageSaver::setDefaultName(const QString &defaultName)
{
    m_defaultName = defaultName;
}

bool ImageSaver::save(const QImage &image, const QString &preferredName)
{
    // Белый список форматов, чтобы в предлагаемых форматах не было всяких ico, webp и прочих
    static const QStringList whiteList = QStringList()
            << QString::fromLatin1("bmp")
            << QString::fromLatin1("jpg")
            << QString::fromLatin1("jpeg")
            << QString::fromLatin1("png")
            << QString::fromLatin1("tif")
            << QString::fromLatin1("tiff");

    QList<QByteArray> supported = QImageWriter::supportedImageFormats();
    QString formats, formatsAll;
    for(QList<QByteArray>::iterator it = supported.begin(); it != supported.end(); ++it)
    {
        *it = (*it).toLower();
        const QString format = QString::fromLatin1(*it);
        if(!whiteList.contains(format, Qt::CaseInsensitive))
            continue;
        formatsAll.append(formatsAll.length() > 0 ? QString::fromLatin1(" *.") : QString::fromLatin1("*.")).append(format);
        if(formats.length() > 0)
            formats.append(QString::fromLatin1(";;"));
        formats.append(format.toUpper()).append(QString::fromLatin1(" ")).append(tr("Images"))
               .append(QString::fromLatin1(" (*.")).append(format).append(QString::fromLatin1(")"));
    }
    formatsAll.prepend(tr("All Images").append(QString::fromLatin1(" ("))).append(QString::fromLatin1(");;"));
    formats.prepend(formatsAll);

    if(m_lastSavedName.isEmpty())
        m_lastSavedName = m_defaultName;
    if(!m_lastSavedName.isEmpty() && !preferredName.isEmpty())
        m_lastSavedName = QFileInfo(m_lastSavedName).absoluteDir().absoluteFilePath(QFileInfo(preferredName).fileName());
    QString filename = QFileDialog::getSaveFileName(m_parent, tr("Save Image File"), m_lastSavedName, formats);
    if(filename.length() == 0)
    {
        if(m_lastSavedName == m_defaultName)
            m_lastSavedName.clear();
        return false;
    }

    const QByteArray defaultExtension("png");
    const QByteArray extension = QFileInfo(filename).suffix().toLower().toLatin1();
    if(!supported.contains(extension))
        filename.append(QString::fromLatin1(".")).append(QString::fromLatin1(defaultExtension));
    m_lastSavedName = filename;

    bool saved = image.save(filename);
    if(!saved)
        QMessageBox::critical(m_parent, tr("Error"), tr("Error: Can't save file"), QMessageBox::Ok, QMessageBox::Ok);
    return saved;
}
