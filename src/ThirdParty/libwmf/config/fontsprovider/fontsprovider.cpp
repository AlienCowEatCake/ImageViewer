#include "fontsprovider.h"

#include <QtGlobal>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

namespace {

#include "resources/n019003l.afm.h"
#include "resources/n019003l.pfb.h"
#include "resources/n019004l.afm.h"
#include "resources/n019004l.pfb.h"
#include "resources/n019023l.afm.h"
#include "resources/n019023l.pfb.h"
#include "resources/n019024l.afm.h"
#include "resources/n019024l.pfb.h"
#include "resources/n021003l.afm.h"
#include "resources/n021003l.pfb.h"
#include "resources/n021004l.afm.h"
#include "resources/n021004l.pfb.h"
#include "resources/n021023l.afm.h"
#include "resources/n021023l.pfb.h"
#include "resources/n021024l.afm.h"
#include "resources/n021024l.pfb.h"
#include "resources/n022003l.afm.h"
#include "resources/n022003l.pfb.h"
#include "resources/n022004l.afm.h"
#include "resources/n022004l.pfb.h"
#include "resources/n022023l.afm.h"
#include "resources/n022023l.pfb.h"
#include "resources/n022024l.afm.h"
#include "resources/n022024l.pfb.h"
#include "resources/s050000l.afm.h"
#include "resources/s050000l.pfb.h"
#include "resources/fontmap.in.h"

class FontsProvider
{
public:
    static FontsProvider &getInstance()
    {
        static FontsProvider provider;
        return provider;
    }

    const char *wmfFontdir()
    {
        if(!m_wmfFontdir.isNull())
            return m_wmfFontdir.constData();
        if(!copyWmfFonts())
            return (m_wmfFontdir = "").constData();
        return (m_wmfFontdir = fontCacheDir().toLocal8Bit()).constData();
    }

    const char *wmfGsFontdir()
    {
        if(!m_wmfGsFontdir.isNull())
            return m_wmfGsFontdir.constData();
        const QList<QByteArray> knownDirs = QList<QByteArray>()
                << "/var/lib/defoma/gs.d/dirs/fonts"
                << "c:/progra~1/gs/gs/lib/fonts"
                << "c:/gs/fonts"
                << "/usr/share/fonts/default/Type1"
                /// @note Advanced
                << "/usr/share/fonts/type1/gsfonts";
        for(QList<QByteArray>::ConstIterator it = knownDirs.constBegin(); it != knownDirs.constEnd(); ++it)
        {
            if(QDir(QString::fromLatin1(*it)).exists())
                return (m_wmfGsFontdir = *it).constData();
        }
        return (m_wmfGsFontdir = "").constData();
    }

    const char *wmfSysFontmap()
    {
        if(!m_wmfSysFontmap.isNull())
            return m_wmfSysFontmap.constData();
        return (m_wmfSysFontmap = "/usr/share/fonts/fontmap").constData();
    }

    const char *wmfXtraFontmap()
    {
        if(!m_wmfXtraFontmap.isNull())
            return m_wmfXtraFontmap.constData();
        if(fontCacheDir().isEmpty())
            return (m_wmfSysFontmap = "").constData();
        const QString fontmapPath = QDir(fontCacheDir()).absoluteFilePath(QString::fromLatin1("fontmap"));
        QFile file(fontmapPath);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return (m_wmfSysFontmap = "").constData();
        file.write(QString::fromLatin1(reinterpret_cast<const char*>(fontmap_in), static_cast<int>(fontmap_in_len))
                   .replace(QString::fromLatin1("@WMF_FONTDIR@"), fontCacheDir())
                   .toLocal8Bit());
        return (m_wmfSysFontmap = fontmapPath.toLocal8Bit()).constData();
    }

