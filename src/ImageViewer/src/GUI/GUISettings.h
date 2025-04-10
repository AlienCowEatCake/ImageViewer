/*
   Copyright (C) 2017-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(GUISETTINGS_H_INCLUDED)
#define GUISETTINGS_H_INCLUDED

#include <QObject>
#include <QString>
#include <QColor>
#include <QByteArray>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "MainWindow/ImageViewerWidget.h"

class GUISettings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(GUISettings)

public:
    enum ToolBarPosition
    {
        TOOLBAR_POSITION_BOTTOM  = 1,
        TOOLBAR_POSITION_TOP     = 2,
        TOOLBAR_POSITION_LEFT    = 3,
        TOOLBAR_POSITION_RIGHT   = 4,
        TOOLBAR_POSITION_MOVABLE = 5,
        TOOLBAR_POSITION_DEFAULT = TOOLBAR_POSITION_BOTTOM
    };

Q_SIGNALS:
    void askBeforeDeleteChanged(bool enabled);
    void moveToTrashChanged(bool enabled);
    void zoomModeChanged(ImageViewerWidget::ZoomMode mode);
    void zoomLevelChanged(qreal level);
    void wheelModeChanged(ImageViewerWidget::WheelMode mode);
    void normalBackgroundColorChanged(const QColor &color);
    void fullScreenBackgroundColorChanged(const QColor &color);
    void lastOpenedPathChanged(const QString &path);
    void smoothTransformationChanged(bool enabled);
    void upscaleOnFitToWindowChanged(bool enabled);
    void rememberEffectsDuringSessionChanged(bool enabled);
    void mainWindowGeometryChanged(const QByteArray &geometry);
    void mainWindowStateChanged(const QByteArray &state);
    void mainWindowMaximizedChanged(bool maximized);
    void slideShowIntervalChanged(int seconds);
    void menuBarVisibleChanged(bool visible);
    void toolBarVisibleChanged(bool visible);
    void toolBarPositionChanged(GUISettings::ToolBarPosition position);

public:
    explicit GUISettings(QObject *parent = Q_NULLPTR);
    ~GUISettings();

    void flush();

    bool askBeforeDelete() const;
    void setAskBeforeDelete(bool enabled);

    bool moveToTrash() const;
    void setMoveToTrash(bool enabled);

    ImageViewerWidget::ZoomMode zoomMode() const;
    void setZoomMode(ImageViewerWidget::ZoomMode mode);

    qreal zoomLevel() const;
    void setZoomLevel(qreal level);

    ImageViewerWidget::WheelMode wheelMode() const;
    void setWheelMode(ImageViewerWidget::WheelMode mode);

    QColor normalBackgroundColor() const;
    void setNormalBackgroundColor(const QColor &color);

    QColor fullScreenBackgroundColor() const;
    void setFullScreenBackgroundColor(const QColor &color);

    QString lastOpenedPath() const;
    void setLastOpenedPath(const QString &path);

    bool smoothTransformation() const;
    void setSmoothTransformation(bool enabled);

    bool upscaleOnFitToWindow() const;
    void setUpscaleOnFitToWindow(bool enabled);

    bool rememberEffectsDuringSession() const;
    void setRememberEffectsDuringSession(bool enabled);

    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray &geometry);

    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray &state);

    bool mainWindowMaximized() const;
    void setMainWindowMaximized(bool maximized);

    int slideShowInterval() const;
    void setSlideShowInterval(int seconds);

    bool menuBarVisible() const;
    void setMenuBarVisible(bool visible);

    bool toolBarVisible() const;
    void setToolBarVisible(bool visible);

    ToolBarPosition toolBarPosition() const;
    void setToolBarPosition(ToolBarPosition position);

private:
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // GUISETTINGS_H_INCLUDED
