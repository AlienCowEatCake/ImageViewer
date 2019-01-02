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

} // namespace

InfoDialog::InfoDialog(const QSharedPointer<IImageData> &imageData, QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
{
    setWindowFlags(Qt::Window |
                   Qt::CustomizeWindowHint |
                   Qt::WindowTitleHint |
                   Qt::WindowSystemMenuHint |
                   Qt::WindowMinimizeButtonHint |
                   Qt::WindowMaximizeButtonHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                   Qt::WindowCloseButtonHint
#endif
                   );
    setWindowTitle(qApp->translate("InfoDialog", "Image Information"));
    setWindowModality(Qt::ApplicationModal);

    if(!imageData)
        return;

    int currentRow = 0;
    QFileInfo fileInfo = QFileInfo(imageData->filePath());

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "File Name")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.fileName()));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Resolution")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::fromLatin1("%1 x %2").arg(imageData->size().width()).arg(imageData->size().height())));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "File Size")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(formatFileSize(fileInfo.size())));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Created")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.created().toString()));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Last Modified")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.lastModified().toString()));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Last Read")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.lastRead().toString()));
    currentRow++;

#if !defined (Q_OS_WIN)
    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Owner")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.owner()));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Group")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(fileInfo.group()));
    currentRow++;

    m_ui->tableWidget->insertRow(currentRow);
    m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(qApp->translate("InfoDialog", "General Info")));
    m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(qApp->translate("InfoDialog", "Permissions")));
    m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(QString::number(fileInfo.permissions() & ~0xf000, 16)));
    currentRow++;
#endif

    const IImageMetaData *metaData = imageData->metaData();
    if(!metaData)
        return;

    const QList<IImageMetaData::MetaDataType> types = metaData->types();
    for(QList<IImageMetaData::MetaDataType>::ConstIterator it = types.constBegin(), itEnd = types.constEnd(); it != itEnd; ++it)
    {
        const IImageMetaData::MetaDataEntryList entryList = metaData->metaData(*it);
        if(entryList.empty())
            continue;

        m_ui->tableWidget->insertRow(currentRow);
        currentRow++;

        for(IImageMetaData::MetaDataEntryList::ConstIterator jt = entryList.constBegin(), jtEnd = entryList.constEnd(); jt != jtEnd; ++jt)
        {
            m_ui->tableWidget->insertRow(currentRow);
            m_ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(*it));
            m_ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(jt->tagTitle));
            m_ui->tableWidget->setItem(currentRow, 2, new QTableWidgetItem(jt->value));

            QString toolTip;
            if(!jt->tagName.isEmpty() && !jt->tagDescription.isEmpty())
                toolTip = QString::fromLatin1("<b>%1:</b> %2").arg(jt->tagName).arg(jt->tagDescription);
            else if(!jt->tagDescription.isEmpty())
                toolTip = FORCE_RICH_TEXT_TEMPLATE.arg(jt->tagDescription);
            else if(!jt->tagName.isEmpty())
                toolTip = FORCE_RICH_TEXT_TEMPLATE.arg(jt->tagName);
            if(!toolTip.isEmpty())
                for(int i = 0; i < 3; i++)
                    m_ui->tableWidget->item(currentRow, i)->setToolTip(toolTip);
            currentRow++;
        }
    }
}

InfoDialog::~InfoDialog()
{}
