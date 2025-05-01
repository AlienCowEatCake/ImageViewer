#if !defined(KIMAGEFORMATS_ZIP_H_INCLUDED)
#define KIMAGEFORMATS_ZIP_H_INCLUDED

#include <QByteArray>
#include <QIODevice>
#include <QScopedPointer>
#include <QString>

QT_BEGIN_NAMESPACE

class KArchiveEntry
{
public:
    virtual ~KArchiveEntry() {}
    virtual bool isFile() const = 0;
};

class KArchiveDirectory : public KArchiveEntry
{
public:
    virtual const KArchiveEntry *entry(const QString &name) const = 0;
};

class KZipFileEntry : public KArchiveEntry
{
public:
    virtual QByteArray data() const = 0;
};

class KZip
{
public:
    explicit KZip(QIODevice *dev);
    ~KZip();
    bool open(QIODevice::OpenMode mode);
    const KArchiveDirectory *directory() const;

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

QT_END_NAMESPACE

#endif // KIMAGEFORMATS_ZIP_H_INCLUDED
