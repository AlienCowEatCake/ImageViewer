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
    if(enabled != oldValue)
        emit askBeforeDeleteChanged(enabled);
    m_impl->settings.setValue(QString::fromLatin1("AskBeforeDelete"), enabled);
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
    if(enabled != oldValue)
        emit moveToTrashChanged(enabled);
    m_impl->settings.setValue(QString::fromLatin1("MoveToTrash"), enabled);
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
    if(mode != oldValue)
        emit zoomModeChanged(mode);
    m_impl->settings.setValue(QString::fromLatin1("ZoomMode"), zoomModeToVariant(mode));
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
    if(level != oldValue)
        emit zoomLevelChanged(level);
    m_impl->settings.setValue(QString::fromLatin1("ZoomLevel"), level);
}

QColor GUISettings::backgroundColor() const
{
    const QColor defaultValue = Qt::white;
    QVariant value = m_impl->settings.value(QString::fromLatin1("BackgroundColor"), defaultValue);
    return (value.isValid() && value.canConvert(QVariant::Color) ? value.value<QColor>() : defaultValue);
}

void GUISettings::setBackgroundColor(const QColor &color)
{
    const QColor oldValue = backgroundColor();
    if(color != oldValue)
        emit backgroundColorChanged(color);
    m_impl->settings.setValue(QString::fromLatin1("BackgroundColor"), color);
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
    if(path != oldValue)
        emit lastOpenedPathChanged(path);
    m_impl->settings.setValue(QString::fromLatin1("LastOpenedPath"), path);
}
