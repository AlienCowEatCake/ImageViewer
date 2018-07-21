/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "LibraryUtils.h"

#include <QApplication>
#include <QLibrary>
#include <QStringList>
#include <QString>
#include <QDir>
#include <QDebug>

namespace LibraryUtils {

bool LoadQLibrary(QLibrary &library, const char *name)
{
    return LoadQLibrary(library, QString::fromLatin1(name));
}

bool LoadQLibrary(QLibrary &library, const QString &name)
{
    return LoadQLibrary(library, QStringList(name));
}

bool LoadQLibrary(QLibrary &library, const QStringList &names)
{
    for(QStringList::ConstIterator it = names.constBegin(), itEnd = names.constEnd(); it != itEnd; ++it)
    {
        qDebug() << "[LoadLibrary]" << "Loading" << *it << "from application directory ...";
        library.setFileName(QDir(qApp->applicationDirPath()).filePath(*it));
        if(library.load())
            break;
        qDebug() << "[LoadLibrary]" << "Error:" << library.errorString();
        qDebug() << "[LoadLibrary]" << "Loading" << *it << "from default directories ...";
        library.setFileName(*it);
        if(library.load())
            break;
        qDebug() << "[LoadLibrary]" << "Error:" << library.errorString();
    }
    const bool status = library.isLoaded();
    qDebug() << "[LoadLibrary]" << (status ? "Loading success!" : "Loading failed!");
    return library.isLoaded();
}

} // namespace LibraryUtils
