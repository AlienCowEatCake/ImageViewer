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

#include <cassert>

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
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QLabel>
#include <QDir>
#include <QVector>
#include <QtAlgorithms>

#include "Utils/SettingsWrapper.h"
#include "Utils/Workarounds.h"
#include "Utils/FileUtils.h"

#include "Decoders/DecodersManager.h"
#include "GUISettings.h"
#include "SettingsDialog.h"

namespace {

bool numericLessThan(const QString &s1, const QString &s2)
{
    const QString sl1 = s1.toLower(), sl2 = s2.toLower();
    QString::ConstIterator it1 = sl1.constBegin(), it2 = sl2.constBegin();
    for(; it1 != sl1.constEnd() && it2 != sl2.constEnd(); ++it1, ++it2)
    {
        QChar c1 = *it1, c2 = *it2;
        if(c1.isNumber() && c2.isNumber())
        {
            QString num1, num2;
            while(c1.isNumber())
            {
                num1.append(c1);
                if((++it1) == sl1.constEnd())
                    break;
                c1 = *it1;
            }
            while(c2.isNumber())
            {
                num2.append(c2);
                if((++it2) == sl2.constEnd())
                    break;
                c2 = *it2;
            }
            if(num1 != num2)
            {
                return num1.toLongLong() < num2.toLongLong();
            }
            else
            {
                if(it1 == sl1.constEnd() || it2 == sl2.constEnd())
                    break;
                --it1;
                --it2;
            }
        }
        else if(c1 != c2)
        {
            return c1 < c2;
        }
    }
    if(it1 == sl1.constEnd() && it2 != sl2.constEnd())
        return true;
    if(it1 != sl1.constEnd() && it2 == sl2.constEnd())
        return false;
    return s1 < s2;
}

} // namespace

struct MainWindow::Impl
{
    Impl(MainWindow *mainWindow)
        : mainWindow(mainWindow)
        , settings(new GUISettings(mainWindow))
        , supportedFormats(DecodersManager::getInstance().supportedFormatsWithWildcards())
    {
        onFileClosed();
    }

    QStringList supportedFilesInDirectory(const QDir &dir) const
    {
        QStringList list = dir.entryList(supportedFormats, QDir::Files | QDir::Readable, QDir::NoSort);
        qSort(list.begin(), list.end(), &numericLessThan);
        return list;
    }

    bool isFileOpened() const
    {
        return mainWindow->m_ui->imageViewerWidget->imageSize().isValid();
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
            mainWindow->m_ui->navigateNext->setEnabled(true);
            mainWindow->m_ui->navigatePrevious->setEnabled(true);
        }
        mainWindow->m_ui->deleteFile->setEnabled(QFileInfo(fileInfo.absolutePath()).isWritable() && isFileOpened());
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
    GUISettings *settings;
    const QStringList supportedFormats;

    QString currentDirectory;
    QVector<QString> filesInCurrentDirectory; /// @todo std::vector?
    int currentIndexInDirectory;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new UI(this))
    , m_impl(new Impl(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAcceptDrops(true);

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

    connect(m_impl->settings, SIGNAL(backgroundColorChanged(const QColor&)), m_ui->imageViewerWidget, SLOT(setBackgroundColor(const QColor&)));

    m_ui->imageViewerWidget->setZoomLevel(m_impl->settings->zoomLevel());
    m_ui->imageViewerWidget->setBackgroundColor(m_impl->settings->backgroundColor());
    onZoomModeChanged(m_impl->settings->zoomMode());

    setLanguage();
}

MainWindow::~MainWindow()
{
    m_impl->settings->setZoomLevel(m_ui->imageViewerWidget->zoomLevel());
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
    if(!m_impl->isFileOpened())
    {
        setWindowTitle(tr("Image Viewer"));
        return;
    }
    const QSize imageSize = m_ui->imageViewerWidget->imageSize();
    QString label = m_impl->settings->lastOpenedPath().split(QChar::fromLatin1('/')).last();
#if defined (Q_OS_WIN32)
    label = label.split(QChar::fromLatin1('\\')).last();
#endif
    label.append(QString::fromLatin1(" (%1x%2) %3% - %4")
                 .arg(imageSize.width())
                 .arg(imageSize.height())
                 .arg(static_cast<quint64>(m_ui->imageViewerWidget->zoomLevel() * 100))
                 .arg(tr("Image Viewer")));
    label.prepend(QString::fromLatin1("[%1/%2] ")
                  .arg(m_impl->currentIndexInDirectory + 1)
                  .arg(m_impl->filesInCurrentDirectory.size()));
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
    SettingsDialog dialog(m_impl->settings, this);
    dialog.exec();
}

void MainWindow::onOpenPreviousRequested()
{
    int bestMatchedIndex;
    if(m_impl->currentIndexInDirectory > 0)
        bestMatchedIndex = m_impl->currentIndexInDirectory - 1;
    else if(m_impl->filesInCurrentDirectory.size() > 0)
        bestMatchedIndex = m_impl->filesInCurrentDirectory.size() - 1;
    else
        return;
    onOpenFileRequested(QDir(m_impl->currentDirectory).absoluteFilePath(m_impl->filesInCurrentDirectory[bestMatchedIndex]));
}

void MainWindow::onOpenNextRequested()
{
    int bestMatchedIndex;
    if(m_impl->currentIndexInDirectory >= 0 && m_impl->currentIndexInDirectory < m_impl->filesInCurrentDirectory.size() - 1)
        bestMatchedIndex = m_impl->currentIndexInDirectory + 1;
    else if(m_impl->filesInCurrentDirectory.size() > 0)
        bestMatchedIndex = 0;
    else
        return;
    onOpenFileRequested(QDir(m_impl->currentDirectory).absoluteFilePath(m_impl->filesInCurrentDirectory[bestMatchedIndex]));
}

void MainWindow::onOpenFirstRequested()
{
    if(m_impl->filesInCurrentDirectory.size() == 0)
        return;
    onOpenFileRequested(QDir(m_impl->currentDirectory).absoluteFilePath(m_impl->filesInCurrentDirectory[0]));
}

void MainWindow::onOpenLastRequested()
{
    if(m_impl->filesInCurrentDirectory.size() == 0)
        return;
    onOpenFileRequested(QDir(m_impl->currentDirectory).absoluteFilePath(m_impl->filesInCurrentDirectory[m_impl->filesInCurrentDirectory.size() - 1]));
}

void MainWindow::onZoomModeChanged(ImageViewerWidget::ZoomMode mode)
{
    m_ui->imageViewerWidget->setZoomMode(mode);
    m_ui->zoomFitToWindow->setChecked(mode == ImageViewerWidget::ZOOM_FIT_TO_WINDOW);
    m_ui->zoomOriginalSize->setChecked(mode == ImageViewerWidget::ZOOM_IDENTITY);
    m_impl->settings->setZoomMode(mode);
    updateWindowTitle();
}

void MainWindow::onOpenFileRequested(const QString &filename)
{
    QGraphicsItem *item = DecodersManager::getInstance().loadImage(filename);
    if(item)
    {
        m_ui->imageViewerWidget->setZoomMode(m_impl->settings->zoomMode());
        m_ui->imageViewerWidget->setGraphicsItem(item);
        m_ui->setImageControlsEnabled(true);
        m_impl->settings->setLastOpenedPath(filename);
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), m_impl->settings->lastOpenedPath(), formatString);
    if(fileName.isEmpty())
        return;
    onOpenFileRequested(fileName);
}

