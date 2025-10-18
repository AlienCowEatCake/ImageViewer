/*
    SPDX-FileCopyrightText: 2014 Alex Merry <alex.merry@kdemail.net>
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <stdio.h>

#include <QBuffer>
#include <QColorSpace>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMetaEnum>
#include <QTextStream>

#include "fuzzyeq.cpp"

QJsonObject readOptionalInfo(const QString &suffix)
{
    auto fi = QFileInfo(QStringLiteral("%1/basic/%2.json").arg(IMAGEDIR, suffix));
    if (!fi.exists()) {
        return {};
    }

    QFile f(fi.filePath());
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return {};
    }

    return doc.object();
}

void setOptionalInfo(QImage &image, const QString &suffix)
{
    auto obj = readOptionalInfo(suffix);
    if (obj.isEmpty()) {
        return;
    }

    // Set resolution
    auto res = obj.value("resolution").toObject();
    if (!res.isEmpty()) {
        image.setDotsPerMeterX(res.value("dotsPerMeterX").toInt());
        image.setDotsPerMeterY(res.value("dotsPerMeterY").toInt());
    }

    // Set metadata
    auto meta = obj.value("metadata").toArray();
    for (auto jv : meta) {
        auto obj = jv.toObject();
        auto key = obj.value("key").toString();
        auto val = obj.value("value").toString();
        image.setText(key, val);
    }
}

QByteArray readSubType(const QString &suffix)
{
    auto obj = readOptionalInfo(suffix);
    if (obj.isEmpty()) {
        return {};
    }
    return obj.value("subType").toString().toLatin1();
}

bool checkOptionalInfo(QImage &image, const QString &suffix)
{
    auto obj = readOptionalInfo(suffix);
    if (obj.isEmpty()) {
        return true;
    }

    // Check if the format match
    if (suffix.compare(obj.value("format").toString(), Qt::CaseInsensitive)) {
        return false;
    }

    // Test resolution
    auto res = obj.value("resolution").toObject();
    if (!res.isEmpty()) {
        auto resx = res.value("dotsPerMeterX").toInt();
        auto resy = res.value("dotsPerMeterY").toInt();
        if (resx != image.dotsPerMeterX()) {
            return false;
        }
        if (resy != image.dotsPerMeterY()) {
            return false;
        }
    }

    // Test metadata
    auto meta = obj.value("metadata").toArray();
    for (auto jv : meta) {
        auto obj = jv.toObject();
        auto key = obj.value("key").toString();
        auto val = obj.value("value").toString();
        auto cur = image.text(key);
        if (cur != val) {
            qDebug() << key;
            return false;
        }
    }

    return true;
}

/*!
 * \brief basicTest
 * Run a basic test on some common images.
 */
