/*
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "templateimage.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

