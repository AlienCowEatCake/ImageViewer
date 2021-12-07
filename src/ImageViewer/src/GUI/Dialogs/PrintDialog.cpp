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

#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QPrinter>
#include <QPrinterInfo>

struct PrintDialog::Impl
{
    const QList<QPrinterInfo> availablePrinters;
    QGraphicsItem * const graphicsItem;
    QScopedPointer<QPrinter> printer;

    Impl(QGraphicsItem *graphicsItem)
        : availablePrinters(QPrinterInfo::availablePrinters())
        , graphicsItem(graphicsItem)
    {}
};

PrintDialog::PrintDialog(QGraphicsItem *graphicsItem, QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
    , m_impl(new Impl(graphicsItem))
{
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                   Qt::WindowCloseButtonHint |
#endif
                   Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);
    setWindowTitle(qApp->translate("PrintDialog", "Print"));
    setWindowModality(Qt::ApplicationModal);

    connect(m_ui->printerSelectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentPrinterChanged(int)));
    connect(m_ui->dialogButtonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(onPrintClicked()));
    connect(m_ui->dialogButtonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(close()));

    ensurePolished();
    adjustSize();
    setFixedSize(minimumSize());

    const QList<QPrinterInfo> printers = m_impl->availablePrinters;
    for(QList<QPrinterInfo>::ConstIterator it = printers.begin(); it != printers.end(); ++it)
    {
        m_ui->printerSelectComboBox->addItem(it->printerName());
        if(it->isDefault())
            m_ui->printerSelectComboBox->setCurrentIndex(m_ui->printerSelectComboBox->count() - 1);
    }
}

PrintDialog::~PrintDialog()
{}

void PrintDialog::onCurrentPrinterChanged(int index)
{
    m_impl->printer.reset();
    if(index < 0 || index >= m_impl->availablePrinters.size())
        return;

    const QPrinterInfo& info = m_impl->availablePrinters[index];
    updatePrinterInfo(info);
    m_impl->printer.reset(new QPrinter(info, QPrinter::HighResolution));
    QPageSetupDialog(m_impl->printer.get(), Q_NULLPTR).accept();
    QPrintDialog(m_impl->printer.get(), Q_NULLPTR).accept();
}

void PrintDialog::onPrintClicked()
{
    if(!m_impl->graphicsItem || !m_impl->printer)
        return;

    /// @todo mirror/rotate
    /// @todo dpi
    QPainter painter(m_impl->printer.get());
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