int basicTest(const QString &suffix, bool lossless, bool ignoreDataCheck, bool skipOptional, uint fuzzarg)
{
    uchar fuzziness = uchar(fuzzarg);

    QByteArray format = suffix.toLatin1();

    QDir imgdir(QStringLiteral("%1/basic").arg(IMAGEDIR));
    if (ignoreDataCheck) {
        imgdir.setNameFilters({QLatin1String("*.png")});
    } else {
        imgdir.setNameFilters(QStringList(QLatin1String("*.") + suffix));
    }
    imgdir.setFilter(QDir::Files);

    int passed = 0;
    int failed = 0;

    QTextStream(stdout) << "********* "
                        << "Starting BASIC write tests for " << suffix << " images *********\n";
    const QFileInfoList lstImgDir = imgdir.entryInfoList();
    for (const QFileInfo &fi : lstImgDir) {
        QString pngfile;
        if (ignoreDataCheck) {
            pngfile = fi.filePath();
        } else {
            int suffixPos = fi.filePath().size() - suffix.size();
            pngfile = fi.filePath().replace(suffixPos, suffix.size(), QStringLiteral("png"));
        }
        QString pngfilename = QFileInfo(pngfile).fileName();

        QImageReader pngReader(pngfile, "png");
        QImage pngImage;
        if (!pngReader.read(&pngImage)) {
            QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not load " << pngfilename << ": " << pngReader.errorString() << "\n";
            ++failed;
            continue;
        }

        QByteArray writtenData;
        {
            QBuffer buffer(&writtenData);
            QImageWriter imgWriter(&buffer, format.constData());
            auto subType = readSubType(suffix);
            if (!subType.isEmpty())
                imgWriter.setSubType(subType);
            if (lossless) {
                imgWriter.setQuality(100);
            }
            if (!skipOptional) {
                setOptionalInfo(pngImage, suffix);
            }
            if (!imgWriter.write(pngImage)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": failed to write image data\n";
                ++failed;
                continue;
            }
        }

        if (!ignoreDataCheck) {
            QFile expFile(fi.filePath());
            if (!expFile.open(QIODevice::ReadOnly)) {
                QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not open " << fi.fileName() << ": " << expFile.errorString() << "\n";
                ++failed;
                continue;
            }
            QByteArray expData = expFile.readAll();
            if (expData.isEmpty()) {
                // check if there was actually anything to read
                expFile.reset();
                char buf[1];
                qint64 result = expFile.read(buf, 1);
                if (result < 0) {
                    QTextStream(stdout) << "ERROR: " << fi.fileName() << ": could not load " << fi.fileName() << ": " << expFile.errorString() << "\n";
                    ++failed;
                    continue;
                }
            }
            if (expData != writtenData) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": written data differs from " << fi.fileName() << "\n";
                ++failed;
                continue;
            }
        }

        QImage reReadImage;
        {
            QBuffer buffer(&writtenData);
            QImageReader imgReader(&buffer, format.constData());
            if (!imgReader.read(&reReadImage)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": could not read back the written data\n";
                ++failed;
                continue;
            }
            if (!skipOptional) {
                if (!checkOptionalInfo(reReadImage, suffix)) {
                    QTextStream(stdout) << "FAIL : " << fi.fileName() << ": optional information does not match\n";
                    ++failed;
                    continue;
                }
            }
            if (reReadImage.colorSpace().isValid()) {
                QColorSpace toColorSpace;
                if (pngImage.colorSpace().isValid()) {
                    reReadImage.convertToColorSpace(pngImage.colorSpace());
                } else {
                    reReadImage.convertToColorSpace(QColorSpace(QColorSpace::SRgb));
                }
            }
            reReadImage = reReadImage.convertToFormat(pngImage.format());
        }

        if (lossless) {
            if (!fuzzyeq(pngImage, reReadImage, fuzziness)) {
                QTextStream(stdout) << "FAIL : " << fi.fileName() << ": re-reading the data resulted in a different image\n";
                if (pngImage.size() == reReadImage.size()) {
                    for (int i = 0; i < pngImage.width(); ++i) {
                        for (int j = 0; j < pngImage.height(); ++j) {
                            if (pngImage.pixel(i, j) != reReadImage.pixel(i, j)) {
                                QTextStream(stdout) << "Pixel is different " << i << ',' << j << ' ' << pngImage.pixel(i, j) << ' ' << reReadImage.pixel(i, j)
                                                    << '\n';
                            }
                        }
                    }
                }
                ++failed;
                continue;
            }
        }

        QTextStream(stdout) << "PASS : " << fi.fileName() << "\n";
        ++passed;
    }

    QTextStream(stdout) << "Totals: " << passed << " passed, " << failed << " failed\n";
    QTextStream(stdout) << "********* "
                        << "Finished BASIC write tests for " << suffix << " images *********\n";

    return failed == 0 ? 0 : 1;
}

QImage formatSourceImage(const QImage::Format &format)
{
    auto image = QImage();
    auto folder = QStringLiteral("%1/format/_images").arg(IMAGEDIR);

    switch (format) {
    case QImage::Format_MonoLSB:
    case QImage::Format_Mono:
        image = QImage(QStringLiteral("%1/mono.png").arg(folder));
        break;

    case QImage::Format_Indexed8:
        image = QImage(QStringLiteral("%1/indexed.png").arg(folder));
        break;

    case QImage::Format_Grayscale8:
    case QImage::Format_Grayscale16:
        image = QImage(QStringLiteral("%1/gray16.png").arg(folder));
        break;

    case QImage::Format_RGB32:
    case QImage::Format_RGB16:
    case QImage::Format_RGB666:
    case QImage::Format_RGB555:
    case QImage::Format_RGB888:
    case QImage::Format_RGB444:
    case QImage::Format_RGBX8888:
    case QImage::Format_BGR30:
    case QImage::Format_RGB30:
    case QImage::Format_RGBX64:
    case QImage::Format_BGR888:
    case QImage::Format_RGBX16FPx4:
    case QImage::Format_RGBX32FPx4:
        image = QImage(QStringLiteral("%1/rgb16.png").arg(folder));
        break;

    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB4444_Premultiplied:
    case QImage::Format_RGBA8888:
    case QImage::Format_RGBA8888_Premultiplied:
    case QImage::Format_A2BGR30_Premultiplied:
    case QImage::Format_A2RGB30_Premultiplied:
    case QImage::Format_RGBA64:
    case QImage::Format_RGBA64_Premultiplied:
    case QImage::Format_RGBA16FPx4:
    case QImage::Format_RGBA16FPx4_Premultiplied:
    case QImage::Format_RGBA32FPx4:
    case QImage::Format_RGBA32FPx4_Premultiplied:
        image = QImage(QStringLiteral("%1/rgba16.png").arg(folder));
        break;

    case QImage::Format_CMYK8888:
        image = QImage(QStringLiteral("%1/cmyk8.tif").arg(folder));
        break;

    default:
        break;
    }

    return image;
}

