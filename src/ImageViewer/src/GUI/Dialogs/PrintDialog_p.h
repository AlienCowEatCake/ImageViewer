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

#if !defined (PRINT_DIALOG_P_H_INCLUDED)
#define PRINT_DIALOG_P_H_INCLUDED

#include "PrintDialog.h"

#include <QApplication>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "Utils/ObjectsUtils.h"

struct PrintDialog::UI
{
    PrintDialog * const printDialog;

    QFrame * const printerSelectFrame;
    QComboBox * const printerSelectComboBox;
    QLabel * const printerNameHeaderLabel;
    QLabel * const printerNameLabel;
    QLabel * const printerDescriptionHeaderLabel;
    QLabel * const printerDescriptionLabel;
    QLabel * const printerDefaultHeaderLabel;
    QLabel * const printerDefaultLabel;
    QLabel * const printerRemoteHeaderLabel;
    QLabel * const printerRemoteLabel;
    QLabel * const printerLocationHeaderLabel;
    QLabel * const printerLocationLabel;
    QLabel * const printerMakeAndModelHeaderLabel;
    QLabel * const printerMakeAndModelLabel;
    QLabel * const printerStateHeaderLabel;
    QLabel * const printerStateLabel;

    QDialogButtonBox * const dialogButtonBox;

    explicit UI(PrintDialog *printDialog)
        : printDialog(printDialog)
        , CONSTRUCT_OBJECT(printerSelectFrame, QFrame, (printDialog))
        , CONSTRUCT_OBJECT(printerSelectComboBox, QComboBox, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerNameHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerNameLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerDescriptionHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerDescriptionLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerDefaultHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerDefaultLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerRemoteHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerRemoteLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerLocationHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerLocationLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerMakeAndModelHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerMakeAndModelLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerStateHeaderLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(printerStateLabel, QLabel, (printerSelectFrame))
        , CONSTRUCT_OBJECT(dialogButtonBox, QDialogButtonBox, (printDialog))
    {
        QGridLayout *printerInfoLayout = new QGridLayout(printerSelectFrame);
        printerInfoLayout->addWidget(printerNameHeaderLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerNameLabel, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDescriptionHeaderLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDescriptionLabel, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDefaultHeaderLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDefaultLabel, 2, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerRemoteHeaderLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerRemoteLabel, 3, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerLocationHeaderLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerLocationLabel, 4, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerMakeAndModelHeaderLabel, 5, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerMakeAndModelLabel, 5, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerStateHeaderLabel, 6, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerStateLabel, 6, 1, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);

        dialogButtonBox->addButton(QDialogButtonBox::Ok);
        dialogButtonBox->addButton(QDialogButtonBox::Cancel);

        QVBoxLayout *layout = new QVBoxLayout(printDialog);
        layout->addWidget(printerSelectComboBox);
        layout->addWidget(printerSelectFrame);
        layout->addWidget(dialogButtonBox);
        layout->addStretch();

        retranslate();
    }

    void retranslate()
    {
        printerNameHeaderLabel->setText(qApp->translate("PrintDialog", "Name:"));
        printerDescriptionHeaderLabel->setText(qApp->translate("PrintDialog", "Description:"));
        printerDefaultHeaderLabel->setText(qApp->translate("PrintDialog", "Default:"));
        printerRemoteHeaderLabel->setText(qApp->translate("PrintDialog", "Remote:"));
        printerLocationHeaderLabel->setText(qApp->translate("PrintDialog", "Location:"));
        printerMakeAndModelHeaderLabel->setText(qApp->translate("PrintDialog", "Make and model:"));
        printerStateHeaderLabel->setText(qApp->translate("PrintDialog", "State:"));

        dialogButtonBox->button(QDialogButtonBox::Ok)->setText(qApp->translate("PrintDialog", "Print"));
        dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(qApp->translate("PrintDialog", "Cancel"));
    }
};

#endif // PRINT_DIALOG_P_H_INCLUDED
