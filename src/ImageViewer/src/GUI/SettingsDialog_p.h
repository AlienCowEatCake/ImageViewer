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
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
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
        , slideShowIntervalFrame(new QFrame(settingsDialog))
        , slideShowIntervalLabel(new QLabel(slideShowIntervalFrame))
        , slideShowSpinBox(new QSpinBox(slideShowIntervalFrame))
        , slideShowSecLabel(new QLabel(slideShowIntervalFrame))
        , backgroundColorsFrame(new QFrame(settingsDialog))
        , backgroundColorsLabel(new QLabel(backgroundColorsFrame))
        , normalBackgroundColorLabel(new QLabel(backgroundColorsFrame))
        , normalBackgroundColorButton(new QToolButton(backgroundColorsFrame))
        , fullScreenBackgroundColorLabel(new QLabel(backgroundColorsFrame))
        , fullScreenBackgroundColorButton(new QToolButton(backgroundColorsFrame))
        , buttonBox(new QDialogButtonBox(settingsDialog))
    {
        askBeforeDeleteCheckbox->setText(qApp->translate("SettingsDialog", "Ask before deleting images"));
        askBeforeDeleteCheckbox->setChecked(settings->askBeforeDelete());

        moveToTrashCheckbox->setText(qApp->translate("SettingsDialog", "Move deleted images to trash"));
        moveToTrashCheckbox->setChecked(settings->moveToTrash());

        smoothTransformationCheckbox->setText(qApp->translate("SettingsDialog", "Use smooth image rendering"));
        smoothTransformationCheckbox->setChecked(settings->smoothTransformation());

        slideShowIntervalLabel->setText(qApp->translate("SettingsDialog", "Slideshow interval"));
        slideShowSecLabel->setText(qApp->translate("SettingsDialog", "sec"));
        slideShowSpinBox->setRange(1, 1000);

        backgroundColorsLabel->setText(qApp->translate("SettingsDialog", "<b>Background colors</b>"));
        normalBackgroundColorLabel->setText(qApp->translate("SettingsDialog", "Normal:"));
        fullScreenBackgroundColorLabel->setText(qApp->translate("SettingsDialog", "Fullscreen:"));

        normalBackgroundColorButton->setFixedSize(COLOR_BUTTON_SIZE);
        normalBackgroundColorButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        normalBackgroundColorButton->setIconSize(COLOR_ICON_SIZE);

        fullScreenBackgroundColorButton->setFixedSize(COLOR_BUTTON_SIZE);
        fullScreenBackgroundColorButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        fullScreenBackgroundColorButton->setIconSize(COLOR_ICON_SIZE);

        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        QHBoxLayout *slideShowIntervalLayout = new QHBoxLayout(slideShowIntervalFrame);
        slideShowIntervalLayout->setContentsMargins(0, 0, 0, 0);
        slideShowIntervalLayout->addWidget(slideShowIntervalLabel);
        slideShowIntervalLayout->addWidget(slideShowSpinBox);
        slideShowIntervalLayout->addWidget(slideShowSecLabel);
        slideShowIntervalLayout->addStretch();

        QGridLayout *backgroundColorsLayout = new QGridLayout(backgroundColorsFrame);
        backgroundColorsLayout->setContentsMargins(0, 0, 0, 0);
        backgroundColorsLayout->addWidget(backgroundColorsLabel, 0, 0, 1, 5, Qt::AlignLeft | Qt::AlignBottom);
        backgroundColorsLayout->addWidget(normalBackgroundColorLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addWidget(normalBackgroundColorButton, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addWidget(fullScreenBackgroundColorLabel, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addWidget(fullScreenBackgroundColorButton, 1, 3, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 4);

        QVBoxLayout *mainLayout = new QVBoxLayout(settingsDialog);
        mainLayout->addWidget(askBeforeDeleteCheckbox);
        mainLayout->addWidget(moveToTrashCheckbox);
        mainLayout->addWidget(smoothTransformationCheckbox);
        mainLayout->addWidget(slideShowIntervalFrame);
        mainLayout->addWidget(backgroundColorsFrame);
        mainLayout->addWidget(buttonBox);

        settingsDialog->setWindowTitle(qApp->translate("SettingsDialog", "Preferences"));
        settingsDialog->ensurePolished();
        settingsDialog->adjustSize();
        settingsDialog->setFixedSize(widget->minimumSize());
        settingsDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint |
#if (QT_VERSION >= QT_VERSION_CHECK(4, 5, 0))
                               Qt::WindowCloseButtonHint |
#endif
                               Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);
        settingsDialog->setWindowModality(Qt::ApplicationModal);
    }

    void updateNormalBackgroundColorButton(const QColor &color)
    {
        QPixmap pixmap(COLOR_ICON_SIZE);
        pixmap.fill(color);
        normalBackgroundColorButton->setIcon(QIcon(pixmap));
    }

    void updateFullScreenBackgroundColorButton(const QColor &color)
    {
        QPixmap pixmap(COLOR_ICON_SIZE);
        pixmap.fill(color);
        fullScreenBackgroundColorButton->setIcon(QIcon(pixmap));
    }

    SettingsDialog *settingsDialog;
    QCheckBox *askBeforeDeleteCheckbox;
    QCheckBox *moveToTrashCheckbox;
    QCheckBox *smoothTransformationCheckbox;
    QFrame *slideShowIntervalFrame;
    QLabel *slideShowIntervalLabel;
    QSpinBox *slideShowSpinBox;
    QLabel *slideShowSecLabel;
    QFrame *backgroundColorsFrame;
    QLabel *backgroundColorsLabel;
    QLabel *normalBackgroundColorLabel;
    QToolButton *normalBackgroundColorButton;
    QLabel *fullScreenBackgroundColorLabel;
    QToolButton *fullScreenBackgroundColorButton;
    QDialogButtonBox *buttonBox;
};

#endif // SETTINGSDIALOG_P_H_INCLUDED