// enable to create the template images for the format test
// #define FORMATTEST_CREATE_MODE

/*!
 * \brief formatTest
 * Check if the plugin is able to write all QImage formats
 */
int formatTest(const QString &suffix, bool createTemplates)
{
    auto formatFolder = QStringLiteral("%1/format/%2").arg(IMAGEDIR, suffix);

    int passed = 0;
    int failed = 0;
    int skipped = 0;

    QTextStream(stdout) << "********* "
                        << "Starting FORMAT write tests for " << suffix << " images *********\n";

    for (int i = QImage::Format_Invalid + 1; i < QImage::NImageFormats; ++i) {
        auto format = QImage::Format(i);
        auto formatName = QString(QMetaEnum::fromType<QImage::Format>().valueToKey(format));

        // get template image
        auto srcImage = formatSourceImage(format);
        if (srcImage.isNull()) {
            ++skipped;
            QTextStream(stdout) << "SKIP : no source image found for " << formatName << "\n";
            continue;
        }

        // convert source to the wanted format
        if (srcImage.format() != format)
            srcImage.convertTo(format);

        QByteArray ba;
        { // writing the image
            QBuffer buffer(&ba);
            QImageWriter writer(&buffer, suffix.toLatin1());
            writer.setQuality(100); // always lossless
            if (!writer.write(srcImage)) {
                ++failed;
                QTextStream(stdout) << "FAIL : failed to write image " << formatName << "\n";
                continue;
            }
        }

        // read the image
        QBuffer buffer(&ba);
        auto writtenImage = QImageReader(&buffer, suffix.toLatin1()).read();
        if (writtenImage.isNull()) {
            if (suffix.toLatin1() == "heif") {
                // libheif + ffmpeg decoder is unable to load all HEIF files.
                ++skipped;
            } else {
                ++failed;
            }
            QTextStream(stdout) << "FAIL : error while reading the image " << formatName << "\n";
            continue;
        }

        // read the template image
        auto templateName = QStringLiteral("%1/%2.%3").arg(formatFolder, formatName, suffix);
        if (createTemplates) {
            QDir().mkdir(formatFolder);
            QFile f(templateName);
            if (!f.open(QFile::WriteOnly) || f.write(ba) == -1) {
                ++failed;
                QTextStream(stdout) << "FAIL : error while creating template image " << formatName << "\n";
                continue;
            }
        }

        if (!QFile::exists(templateName)) {
            ++skipped;
            QTextStream(stdout) << "SKIP : no template image found for " << formatName << "\n";
            continue;
        }
        auto tmplImage = QImageReader(templateName, suffix.toLatin1()).read();
        if (tmplImage.isNull()) {
            ++failed;
            QTextStream(stdout) << "FAIL : error while reading template image " << formatName << "\n";
            continue;
        }

        // checking the format: must be the same
        if (writtenImage.format() != tmplImage.format()) {
            ++failed;
            auto writtenformatName = QString(QMetaEnum::fromType<QImage::Format>().valueToKey(writtenImage.format()));
            auto tmplformatName = QString(QMetaEnum::fromType<QImage::Format>().valueToKey(tmplImage.format()));
            QTextStream(stdout) << "FAIL : format mismatch " << formatName << " (";
            if (format == writtenImage.format()) {
                QTextStream(stdout) << "template image: " << tmplformatName << ")\n";
            } else if (format == tmplImage.format()) {
                QTextStream(stdout) << "written image: " << writtenformatName << ")\n";
            } else {
                QTextStream(stdout) << writtenformatName << " != " << tmplformatName << ")\n";
            }
            continue;
        }

        // This comparison is only to understand if the plugin has written a completely wrong image. I therefore have no
        // qualms about converting them to a more convenient format or to tolerate slightly different pixels.
        auto compareFormat = writtenImage.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32;
        if (!fuzzyeq(writtenImage.convertToFormat(compareFormat), tmplImage.convertToFormat(compareFormat), 5)) {
            ++failed;
            QTextStream(stdout) << "FAIL : re-reading the data resulted in a different image " << formatName << "\n";
            continue;
        }

        // test passed!
        QTextStream(stdout) << "PASS : " << formatName << "\n";
        ++passed;
    }

    QTextStream(stdout) << "Totals: " << passed << " passed, " << failed << " failed, " << skipped << " skipped\n";
    QTextStream(stdout) << "********* "
                        << "Finished FORMAT write tests for " << suffix << " images *********\n";

    return failed == 0 ? 0 : 1;
}

