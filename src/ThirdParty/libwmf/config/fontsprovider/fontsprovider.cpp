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
        const QList<QByteArray> knownDirs = QList<QByteArray>()
                << "/usr/share/fonts/fontmap"
                << "/usr/share/libwmf/fonts/fontmap"
                << "/ucrt64/share/libwmf/fonts/fontmap"
                << "/mingw64/share/libwmf/fonts/fontmap"
                << "/mingw32/share/libwmf/fonts/fontmap"
                << "/clang64/share/libwmf/fonts/fontmap"
                << "/clang32/share/libwmf/fonts/fontmap"
                << "/clangarm64/share/libwmf/fonts/fontmap";
        for(QList<QByteArray>::ConstIterator it = knownDirs.constBegin(); it != knownDirs.constEnd(); ++it)
        {
            if(QDir(QString::fromLatin1(*it)).exists())
                return (m_wmfSysFontmap = *it).constData();
        }
        return (m_wmfSysFontmap = "").constData();
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
        if(appDataLocation.isEmpty())
            return (m_fontCacheDir = QString::fromLatin1(""));
        if(fileExists(appDataLocation, QString::fromLatin1("n019003l.afm")) || fileExists(appDataLocation, QString::fromLatin1("fontmap")))
            cleanupPrevious(appDataLocation); ///< Previous version compatibility
        const QString fontCacheDir = QDir(appDataLocation).absoluteFilePath(QString::fromLatin1("libwmf/fonts"));
        if(fontCacheDir.isEmpty() || !QDir().mkpath(fontCacheDir))
            return (m_fontCacheDir = QString::fromLatin1(""));
        return (m_fontCacheDir = QFileInfo(fontCacheDir).absoluteFilePath());
    }

    static bool fileExists(const QString &root, const QString &fileName)
    {
        if(root.isEmpty() || fileName.isEmpty())
            return false;
        const QFileInfo fi = QFileInfo(QDir(root).absoluteFilePath(fileName));
        return fi.exists() && fi.isFile();
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
        status &= copyResource(QString::fromLatin1("NimbusSans-Regular.afm"));
        status &= copyResource(QString::fromLatin1("NimbusSans-Regular.t1"));
        status &= copyResource(QString::fromLatin1("NimbusSans-Bold.afm"));
        status &= copyResource(QString::fromLatin1("NimbusSans-Bold.t1"));
        status &= copyResource(QString::fromLatin1("NimbusSans-Italic.afm"));
        status &= copyResource(QString::fromLatin1("NimbusSans-Italic.t1"));
        status &= copyResource(QString::fromLatin1("NimbusSans-BoldItalic.afm"));
        status &= copyResource(QString::fromLatin1("NimbusSans-BoldItalic.t1"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-Regular.afm"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-Regular.t1"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-Bold.afm"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-Bold.t1"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-Italic.afm"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-Italic.t1"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-BoldItalic.afm"));
        status &= copyResource(QString::fromLatin1("NimbusRoman-BoldItalic.t1"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-Regular.afm"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-Regular.t1"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-Bold.afm"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-Bold.t1"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-Italic.afm"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-Italic.t1"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-BoldItalic.afm"));
        status &= copyResource(QString::fromLatin1("NimbusMonoPS-BoldItalic.t1"));
        status &= copyResource(QString::fromLatin1("StandardSymbolsPS.afm"));
        status &= copyResource(QString::fromLatin1("StandardSymbolsPS.t1"));
        return status;
    }

    void cleanupPrevious(const QString &appDataLocation)
    {
        const QList<QByteArray> previousAppDataFiles = QList<QByteArray>()
                << "n019003l.afm"
                << "n019003l.pfb"
                << "n019004l.afm"
                << "n019004l.pfb"
                << "n019023l.afm"
                << "n019023l.pfb"
                << "n019024l.afm"
                << "n019024l.pfb"
                << "n021003l.afm"
                << "n021003l.pfb"
                << "n021004l.afm"
                << "n021004l.pfb"
                << "n021023l.afm"
                << "n021023l.pfb"
                << "n021024l.afm"
                << "n021024l.pfb"
                << "n022003l.afm"
                << "n022003l.pfb"
                << "n022004l.afm"
                << "n022004l.pfb"
                << "n022023l.afm"
                << "n022023l.pfb"
                << "n022024l.afm"
                << "n022024l.pfb"
                << "s050000l.afm"
                << "s050000l.pfb"
                << "fontmap";
        for(QList<QByteArray>::ConstIterator it = previousAppDataFiles.constBegin(); it != previousAppDataFiles.constEnd(); ++it)
            QFile::remove(QString::fromLatin1("%1/%2").arg(appDataLocation, QString::fromLatin1(*it)));
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

const char *ProvideWmfFontdir(void)
{
    return FontsProvider::getInstance().wmfFontdir();
}

const char *ProvideWmfGsFontdir(void)
{
    return FontsProvider::getInstance().wmfGsFontdir();
}

const char *ProvideWmfSysFontmap(void)
{
    return FontsProvider::getInstance().wmfSysFontmap();
}

const char *ProvideWmfXtraFontmap(void)
{
    return FontsProvider::getInstance().wmfXtraFontmap();
}

const char *ProvideWmfGsFontmap(void)
{
    return FontsProvider::getInstance().wmfGsFontmap();
}
