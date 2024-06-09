/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <stdio.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QTextStream>

#include "../tests/format-enum.h"
#include "templateimage.h"

#include "fuzzyeq.cpp"

/**
 * @brief The SequentialFile class
 * Class to make a file a sequential access device. This class is used to check if the plugins could works
 * on a sequential device such as a socket.
 */
class SequentialFile : public QFile
{
public:
    SequentialFile()
        : QFile()
    {
    }
    explicit SequentialFile(const QString &name)
        : QFile(name)
    {
    }
#ifndef QT_NO_QOBJECT
    explicit SequentialFile(QObject *parent)
        : QFile(parent)
    {
    }
    SequentialFile(const QString &name, QObject *parent)
        : QFile(name, parent)
    {
    }
#endif

    bool isSequential() const override
    {
        return true;
    }

    qint64 size() const override
    {
        return bytesAvailable();
    }
};

static void writeImageData(const char *name, const QString &filename, const QImage &image)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        qint64 written = file.write(reinterpret_cast<const char *>(image.bits()), image.sizeInBytes());
        if (written == image.sizeInBytes()) {
            QTextStream(stdout) << "       " << name << " written to " << filename << "\n";
        } else {
            QTextStream(stdout) << "       could not write " << name << " to " << filename << ":" << file.errorString() << "\n";
        }
    } else {
        QTextStream(stdout) << "       could not open " << filename << ":" << file.errorString() << "\n";
    }
}