int sizeTest(const QString &suffix)
{
    auto formatFolder = QStringLiteral("%1/format/%2").arg(IMAGEDIR, suffix);

    int passed = 0;
    int failed = 0;
    int skipped = 0;

    QTextStream(stdout) << "********* "
                        << "Starting SIZE write tests for " << suffix << " images *********\n";

    for (int i = QImage::Format_Invalid + 1; i < QImage::NImageFormats; ++i) {
        auto format = QImage::Format(i);
        auto formatName = QString(QMetaEnum::fromType<QImage::Format>().valueToKey(format));

        // read template image
        auto templateImage = QImageReader(QStringLiteral("%1/%2.%3").arg(formatFolder, formatName, suffix)).read();
        if (templateImage.isNull()) {
            ++skipped;
            QTextStream(stdout) << "SKIP : no source image found for " << formatName << "\n";
            continue;
        }

        // clang-format off
        auto sizeList = QList<QSize>()
            // check for byte alignment
            << (templateImage.size() + QSize(1, 1))
            << (templateImage.size() + QSize(2, 2))
            << (templateImage.size() + QSize(3, 3))
            << (templateImage.size() + QSize(4, 4))
            << (templateImage.size() + QSize(5, 5))
            << (templateImage.size() + QSize(6, 6))
            << (templateImage.size() + QSize(7, 7))
            // Check for dividers (using prime numbers)
            << QSize(113, 157)
            << QSize(163, 109);
        // clang-format on

        for (auto &&sz : sizeList) {
            auto debugInfo = QStringLiteral("%1 @%2x%3").arg(formatName).arg(sz.width()).arg(sz.height());

            // resize the template image
            auto resizeImage = templateImage.scaled(sz);
            // make sure the format is not changed by resize (paranoia)
            resizeImage.convertTo(templateImage.format());

            // save test
            QByteArray ba;
            { // writing the image
                QBuffer buffer(&ba);
                QImageWriter writer(&buffer, suffix.toLatin1());
                writer.setQuality(100); // always lossless
                if (!writer.write(resizeImage)) {
                    ++failed;
                    QTextStream(stdout) << "FAIL : failed to write image " << debugInfo << "\n";
                    continue;
                }
            }

            // read test
            QBuffer buffer(&ba);
            auto writtenImage = QImageReader(&buffer, suffix.toLatin1()).read();
            if (writtenImage.isNull()) {
                ++failed;
                QTextStream(stdout) << "FAIL : error while reading the image " << debugInfo << "\n";
                continue;
            }

            // This comparison is only to understand if the plugin has written a completely wrong image. I therefore have no
            // qualms about converting them to a more convenient format or to tolerate slightly different pixels.
            auto compareFormat = writtenImage.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32;
            if (!fuzzyeq(writtenImage.convertToFormat(compareFormat), resizeImage.convertToFormat(compareFormat), 5)) {
                ++failed;
                QTextStream(stdout) << "FAIL : re-reading the data resulted in a different image " << debugInfo << "\n";
                continue;
            }

            // test passed!
            QTextStream(stdout) << "PASS : " << debugInfo << "\n";
            ++passed;
        }
    }

    QTextStream(stdout) << "Totals: " << passed << " passed, " << failed << " failed, " << skipped << " skipped\n";
    QTextStream(stdout) << "********* "
                        << "Finished SIZE write tests for " << suffix << " images *********\n";

    return failed == 0 ? 0 : 1;
}

