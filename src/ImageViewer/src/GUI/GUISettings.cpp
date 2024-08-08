/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

ImageViewerWidget::WheelMode wheelModeFromVariant(const QVariant &variant, ImageViewerWidget::WheelMode defaultValue)
{
    bool ok;
    int value = variant.toInt(&ok);
    if(!ok)
        return defaultValue;
    switch(value)
    {
    case ImageViewerWidget::WHEEL_SCROLL:
        return ImageViewerWidget::WHEEL_SCROLL;
    case ImageViewerWidget::WHEEL_ZOOM:
        return ImageViewerWidget::WHEEL_ZOOM;
    default:
        break;
    }
    return defaultValue;
}

QVariant wheelModeToVariant(ImageViewerWidget::WheelMode mode)
{
    return mode;
}

GUISettings::ToolBarPosition toolBarPositionFromVariant(const QVariant &variant, GUISettings::ToolBarPosition defaultValue)
{
    bool ok;
    int value = variant.toInt(&ok);
    if(!ok)
        return defaultValue;
    switch(value)
    {
    case GUISettings::TOOLBAR_POSITION_BOTTOM:
        return GUISettings::TOOLBAR_POSITION_BOTTOM;
    case GUISettings::TOOLBAR_POSITION_TOP:
        return GUISettings::TOOLBAR_POSITION_TOP;
    case GUISettings::TOOLBAR_POSITION_LEFT:
        return GUISettings::TOOLBAR_POSITION_LEFT;
    case GUISettings::TOOLBAR_POSITION_RIGHT:
        return GUISettings::TOOLBAR_POSITION_RIGHT;
    case GUISettings::TOOLBAR_POSITION_MOVABLE:
        return GUISettings::TOOLBAR_POSITION_MOVABLE;
    default:
        break;
    }
    return defaultValue;
}

QVariant toolBarPositionToVariant(GUISettings::ToolBarPosition mode)
{
    return mode;
}

const QString ASK_BEFORE_DELETE_KEY             = QString::fromLatin1("AskBeforeDelete");
const QString MOVE_TO_TRASH_KEY                 = QString::fromLatin1("MoveToTrash");
const QString ZOOM_MODE_KEY                     = QString::fromLatin1("ZoomMode");
const QString ZOOM_LEVEL_KEY                    = QString::fromLatin1("ZoomLevel");
const QString WHEEL_MODE_KEY                    = QString::fromLatin1("WheelMode");
const QString NORMAL_BACKGROUND_COLOR_KEY       = QString::fromLatin1("NormalBackgroundColor");
const QString FULLSCREEN_BACKGROUND_COLOR_KEY   = QString::fromLatin1("FullScreenBackgroundColor");
const QString LAST_OPENED_PATH_KEY              = QString::fromLatin1("LastOpenedPath");
const QString SMOOTH_TRANSFORMATION_KEY         = QString::fromLatin1("SmoothTransformation");
const QString UPSCALE_ON_FIT_TO_WINDOW_KEY      = QString::fromLatin1("UpscaleOnFitToWindow");
const QString REMEMBER_EFFECTS_DURING_SESSION   = QString::fromLatin1("RememberEffectsDuringSession");
const QString MAIN_WINDOW_GEOMETRY_KEY          = QString::fromLatin1("MainWindowGeometry");
const QString MAIN_WINDOW_STATE_KEY             = QString::fromLatin1("MainWindowState");
const QString SLIDESHOW_INTERVAL_KEY            = QString::fromLatin1("SlideShowInterval");
const QString MENUBAR_VISIBLE_KEY               = QString::fromLatin1("MenuBarVisible");
const QString TOOLBAR_VISIBLE_KEY               = QString::fromLatin1("ToolBarVisible");
const QString TOOLBAR_POSITION_KEY              = QString::fromLatin1("ToolBarPosition");

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

void GUISettings::flush()
{
    m_impl->settings.flush();
}