void MainWindow::onSaveAsRequested()
{
    /// @todo
    QMessageBox::critical(this, tr("Error"), tr("Not implemented yet =("));
}

void MainWindow::onDeleteFileRequested()
{
    if(!m_impl->isFileOpened())
        return;

    if(m_impl->settings->askBeforeDelete())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this, tr("Delete File"), tr("Are you sure you want to delete \"%1\"?")
                .arg(m_impl->settings->lastOpenedPath()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if(reply == QMessageBox::No)
            return;
    }

    if(m_impl->settings->moveToTrash())
    {
        QString errorDescription;
        bool status = FileUtils::MoveToTrash(m_impl->settings->lastOpenedPath(), &errorDescription);
        if(!status)
        {
            QMessageBox::critical(this, tr("Error"), errorDescription);
            return;
        }
    }
    else
    {
        QFile file(m_impl->settings->lastOpenedPath());
        bool status = file.remove();
        if(!status)
        {
            QMessageBox::critical(this, tr("Error"), tr("Unable to delete \"%1\"!").arg(m_impl->settings->lastOpenedPath()));
            return;
        }
    }

    m_ui->imageViewerWidget->clear();
    m_ui->setImageControlsEnabled(false);
    if(m_impl->currentIndexInDirectory >= 0)
        m_impl->filesInCurrentDirectory.erase(m_impl->filesInCurrentDirectory.begin() + m_impl->currentIndexInDirectory);
    if(m_impl->filesInCurrentDirectory.size() > 0)
    {
        m_impl->currentIndexInDirectory--;
        onOpenNextRequested();
    }
    else
    {
        m_impl->onFileClosed();
        updateWindowTitle();
    }
}

void MainWindow::onExitRequested()
{
    close();
}

void MainWindow::onZoomFitToWindowClicked()
{
    ImageViewerWidget::ZoomMode mode;
    if(m_ui->zoomFitToWindow->isChecked())
        mode = ImageViewerWidget::ZOOM_FIT_TO_WINDOW;
    else
        mode = ImageViewerWidget::ZOOM_CUSTOM;
    onZoomModeChanged(mode);
}

void MainWindow::onZoomOriginalSizeClicked()
{
    ImageViewerWidget::ZoomMode mode;
    if(m_ui->zoomOriginalSize->isChecked())
        mode = ImageViewerWidget::ZOOM_IDENTITY;
    else
        mode = ImageViewerWidget::ZOOM_CUSTOM;
    onZoomModeChanged(mode);
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
        if(!event->isAutoRepeat())
            m_ui->navigatePrevious->animateClick();
        else
            onOpenPreviousRequested();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_Space:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if(!event->isAutoRepeat())
            m_ui->navigateNext->animateClick();
        else
            onOpenNextRequested();
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        if(!event->isAutoRepeat())
            m_ui->zoomOut->animateClick();
        else
            m_ui->imageViewerWidget->zoomOut();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        if(!event->isAutoRepeat())
            m_ui->zoomIn->animateClick();
        else
            m_ui->imageViewerWidget->zoomIn();
        break;
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
        if(!event->isAutoRepeat())
            m_ui->deleteFile->animateClick();
        else
            onDeleteFileRequested();
        break;
    case Qt::Key_Home:
        onOpenFirstRequested();
        break;
    case Qt::Key_End:
        onOpenLastRequested();
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(event);
}
