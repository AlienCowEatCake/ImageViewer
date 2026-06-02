#include "fontsprovider.h"

#include <QtGlobal>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QList>

namespace {

class FontsProvider
{
public:
    static FontsProvider &getInstance()
    {
        static FontsProvider provider;
        return provider;
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
                    const QString path = (*file).arg(ghostscriptShare.absolutePath(), *dir);
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

    Q_DISABLE_COPY(FontsProvider)

    QByteArray m_wmfGsFontdir;
    QByteArray m_wmfSysFontmap;
    QByteArray m_wmfGsFontmap;
};

} // namespace

const char *ProvideWmfGsFontdir(void)
{
    return FontsProvider::getInstance().wmfGsFontdir();
}

const char *ProvideWmfSysFontmap(void)
{
    return FontsProvider::getInstance().wmfSysFontmap();
}

const char *ProvideWmfGsFontmap(void)
{
    return FontsProvider::getInstance().wmfGsFontmap();
}