bool GUISettings::askBeforeDelete() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(ASK_BEFORE_DELETE_KEY, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setAskBeforeDelete(bool enabled)
{
    const bool oldValue = askBeforeDelete();
    m_impl->settings.setValue(ASK_BEFORE_DELETE_KEY, enabled);
    if(enabled != oldValue)
        Q_EMIT askBeforeDeleteChanged(enabled);
}

bool GUISettings::moveToTrash() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(MOVE_TO_TRASH_KEY, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setMoveToTrash(bool enabled)
{
    const bool oldValue = moveToTrash();
    m_impl->settings.setValue(MOVE_TO_TRASH_KEY, enabled);
    if(enabled != oldValue)
        Q_EMIT moveToTrashChanged(enabled);
}

ImageViewerWidget::ZoomMode GUISettings::zoomMode() const
{
    const ImageViewerWidget::ZoomMode defaultMode = ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    const QVariant defaultValue = zoomModeToVariant(defaultMode);
    QVariant value = m_impl->settings.value(ZOOM_MODE_KEY, defaultValue);
    return zoomModeFromVariant((value.isValid() ? value : defaultValue), defaultMode);
}

void GUISettings::setZoomMode(ImageViewerWidget::ZoomMode mode)
{
    const ImageViewerWidget::ZoomMode oldValue = zoomMode();
    m_impl->settings.setValue(ZOOM_MODE_KEY, zoomModeToVariant(mode));
    if(mode != oldValue)
        Q_EMIT zoomModeChanged(mode);
}

qreal GUISettings::zoomLevel() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Double);
#else
    const QVariant::Type type = QVariant::Double;
#endif
    const qreal defaultValue = 1;
    QVariant variant = m_impl->settings.value(ZOOM_LEVEL_KEY);
    if(!variant.isValid() || !variant.canConvert(type))
        return defaultValue;
    bool ok;
    qreal value = variant.toDouble(&ok);
    return (ok && value > 0 && value < std::numeric_limits<qreal>::max()) ? static_cast<qreal>(value) : defaultValue;
}

void GUISettings::setZoomLevel(qreal level)
{
    const qreal oldValue = zoomLevel();
    m_impl->settings.setValue(ZOOM_LEVEL_KEY, level);
    if(level != oldValue)
        Q_EMIT zoomLevelChanged(level);
}

ImageViewerWidget::WheelMode GUISettings::wheelMode() const
{
    const ImageViewerWidget::WheelMode defaultMode = ImageViewerWidget::WHEEL_SCROLL;
    const QVariant defaultValue = wheelModeToVariant(defaultMode);
    QVariant value = m_impl->settings.value(WHEEL_MODE_KEY, defaultValue);
    return wheelModeFromVariant((value.isValid() ? value : defaultValue), defaultMode);
}

void GUISettings::setWheelMode(ImageViewerWidget::WheelMode mode)
{
    const ImageViewerWidget::WheelMode oldValue = wheelMode();
    m_impl->settings.setValue(WHEEL_MODE_KEY, wheelModeToVariant(mode));
    if(mode != oldValue)
        Q_EMIT wheelModeChanged(mode);
}

QColor GUISettings::normalBackgroundColor() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::QColor);
#else
    const QVariant::Type type = QVariant::Color;
#endif
    const QColor defaultValue(255, 255, 255, 128);
    QVariant value = m_impl->settings.value(NORMAL_BACKGROUND_COLOR_KEY, defaultValue);
    return (value.isValid() && value.canConvert(type) ? value.value<QColor>() : defaultValue);
}

void GUISettings::setNormalBackgroundColor(const QColor &color)
{
    const QColor oldValue = normalBackgroundColor();
    m_impl->settings.setValue(NORMAL_BACKGROUND_COLOR_KEY, color);
    if(color != oldValue)
        Q_EMIT normalBackgroundColorChanged(color);
}

QColor GUISettings::fullScreenBackgroundColor() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::QColor);
#else
    const QVariant::Type type = QVariant::Color;
#endif
    const QColor defaultValue(0, 0, 0, 128);
    QVariant value = m_impl->settings.value(FULLSCREEN_BACKGROUND_COLOR_KEY, defaultValue);
    return (value.isValid() && value.canConvert(type) ? value.value<QColor>() : defaultValue);
}

void GUISettings::setFullScreenBackgroundColor(const QColor &color)
{
    const QColor oldValue = fullScreenBackgroundColor();
    m_impl->settings.setValue(FULLSCREEN_BACKGROUND_COLOR_KEY, color);
    if(color != oldValue)
        Q_EMIT fullScreenBackgroundColorChanged(color);
}

QString GUISettings::lastOpenedPath() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::QString);
#else
    const QVariant::Type type = QVariant::String;
#endif
    const QString defaultValue;
    QVariant value = m_impl->settings.value(LAST_OPENED_PATH_KEY, defaultValue);
    return (value.isValid() && value.canConvert(type) ? value.toString() : defaultValue);
}

void GUISettings::setLastOpenedPath(const QString &path)
{
    const QString oldValue = lastOpenedPath();
    m_impl->settings.setValue(LAST_OPENED_PATH_KEY, path);
    if(path != oldValue)
        Q_EMIT lastOpenedPathChanged(path);
}

