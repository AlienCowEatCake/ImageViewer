/*
   Copyright (C) 2019-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (INFO_DIALOG_P_H_INCLUDED)
#define INFO_DIALOG_P_H_INCLUDED

#include "InfoDialog.h"

#include <QApplication>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>

#include "Utils/ObjectsUtils.h"

namespace {

const int WINDOW_DEFAULT_WIDTH  = 640;
const int WINDOW_DEFAULT_HEIGHT = 480;

} // namespace

struct InfoDialog::UI
{
    QTableWidget * const tableWidget;

    explicit UI(InfoDialog *infoDialog)
        : CONSTRUCT_OBJECT(tableWidget, QTableWidget, (infoDialog))
    {
        infoDialog->resize(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);

        tableWidget->setColumnCount(3);
        tableWidget->setShowGrid(true);

        const QStringList headers = QStringList()
                << QCoreApplication::translate("InfoDialog", "Type")
                << QCoreApplication::translate("InfoDialog", "Tag")
                << QCoreApplication::translate("InfoDialog", "Value");
        tableWidget->setHorizontalHeaderLabels(headers);
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->horizontalHeader()->setStretchLastSection(true);

        QVBoxLayout *layout = new QVBoxLayout(infoDialog);
        layout->addWidget(tableWidget);
    }
};

#endif // INFO_DIALOG_P_H_INCLUDED
