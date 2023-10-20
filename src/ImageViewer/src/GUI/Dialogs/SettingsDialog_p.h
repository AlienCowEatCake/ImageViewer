/*
   Copyright (C) 2017-2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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
#include <QTabWidget>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QSpinBox>
#include <QRadioButton>
#include <QComboBox>
#include <QButtonGroup>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPixmap>

#include "Utils/ObjectsUtils.h"
#include "Utils/SettingsWrapper.h"

#if defined(ENABLE_UPDATE_CHECKING)
#include "Updater/SettingsKeys.h"
#endif

#include "../GUISettings.h"

namespace {

const QSize COLOR_BUTTON_SIZE   (40, 24);
const QSize COLOR_ICON_SIZE     (32, 16);

} // namespace

struct SettingsDialog::UI
{
    UI(SettingsDialog *widget, GUISettings *settings)
        : settingsDialog(widget)
        , CONSTRUCT_OBJECT(tabWidget, QTabWidget, (settingsDialog))
        , CONSTRUCT_OBJECT(generalTabFrame, QFrame, (tabWidget))
#if defined(ENABLE_UPDATE_CHECKING)
        , CONSTRUCT_OBJECT(autoCheckForUpdatesCheckbox, QCheckBox, (generalTabFrame))
#endif
        , CONSTRUCT_OBJECT(askBeforeDeleteCheckbox, QCheckBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(moveToTrashCheckbox, QCheckBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(smoothTransformationCheckbox, QCheckBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(upscaleOnFitToWindowCheckbox, QCheckBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(slideShowIntervalFrame, QFrame, (generalTabFrame))
        , CONSTRUCT_OBJECT(slideShowIntervalLabel, QLabel, (slideShowIntervalFrame))
        , CONSTRUCT_OBJECT(slideShowSpinBox, QSpinBox, (slideShowIntervalFrame))
        , CONSTRUCT_OBJECT(slideShowSecLabel, QLabel, (slideShowIntervalFrame))
        , CONSTRUCT_OBJECT(wheelModeFrame, QFrame, (generalTabFrame))
        , CONSTRUCT_OBJECT(wheelModeLabel, QLabel, (wheelModeFrame))
        , CONSTRUCT_OBJECT(wheelScrollRadioButton, QRadioButton, (wheelModeFrame))
        , CONSTRUCT_OBJECT(wheelZoomRadioButton, QRadioButton, (wheelModeFrame))
        , CONSTRUCT_OBJECT(interfaceTabFrame, QFrame, (tabWidget))
        , CONSTRUCT_OBJECT(languageLabel, QLabel, (interfaceTabFrame))
        , CONSTRUCT_OBJECT(languageComboBox, QComboBox, (interfaceTabFrame))
        , CONSTRUCT_OBJECT(themeLabel, QLabel, (interfaceTabFrame))
        , CONSTRUCT_OBJECT(themeComboBox, QComboBox, (interfaceTabFrame))
#if !defined(HAS_MAC_TOOLBAR)
        , CONSTRUCT_OBJECT(toolBarPositionLabel, QLabel, (interfaceTabFrame))
        , CONSTRUCT_OBJECT(toolBarPositionComboBox, QComboBox, (interfaceTabFrame))
#endif
        , CONSTRUCT_OBJECT(backgroundColorsFrame, QFrame, (interfaceTabFrame))
        , CONSTRUCT_OBJECT(backgroundColorsLabel, QLabel, (backgroundColorsFrame))
        , CONSTRUCT_OBJECT(normalBackgroundColorLabel, QLabel, (backgroundColorsFrame))
        , CONSTRUCT_OBJECT(normalBackgroundColorButton, QToolButton, (backgroundColorsFrame))
        , CONSTRUCT_OBJECT(fullScreenBackgroundColorLabel, QLabel, (backgroundColorsFrame))
        , CONSTRUCT_OBJECT(fullScreenBackgroundColorButton, QToolButton, (backgroundColorsFrame))
        , CONSTRUCT_OBJECT(decodersTabFrame, QFrame, (tabWidget))
        , CONSTRUCT_OBJECT(enabledDecodersLabel, QLabel, (decodersTabFrame))
        , CONSTRUCT_OBJECT(enabledDecodersScrollArea, QScrollArea, (decodersTabFrame))
        , CONSTRUCT_OBJECT(enabledDecodersFrame, QFrame, (enabledDecodersScrollArea))
        , CONSTRUCT_OBJECT(buttonBox, QDialogButtonBox, (settingsDialog))
    {
        tabWidget->addTab(generalTabFrame, qApp->translate("SettingsDialog", "General"));
        tabWidget->addTab(interfaceTabFrame, qApp->translate("SettingsDialog", "Interface"));
        tabWidget->addTab(decodersTabFrame, qApp->translate("SettingsDialog", "Decoders"));

#if defined(ENABLE_UPDATE_CHECKING)
        autoCheckForUpdatesCheckbox->setText(qApp->translate("SettingsDialog", "Automatically check for updates"));
        autoCheckForUpdatesCheckbox->setChecked(SettingsWrapper(GROUP_UPDATE_MANAGER).value(KEY_AUTO_CHECK_FOR_UPDATES).toBool());
#endif

        askBeforeDeleteCheckbox->setText(qApp->translate("SettingsDialog", "Ask before deleting images"));
        askBeforeDeleteCheckbox->setChecked(settings->askBeforeDelete());

        moveToTrashCheckbox->setText(qApp->translate("SettingsDialog", "Move deleted images to trash"));
        moveToTrashCheckbox->setChecked(settings->moveToTrash());

        smoothTransformationCheckbox->setText(qApp->translate("SettingsDialog", "Use smooth image rendering"));
        smoothTransformationCheckbox->setChecked(settings->smoothTransformation());

        upscaleOnFitToWindowCheckbox->setText(qApp->translate("SettingsDialog", "Allow fit to window for small images"));
        upscaleOnFitToWindowCheckbox->setChecked(settings->upscaleOnFitToWindow());

        slideShowIntervalLabel->setText(qApp->translate("SettingsDialog", "Slideshow interval"));
        slideShowSecLabel->setText(qApp->translate("SettingsDialog", "sec"));
        slideShowSpinBox->setValue(settings->slideShowInterval());
        slideShowSpinBox->setRange(1, 1000);

        wheelModeLabel->setText(qApp->translate("SettingsDialog", "<b>Mouse wheel action</b>"));
        wheelScrollRadioButton->setText(qApp->translate("SettingsDialog", "Scroll", "WheelMode"));
        wheelZoomRadioButton->setText(qApp->translate("SettingsDialog", "Zoom", "WheelMode"));

        const ImageViewerWidget::WheelMode wheelMode = settings->wheelMode();
        wheelScrollRadioButton->setChecked(wheelMode == ImageViewerWidget::WHEEL_SCROLL);
        wheelZoomRadioButton->setChecked(wheelMode == ImageViewerWidget::WHEEL_ZOOM);

        CREATE_OBJECT(wheelModeGroup, QButtonGroup, (wheelModeFrame));
        wheelModeGroup->addButton(wheelScrollRadioButton);
        wheelModeGroup->addButton(wheelZoomRadioButton);
        wheelModeGroup->setExclusive(true);

        QHBoxLayout *slideShowIntervalLayout = new QHBoxLayout(slideShowIntervalFrame);
        slideShowIntervalLayout->setContentsMargins(0, 0, 0, 0);
        slideShowIntervalLayout->addWidget(slideShowIntervalLabel);
        slideShowIntervalLayout->addWidget(slideShowSpinBox);
        slideShowIntervalLayout->addWidget(slideShowSecLabel);
        slideShowIntervalLayout->addStretch();

        QGridLayout *wheelModeLayout = new QGridLayout(wheelModeFrame);
        wheelModeLayout->setContentsMargins(0, 0, 0, 0);
        wheelModeLayout->addWidget(wheelModeLabel, 0, 0, 1, 3, Qt::AlignLeft | Qt::AlignBottom);
        wheelModeLayout->addWidget(wheelScrollRadioButton, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        wheelModeLayout->addWidget(wheelZoomRadioButton, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        wheelModeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 2);

        QVBoxLayout *generalTabLayout = new QVBoxLayout(generalTabFrame);
#if defined(ENABLE_UPDATE_CHECKING)
        generalTabLayout->addWidget(autoCheckForUpdatesCheckbox);
#endif
        generalTabLayout->addWidget(askBeforeDeleteCheckbox);
        generalTabLayout->addWidget(moveToTrashCheckbox);
        generalTabLayout->addWidget(smoothTransformationCheckbox);
        generalTabLayout->addWidget(upscaleOnFitToWindowCheckbox);
        generalTabLayout->addWidget(slideShowIntervalFrame);
        generalTabLayout->addWidget(wheelModeFrame);
        generalTabLayout->addStretch();

        languageLabel->setText(qApp->translate("SettingsDialog", "<b>Language</b>"));
        themeLabel->setText(qApp->translate("SettingsDialog", "<b>Theme</b>"));
#if !defined(HAS_MAC_TOOLBAR)
        toolBarPositionLabel->setText(qApp->translate("SettingsDialog", "<b>Toolbar Position</b>"));
#endif

        backgroundColorsLabel->setText(qApp->translate("SettingsDialog", "<b>Background colors</b>"));
        normalBackgroundColorLabel->setText(qApp->translate("SettingsDialog", "Normal:"));
        fullScreenBackgroundColorLabel->setText(qApp->translate("SettingsDialog", "Fullscreen:"));

        normalBackgroundColorButton->setFixedSize(COLOR_BUTTON_SIZE);
        normalBackgroundColorButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        normalBackgroundColorButton->setIconSize(COLOR_ICON_SIZE);

        fullScreenBackgroundColorButton->setFixedSize(COLOR_BUTTON_SIZE);
        fullScreenBackgroundColorButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        fullScreenBackgroundColorButton->setIconSize(COLOR_ICON_SIZE);

        QGridLayout *backgroundColorsLayout = new QGridLayout(backgroundColorsFrame);
        backgroundColorsLayout->setContentsMargins(0, 0, 0, 0);
        backgroundColorsLayout->addWidget(backgroundColorsLabel, 0, 0, 1, 5, Qt::AlignLeft | Qt::AlignBottom);
        backgroundColorsLayout->addWidget(normalBackgroundColorLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addWidget(normalBackgroundColorButton, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addWidget(fullScreenBackgroundColorLabel, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addWidget(fullScreenBackgroundColorButton, 1, 3, Qt::AlignLeft | Qt::AlignVCenter);
        backgroundColorsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 4);

        QVBoxLayout *interfaceTabLayout = new QVBoxLayout(interfaceTabFrame);
        interfaceTabLayout->addWidget(languageLabel);
        interfaceTabLayout->addWidget(languageComboBox);
        interfaceTabLayout->addWidget(themeLabel);
        interfaceTabLayout->addWidget(themeComboBox);
#if !defined(HAS_MAC_TOOLBAR)
        interfaceTabLayout->addWidget(toolBarPositionLabel);
        interfaceTabLayout->addWidget(toolBarPositionComboBox);
#endif
        interfaceTabLayout->addWidget(backgroundColorsFrame);
        interfaceTabLayout->addStretch();

        enabledDecodersLabel->setText(qApp->translate("SettingsDialog", "<b>Enabled decoders:</b>"));

        QVBoxLayout *decodersTabLayout = new QVBoxLayout(decodersTabFrame);
        decodersTabLayout->addWidget(enabledDecodersLabel);
        decodersTabLayout->addWidget(enabledDecodersScrollArea);
        decodersTabLayout->addStretch();

        buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

        QVBoxLayout *mainLayout = new QVBoxLayout(settingsDialog);
        mainLayout->addWidget(tabWidget);
        mainLayout->addWidget(buttonBox);
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
    QTabWidget *tabWidget;
    QFrame *generalTabFrame;
#if defined(ENABLE_UPDATE_CHECKING)
    QCheckBox *autoCheckForUpdatesCheckbox;
#endif
    QCheckBox *askBeforeDeleteCheckbox;
    QCheckBox *moveToTrashCheckbox;
    QCheckBox *smoothTransformationCheckbox;
    QCheckBox *upscaleOnFitToWindowCheckbox;
    QFrame *slideShowIntervalFrame;
    QLabel *slideShowIntervalLabel;
    QSpinBox *slideShowSpinBox;
    QLabel *slideShowSecLabel;
    QFrame *wheelModeFrame;
    QLabel *wheelModeLabel;
    QRadioButton *wheelScrollRadioButton;
    QRadioButton *wheelZoomRadioButton;
    QFrame *interfaceTabFrame;
    QLabel *languageLabel;
    QComboBox *languageComboBox;
    QLabel *themeLabel;
    QComboBox *themeComboBox;
#if !defined(HAS_MAC_TOOLBAR)
    QLabel *toolBarPositionLabel;
    QComboBox *toolBarPositionComboBox;
#endif
    QFrame *backgroundColorsFrame;
    QLabel *backgroundColorsLabel;
    QLabel *normalBackgroundColorLabel;
    QToolButton *normalBackgroundColorButton;
    QLabel *fullScreenBackgroundColorLabel;
    QToolButton *fullScreenBackgroundColorButton;
    QFrame *decodersTabFrame;
    QLabel *enabledDecodersLabel;
    QScrollArea *enabledDecodersScrollArea;
    QFrame *enabledDecodersFrame;
    QDialogButtonBox *buttonBox;
};

#endif // SETTINGSDIALOG_P_H_INCLUDED
