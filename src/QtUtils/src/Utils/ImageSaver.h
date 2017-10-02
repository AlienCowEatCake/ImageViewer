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

#if !defined (QTUTILS_IMAGESAVER_H_INCLUDED)
#define QTUTILS_IMAGESAVER_H_INCLUDED

#include <QWidget>
#include <QString>

class QImage;

class ImageSaver: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ImageSaver)

public:
    ImageSaver(QWidget *parent = NULL);
    ~ImageSaver();

    QString defaultFilePath() const;
    void setDefaultFilePath(const QString &defaultFilePath);

    bool save(const QImage &image, const QString &preferredName = QString());

private:
    QWidget *m_parent;
    QString m_defaultFilePath;
    QString m_lastSavedFilePath;
};

#endif // QTUTILS_IMAGESAVER_H_INCLUDED

