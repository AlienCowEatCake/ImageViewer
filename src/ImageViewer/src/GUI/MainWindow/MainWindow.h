/*
   Copyright (C) 2017-2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined(MAINWINDOW_H_INCLUDED)
#define MAINWINDOW_H_INCLUDED

#include <QMainWindow>

#include "Utils/ScopedPointer.h"
#include "ImageViewerWidget.h"
#include "../UIState.h"

class QString;
class QStringList;
class GUISettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

signals:
    void selectFirstRequested();
    void selectLastRequested();
    void selectPreviousRequested();
    void selectNextRequested();
    void openPathRequested(const QString &path);
    void openPathsRequested(const QStringList &paths);
    void openFileWithDialogRequested();
    void openFolderWithDialogRequested();
    void deleteFileRequested();

    void newWindowRequested();
    void preferencesRequested();
    void aboutRequested();
    void aboutQtRequested();
    void editStylesheetRequested();

    void closed();

public:
    MainWindow(GUISettings *settings, QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateWindowTitle();
    void switchFullScreenMode();
    void switchSlideShowMode();
    void switchShowMenuBar();
    void switchShowToolBar();

    void onZoomModeChanged(ImageViewerWidget::ZoomMode mode);
    void onSaveAsRequested();

    void onZoomFitToWindowRequested();
    void onZoomOriginalSizeRequested();

    void updateUIState(const UIState &state, const UIChangeFlags &changeFlags);

    void saveGeometrySettings();

private slots:
    void updateSlideShowInterval();
    void updateBackgroundColor();
    void onActionReopenWithTriggered(QAction *action);
    void rotateClockwise();
    void rotateCounterclockwise();

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    struct UI;
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAINWINDOW_H_INCLUDED
