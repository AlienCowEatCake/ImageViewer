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

#if !defined(MAINWINDOW_H_INCLUDED)
#define MAINWINDOW_H_INCLUDED

#include <QMainWindow>
#include <QString>

#include "Utils/ScopedPointer.h"
#include "ImageViewerWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setLanguage(const QString &newLanguage = QString());

public slots:
    void openNewWindow(const QString &filename = QString());
    void updateWindowTitle();
    void showAbout();
    void showPreferences();

    void onOpenPreviousRequested();
    void onOpenNextRequested();
    void onOpenFirstRequested();
    void onOpenLastRequested();
    void onZoomModeChanged(ImageViewerWidget::ZoomMode mode);
    void onOpenFileRequested(const QString &filename);
    void onOpenPathRequested(const QString &path);
    void onOpenFileWithDialogRequested();
    void onSaveAsRequested();
    void onDeleteFileRequested();
    void onExitRequested();

    void onZoomFitToWindowClicked();
    void onZoomOriginalSizeClicked();

    void onActionEnglishTriggered();
    void onActionRussianTriggered();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    struct UI;
    QScopedPointer<UI> m_ui;
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAINWINDOW_H_INCLUDED
