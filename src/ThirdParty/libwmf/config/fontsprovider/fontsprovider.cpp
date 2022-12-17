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

static void libWmfFontsProviderInitResources()
{
    Q_INIT_RESOURCE(libwmf_fontsprovider);
}

namespace {

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
//        if(!copyWmfFonts())
//            return (m_wmfFontdir = "").constData();
        copyWmfFonts(); ///< Try to load as is
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
            return (m_wmfXtraFontmap = "").constData();
        const QString fontmapPath = QDir(fontCacheDir()).absoluteFilePath(QString::fromLatin1("fontmap"));
        const QFileInfo currentFontmapInfo(fontmapPath);
        if(currentFontmapInfo.exists() && currentFontmapInfo.isFile())
            return (m_wmfXtraFontmap = fontmapPath.toLocal8Bit()).constData();
        QFile inFile(QString::fromLatin1(":/libwmf/fontsprovider/fontmap.in"));
        if(!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return (m_wmfXtraFontmap = "").constData();
        QFile outFile(fontmapPath);
        if(!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return (m_wmfXtraFontmap = "").constData();
        outFile.write(QString::fromLatin1(inFile.readAll())
                   .replace(QString::fromLatin1("@WMF_FONTDIR@"), fontCacheDir())
                   .toLocal8Bit());
        return (m_wmfXtraFontmap = fontmapPath.toLocal8Bit()).constData();
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
    {
        libWmfFontsProviderInitResources();
    }

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

    bool copyResource(const QString &fileName)
    {
        if(fontCacheDir().isEmpty())
            return false;
        const QString filePath = QDir(fontCacheDir()).absoluteFilePath(fileName);
        const QFileInfo currentFileInfo(filePath);
        if(currentFileInfo.exists() && currentFileInfo.isFile())
            return true;
        QFile inFile(QString::fromLatin1(":/libwmf/fontsprovider/") + fileName);
        if(!inFile.open(QIODevice::ReadOnly))
            return false;
        QFile outFile(filePath);
        if(!outFile.open(QIODevice::WriteOnly))
            return false;
        return outFile.write(inFile.readAll()) >= 0;
    }

    bool copyWmfFonts()
    {
        bool status = true;
        status &= copyResource(QString::fromLatin1("n019003l.afm"));
        status &= copyResource(QString::fromLatin1("n019003l.pfb"));
        status &= copyResource(QString::fromLatin1("n019004l.afm"));
        status &= copyResource(QString::fromLatin1("n019004l.pfb"));
        status &= copyResource(QString::fromLatin1("n019023l.afm"));
        status &= copyResource(QString::fromLatin1("n019023l.pfb"));
        status &= copyResource(QString::fromLatin1("n019024l.afm"));
        status &= copyResource(QString::fromLatin1("n019024l.pfb"));
        status &= copyResource(QString::fromLatin1("n021003l.afm"));
        status &= copyResource(QString::fromLatin1("n021003l.pfb"));
        status &= copyResource(QString::fromLatin1("n021004l.afm"));
        status &= copyResource(QString::fromLatin1("n021004l.pfb"));
        status &= copyResource(QString::fromLatin1("n021023l.afm"));
        status &= copyResource(QString::fromLatin1("n021023l.pfb"));
        status &= copyResource(QString::fromLatin1("n021024l.afm"));
        status &= copyResource(QString::fromLatin1("n021024l.pfb"));
        status &= copyResource(QString::fromLatin1("n022003l.afm"));
        status &= copyResource(QString::fromLatin1("n022003l.pfb"));
        status &= copyResource(QString::fromLatin1("n022004l.afm"));
        status &= copyResource(QString::fromLatin1("n022004l.pfb"));
        status &= copyResource(QString::fromLatin1("n022023l.afm"));
        status &= copyResource(QString::fromLatin1("n022023l.pfb"));
        status &= copyResource(QString::fromLatin1("n022024l.afm"));
        status &= copyResource(QString::fromLatin1("n022024l.pfb"));
        status &= copyResource(QString::fromLatin1("s050000l.afm"));
        status &= copyResource(QString::fromLatin1("s050000l.pfb"));
        return status;
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
