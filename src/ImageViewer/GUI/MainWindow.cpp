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

#include "MainWindow.h"
#include "MainWindow_p.h"

#include <QApplication>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QStringList>
#include <QProcess>
#include <QTranslator>
#include <QLibraryInfo>
#include <QGraphicsItem>

#include "Utils/SettingsWrapper.h"
#include "Utils/Workarounds.h"

#include "Decoders/DecodersManager.h"

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
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    connect(m_ui->zoomIn, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(zoomIn()));
    connect(m_ui->zoomOut, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(zoomOut()));
    connect(m_ui->zoomFitToWindow, SIGNAL(clicked()), this, SLOT(onZoomFitToWindowRequested()));
    connect(m_ui->zoomOriginalSize, SIGNAL(clicked()), this, SLOT(onZoomOriginalSizeRequested()));
    connect(m_ui->imageViewerWidget, SIGNAL(zoomModeChanged(ImageViewerWidget::ZoomMode)), this, SLOT(onZoomModeChanged(ImageViewerWidget::ZoomMode)));
    connect(m_ui->imageViewerWidget, SIGNAL(zoomLevelChanged(qreal)), this, SLOT(updateWindowTitle()));
    connect(m_ui->rotateCounterclockwise, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(rotateCounterclockwise()));
    connect(m_ui->rotateClockwise, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(rotateClockwise()));
    connect(m_ui->openFile, SIGNAL(clicked()), this, SLOT(onOpenFileWithDialogRequested()));
    connect(m_ui->exit, SIGNAL(clicked()), this, SLOT(onExitRequested()));
    connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpenFileWithDialogRequested()));
    connect(m_ui->actionExit, SIGNAL(triggered()), this, SLOT(onExitRequested()));
    connect(m_ui->actionEnglish, SIGNAL(triggered()), this, SLOT(onActionEnglishTriggered()));
    connect(m_ui->actionRussian, SIGNAL(triggered()), this, SLOT(onActionRussianTriggered()));
    connect(m_ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_FIT_TO_WINDOW);

    setLanguage();
}

MainWindow::~MainWindow()
{
    qApp->quit();
}

void MainWindow::setLanguage(const QString &newLanguage)
{
    // Отображение название языка -> соответствующая ему менюшка
    static QMap<QString, QAction *> languagesMap;
    if(languagesMap.isEmpty())
    {
        languagesMap[QString::fromLatin1("en")] = m_ui->actionEnglish;
        languagesMap[QString::fromLatin1("ru")] = m_ui->actionRussian;
    }

    // Определим системную локаль
    static QString systemLang;
    if(systemLang.isEmpty())
    {
        const QString systemLocale = QLocale::system().name().toLower();
        for(QMap<QString, QAction *>::Iterator it = languagesMap.begin(); it != languagesMap.end(); ++it)
        {
            if(systemLocale.startsWith(it.key()))
            {
                systemLang = it.key();
                break;
            }
        }
        if(systemLang.isEmpty())
            systemLang = QString::fromLatin1("en");
    }

    // Посмотрим в настройки, не сохранен ли случайно в них язык
    SettingsWrapper settings;
    QString language = newLanguage;
    if(language.isEmpty())
    {
        const QVariant rawValue = settings.value(QString::fromLatin1("Language"), systemLang);
        const QString value = rawValue.isValid() ? rawValue.toString() : systemLang;
        language = languagesMap.find(value) != languagesMap.end() ? value : systemLang;
    }
    else
    {
        settings.setValue(QString::fromLatin1("Language"), language);
    }

    // Удалим старый перевод, установим новый
    static QTranslator qtTranslator;
    static QTranslator appTranslator;
    if(!qtTranslator.isEmpty())
        qApp->removeTranslator(&qtTranslator);
    if(!appTranslator.isEmpty())
        qApp->removeTranslator(&appTranslator);
    qtTranslator.load(QString::fromLatin1("qt_%1").arg(language), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    appTranslator.load(QString::fromLatin1(":/translations/%1").arg(language));
    qApp->installTranslator(&qtTranslator);
    qApp->installTranslator(&appTranslator);

    // Пофиксим шрифты
    Workarounds::FontsFix(language);

    // Проставим галочку на нужном нам языке и снимем с остальных
    languagesMap[language]->setChecked(true);

    // Перегрузим ресурсы в окнах
    m_ui->retranslate();
    updateWindowTitle();
}

void MainWindow::openNewWindow(const QString &filename)
{
    const QStringList args = filename.isEmpty() ? QStringList() : QStringList(filename);
    QProcess::startDetached(QApplication::applicationFilePath(), args, QDir::currentPath());
}

void MainWindow::updateWindowTitle()
{
    const QSize imageSize = m_ui->imageViewerWidget->imageSize();
    if(!imageSize.isValid())
    {
        setWindowTitle(tr("Image Viewer"));
        return;
    }
    QString label = m_impl->lastOpenedFilename.split(QChar::fromLatin1('/')).last();
#if defined (Q_OS_WIN32)
    label = label.split(QChar::fromLatin1('\\')).last();
#endif
    label.append(QString::fromLatin1(" (%1x%2) %3% - %4")
                 .arg(imageSize.width())
                 .arg(imageSize.height())
                 .arg(static_cast<quint64>(m_ui->imageViewerWidget->zoomLevel() * 100))
                 .arg(tr("Image Viewer")));
    setWindowTitle(label);
}

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
    QGraphicsItem *item = DecodersManager::getInstance().loadImage(filename);
    if(item)
    {
        m_ui->imageViewerWidget->setGraphicsItem(item);
        m_ui->setImageControlsEnabled(true);
        m_impl->lastOpenedFilename = filename;
    }
    else
    {
        m_ui->imageViewerWidget->clear();
        m_ui->setImageControlsEnabled(false);
        /// @todo MessageBox with error
    }
    updateWindowTitle();
}

void MainWindow::onOpenFileWithDialogRequested()
{
    const QStringList supportedFormats = DecodersManager::getInstance().supportedFormats();
    const QString formatString = QString::fromLatin1("%2 (%1);;%3 (*.*)")
            .arg(supportedFormats.join(QString::fromLatin1(" *.")).prepend(QString::fromLatin1("*.")))
            .arg(tr("All Supported Images")).arg(tr("All Files"));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), m_impl->lastOpenedFilename, formatString);
    if(fileName.isEmpty())
        return;
    onOpenFileRequested(fileName);
}

void MainWindow::onExitRequested()
{
    close();
}

void MainWindow::onActionEnglishTriggered()
{
    setLanguage(QString::fromLatin1("en"));
}

void MainWindow::onActionRussianTriggered()
{
    setLanguage(QString::fromLatin1("ru"));
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        QList<QUrl>::ConstIterator it = urlList.begin();
        onOpenFileRequested((it++)->toLocalFile());
        for(; it != urlList.end(); ++it)
            openNewWindow(it->toLocalFile());
    }
}
