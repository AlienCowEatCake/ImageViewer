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
#include <QMessageBox>
#include <QLabel>
#include <QDir>
#include <QVector>

#include "Utils/SettingsWrapper.h"
#include "Utils/Workarounds.h"

#include "Decoders/DecodersManager.h"

struct MainWindow::Impl
{
    Impl(MainWindow *mainWindow)
        : mainWindow(mainWindow)
        , supportedFormats(DecodersManager::getInstance().supportedFormatsWithWildcards())
    {
        onFileClosed();
    }

    QStringList supportedFilesInDirectory(const QDir &dir) const
    {
        return dir.entryList(supportedFormats, QDir::Files | QDir::Readable, QDir::Name | QDir::IgnoreCase);
    }

    void onFileOpened(const QString &path)
    {
        const QFileInfo fileInfo = QFileInfo(path);
        const QDir dir = fileInfo.dir();
        const QString directoryPath = dir.absolutePath();
        const QString fileName = fileInfo.fileName();
        if(directoryPath != currentDirectory)
        {
            currentDirectory = directoryPath;
            filesInCurrentDirectory.clear();
            currentIndexInDirectory = -1;
            const QStringList list = supportedFilesInDirectory(dir);
            for(QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
            {
                const QString &name = *it;
                filesInCurrentDirectory.append(name);
                if(currentIndexInDirectory == -1 && name == fileName)
                    currentIndexInDirectory = filesInCurrentDirectory.size() - 1;
            }
        }
        else
        {
            if(currentIndexInDirectory > 0 && filesInCurrentDirectory[currentIndexInDirectory - 1] == fileName)
                currentIndexInDirectory--;
            else if(currentIndexInDirectory < filesInCurrentDirectory.size() - 1 && filesInCurrentDirectory[currentIndexInDirectory + 1] == fileName)
                currentIndexInDirectory++;
            else
                currentIndexInDirectory = filesInCurrentDirectory.indexOf(fileName);
        }

        if(currentIndexInDirectory < 0)
        {
            filesInCurrentDirectory.clear();
            currentDirectory.clear();
            mainWindow->m_ui->navigateNext->setEnabled(false);
            mainWindow->m_ui->navigatePrevious->setEnabled(false);
        }
        else
        {
            mainWindow->m_ui->navigateNext->setEnabled(currentIndexInDirectory < filesInCurrentDirectory.size() - 1);
            mainWindow->m_ui->navigatePrevious->setEnabled(currentIndexInDirectory > 0);
        }
        mainWindow->m_ui->deleteFile->setEnabled(fileInfo.isWritable());
    }

    void onFileClosed()
    {
        filesInCurrentDirectory.clear();
        currentDirectory.clear();
        currentIndexInDirectory = -1;
        mainWindow->m_ui->navigateNext->setEnabled(false);
        mainWindow->m_ui->navigatePrevious->setEnabled(false);
        mainWindow->m_ui->deleteFile->setEnabled(false);
    }

    MainWindow *mainWindow;
    const QStringList supportedFormats;

    QString lastOpenedFilename;

    QString currentDirectory;
    QVector<QString> filesInCurrentDirectory;
    int currentIndexInDirectory;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new UI(this))
    , m_impl(new Impl(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

    connect(m_ui->imageViewerWidget, SIGNAL(zoomModeChanged(ImageViewerWidget::ZoomMode)), this, SLOT(onZoomModeChanged(ImageViewerWidget::ZoomMode)));
    connect(m_ui->imageViewerWidget, SIGNAL(zoomLevelChanged(qreal)), this, SLOT(updateWindowTitle()));

    connect(m_ui->navigatePrevious, SIGNAL(clicked()), this, SLOT(onOpenPreviousRequested()));
    connect(m_ui->navigateNext, SIGNAL(clicked()), this, SLOT(onOpenNextRequested()));
    connect(m_ui->zoomIn, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(zoomIn()));
    connect(m_ui->zoomOut, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(zoomOut()));
    connect(m_ui->zoomFitToWindow, SIGNAL(clicked()), this, SLOT(onZoomFitToWindowClicked()));
    connect(m_ui->zoomOriginalSize, SIGNAL(clicked()), this, SLOT(onZoomOriginalSizeClicked()));
    connect(m_ui->rotateCounterclockwise, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(rotateCounterclockwise()));
    connect(m_ui->rotateClockwise, SIGNAL(clicked()), m_ui->imageViewerWidget, SLOT(rotateClockwise()));
    connect(m_ui->openFile, SIGNAL(clicked()), this, SLOT(onOpenFileWithDialogRequested()));
    connect(m_ui->saveFileAs, SIGNAL(clicked()), this, SLOT(onSaveAsRequested()));
    connect(m_ui->deleteFile, SIGNAL(clicked()), this, SLOT(onDeleteFileRequested()));
    connect(m_ui->preferences, SIGNAL(clicked()), this, SLOT(showPreferences()));
    connect(m_ui->exit, SIGNAL(clicked()), this, SLOT(onExitRequested()));

    connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(onOpenFileWithDialogRequested()));
    connect(m_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(onSaveAsRequested()));
    connect(m_ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferences()));
    connect(m_ui->actionExit, SIGNAL(triggered()), this, SLOT(onExitRequested()));
    connect(m_ui->actionEnglish, SIGNAL(triggered()), this, SLOT(onActionEnglishTriggered()));
    connect(m_ui->actionRussian, SIGNAL(triggered()), this, SLOT(onActionRussianTriggered()));
    connect(m_ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
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

void MainWindow::showAbout()
{
    QMessageBox msgBox(this);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setWindowTitle(tr("About"));
    msgBox.setText(tr("<b>Image Viewer v%1</b>").arg(QString::fromLatin1("1.0")));
    msgBox.setInformativeText(QString::fromLatin1(
                      "<a href=\"https://fami.codefreak.ru/gitlab/peter/ImageViewer\">https://fami.codefreak.ru/gitlab/peter/ImageViewer</a><br>"
                      "%1: <a href=\"http://www.gnu.org/copyleft/gpl.html\">GNU GPL v3</a><br><br>"
                      "Copyright &copy; 2017<br>"
                      "%2 &lt;<a href=\"mailto:peter.zhigalov@gmail.com\">peter.zhigalov@gmail.com</a>&gt;"
                      ).arg(tr("License")).arg(tr("Peter S. Zhigalov")));
    const QList<QLabel*> labels = msgBox.findChildren<QLabel*>();
    for(QList<QLabel*>::ConstIterator it = labels.begin(); it != labels.end(); ++it)
        (*it)->setWordWrap(false);
    msgBox.setIconPixmap(QString::fromLatin1(":/icon/icon_64.png"));
    msgBox.exec();
}

void MainWindow::showPreferences()
{
    /// @todo
    QMessageBox::critical(this, QString::fromLatin1("Error"), QString::fromLatin1("Not implemented yet =("));
}

void MainWindow::onOpenPreviousRequested()
{
    if(m_impl->currentIndexInDirectory > 0)
        onOpenFileRequested(QDir(m_impl->currentDirectory).absoluteFilePath(m_impl->filesInCurrentDirectory[m_impl->currentIndexInDirectory - 1]));
}

void MainWindow::onOpenNextRequested()
{
    if(m_impl->currentIndexInDirectory >= 0 && m_impl->currentIndexInDirectory < m_impl->filesInCurrentDirectory.size() - 1)
        onOpenFileRequested(QDir(m_impl->currentDirectory).absoluteFilePath(m_impl->filesInCurrentDirectory[m_impl->currentIndexInDirectory + 1]));
}

void MainWindow::onZoomModeChanged(ImageViewerWidget::ZoomMode mode)
{
    m_ui->zoomFitToWindow->setChecked(mode == ImageViewerWidget::ZOOM_FIT_TO_WINDOW);
    m_ui->zoomOriginalSize->setChecked(mode == ImageViewerWidget::ZOOM_IDENTITY);
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
        QMessageBox::critical(this, tr("Error"), tr("Failed to open file \"%1\"").arg(filename));
    }
    m_impl->onFileOpened(filename);
    updateWindowTitle();
}

void MainWindow::onOpenFileWithDialogRequested()
{
    const QString formatString = QString::fromLatin1("%2 (%1);;%3 (*.*)")
            .arg(m_impl->supportedFormats.join(QString::fromLatin1(" ")))
            .arg(tr("All Supported Images")).arg(tr("All Files"));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), m_impl->lastOpenedFilename, formatString);
    if(fileName.isEmpty())
        return;
    onOpenFileRequested(fileName);
}

