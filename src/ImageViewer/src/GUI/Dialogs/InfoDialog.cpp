/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "InfoDialog.h"
#include "InfoDialog_p.h"

#include <QDateTime>
#include <QFileInfo>

#include "Decoders/IImageData.h"
#include "Decoders/IImageMetaData.h"

namespace {

const QString FORCE_RICH_TEXT_TEMPLATE = QString::fromLatin1("<i></i>%1");
const int MAX_METADATA_ENTRY_VALUE_LENGTH = 10000;

QString formatFileSize(qint64 fileSize)
{
    qint64 unitSize = 1;
    const QStringList labels = QStringList()
            << qApp->translate("InfoDialog", "%1 B")
            << qApp->translate("InfoDialog", "%1 KiB")
            << qApp->translate("InfoDialog", "%1 MiB")
            << qApp->translate("InfoDialog", "%1 GiB")
            << qApp->translate("InfoDialog", "%1 TiB");
    for(QStringList::ConstIterator it = labels.begin(), itEnd = labels.end(); it != itEnd; ++it)
    {
        const qint64 multiplied = unitSize * 1024;
        if(fileSize <= multiplied)
            return it->arg(static_cast<double>(fileSize) / static_cast<double>(unitSize), 0, 'f', 3);
        unitSize = multiplied;
    }
    return labels.first().arg(fileSize);
}

QString formatMetaDataEntryValue(const QString &value)
{
    if(value.length() <= MAX_METADATA_ENTRY_VALUE_LENGTH)
        return value;
    return value.left(MAX_METADATA_ENTRY_VALUE_LENGTH).append(QString::fromUtf8("…"));
}

} // namespace

InfoDialog::InfoDialog(const QSharedPointer<IImageData> &imageData, QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
{
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                   Qt::WindowCloseButtonHint |
#endif
                   Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setWindowTitle(qApp->translate("InfoDialog", "Image Information"));
    setWindowModality(Qt::ApplicationModal);

    if(!imageData)
        return;

    int currentRow = 0;
    QFileInfo fileInfo = QFileInfo(imageData->filePath());
    QTableWidget * const tableWidget = m_ui->tableWidget;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "File Name")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.fileName()));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Resolution")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::fromLatin1("%1 x %2").arg(imageData->size().width()).arg(imageData->size().height())));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Decoder")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(imageData->decoderName()));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "File Size")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(formatFileSize(fileInfo.size())));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Created")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.created().toString()));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Last Modified")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.lastModified().toString()));
    currentRow++;

#if !defined (Q_OS_WIN)
    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Owner")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.owner()));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Group")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.group()));
    currentRow++;

    tableWidget->insertRow(currentRow);
    tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Permissions")));
    tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(fileInfo.permissions() & ~0xf000, 16)));
    currentRow++;
#endif

    IImageMetaData *metaData = imageData->metaData();
    if(!metaData)
        return;

    const QList<IImageMetaData::MetaDataType> types = metaData->types();
    for(QList<IImageMetaData::MetaDataType>::ConstIterator it = types.constBegin(), itEnd = types.constEnd(); it != itEnd; ++it)
    {
        const IImageMetaData::MetaDataEntryList entryList = metaData->metaData(*it);
        if(entryList.empty())
            continue;

        tableWidget->insertRow(currentRow);
        currentRow++;

        for(IImageMetaData::MetaDataEntryList::ConstIterator jt = entryList.constBegin(), jtEnd = entryList.constEnd(); jt != jtEnd; ++jt)
        {
            tableWidget->insertRow(currentRow);
            tableWidget->setItem(currentRow, 0, new QTableWidgetItem(*it));
            tableWidget->setItem(currentRow, 1, new QTableWidgetItem(jt->tagTitle.isEmpty() ? jt->tagName : jt->tagTitle));
            tableWidget->setItem(currentRow, 2, new QTableWidgetItem(formatMetaDataEntryValue(jt->value)));

            QString toolTip;
            if(!jt->tagName.isEmpty() && !jt->tagDescription.isEmpty())
                toolTip = QString::fromLatin1("<b>%1:</b> %2").arg(jt->tagName).arg(jt->tagDescription);
            else if(!jt->tagDescription.isEmpty())
                toolTip = FORCE_RICH_TEXT_TEMPLATE.arg(jt->tagDescription);
            else if(!jt->tagName.isEmpty() && !jt->tagTitle.isEmpty())
                toolTip = FORCE_RICH_TEXT_TEMPLATE.arg(jt->tagName);
            if(!toolTip.isEmpty())
                for(int i = 0; i < 3; i++)
                    tableWidget->item(currentRow, i)->setToolTip(toolTip);
            currentRow++;
        }
    }
}

InfoDialog::~InfoDialog()
{}