/*!
 * \brief nullDeviceTest
 * Checks the plugin behaviour when using a NULL device.
 */
int nullDeviceTest(const QString &suffix)
{
    QTextStream(stdout) << "********* "
                        << "Starting NULL device write tests for " << suffix << " images *********\n";

    int passed = 0;
    int failed = 0;
    int skipped = 0;

    QImageWriter writer;
    writer.setFormat(suffix.toLatin1());

    if (writer.canWrite()) {
        QTextStream(stdout) << "FAIL : canWrite() returns TRUE\n";
        ++failed;
    }

    if (writer.write(QImage(16, 16, QImage::Format_ARGB32))) {
        QTextStream(stdout) << "FAIL : write() returns TRUE\n";
        ++failed;
    }

    // test for crash only
    writer.compression();
    writer.quality();
    writer.transformation();
    writer.subType();
    writer.supportedSubTypes();
    writer.optimizedWrite();
    writer.progressiveScanWrite();

    if (failed == 0) { // success
        ++passed;
    }

    QTextStream(stdout) << "Totals: " << passed << " passed, " << failed << " failed, " << skipped << " skipped\n";
    QTextStream(stdout) << "********* "
                        << "Finished format write tests for " << suffix << " images *********\n";

    return failed == 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::removeLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::addLibraryPath(QStringLiteral(PLUGIN_DIR));
    QCoreApplication::setApplicationName(QStringLiteral("writetest"));
    QCoreApplication::setApplicationVersion(QStringLiteral("2.0.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Performs basic image conversion checking."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("format"), QStringLiteral("format to test."));
    QCommandLineOption lossless(QStringList() << QStringLiteral("l") << QStringLiteral("lossless"),
                                QStringLiteral("Check that reading back the data gives the same image."));
    QCommandLineOption ignoreDataCheck({QStringLiteral("no-data-check")}, QStringLiteral("Don't check that write data is exactly the same."));
    QCommandLineOption fuzz(QStringList() << QStringLiteral("f") << QStringLiteral("fuzz"),
                            QStringLiteral("Allow for some deviation in ARGB data."),
                            QStringLiteral("max"));
    QCommandLineOption createFormatTempates({QStringLiteral("create-format-templates")},
                                            QStringLiteral("Create template images for all formats supported by QImage."));
    QCommandLineOption skipOptTest({QStringLiteral("skip-optional-tests")}, QStringLiteral("Skip optional data tests (metadata, resolution, etc.)."));

    parser.addOption(lossless);
    parser.addOption(ignoreDataCheck);
    parser.addOption(fuzz);
    parser.addOption(createFormatTempates);
    parser.addOption(skipOptTest);

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.count() < 1) {
        QTextStream(stderr) << "Must provide a format\n";
        parser.showHelp(1);
    } else if (args.count() > 1) {
        QTextStream(stderr) << "Too many arguments\n";
        parser.showHelp(1);
    }

    uint fuzzarg = 0;
    if (parser.isSet(fuzz)) {
        bool ok;
        fuzzarg = parser.value(fuzz).toUInt(&ok);
        if (!ok || fuzzarg > 255) {
            QTextStream(stderr) << "Error: max fuzz argument must be a number between 0 and 255\n";
            parser.showHelp(1);
        }
    }

    auto suffix = args.at(0);

    // skip test if libheif configuration is obviously incomplete
    QByteArray format = suffix.toLatin1();
    const QList<QByteArray> read_formats = QImageReader::supportedImageFormats();
    const QList<QByteArray> write_formats = QImageWriter::supportedImageFormats();

    if (!read_formats.contains(format)) {
        if (format == "heif" || format == "hej2") {
            QTextStream(stdout) << "WARNING : libheif configuration is missing necessary decoder(s)!\n";
            return 0;
        }
    }

    if (!write_formats.contains(format)) {
        if (format == "heif" || format == "hej2") {
            QTextStream(stdout) << "WARNING : libheif configuration is missing necessary encoder(s)!\n";
            return 0;
        }
    }

    // run test
    auto ret = basicTest(suffix, parser.isSet(lossless), parser.isSet(ignoreDataCheck), parser.isSet(skipOptTest), fuzzarg);
    if (ret == 0) {
        ret = formatTest(suffix, parser.isSet(createFormatTempates));
    }
    if (ret == 0) {
        ret = sizeTest(suffix);
    }
    if (ret == 0) {
        ret = nullDeviceTest(suffix);
    }

    return ret;
}
