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

#if !defined(ABOUTDIALOG_P_H_INCLUDED)
#define ABOUTDIALOG_P_H_INCLUDED

#include "AboutDialog.h"

#include <QLabel>
#include <QDialogButtonBox>
#include <QTextBrowser>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QPixmap>

#include "Utils/ObjectsUtils.h"
#include "Widgets/AdjustableFrame.h"

struct AboutDialog::UI
{
    AdjustableFrame *centralWidget;
    QLabel *iconLabel;
    QLabel *titleLabel;
    QLabel *textLabel;
    QTextBrowser *textBrowser;
    QDialogButtonBox *buttonBox;

    UI(QWidget *parent)
        : CONSTRUCT_OBJECT(centralWidget, AdjustableFrame, (parent))
        , CONSTRUCT_OBJECT(iconLabel, QLabel, (centralWidget))
        , CONSTRUCT_OBJECT(textLabel, QLabel, (centralWidget))
        , CONSTRUCT_OBJECT(textBrowser, QTextBrowser, (centralWidget))
        , CONSTRUCT_OBJECT(buttonBox, QDialogButtonBox, (centralWidget))
    {
        QPalette palette = textBrowser->palette();
        palette.setColor(QPalette::Base, palette.color(QPalette::Window));
        textBrowser->setPalette(palette);

        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        textLabel->setOpenExternalLinks(true);

        QGridLayout *centralLayout = new QGridLayout(centralWidget);
        centralLayout->addWidget(iconLabel, 0, 0, 2, 1, Qt::AlignTop | Qt::AlignCenter);
        centralLayout->addWidget(textLabel, 0, 1, 1, 1, Qt::AlignTop | Qt::AlignLeft);
        centralLayout->addWidget(textBrowser, 1, 0, 1, 2, Qt::AlignCenter);
        centralLayout->addWidget(buttonBox, 2, 0, 1, 2, Qt::AlignVCenter | Qt::AlignRight);

        QVBoxLayout *dialogLayout = new QVBoxLayout(parent);
        dialogLayout->setContentsMargins(0, 0, 0, 0);
        dialogLayout->addWidget(centralWidget);

        parent->ensurePolished();
    }
};

#endif // ABOUTDIALOG_P_H_INCLUDED
