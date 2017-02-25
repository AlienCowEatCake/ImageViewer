#ifndef MAINWINDOW_P_H_INCLUDED
#define MAINWINDOW_P_H_INCLUDED

#include "MainWindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QToolButton>

#include "ImageViewerWidget.h"

struct MainWindow::UI
{
    QFrame *centralWidget;
    ImageViewerWidget *imageViewerWidget;
    QFrame *toolbar;
    QToolButton *navigatePrevious;
    QToolButton *navigateNext;
    QToolButton *zoomOut;
    QToolButton *zoomIn;
    QToolButton *zoomFitToWindow;
    QToolButton *zoomOriginalSize;
    QToolButton *rotateCounterclockwise;
    QToolButton *rotateClockwise;
    QToolButton *openFile;
    QToolButton *saveFileAs;
    QToolButton *deleteFile;
    QToolButton *preferences;
    QToolButton *quit;


    UI(MainWindow *mainWindow)
        : centralWidget(new QFrame(mainWindow))
        , imageViewerWidget(new ImageViewerWidget(centralWidget))
        , toolbar(new QFrame(centralWidget))
        , navigatePrevious(new QToolButton(toolbar))
        , navigateNext(new QToolButton(toolbar))
        , zoomOut(new QToolButton(toolbar))
        , zoomIn(new QToolButton(toolbar))
        , zoomFitToWindow(new QToolButton(toolbar))
        , zoomOriginalSize(new QToolButton(toolbar))
        , rotateCounterclockwise(new QToolButton(toolbar))
        , rotateClockwise(new QToolButton(toolbar))
        , openFile(new QToolButton(toolbar))
        , saveFileAs(new QToolButton(toolbar))
        , deleteFile(new QToolButton(toolbar))
        , preferences(new QToolButton(toolbar))
        , quit(new QToolButton(toolbar))
    {
        zoomFitToWindow->setCheckable(true);
        zoomOriginalSize->setCheckable(true);

        navigatePrevious->setText("<");
        navigateNext->setText(">");
        zoomOut->setText("-");
        zoomIn->setText("+");
        zoomFitToWindow->setText("[]");
        zoomOriginalSize->setText("1");
        rotateCounterclockwise->setText("\\");
        rotateClockwise->setText("/");
        openFile->setText("O");
        saveFileAs->setText("S");
        deleteFile->setText("D");
        preferences->setText("P");
        quit->setText("Q");

        QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
        toolbarLayout->setContentsMargins(0, 0, 0, 0);
        toolbarLayout->setSpacing(0);
        toolbarLayout->addStretch();
        toolbarLayout->addWidget(navigatePrevious);
        toolbarLayout->addWidget(navigateNext);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(zoomOut);
        toolbarLayout->addWidget(zoomIn);
        toolbarLayout->addWidget(zoomFitToWindow);
        toolbarLayout->addWidget(zoomOriginalSize);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(rotateCounterclockwise);
        toolbarLayout->addWidget(rotateClockwise);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(openFile);
        toolbarLayout->addWidget(saveFileAs);
        toolbarLayout->addWidget(deleteFile);
        toolbarLayout->addWidget(createVerticalSeparator(toolbar));
        toolbarLayout->addWidget(preferences);
        toolbarLayout->addWidget(quit);
        toolbarLayout->addStretch();

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);
        mainLayout->addWidget(imageViewerWidget);
        mainLayout->addWidget(toolbar);

        mainWindow->setCentralWidget(centralWidget);
        retranslate();
    }

    ~UI()
    {}

    void retranslate()
    {
        navigatePrevious->setToolTip(qApp->translate("MainWindow", "Previous"));
        navigateNext->setToolTip(qApp->translate("MainWindow", "Next"));
        zoomOut->setToolTip(qApp->translate("MainWindow", "Zoom Out"));
        zoomIn->setToolTip(qApp->translate("MainWindow", "Zoom In"));
        zoomFitToWindow->setToolTip(qApp->translate("MainWindow", "Fit Image To Window Size"));
        zoomOriginalSize->setToolTip(qApp->translate("MainWindow", "Original Size"));
        rotateCounterclockwise->setToolTip(qApp->translate("MainWindow", "Rotate Counterclockwise"));
        rotateClockwise->setToolTip(qApp->translate("MainWindow", "Rotate Clockwise"));
        openFile->setToolTip(qApp->translate("MainWindow", "Open File"));
        saveFileAs->setToolTip(qApp->translate("MainWindow", "Save File As"));
        deleteFile->setToolTip(qApp->translate("MainWindow", "Delete File"));
        preferences->setToolTip(qApp->translate("MainWindow", "Preferences"));
        quit->setToolTip(qApp->translate("MainWindow", "Quit"));
    }

private:

    QWidget *createVerticalSeparator(QWidget *parent)
    {
        QFrame *separator = new QFrame(parent);
        separator->setFrameShape(QFrame::VLine);
        return separator;
    }
};

#endif // MAINWINDOW_P_H_INCLUDED
