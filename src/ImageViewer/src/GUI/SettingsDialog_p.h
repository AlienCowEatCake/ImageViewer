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

#if !defined(SETTINGSDIALOG_P_H_INCLUDED)
#define SETTINGSDIALOG_P_H_INCLUDED

#include "SettingsDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>

#include "GUISettings.h"

namespace {

const QSize COLOR_BUTTON_SIZE   (40, 24);
const QSize COLOR_ICON_SIZE     (32, 16);

} // namespace

struct SettingsDialog::UI
{
    UI(SettingsDialog *widget, GUISettings *settings)
        : settingsDialog(widget)
        , askBeforeDeleteCheckbox(new QCheckBox(settingsDialog))
        , moveToTrashCheckbox(new QCheckBox(settingsDialog))
        , smoothTransformationCheckbox(new QCheckBox(settingsDialog))
        , backgroundColorFrame(new QFrame(settingsDialog))
        , backgroundColorLabel(new QLabel(backgroundColorFrame))
        , backgroundColorButton(new QToolButton(backgroundColorFrame))
        , buttonBox(new QDialogButtonBox(settingsDialog))
    {
        askBeforeDeleteCheckbox->setText(qApp->translate("SettingsDialog", "Ask before deleting images"));
        askBeforeDeleteCheckbox->setChecked(settings->askBeforeDelete());

        moveToTrashCheckbox->setText(qApp->translate("SettingsDialog", "Move deleted images to trash"));
        moveToTrashCheckbox->setChecked(settings->moveToTrash());

        smoothTransformationCheckbox->setText(qApp->translate("SettingsDialog", "Use smooth image rendering"));
        smoothTransformationCheckbox->setChecked(settings->smoothTransformation());

        backgroundColorLabel->setText(qApp->translate("SettingsDialog", "Background color"));

        backgroundColorButton->setFixedSize(COLOR_BUTTON_SIZE);
        backgroundColorButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        backgroundColorButton->setIconSize(COLOR_ICON_SIZE);

        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        QHBoxLayout *backgroundColorLayout = new QHBoxLayout(backgroundColorFrame);
        backgroundColorLayout->setContentsMargins(0, 0, 0, 0);
        backgroundColorLayout->addWidget(backgroundColorLabel);
        backgroundColorLayout->addWidget(backgroundColorButton);
        backgroundColorLayout->addStretch();

        QVBoxLayout *mainLayout = new QVBoxLayout(settingsDialog);
        mainLayout->addWidget(askBeforeDeleteCheckbox);
        mainLayout->addWidget(moveToTrashCheckbox);
        mainLayout->addWidget(smoothTransformationCheckbox);
        mainLayout->addWidget(backgroundColorFrame);
        mainLayout->addWidget(buttonBox);

        settingsDialog->setWindowTitle(qApp->translate("SettingsDialog", "Preferences"));
        settingsDialog->ensurePolished();
        settingsDialog->adjustSize();
        settingsDialog->resize(widget->minimumSize());
        settingsDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                               Qt::WindowCloseButtonHint |
#endif
                               Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);
        settingsDialog->setWindowModality(Qt::ApplicationModal);
    }

    void updateBackgroundColorButton(const QColor &color)
    {
        QPixmap pixmap(COLOR_ICON_SIZE);
        pixmap.fill(color);
        backgroundColorButton->setIcon(QIcon(pixmap));
    }

    SettingsDialog *settingsDialog;
    QCheckBox *askBeforeDeleteCheckbox;
    QCheckBox *moveToTrashCheckbox;
    QCheckBox *smoothTransformationCheckbox;
    QFrame *backgroundColorFrame;
    QLabel *backgroundColorLabel;
    QToolButton *backgroundColorButton;
    QDialogButtonBox *buttonBox;
};

#endif // SETTINGSDIALOG_P_H_INCLUDED
