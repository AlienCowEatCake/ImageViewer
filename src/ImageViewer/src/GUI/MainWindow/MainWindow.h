/*
   Copyright (C) 2017-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QStringList>

#include "Utils/Global.h"
#include "Utils/ScopedPointer.h"

#include "ImageViewerWidget.h"
#include "../UIState.h"

class QString;
class GUISettings;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

Q_SIGNALS:
    void selectFirstRequested();
    void selectLastRequested();
    void selectPreviousRequested();
    void selectNextRequested();
    void openPathRequested(const QString &path);
    void openPathsRequested(const QStringList &paths);
    void openFileWithDialogRequested();
    void openFolderWithDialogRequested();
    void deleteFileRequested();
    void reopenWithRequested(const QString &decoderName);

    void newWindowRequested();
    void imageInformationRequested();
    void preferencesRequested();
    void aboutRequested();
    void aboutQtRequested();
    void checkForUpdatesRequested();
    void editStylesheetRequested();

    void closed();

public:
    MainWindow(GUISettings *settings, QWidget *parent = Q_NULLPTR);
    ~MainWindow();

public Q_SLOTS:
    void updateWindowTitle();
    void switchFullScreenMode();
    void switchSlideShowMode();
    void switchShowMenuBar();
    void switchShowToolBar();

    void onZoomModeChanged(ImageViewerWidget::ZoomMode mode);
    void onSaveAsRequested();
    void onPrintRequested();

    void onZoomCustomRequested();
    void onZoomFitToWindowRequested();
    void onZoomOriginalSizeRequested();

    void updateUIState(const UIState &state, const UIChangeFlags &changeFlags);

    void saveGeometrySettings();

private Q_SLOTS:
    void updateSlideShowInterval();
    void updateBackgroundColor();
    void updateToolBarPosition();
    void onActionReopenWithTriggered(QAction *action);

protected:
    void changeEvent(QEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent *event) Q_DECL_OVERRIDE;
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    void dragLeaveEvent(QDragLeaveEvent *event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;

private:
    struct UI;
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAINWINDOW_H_INCLUDED
