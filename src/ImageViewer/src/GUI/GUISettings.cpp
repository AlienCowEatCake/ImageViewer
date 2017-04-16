/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "GUISettings.h"
#include <limits>
#include "Utils/SettingsWrapper.h"

namespace {

ImageViewerWidget::ZoomMode zoomModeFromVariant(const QVariant &variant, ImageViewerWidget::ZoomMode defaultValue)
{
    bool ok;
    int value = variant.toInt(&ok);
    if(!ok)
        return defaultValue;
    switch(value)
    {
    case ImageViewerWidget::ZOOM_IDENTITY:
        return ImageViewerWidget::ZOOM_IDENTITY;
    case ImageViewerWidget::ZOOM_FIT_TO_WINDOW:
        return ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    case ImageViewerWidget::ZOOM_CUSTOM:
        return ImageViewerWidget::ZOOM_CUSTOM;
    default:
        break;
    }
    return defaultValue;
}

QVariant zoomModeToVariant(ImageViewerWidget::ZoomMode mode)
{
    return mode;
}

} // namespace

struct GUISettings::Impl
{
    Impl()
        : settings(QString::fromLatin1("GUISettings"))
    {}
    SettingsWrapper settings;
};

GUISettings::GUISettings(QObject *parent)
    : QObject(parent)
    , m_impl(new Impl)
{}

GUISettings::~GUISettings()
{}

bool GUISettings::askBeforeDelete() const
{
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(QString::fromLatin1("AskBeforeDelete"), defaultValue);
    return value.isValid() && value.canConvert(QVariant::Bool) ? value.toBool() : defaultValue;
}

void GUISettings::setAskBeforeDelete(bool enabled)
{
    const bool oldValue = askBeforeDelete();
    m_impl->settings.setValue(QString::fromLatin1("AskBeforeDelete"), enabled);
    if(enabled != oldValue)
        emit askBeforeDeleteChanged(enabled);
}

bool GUISettings::moveToTrash() const
{
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(QString::fromLatin1("MoveToTrash"), defaultValue);
    return value.isValid() && value.canConvert(QVariant::Bool) ? value.toBool() : defaultValue;
}

void GUISettings::setMoveToTrash(bool enabled)
{
    const bool oldValue = moveToTrash();
    m_impl->settings.setValue(QString::fromLatin1("MoveToTrash"), enabled);
    if(enabled != oldValue)
        emit moveToTrashChanged(enabled);
}

ImageViewerWidget::ZoomMode GUISettings::zoomMode() const
{
    const ImageViewerWidget::ZoomMode defaultMode = ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    const QVariant defaultValue = zoomModeToVariant(defaultMode);
    QVariant value = m_impl->settings.value(QString::fromLatin1("ZoomMode"), defaultValue);
    return zoomModeFromVariant((value.isValid() ? value : defaultValue), defaultMode);
}

void GUISettings::setZoomMode(ImageViewerWidget::ZoomMode mode)
{
    const ImageViewerWidget::ZoomMode oldValue = zoomMode();
    m_impl->settings.setValue(QString::fromLatin1("ZoomMode"), zoomModeToVariant(mode));
    if(mode != oldValue)
        emit zoomModeChanged(mode);
}

qreal GUISettings::zoomLevel() const
{
    const qreal defaultValue = 1;
    QVariant value = m_impl->settings.value(QString::fromLatin1("ZoomLevel"), defaultValue);
    double newValue = (value.isValid() && value.canConvert(QVariant::Double) ? value.toDouble() : defaultValue);
    if(newValue > 0 && newValue < std::numeric_limits<qreal>::max())
        return static_cast<qreal>(newValue);
    return defaultValue;
}

void GUISettings::setZoomLevel(qreal level)
{
    const qreal oldValue = zoomLevel();
    m_impl->settings.setValue(QString::fromLatin1("ZoomLevel"), level);
    if(level != oldValue)
        emit zoomLevelChanged(level);
}

QColor GUISettings::normalBackgroundColor() const
{
    const QColor defaultValue(255, 255, 255, 128);
    QVariant value = m_impl->settings.value(QString::fromLatin1("NormalBackgroundColor"), defaultValue);
    return (value.isValid() && value.canConvert(QVariant::Color) ? value.value<QColor>() : defaultValue);
}

void GUISettings::setNormalBackgroundColor(const QColor &color)
{
    const QColor oldValue = normalBackgroundColor();
    m_impl->settings.setValue(QString::fromLatin1("NormalBackgroundColor"), color);
    if(color != oldValue)
        emit normalBackgroundColorChanged(color);
}

QColor GUISettings::fullScreenBackgroundColor() const
{
    const QColor defaultValue(0, 0, 0, 128);
    QVariant value = m_impl->settings.value(QString::fromLatin1("FullScreenBackgroundColor"), defaultValue);
    return (value.isValid() && value.canConvert(QVariant::Color) ? value.value<QColor>() : defaultValue);
}

void GUISettings::setFullScreenBackgroundColor(const QColor &color)
{
    const QColor oldValue = fullScreenBackgroundColor();
    m_impl->settings.setValue(QString::fromLatin1("FullScreenBackgroundColor"), color);
    if(color != oldValue)
        emit fullScreenBackgroundColorChanged(color);
}

QString GUISettings::lastOpenedPath() const
{
    const QString defaultValue;
    QVariant value = m_impl->settings.value(QString::fromLatin1("LastOpenedPath"), defaultValue);
    return (value.isValid() && value.canConvert(QVariant::String) ? value.toString() : defaultValue);
}

void GUISettings::setLastOpenedPath(const QString &path)
{
    const QString oldValue = lastOpenedPath();
    m_impl->settings.setValue(QString::fromLatin1("LastOpenedPath"), path);
    if(path != oldValue)
        emit lastOpenedPathChanged(path);
}

bool GUISettings::smoothTransformation() const
{
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(QString::fromLatin1("SmoothTransformation"), defaultValue);
    return value.isValid() && value.canConvert(QVariant::Bool) ? value.toBool() : defaultValue;
}

void GUISettings::setSmoothTransformation(bool enabled)
{
    const bool oldValue = smoothTransformation();
    m_impl->settings.setValue(QString::fromLatin1("SmoothTransformation"), enabled);
    if(enabled != oldValue)
        emit smoothTransformationChanged(enabled);
}

QByteArray GUISettings::mainWindowGeometry() const
{
    QVariant value = m_impl->settings.value(QString::fromLatin1("MainWindowGeometry"));
    return value.isValid() && value.canConvert(QVariant::ByteArray) ? value.toByteArray() : QByteArray();
}

void GUISettings::setMainWindowGeometry(const QByteArray &geometry)
{
    const QByteArray oldValue = mainWindowGeometry();
    m_impl->settings.setValue(QString::fromLatin1("MainWindowGeometry"), geometry);
    if(geometry != oldValue)
        emit mainWindowGeometryChanged(geometry);
}

QByteArray GUISettings::mainWindowState() const
{
    QVariant value = m_impl->settings.value(QString::fromLatin1("MainWindowState"));
    return value.isValid() && value.canConvert(QVariant::ByteArray) ? value.toByteArray() : QByteArray();
}

void GUISettings::setMainWindowState(const QByteArray &state)
{
    const QByteArray oldValue = mainWindowState();
    m_impl->settings.setValue(QString::fromLatin1("MainWindowState"), state);
    if(state != oldValue)
        emit mainWindowStateChanged(state);
}
