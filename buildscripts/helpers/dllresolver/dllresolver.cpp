#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

namespace {

QStringList split(const QString &str, const QString &re)
{
    return str.split(
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                QRegularExpression(re),
#else
                QRegExp(re),
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
                Qt::SkipEmptyParts);
#else
                QString::SkipEmptyParts);
#endif
}

QStringList getBinaryDepsDumpbin(const QString &filePath)
{
    QProcess p;
    p.start("dumpbin", QStringList() << "/NOLOGO" << "/DEPENDENTS" << QDir::toNativeSeparators(filePath));
    p.waitForFinished();
    QStringList lines = split(QString::fromLocal8Bit(p.readAllStandardOutput()), "[\r\n]");
    QStringList result;
    for(QStringList::ConstIterator it = lines.constBegin(); it != lines.constEnd(); ++it)
    {
        const QStringList words = split(*it, "[ ]");
        if(words.size() != 1)
            continue;
        if(words[0].toLower().endsWith(".dll"))
            result.append(words[0]);
    }
    return result;
}

QStringList getBinaryDepsObjdump(const QString &filePath)
{
    QProcess p;
    p.start("objdump", QStringList() << "--private-headers" << QDir::toNativeSeparators(filePath));
    p.waitForFinished();
    QStringList lines = split(QString::fromLocal8Bit(p.readAllStandardOutput()), "[\r\n]");
    QStringList result;
    for(QStringList::ConstIterator it = lines.constBegin(); it != lines.constEnd(); ++it)
    {
        if((*it).contains("DLL Name:"))
        {
            const QStringList words = split(*it, "[ ]");
            result.append(words.last());
        }
    }
    return result;
}

QStringList getBinaryDeps(const QString &filePath)
{
    static bool isInitialized = false;
    static bool dumpbinWorks = false;
    static bool objdumpWorks = false;
    if(!isInitialized)
    {
        const QString selfPath = qApp->applicationFilePath();
        const QStringList dumpbinDeps = getBinaryDepsDumpbin(selfPath);
        dumpbinWorks = !dumpbinDeps.empty();
        qWarning() << "Test dumpbin:" << dumpbinDeps << (dumpbinWorks ? "[OK]" : "[FAIL]");
        const QStringList objdumpDeps = getBinaryDepsObjdump(selfPath);
        objdumpWorks = !objdumpDeps.empty();
        qWarning() << "Test objdump:" << objdumpDeps << (objdumpWorks ? "[OK]" : "[FAIL]");
        isInitialized = true;
    }
    if(dumpbinWorks)
        return getBinaryDepsDumpbin(filePath);
    if(objdumpWorks)
        return getBinaryDepsObjdump(filePath);
    return QStringList();
}

QStringList getDeps(const QString &dir)
{
    QStringList result;
    QDirIterator it(dir, QStringList() << "*.exe" << "*.dll", QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        const QStringList deps = getBinaryDeps(it.next());
        for(QStringList::ConstIterator jt = deps.constBegin(); jt != deps.constEnd(); ++jt)
            result.append(*jt);
    }
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
    result.sort();
    result.removeDuplicates();
#else
    result = QStringList::fromSet(result.toSet());
    result.sort();
#endif
    return result;
}

bool checkDll(const QString &dir, const QString &dll)
{
    return QDir(dir).exists(dll);
}

void copyDlls(const QString &dir, const QStringList &searchPaths)
{
    for(int i = 1;; ++i)
    {
        qWarning() << "======= Iteration" << i << "=======";
        bool changed = false;
        const QStringList deps = getDeps(dir);
        for(QStringList::ConstIterator it = deps.constBegin(); it !=  deps.constEnd(); ++it)
        {
            bool resolved = false;
            if(checkDll(dir, *it))
                continue;
            for(QStringList::ConstIterator jt = searchPaths.constBegin(); jt != searchPaths.constEnd(); ++jt)
            {
                if(!checkDll(*jt, *it))
                    continue;
                if(!QFile::copy(QDir(*jt).absoluteFilePath(*it), QDir(dir).absoluteFilePath(*it)))
                    continue;
                qWarning() << "Resolved:" << QDir(*jt).absoluteFilePath(*it);
                resolved = true;
                changed = true;
                break;
            }
            if(!resolved)
                qWarning() << "Unresolved:" << (*it);
        }
        if(!changed)
            break;
    }
    qWarning() << "======= Done =======";
}

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QStringList arguments = a.arguments();
    if(arguments.size() < 3)
    {
        qWarning() << "Usage:" << argv[0] << "<target directory> <search directory #1> [search directory #2] ...";
        return 1;
    }
    arguments.removeFirst();
    const QString dir = arguments.first();
    arguments.removeFirst();
    qWarning() << "Target Directory:" << dir;
    qWarning() << "Search Directories:" << arguments;
    copyDlls(dir, arguments);
    return 0;
}