void MainWindow::onSaveAsRequested()
{
    /// @todo
    QMessageBox::critical(this, QString::fromLatin1("Error"), QString::fromLatin1("Not implemented yet =("));
}

void MainWindow::onDeleteFileRequested()
{
    /// @todo
    QMessageBox::critical(this, QString::fromLatin1("Error"), QString::fromLatin1("Not implemented yet =("));
}

void MainWindow::onExitRequested()
{
    close();
}

void MainWindow::onZoomFitToWindowClicked()
{
    if(m_ui->zoomFitToWindow->isChecked())
        m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_FIT_TO_WINDOW);
    else
        m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_CUSTOM);
}

void MainWindow::onZoomOriginalSizeClicked()
{
    if(m_ui->zoomOriginalSize->isChecked())
        m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_IDENTITY);
    else
        m_ui->imageViewerWidget->setZoomMode(ImageViewerWidget::ZOOM_CUSTOM);
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
        QFileInfo info((it++)->toLocalFile());
        if(info.isDir())
        {
            const QStringList files = m_impl->supportedFilesInDirectory(info.dir());
            if(!files.empty())
                onOpenFileRequested(info.dir().absoluteFilePath(files.first()));
        }
        else
        {
            onOpenFileRequested(info.absoluteFilePath());
        }
        for(; it != urlList.end(); ++it)
        {
            QFileInfo info(it->toLocalFile());
            if(info.isDir())
            {
                const QStringList files = m_impl->supportedFilesInDirectory(info.dir());
                if(!files.empty())
                    openNewWindow(info.dir().absoluteFilePath(files.first()));
            }
            else
            {
                openNewWindow(info.absoluteFilePath());
            }
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Left:
    case Qt::Key_Backspace:
        m_ui->navigatePrevious->animateClick();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        m_ui->navigateNext->animateClick();
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        m_ui->zoomOut->animateClick();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        m_ui->zoomIn->animateClick();
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(event);
}
