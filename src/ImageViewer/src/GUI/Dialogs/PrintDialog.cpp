/*
   Copyright (C) 2021 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "PrintDialog.h"
#include "PrintDialog_p.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QPainter>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QPrinter>
#include <QPrinterInfo>

#include "Utils/SignalBlocker.h"

struct PrintDialog::Impl
{
    const QList<QPrinterInfo> availablePrinters;
    QGraphicsItem * const graphicsItem;
    const int rotateAngle;
    const Qt::Orientations flipOrientations;
    const QString filePath;
    QScopedPointer<QPrinter> printer;

    Impl(QGraphicsItem *graphicsItem, int rotateAngle, const Qt::Orientations &flipOrientations, const QString &filePath)
        : availablePrinters(QPrinterInfo::availablePrinters())
        , graphicsItem(graphicsItem)
        , rotateAngle(rotateAngle)
        , flipOrientations(flipOrientations)
        , filePath(filePath)
    {}

    Qt::Orientation itemOrientation() const
    {
        if(!graphicsItem)
            return Qt::Vertical;
        QRectF boundingRect = graphicsItem->boundingRect().normalized();
        QPointF center = boundingRect.center();
        QTransform t = QTransform().translate(center.x(), center.y()).rotate(rotateAngle).translate(-center.x(), -center.y());
        boundingRect = t.mapRect(boundingRect);
        return boundingRect.height() >= boundingRect.width() ? Qt::Vertical : Qt::Horizontal;
    }
};

PrintDialog::PrintDialog(QGraphicsItem *graphicsItem,
                         int rotateAngle,
                         const Qt::Orientations &flipOrientations,
                         const QString &filePath,
                         QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
    , m_impl(new Impl(graphicsItem, rotateAngle, flipOrientations, filePath))
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                   Qt::WindowCloseButtonHint |
#endif
                   Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle(qApp->translate("PrintDialog", "Print"));
    setWindowModality(Qt::ApplicationModal);

    m_ui->copiesSpinBox->setMinimum(1);
    m_ui->copiesSpinBox->setMaximum(999);

    const QList<QPrinterInfo> printers = m_impl->availablePrinters;
    for(QList<QPrinterInfo>::ConstIterator it = printers.begin(); it != printers.end(); ++it)
    {
        m_ui->printerSelectComboBox->addItem(it->printerName());
        if(it->isDefault())
            m_ui->printerSelectComboBox->setCurrentIndex(m_ui->printerSelectComboBox->count() - 1);
    }
    onCurrentPrinterChanged(m_ui->printerSelectComboBox->currentIndex());

    m_ui->autoRotateCheckBox->setChecked(true);
    onAutoRotateStateChanged();

    connect(m_ui->printerSelectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentPrinterChanged(int)));
    connect(m_ui->pageSetupButton, SIGNAL(clicked()), this, SLOT(onPageSetupClicked()));
    connect(m_ui->autoRotateCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onAutoRotateStateChanged()));
    connect(m_ui->portraitRadioButton, SIGNAL(toggled(bool)), this, SLOT(onPortraitToggled(bool)));
    connect(m_ui->landscapeRadioButton, SIGNAL(toggled(bool)), this, SLOT(onLandscapeToggled(bool)));
    connect(m_ui->copiesSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onNumCopiesChanged(int)));
    connect(m_ui->colorModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onColorModeChanged(int)));
    connect(m_ui->dialogButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onPrintClicked()));
    connect(m_ui->dialogButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));

    ensurePolished();
    adjustSize();
    setFixedSize(minimumSize());
}

PrintDialog::~PrintDialog()
{}

void PrintDialog::onCurrentPrinterChanged(int index)
{
    m_impl->printer.reset();
    updatePrinterInfo(QPrinterInfo());
    updatePageInfo();
    m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if(index < 0 || index >= m_impl->availablePrinters.size())
        return;

    const QPrinterInfo& info = m_impl->availablePrinters[index];
    m_impl->printer.reset(new QPrinter(info, QPrinter::HighResolution));
    QPageSetupDialog(m_impl->printer.data(), Q_NULLPTR).accept();
    QPrintDialog(m_impl->printer.data(), Q_NULLPTR).accept();

    m_impl->printer->setDocName(QFileInfo(m_impl->filePath).fileName());
    m_impl->printer->setCreator(qApp->applicationName() + QString::fromLatin1(" ") + qApp->applicationVersion());

    if(true)
    {
        const QSignalBlocker guard(m_ui->copiesSpinBox);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
        m_ui->copiesSpinBox->setValue(m_impl->printer->copyCount());
#else
        m_ui->copiesSpinBox->setValue(m_impl->printer->numCopies());
#endif
    }
    if(true)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
        const QList<QPrinter::ColorMode> supportedColorModes = info.supportedColorModes();
#else
        const QList<QPrinter::ColorMode> supportedColorModes = QList<QPrinter::ColorMode>() << QPrinter::Color << QPrinter::GrayScale;
#endif
        const QSignalBlocker guard(m_ui->colorModeComboBox);
        m_ui->colorModeComboBox->clear();
        if(supportedColorModes.contains(QPrinter::Color))
            m_ui->colorModeComboBox->addItem(qApp->translate("PrintDialog", "Color"), static_cast<int>(QPrinter::Color));
        if(supportedColorModes.contains(QPrinter::GrayScale))
            m_ui->colorModeComboBox->addItem(qApp->translate("PrintDialog", "Grayscale"), static_cast<int>(QPrinter::GrayScale));
        m_ui->colorModeComboBox->setCurrentIndex(m_ui->colorModeComboBox->findData(static_cast<int>(m_impl->printer->colorMode())));
    }

    updatePrinterInfo(info);
    updatePageInfo();
    m_ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(m_impl->graphicsItem);
}

void PrintDialog::onPrintClicked()
{
    if(!m_impl->graphicsItem || !m_impl->printer)
        return;

    /// @todo mirror/rotate
    /// @todo dpi
    QPainter painter(m_impl->printer.data());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const qreal defaultDpi = 96;
    const qreal scaleX = static_cast<qreal>(painter.device()->logicalDpiX()) / defaultDpi;
    const qreal scaleY = static_cast<qreal>(painter.device()->logicalDpiY()) / defaultDpi;
    painter.scale(scaleX, scaleY);

    QStyleOptionGraphicsItem options;
    options.exposedRect = m_impl->graphicsItem->boundingRect();
    m_impl->graphicsItem->paint(&painter, &options);
    painter.end();

    close();
}

void PrintDialog::onPageSetupClicked()
{
    if(!m_impl->graphicsItem || !m_impl->printer)
        return;

    QPageSetupDialog *dialog = new QPageSetupDialog(m_impl->printer.data(), this);
    if(dialog->exec() == QDialog::Accepted)
        updatePageInfo();
    dialog->hide();
    dialog->deleteLater();
}

void PrintDialog::onAutoRotateStateChanged()
{
    const bool autoRotate = m_ui->autoRotateCheckBox->checkState() == Qt::Checked;
    m_ui->portraitRadioButton->setDisabled(autoRotate);
    m_ui->landscapeRadioButton->setDisabled(autoRotate);
    updatePageOrientation();
}

void PrintDialog::onPortraitToggled(bool checked)
{
    if(!checked || !m_impl->printer)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    m_impl->printer->setPageOrientation(QPageLayout::Portrait);
#else
    m_impl->printer->setOrientation(QPrinter::Portrait);
#endif
    updatePageOrientation();
}

void PrintDialog::onLandscapeToggled(bool checked)
{
    if(!checked || !m_impl->printer)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    m_impl->printer->setPageOrientation(QPageLayout::Landscape);
#else
    m_impl->printer->setOrientation(QPrinter::Landscape);
#endif
    updatePageOrientation();
}

void PrintDialog::onNumCopiesChanged(int value)
{
    if(!m_impl->printer)
        return;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    m_impl->printer->setCopyCount(value);
#else
    m_impl->printer->setNumCopies(value);
#endif
    const QSignalBlocker guard(m_ui->copiesSpinBox);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
    m_ui->copiesSpinBox->setValue(m_impl->printer->copyCount());
#else
    m_ui->copiesSpinBox->setValue(m_impl->printer->numCopies());
#endif
}

void PrintDialog::onColorModeChanged(int index)
{
    if(!m_impl->printer)
        return;
    m_impl->printer->setColorMode(static_cast<QPrinter::ColorMode>(m_ui->colorModeComboBox->itemData(index).toInt()));
    const QSignalBlocker guard(m_ui->colorModeComboBox);
    m_ui->colorModeComboBox->setCurrentIndex(m_ui->colorModeComboBox->findData(static_cast<int>(m_impl->printer->colorMode())));
}

void PrintDialog::updatePrinterInfo(const QPrinterInfo& info)
{
    m_ui->printerNameLabel->setText(info.printerName());

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_ui->printerDescriptionLabel->setText(info.description());
#else
    m_ui->printerDescriptionHeaderLabel->setEnabled(false);
    m_ui->printerDescriptionLabel->setEnabled(false);
#endif

    if(info.isDefault())
        m_ui->printerDefaultLabel->setText(qApp->translate("PrintDialog", "Yes", "Default"));
    else
        m_ui->printerDefaultLabel->setText(qApp->translate("PrintDialog", "No", "Default"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    if(info.isRemote())
        m_ui->printerRemoteLabel->setText(qApp->translate("PrintDialog", "Yes", "Remote"));
    else
        m_ui->printerRemoteLabel->setText(qApp->translate("PrintDialog", "No", "Remote"));
#else
    m_ui->printerRemoteHeaderLabel->setEnabled(false);
    m_ui->printerRemoteLabel->setEnabled(false);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_ui->printerLocationLabel->setText(info.location());
#else
    m_ui->printerLocationHeaderLabel->setEnabled(false);
    m_ui->printerLocationLabel->setEnabled(false);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    m_ui->printerMakeAndModelLabel->setText(info.makeAndModel());
#else
    m_ui->printerMakeAndModelHeaderLabel->setEnabled(false);
    m_ui->printerMakeAndModelLabel->setEnabled(false);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    switch(info.state())
    {
    case QPrinter::Idle:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Idle", "State"));
        break;
    case QPrinter::Active:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Active", "State"));
        break;
    case QPrinter::Aborted:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Aborted", "State"));
        break;
    case QPrinter::Error:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Error", "State"));
        break;
    default:
        m_ui->printerStateLabel->setText(qApp->translate("PrintDialog", "Unknown (%1)", "State").arg(static_cast<int>(info.state())));
        break;
    }
#else
    m_ui->printerStateHeaderLabel->setEnabled(false);
    m_ui->printerStateLabel->setEnabled(false);
#endif
}

void PrintDialog::updatePageInfo()
{
    updatePageOrientation();
}

void PrintDialog::updatePageOrientation()
{
    if(!m_impl->printer)
        return;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    QPageLayout::Orientation orientation;
    const QPageLayout::Orientation portrait = QPageLayout::Portrait;
    const QPageLayout::Orientation landscape = QPageLayout::Landscape;
#else
    QPrinter::Orientation orientation;
    const QPrinter::Orientation portrait = QPrinter::Portrait;
    const QPrinter::Orientation landscape = QPrinter::Landscape;
#endif

    const bool autoRotate = m_ui->autoRotateCheckBox->checkState() == Qt::Checked;
    if(autoRotate)
    {
        orientation = (m_impl->itemOrientation() == Qt::Vertical) ? portrait : landscape;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
        m_impl->printer->setPageOrientation(orientation);
#else
        m_impl->printer->setOrientation(orientation);
#endif
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 3, 0))
    orientation = m_impl->printer->pageLayout().orientation();
#else
    orientation = m_impl->printer->orientation();
#endif

    const QSignalBlocker portraitRadioButtonBlocker(m_ui->portraitRadioButton);
    const QSignalBlocker landscapeRadioButtonBlocker(m_ui->landscapeRadioButton);
    m_ui->portraitRadioButton->setChecked(orientation == portrait);
    m_ui->landscapeRadioButton->setChecked(orientation == landscape);
}
