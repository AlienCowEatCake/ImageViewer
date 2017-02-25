#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include <QMainWindow>
#include <QString>

#include "ImageViewerWidget.h"
#include <QScopedPointer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onZoomModeChanged(ImageViewerWidget::ZoomMode mode);
    void onZoomFitToWindowRequested();
    void onZoomOriginalSizeRequested();
    void onOpenFileRequested(const QString &filename);
    void onOpenFileWithDialogRequested();
    void onExitRequested();

private:
    struct UI;
    QScopedPointer<UI> m_ui;
    struct Impl;
    QScopedPointer<Impl> m_impl;
};

#endif // MAINWINDOW_H_INCLUDED
