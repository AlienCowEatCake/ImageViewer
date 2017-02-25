#include "MainWindow.h"
#include "MainWindow_p.h"

#include <QLabel>
#include <QMovie>
#include <QGraphicsProxyWidget>
#include <QGraphicsSvgItem>
#include <QFileDialog>
#include <QApplication>

struct MainWindow::Impl
{
    Impl(MainWindow *mainWindow)
        : mainWindow(mainWindow)
    {}

    QString lastOpenedFilename;
    MainWindow *mainWindow;
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new UI(this))
    , m_impl(new Impl(this))
{
    connect(m_ui->zoomIn, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(zoomIn()));
    connect(m_ui->zoomOut, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(zoomOut()));
    connect(m_ui->zoomFitToWindow, SIGNAL(clicked()), this, SLOT(onZoomFitToWindowRequested()));
    connect(m_ui->zoomOriginalSize, SIGNAL(clicked()), this, SLOT(onZoomOriginalSizeRequested()));
    connect(m_ui->imageViewerWidget, SIGNAL(zoomModeChanged(ImageViewerWidget::ZoomMode)), this, SLOT(onZoomModeChanged(ImageViewerWidget::ZoomMode)));
    connect(m_ui->rotateCounterclockwise, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(rotateCounterclockwise()));
    connect(m_ui->rotateClockwise, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(rotateClockwise()));
    connect(m_ui->openFile, SIGNAL(clicked()), this, SLOT(onOpenFileWithDialogRequested()));
    connect(m_ui->quit, SIGNAL(clicked()), this, SLOT(onExitRequested()));

    m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_FIT_TO_WINDOW);
}

MainWindow::~MainWindow()
{}

void MainWindow::onZoomModeChanged(ImageViewerWidget::ZoomMode mode)
{
    m_ui->zoomFitToWindow->setChecked(mode == ImageViewerWidget::ZOOM_FIT_TO_WINDOW);
    m_ui->zoomOriginalSize->setChecked(mode == ImageViewerWidget::ZOOM_IDENTITY);
}

void MainWindow::onZoomFitToWindowRequested()
{
    m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_FIT_TO_WINDOW);
}

void MainWindow::onZoomOriginalSizeRequested()
{
    m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_IDENTITY);
}

void MainWindow::onOpenFileRequested(const QString &filename)
{
    // Pixmap example
//    m_ui->imageViewerWidget->setGraphicsItem(new QGraphicsPixmapItem(QPixmap("/home/peter/Projects/build-ImageViewer-Desktop_Qt_5_7_1_qt5-Debug/800px-RCA_Indian_Head_test_pattern.jpg")));

    // SVG example
//    m_ui->imageViewerWidget->setGraphicsItem(new QGraphicsSvgItem("/home/peter/Projects/build-ImageViewer-Desktop_Qt_5_7_1_qt5-Debug/splash_en.svg"));

    // GIF example
//    QLabel *gif_anim = new QLabel();
//    QMovie *movie = new QMovie("/home/peter/Projects/build-ImageViewer-Desktop_Qt_5_7_1_qt5-Debug/file.gif");
//    gif_anim->setMovie(movie);
//    movie->start();
//    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget();
//    proxy->setWidget(gif_anim);
//    m_ui->imageViewerWidget->setGraphicsItem(proxy);

    m_ui->imageViewerWidget->setGraphicsItem(new QGraphicsPixmapItem(QPixmap(filename)));

    m_impl->lastOpenedFilename = filename;
}

void MainWindow::onOpenFileWithDialogRequested()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), m_impl->lastOpenedFilename,
            QString("%1 (*.png *.jpg);;%2 (*.*)").arg(tr("All Supported Images")).arg(tr("All Files")));
    if(fileName.isEmpty())
        return;
    onOpenFileRequested(fileName);
}

void MainWindow::onExitRequested()
{
    close();
}