    const char *wmfGsFontmap()
    {
        if(!m_wmfGsFontmap.isNull())
            return m_wmfGsFontmap.constData();
        const QList<QByteArray> knownFiles = QList<QByteArray>()
                << "/var/lib/defoma/gs.d/dirs/fonts/Fontmap.GS"
                << "/var/lib/defoma/gs.d/dirs/fonts/Fontmap"
                << "c:/progra~1/gs/gs/lib/fontmap.gs"
                << "c:/progra~1/gs/gs/lib/fontmap"
                << "c:/gs/fonts/fontmap.gs"
                << "c:/gs/fonts/fontmap"
                /// @note Advanced
                << "/var/lib/ghostscript/fonts/Fontmap";
        for(QList<QByteArray>::ConstIterator it = knownFiles.constBegin(); it != knownFiles.constEnd(); ++it)
        {
            if(QFile(QString::fromLatin1(*it)).exists())
                return (m_wmfGsFontmap = *it).constData();
        }
        const QDir ghostscriptShare(QString::fromLatin1("/usr/share/ghostscript"));
        if(ghostscriptShare.exists())
        {
            const QStringList knownShareFileTemplates = QStringList()
                    << QString::fromLatin1("%1/%2/Fontmap.GS")
                    << QString::fromLatin1("%1/%2/Fontmap")
                    << QString::fromLatin1("%1/%2/lib/Fontmap.GS")
                    << QString::fromLatin1("%1/%2/lib/Fontmap");
            const QStringList ghostscriptSubdirList = ghostscriptShare.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for(QStringList::ConstIterator dir = ghostscriptSubdirList.constBegin(); dir != ghostscriptSubdirList.constEnd(); ++dir)
            {
                for(QStringList::ConstIterator file = knownShareFileTemplates.constBegin(); file != knownShareFileTemplates.constEnd(); ++file)
                {
                    const QString path = (*file).arg(ghostscriptShare.absolutePath()).arg(*dir);
                    if(QFile(path).exists())
                        return (m_wmfGsFontmap = path.toLocal8Bit()).constData();
                }
            }
        }
        return (m_wmfGsFontmap = "").constData();
    }

private:
    FontsProvider()
    {}

    ~FontsProvider()
    {}

    QString fontCacheDir()
    {
        if(!m_fontCacheDir.isNull())
            return m_fontCacheDir;
        const QString appDataLocation =
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
            QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
#elif (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
            QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
        if(!appDataLocation.isEmpty() && QDir().mkpath(appDataLocation))
            return (m_fontCacheDir = QFileInfo(appDataLocation).absoluteFilePath());
        return (m_fontCacheDir = QString::fromLatin1(""));
    }

    bool copyResource(const QString &fileName, unsigned char *data, unsigned int size)
    {
        if(fontCacheDir().isEmpty())
            return false;
        QFile file(QDir(fontCacheDir()).absoluteFilePath(fileName));
        if(!file.open(QIODevice::WriteOnly))
            return false;
        return file.write(reinterpret_cast<const char*>(data), static_cast<qint64>(size)) >= 0;
    }

    bool copyWmfFonts()
    {
        if(!copyResource(QString::fromLatin1("n019003l.afm"), n019003l_afm, n019003l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n019003l.pfb"), n019003l_pfb, n019003l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n019004l.afm"), n019004l_afm, n019004l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n019004l.pfb"), n019004l_pfb, n019004l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n019023l.afm"), n019023l_afm, n019023l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n019023l.pfb"), n019023l_pfb, n019023l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n019024l.afm"), n019024l_afm, n019024l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n019024l.pfb"), n019024l_pfb, n019024l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n021003l.afm"), n021003l_afm, n021003l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n021003l.pfb"), n021003l_pfb, n021003l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n021004l.afm"), n021004l_afm, n021004l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n021004l.pfb"), n021004l_pfb, n021004l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n021023l.afm"), n021023l_afm, n021023l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n021023l.pfb"), n021023l_pfb, n021023l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n021024l.afm"), n021024l_afm, n021024l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n021024l.pfb"), n021024l_pfb, n021024l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n022003l.afm"), n022003l_afm, n022003l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n022003l.pfb"), n022003l_pfb, n022003l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n022004l.afm"), n022004l_afm, n022004l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n022004l.pfb"), n022004l_pfb, n022004l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n022023l.afm"), n022023l_afm, n022023l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n022023l.pfb"), n022023l_pfb, n022023l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("n022024l.afm"), n022024l_afm, n022024l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("n022024l.pfb"), n022024l_pfb, n022024l_pfb_len)) return false;
        if(!copyResource(QString::fromLatin1("s050000l.afm"), s050000l_afm, s050000l_afm_len)) return false;
        if(!copyResource(QString::fromLatin1("s050000l.pfb"), s050000l_pfb, s050000l_pfb_len)) return false;
        return true;
    }

    Q_DISABLE_COPY(FontsProvider)

    QString m_fontCacheDir;
    QByteArray m_wmfFontdir;
    QByteArray m_wmfGsFontdir;
    QByteArray m_wmfSysFontmap;
    QByteArray m_wmfXtraFontmap;
    QByteArray m_wmfGsFontmap;
};

} // namespace

const char *ProvideWmfFontdir()
{
    return FontsProvider::getInstance().wmfFontdir();
}

const char *ProvideWmfGsFontdir()
{
    return FontsProvider::getInstance().wmfGsFontdir();
}

const char *ProvideWmfSysFontmap()
{
    return FontsProvider::getInstance().wmfSysFontmap();
}

const char *ProvideWmfXtraFontmap()
{
    return FontsProvider::getInstance().wmfXtraFontmap();
}

const char *ProvideWmfGsFontmap()
{
    return FontsProvider::getInstance().wmfGsFontmap();
}