bool GUISettings::smoothTransformation() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(SMOOTH_TRANSFORMATION_KEY, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setSmoothTransformation(bool enabled)
{
    const bool oldValue = smoothTransformation();
    m_impl->settings.setValue(SMOOTH_TRANSFORMATION_KEY, enabled);
    if(enabled != oldValue)
        Q_EMIT smoothTransformationChanged(enabled);
}

bool GUISettings::upscaleOnFitToWindow() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = false;
    QVariant value = m_impl->settings.value(UPSCALE_ON_FIT_TO_WINDOW_KEY, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setUpscaleOnFitToWindow(bool enabled)
{
    const bool oldValue = upscaleOnFitToWindow();
    m_impl->settings.setValue(UPSCALE_ON_FIT_TO_WINDOW_KEY, enabled);
    if(enabled != oldValue)
        Q_EMIT upscaleOnFitToWindowChanged(enabled);
}

bool GUISettings::rememberEffectsDuringSession() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = false;
    QVariant value = m_impl->settings.value(REMEMBER_EFFECTS_DURING_SESSION, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setRememberEffectsDuringSession(bool enabled)
{
    const bool oldValue = rememberEffectsDuringSession();
    m_impl->settings.setValue(REMEMBER_EFFECTS_DURING_SESSION, enabled);
    if(enabled != oldValue)
        Q_EMIT rememberEffectsDuringSessionChanged(enabled);
}

QByteArray GUISettings::mainWindowGeometry() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::QByteArray);
#else
    const QVariant::Type type = QVariant::ByteArray;
#endif
    QVariant value = m_impl->settings.value(MAIN_WINDOW_GEOMETRY_KEY);
    return value.isValid() && value.canConvert(type) ? value.toByteArray() : QByteArray();
}

void GUISettings::setMainWindowGeometry(const QByteArray &geometry)
{
    const QByteArray oldValue = mainWindowGeometry();
    m_impl->settings.setValue(MAIN_WINDOW_GEOMETRY_KEY, geometry);
    if(geometry != oldValue)
        Q_EMIT mainWindowGeometryChanged(geometry);
}

QByteArray GUISettings::mainWindowState() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::QByteArray);
#else
    const QVariant::Type type = QVariant::ByteArray;
#endif
    QVariant value = m_impl->settings.value(MAIN_WINDOW_STATE_KEY);
    return value.isValid() && value.canConvert(type) ? value.toByteArray() : QByteArray();
}

void GUISettings::setMainWindowState(const QByteArray &state)
{
    const QByteArray oldValue = mainWindowGeometry();
    m_impl->settings.setValue(MAIN_WINDOW_STATE_KEY, state);
    if(state != oldValue)
        Q_EMIT mainWindowGeometryChanged(state);
}

int GUISettings::slideShowInterval() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Int);
#else
    const QVariant::Type type = QVariant::Int;
#endif
    const int defaultValue = 5;
    QVariant variant = m_impl->settings.value(SLIDESHOW_INTERVAL_KEY);
    if(!variant.isValid() || !variant.canConvert(type))
        return defaultValue;
    bool ok;
    int value = variant.toInt(&ok);
    return (ok && value >= 1 && value <= 1000) ? value : defaultValue;
}

void GUISettings::setSlideShowInterval(int seconds)
{
    const int oldValue = slideShowInterval();
    m_impl->settings.setValue(SLIDESHOW_INTERVAL_KEY, seconds);
    if(seconds != oldValue)
        Q_EMIT slideShowIntervalChanged(seconds);
}

bool GUISettings::menuBarVisible() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(MENUBAR_VISIBLE_KEY, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setMenuBarVisible(bool visible)
{
    const bool oldValue = menuBarVisible();
    m_impl->settings.setValue(MENUBAR_VISIBLE_KEY, visible);
    if(visible != oldValue)
        Q_EMIT menuBarVisibleChanged(visible);
}

bool GUISettings::toolBarVisible() const
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    const QMetaType type(QMetaType::Bool);
#else
    const QVariant::Type type = QVariant::Bool;
#endif
    const bool defaultValue = true;
    QVariant value = m_impl->settings.value(TOOLBAR_VISIBLE_KEY, defaultValue);
    return value.isValid() && value.canConvert(type) ? value.toBool() : defaultValue;
}

void GUISettings::setToolBarVisible(bool visible)
{
    const bool oldValue = toolBarVisible();
    m_impl->settings.setValue(TOOLBAR_VISIBLE_KEY, visible);
    if(visible != oldValue)
        Q_EMIT toolBarVisibleChanged(visible);
}

GUISettings::ToolBarPosition GUISettings::toolBarPosition() const
{
    const ToolBarPosition defaultPosition = TOOLBAR_POSITION_DEFAULT;
    const QVariant defaultValue = toolBarPositionToVariant(defaultPosition);
    QVariant value = m_impl->settings.value(TOOLBAR_POSITION_KEY, defaultValue);
    return toolBarPositionFromVariant((value.isValid() ? value : defaultValue), defaultPosition);
}

void GUISettings::setToolBarPosition(ToolBarPosition position)
{
    const ToolBarPosition oldValue = toolBarPosition();
    m_impl->settings.setValue(TOOLBAR_POSITION_KEY, toolBarPositionToVariant(position));
    if(position != oldValue)
        Q_EMIT toolBarPositionChanged(position);
}