// Returns the original format if we support, or returns
// format which we preferred to use for `fuzzyeq()`.
// We do only support formats with 8-bits/16-bits pre pixel.
// If that changed, don't forget to update `fuzzyeq()` too
static QImage::Format preferredFormat(QImage::Format fmt)
{
    switch (fmt) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_RGBX64:
    case QImage::Format_RGBA64:
        return fmt;
    default:
        return QImage::Format_ARGB32;
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::removeLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::addLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::setApplicationName(QStringLiteral("readtest"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.2.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Performs basic image conversion checking."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("format"), QStringLiteral("format to test"));
    QCommandLineOption fuzz(QStringList() << QStringLiteral("f") << QStringLiteral("fuzz"),
                            QStringLiteral("Allow for some deviation in ARGB data."),
                            QStringLiteral("max"));
    parser.addOption(fuzz);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        QTextStream(stderr) << "Must provide a format\n";
        parser.showHelp(1);
    } else if (args.count() > 1) {
        QTextStream(stderr) << "Too many arguments\n";
        parser.showHelp(1);
    }

    uchar fuzziness = 0;
    if (parser.isSet(fuzz)) {
        bool ok;
        uint fuzzarg = parser.value(fuzz).toUInt(&ok);
        if (!ok || fuzzarg > 255) {
            QTextStream(stderr) << "Error: max fuzz argument must be a number between 0 and 255\n";
            parser.showHelp(1);
        }
        fuzziness = uchar(fuzzarg);
    }

    QString suffix = args.at(0);
    QByteArray format = suffix.toLatin1();

    QDir imgdir(QLatin1String(IMAGEDIR "/") + suffix);
    imgdir.setFilter(QDir::Files);

    int passed = 0;
    int failed = 0;
    int skipped = 0;

    QTextStream(stdout) << "********* "
                        << "Starting basic read tests for " << suffix << " images *********\n";

    const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QStringList formatStrings;
    formatStrings.reserve(formats.size());
    std::transform(formats.begin(), formats.end(), std::back_inserter(formatStrings), [](const QByteArray &format) {
        return QString(format);
    });
    QTextStream(stdout) << "QImageReader::supportedImageFormats: " << formatStrings.join(", ") << "\n";

    const QFileInfoList lstImgDir = imgdir.entryInfoList();
    // Launch 2 runs for each test: first run on a random access device, second run on a sequential access device
    for (int seq = 0; seq < 2; ++seq) {
        if (seq) {
            QTextStream(stdout) << "* Run on SEQUENTIAL ACCESS device\n";
        } else {
            QTextStream(stdout) << "* Run on RANDOM ACCESS device\n";
        }
        for (const QFileInfo &fi : lstImgDir) {
            TemplateImage timg(fi);
            if (timg.isTemplate()) {
                continue;
            }

            bool skipTest = false;
            QFileInfo expFileInfo = timg.compareImage(skipTest);
            if (skipTest) {
                QTextStream(stdout) << "SKIP : " << fi.fileName() << ": image format not supported by current Qt version!\n";
                ++skipped;
                continue;
            }
            if (!formatStrings.contains(expFileInfo.suffix(), Qt::CaseInsensitive)) {
                // Work Around for CCBUG: 468288
                QTextStream(stdout) << "SKIP : " << fi.fileName() << ": comparison image " << expFileInfo.fileName() << " cannot be loaded due to the lack of "
                                    << expFileInfo.suffix().toUpper() << " plugin!\n";
                ++skipped;
                continue;
            }
            QString expfilename = expFileInfo.fileName();

            std::unique_ptr<QIODevice> inputDevice(seq ? new SequentialFile(fi.filePath()) : new QFile(fi.filePath()));
            QImageReader inputReader(inputDevice.get(), format);
            QImageReader expReader(expFileInfo.filePath());

            QImage inputImage;
            QImage expImage;

            // inputImage is auto-rotated to final orientation
            inputReader.setAutoTransform(true);

            if (!expReader.read(&expImage)) {
                QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not load " << expfilename << ": " << expReader.errorString() << "\n";
                ++failed;
                continue;
            }
            if (!inputReader.canRead()) {
                // All plugins must pass the test on a random device.
                // canRead() must also return false if the plugin is unable to run on a sequential device.
                if (inputDevice->isSequential()) {
                    QTextStream(stdout) << "SKIP : " << fi.fileName() << ": cannot read on a sequential device (don't worry, it's ok)\n";
                    ++skipped;
                } else {
                    QTextStream(stdout) << "FAIL : " << fi.fileName() << ": failed can read: " << inputReader.errorString() << "\n";
                    ++failed;
                }
                continue;
            }
            if (!inputReader.read(&inputImage)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": failed to load: " << inputReader.errorString() << "\n";
                ++failed;
                continue;
            }
            if (expImage.width() != inputImage.width()) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": width was " << inputImage.width() << " but " << expfilename << " width was "
                                    << expImage.width() << "\n";
                ++failed;
            } else if (expImage.height() != inputImage.height()) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": height was " << inputImage.height() << " but " << expfilename << " height was "
                                    << expImage.height() << "\n";
                ++failed;
            } else {
                QImage::Format inputFormat = preferredFormat(inputImage.format());
                QImage::Format expFormat = preferredFormat(expImage.format());
                QImage::Format cmpFormat = inputFormat == expFormat ? inputFormat : QImage::Format_ARGB32;

                if (inputImage.format() != cmpFormat) {
                    QTextStream(stdout) << "INFO : " << fi.fileName() << ": converting " << fi.fileName() << " from " << formatToString(inputImage.format())
                                        << " to " << formatToString(cmpFormat) << '\n';
                    inputImage = inputImage.convertToFormat(cmpFormat);
                }
                if (expImage.format() != cmpFormat) {
                    QTextStream(stdout) << "INFO : " << fi.fileName() << ": converting " << expfilename << " from " << formatToString(expImage.format())
                                        << " to " << formatToString(cmpFormat) << '\n';
                    expImage = expImage.convertToFormat(cmpFormat);
                }
                if (fuzzyeq(inputImage, expImage, fuzziness)) {
                    QTextStream(stdout) << "PASS : " << fi.fileName() << "\n";
                    ++passed;
                } else {
                    QTextStream(stdout) << "FAIL : " << fi.fileName() << ": differs from " << expfilename << "\n";
                    writeImageData("expected data", fi.fileName() + QLatin1String("-expected.data"), expImage);
                    writeImageData("actual data", fi.fileName() + QLatin1String("-actual.data"), inputImage);
                    ++failed;
                }
            }
        }
    }

    QTextStream(stdout) << "Totals: " << passed << " passed, " << skipped << " skipped, " << failed << " failed\n";
    QTextStream(stdout) << "********* "
                        << "Finished basic read tests for " << suffix << " images *********\n";

    return failed == 0 ? 0 : 1;
}
