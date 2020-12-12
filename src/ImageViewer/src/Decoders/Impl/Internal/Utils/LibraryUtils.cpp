/*
   Copyright (C) 2018-2020 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QString>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QTextStream>
#include <QByteArray>
#include <QDebug>

#include "Utils/Global.h"

#if defined (Q_OS_LINUX)

#include <glob.h>

namespace {

class StandardLibraryPathProvider
{
public:
    StandardLibraryPathProvider()
        : m_standardPaths(parseConf(QString::fromLatin1("/etc/ld.so.conf")))
    {
        const QStringList trustedDirectories = QStringList()
                << QString::fromLatin1(sizeof(void*) == 4 ? "/usr/lib32" : "/usr/lib64")
                << QString::fromLatin1(sizeof(void*) == 4 ? "/lib32" : "/lib64")
                << QString::fromLatin1("/usr/lib")
                << QString::fromLatin1("/lib");
        for(QStringList::ConstIterator it = trustedDirectories.constBegin(), itEnd = trustedDirectories.constEnd(); it != itEnd; ++it)
            if(!m_standardPaths.contains(*it))
                m_standardPaths.append(*it);
        qDebug() << "[StandardLibraryPathProvider] Paths order:" << m_standardPaths;
    }

    QStringList getPaths() const
    {
        return m_standardPaths;
    }

private:
    QStringList glob(const QString &pattern) const
    {
        const QByteArray patternData = pattern.toLocal8Bit();
        QStringList result;
        glob64_t gl;
        switch(glob64(patternData.data(), 0, Q_NULLPTR, &gl))
        {
        case 0:
            for(size_t i = 0; i < gl.gl_pathc; ++i)
                result.append(QString::fromLocal8Bit(gl.gl_pathv[i]));
            globfree64(&gl);
            break;
        case GLOB_NOMATCH:
            break;
        case GLOB_NOSPACE:
            qWarning() << "[StandardLibraryPathProvider] Out of memory error in glob64() for" << pattern;
            break;
        case GLOB_ABORTED:
            qWarning() << "[StandardLibraryPathProvider] Read error in glob64() for" << pattern;
            break;
        default:
            qWarning() << "[StandardLibraryPathProvider] Unexpected error in glob64() for" << pattern;
            break;
        }
        return result;
    }

    QStringList parseConfInclude(const QString &configFile, const QString &pattern) const
    {
        QString absolutePattern = pattern;
        if(!pattern.startsWith(QChar::fromLatin1('/')) && configFile.contains(QChar::fromLatin1('/')))
            absolutePattern = QString::fromLatin1("%1/%2").arg(QFileInfo(configFile).absolutePath()).arg(pattern);

        QStringList result;
        const QStringList globbed = glob(pattern);
        for(QStringList::ConstIterator it = globbed.constBegin(), itEnd = globbed.constEnd(); it != itEnd; ++it)
        {
            const QStringList paths = parseConf(*it);
            for(QStringList::ConstIterator jt = paths.constBegin(), jtEnd = paths.constEnd(); jt != jtEnd; ++jt)
                result.append(*jt);
        }

        return result;
    }

    QStringList parseConf(const QString &configFile) const
    {
        QFile file(configFile);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qWarning() << "[StandardLibraryPathProvider] Can't open" << file.fileName();
            return QStringList();
        }

        QStringList result;
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            QString line = stream.readLine();
            const int pos = line.indexOf(QChar::fromLatin1('#'));
            if(pos >= 0)
                line = line.left(pos);
            line = line.trimmed();
            if(line.isEmpty())
                continue;

            if(line.startsWith(QString::fromLatin1("include"), Qt::CaseInsensitive) && line[7].isSpace())
            {
                line = line.mid(7).trimmed();
                const QStringList paths = parseConfInclude(configFile, line);
                for(QStringList::ConstIterator jt = paths.constBegin(), jtEnd = paths.constEnd(); jt != jtEnd; ++jt)
                    result.append(*jt);
            }
            else if(!(line.startsWith(QString::fromLatin1("hwcap"), Qt::CaseInsensitive) && line[5].isSpace()))
            {
                const int pos = line.indexOf(QChar::fromLatin1('='));
                if(pos >= 0)
                    line = line.left(pos).trimmed();
                result.append(line);
            }
        }
        file.close();
        return result;
    }

private:
    QStringList m_standardPaths;
};

} // namespace

#endif

#if defined (Q_OS_MAC)

#include <CoreFoundation/CoreFoundation.h>
#include "Utils/CFTypePtr.h"

namespace {

class StandardLibraryPathProvider
{
public:
    StandardLibraryPathProvider()
    {
        CFTypePtr<CFBundleRef> currentBundle = CFTypePtrFromGet(CFBundleGetMainBundle());
        if(!currentBundle)
            return;
        appendPath(CFTypePtrFromCreate(CFBundleCopyPrivateFrameworksURL(currentBundle)));
        appendPath(CFTypePtrFromCreate(CFBundleCopySharedFrameworksURL(currentBundle)));
        appendPath(QString::fromLatin1("/usr/local/lib"));
        appendPath(QString::fromLatin1("/usr/lib"));
        qDebug() << "[StandardLibraryPathProvider] Paths order:" << m_standardPaths;
    }

    QStringList getPaths() const
    {
        return m_standardPaths;
    }

private:
    void appendPath(const CFTypePtr<CFURLRef> &url)
    {
        const QString path = extractPath(url);
        appendPath(path);
    }

    void appendPath(const QString &path)
    {
        if(path.isEmpty())
            return;
        if(m_standardPaths.contains(path))
            return;
        m_standardPaths.append(path);
    }

    QString extractPath(const CFTypePtr<CFURLRef> &url) const
    {
        if(url)
            if(CFTypePtr<CFURLRef> absoluteUrl = CFTypePtrFromCreate(CFURLCopyAbsoluteURL(url)))
                if(CFTypePtr<CFStringRef> path = CFTypePtrFromCreate(CFURLCopyFileSystemPath(absoluteUrl, kCFURLPOSIXPathStyle)))
                    return QStringFromCFString(path);
        return QString();
    }

    QString QStringFromCFString(const CFTypePtr<CFStringRef> &str) const
    {
        const CFIndex strLength = CFStringGetLength(str);
        CFIndex byteLength = strLength * 6;
        CFStringGetBytes(str, CFRangeMake(0, strLength), kCFStringEncodingUTF8, 0, false, Q_NULLPTR, byteLength, &byteLength);
        QByteArray buffer(static_cast<int>(byteLength) + 1, '\0');
        CFStringGetBytes(str, CFRangeMake(0, strLength), kCFStringEncodingUTF8, 0, false, reinterpret_cast<UInt8*>(buffer.data()), buffer.size(), Q_NULLPTR);
        return QString::fromUtf8(buffer);
    }

private:
    QStringList m_standardPaths;
};

} // namespace

#endif

#if defined (Q_OS_WIN)

namespace {

class StandardLibraryPathProvider
{
public:
    StandardLibraryPathProvider()
    {
        appendPath(qApp->applicationDirPath());
        qDebug() << "[StandardLibraryPathProvider] Paths order:" << m_standardPaths;
    }

    QStringList getPaths() const
    {
        return m_standardPaths;
    }

private:
    void appendPath(const QString &path)
    {
        if(path.isEmpty())
            return;
        if(m_standardPaths.contains(path))
            return;
        m_standardPaths.append(path);
    }

private:
    QStringList m_standardPaths;
};

} // namespace

#endif

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
#if defined (Q_OS_LINUX)
        qDebug() << "[LoadLibrary]" << "Loading" << *it << "from standard directories ...";
        static const QStringList standardLibDirs = StandardLibraryPathProvider().getPaths();
        for(QStringList::ConstIterator dIt = standardLibDirs.constBegin(), dItEnd = standardLibDirs.constEnd(); dIt != dItEnd; ++dIt)
        {
            if(!QFileInfo(*dIt).exists())
                continue;
            if(QFileInfo(*it).isAbsolute())
                continue;
            const QStringList nameFilter(*it + QString::fromLatin1("*.so*"));
            const QDir directory(*dIt);
            const QStringList libraries = directory.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
            for(QStringList::ConstIterator lIt = libraries.constBegin(), lItEnd = libraries.constEnd(); lIt != lItEnd; ++lIt)
            {
                const QString libraryPath = QDir(*dIt).filePath(*lIt);
                if(!QFileInfo(libraryPath).exists())
                    continue;
                if(!QLibrary::isLibrary(libraryPath))
                    continue;
                library.setFileName(libraryPath);
                if(library.load())
                    break;
                qDebug() << "[LoadLibrary]" << "Error:" << library.errorString();
            }
            if(library.isLoaded())
                break;
        }
        if(library.isLoaded())
            break;
#endif
#if defined (Q_OS_MAC)
        qDebug() << "[LoadLibrary]" << "Loading" << *it << "from standard directories ...";
        static const QStringList standardLibDirs = StandardLibraryPathProvider().getPaths();
        for(QStringList::ConstIterator dIt = standardLibDirs.constBegin(), dItEnd = standardLibDirs.constEnd(); dIt != dItEnd; ++dIt)
        {
            if(!QFileInfo(*dIt).exists())
                continue;
            if(QFileInfo(*it).isAbsolute())
                continue;
            const QStringList nameFilter(*it + QString::fromLatin1("*.dylib*"));
            const QDir directory(*dIt);
            const QStringList libraries = directory.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
            for(QStringList::ConstIterator lIt = libraries.constBegin(), lItEnd = libraries.constEnd(); lIt != lItEnd; ++lIt)
            {
                const QString libraryPath = QDir(*dIt).filePath(*lIt);
                if(!QFileInfo(libraryPath).exists())
                    continue;
                if(!QLibrary::isLibrary(libraryPath))
                    continue;
                library.setFileName(libraryPath);
                if(library.load())
                    break;
                qDebug() << "[LoadLibrary]" << "Error:" << library.errorString();
            }
            if(library.isLoaded())
                break;
        }
        if(library.isLoaded())
            break;
#endif
#if defined (Q_OS_WIN)
        qDebug() << "[LoadLibrary]" << "Loading" << *it << "from standard directories ...";
        static const QStringList standardLibDirs = StandardLibraryPathProvider().getPaths();
        for(QStringList::ConstIterator dIt = standardLibDirs.constBegin(), dItEnd = standardLibDirs.constEnd(); dIt != dItEnd; ++dIt)
        {
            if(!QFileInfo(*dIt).exists())
                continue;
            if(QFileInfo(*it).isAbsolute())
                continue;
            const QStringList nameFilter(*it + QString::fromLatin1("*.dll"));
            const QDir directory(*dIt);
            const QStringList libraries = directory.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);
            for(QStringList::ConstIterator lIt = libraries.constBegin(), lItEnd = libraries.constEnd(); lIt != lItEnd; ++lIt)
            {
                const QString libraryPath = QDir(*dIt).filePath(*lIt);
                if(!QFileInfo(libraryPath).exists())
                    continue;
                if(!QLibrary::isLibrary(libraryPath))
                    continue;
                library.setFileName(libraryPath);
                if(library.load())
                    break;
                qDebug() << "[LoadLibrary]" << "Error:" << library.errorString();
            }
            if(library.isLoaded())
                break;
        }
        if(library.isLoaded())
            break;
#endif
    }
    const bool status = library.isLoaded();
    qDebug() << "[LoadLibrary]" << (status ? "Loading success!" : "Loading failed!");
    return library.isLoaded();
}

} // namespace LibraryUtils
