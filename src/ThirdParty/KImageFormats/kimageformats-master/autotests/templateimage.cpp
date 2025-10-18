/*
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "templateimage.h"

#include <QColorSpace>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QVersionNumber>

static QJsonObject searchObject(const QFileInfo& file)
{
    auto fi = QFileInfo(QStringLiteral("%1.json").arg(file.filePath()));
    if (!fi.exists()) {
        return {};
    }

    QFile f(fi.filePath());
    if (!f.open(QFile::ReadOnly)) {
        return {};
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
        return {};
    }

    auto currentQt = QVersionNumber::fromString(qVersion());
    auto arr = doc.array();
    for (auto val : arr) {
        if (!val.isObject())
            continue;
        auto obj = val.toObject();
        auto minQt = QVersionNumber::fromString(obj.value("minQtVersion").toString());
        auto maxQt = QVersionNumber::fromString(obj.value("maxQtVersion").toString());
        auto name = obj.value("fileName").toString();
        auto unsupportedFormat = obj.value("unsupportedFormat").toBool();

        // filter
        if (name.isEmpty() && !unsupportedFormat)
            continue;
        if (!minQt.isNull() && currentQt < minQt)
            continue;
        if (!maxQt.isNull() && currentQt > maxQt)
            continue;
        return obj;
    }

    return {};
}


TemplateImage::TemplateImage(const QFileInfo &fi) :
    m_fi(fi)
{

}

bool TemplateImage::isTemplate() const
{
    auto list = suffixes();
    for (auto&& suffix : list) {
        if (!m_fi.suffix().compare(suffix, Qt::CaseInsensitive))
            return true;
    }
    return false;
}

bool TemplateImage::isLicense() const
{
    return !m_fi.suffix().compare(QStringLiteral("license"), Qt::CaseInsensitive);
}

bool TemplateImage::skipSequentialDeviceTest() const
{
    auto obj = searchObject(m_fi);
    if (obj.isEmpty()) {
        return false;
    }
    return obj.value("skipSequential").toBool();
}

QFileInfo TemplateImage::compareImage(TestFlags &flags, QString& comment) const
{
    auto fi = jsonImage(flags, comment);
    if ((flags & TestFlag::SkipTest) == TestFlag::SkipTest) {
        return {};
    }
    if (fi.exists()) {
        return fi;
    }
    return legacyImage();
}

bool TemplateImage::checkOptionaInfo(const QImage& image, QString& error) const
{
    auto obj = searchObject(m_fi);
    if (obj.isEmpty()) {
        return true;
    }

    // test color space (icc profile)
    auto color = obj.value("colorSpace").toObject();
    if (!color.isEmpty()) {
        auto dsc = color.value("description").toString();
        auto clm = color.value("colorModel").toString(QStringLiteral("Rgb"));
        auto prm = color.value("primaries").toString();
        auto trf = color.value("transferFunction").toString();
        auto gmm = color.value("gamma").toDouble();
        auto cs = image.colorSpace();

        if (cs.description() != dsc) {
            error = QStringLiteral("ColorSpace Description mismatch (current: %1, expected: %2)!").arg(cs.description(), dsc);
            return false;
        }

        auto prmName = QString(QMetaEnum::fromType<QColorSpace::Primaries>().valueToKey(quint64(cs.primaries())));
        if (prmName != prm) {
            error = QStringLiteral("ColorSpace Primaries mismatch (current: %1, expected: %2)!").arg(prmName, prm);
            return false;
        }

        auto trfName = QString(QMetaEnum::fromType<QColorSpace::TransferFunction>().valueToKey(quint64(cs.transferFunction())));
        if (trfName != trf) {
            error = QStringLiteral("ColorSpace TransferFunction mismatch (current: %1, expected: %2)!").arg(trfName, trf);
            return false;
        }

        if (qAbs(cs.gamma() - gmm) > 0.01) {
            error = QStringLiteral("ColorSpace Gamma mismatch (current: %1, expected: %2)!").arg(cs.gamma()).arg(gmm);
            return false;
        }

        auto clmName = QString(QMetaEnum::fromType<QColorSpace::ColorModel>().valueToKey(quint64(cs.colorModel())));
        if (clmName != clm) {
            error = QStringLiteral("ColorSpace ColorModel mismatch (current: %1, expected: %2)!").arg(clmName, clm);
            return false;
        }
    }

    // Test resolution
    auto res = obj.value("resolution").toObject();
    if (!res.isEmpty()) {
        auto resx = res.value("dotsPerMeterX").toInt();
        auto resy = res.value("dotsPerMeterY").toInt();
        if (resx != image.dotsPerMeterX()) {
            error = QStringLiteral("X resolution mismatch (current: %1, expected: %2)!").arg(image.dotsPerMeterX()).arg(resx);
            return false;
        }
        if (resy != image.dotsPerMeterY()) {
            error = QStringLiteral("Y resolution mismatch (current: %1, expected: %2)!").arg(image.dotsPerMeterY()).arg(resy);
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
            error = QStringLiteral("Metadata '%1' mismatch (current: '%2', expected:'%3')!").arg(key, cur, val);
            return false;
        }
    }

    return true;
}

quint8 TemplateImage::fuzziness() const
{
    auto obj = searchObject(m_fi);
    if (obj.isEmpty()) {
        return quint8(0);
    }
    return quint8(obj.value("fuzziness").toInt());
}

bool TemplateImage::perceptiveFuzziness() const
{
    auto obj = searchObject(m_fi);
    if (obj.isEmpty()) {
        return false;
    }
    return quint8(obj.value("perceptiveFuzziness").toBool());
}

QStringList TemplateImage::suffixes()
{
    return QStringList({"png", "tif", "tiff", "json"});
}

QFileInfo TemplateImage::legacyImage() const
{
    auto list = suffixes();
    for (auto&& suffix : list) {
        auto fi = QFileInfo(QStringLiteral("%1/%2.%3").arg(m_fi.path(), m_fi.completeBaseName(), suffix));
        if (fi.exists()) {
            return fi;
        }
    }
    return {};
}

QFileInfo TemplateImage::jsonImage(TestFlags &flags, QString& comment) const
{
    flags = TestFlag::None;

    auto obj = searchObject(m_fi);
    if (obj.isEmpty()) {
        return {};
    }

    auto name = obj.value("fileName").toString();
    auto unsupportedFormat = obj.value("unsupportedFormat").toBool();
    comment = obj.value("comment").toString();

    if(obj.value("disableAutoTransform").toBool()) {
        flags |= TestFlag::DisableAutotransform;
    }

    if (unsupportedFormat) {
        flags |= TestFlag::SkipTest;
        return {};
    }

    return QFileInfo(QStringLiteral("%1/%2").arg(m_fi.path(), name));
}

