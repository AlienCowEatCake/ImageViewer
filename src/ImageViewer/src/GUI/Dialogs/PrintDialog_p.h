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
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Utils/ObjectsUtils.h"

struct PrintDialog::UI
{
    PrintDialog * const printDialog;
    QTabWidget * const tabWidget;

    QFrame * const generalTabFrame;
    QFrame * const imageSettingsTabFrame;

    QGroupBox * const printerSelectGroup;
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

    QGroupBox * const pageGroup;
    QRadioButton * const portraitRadioButton;
    QRadioButton * const landscapeRadioButton;
    QCheckBox * const autoRotateCheckBox;
    QPushButton * const pageSetupButton;

    QGroupBox * const detailsGroup;
    QLabel * const copiesLabel;
    QSpinBox * const copiesSpinBox;
    QLabel * const colorModeLabel;
    QComboBox * const colorModeComboBox;

    QGroupBox * const sizeGroup;
    QLabel * const widthLabel;
    QSpinBox * const widthSpinBox;
    QLabel * const heightLabel;
    QSpinBox * const heightSpinBox;
    QComboBox * const sizeUnitsComboBox;
    QLabel * const xResolutionLabel;
    QSpinBox * const xResolutionSpinBox;
    QLabel * const yResolutionLabel;
    QSpinBox * const yResolutionSpinBox;
    QCheckBox * const keepAspectCheckBox;
    QComboBox * const resolutionUnitsComboBox;
    QPushButton * const loadDefaultsButton;

    QGroupBox * const positionGroup;
    QLabel * const leftLabel;
    QSpinBox * const leftSpinBox;
    QLabel * const rightLabel;
    QSpinBox * const rightSpinBox;
    QLabel * const topLabel;
    QSpinBox * const topSpinBox;
    QLabel * const bottomLabel;
    QSpinBox * const bottomSpinBox;
    QLabel * const centerLabel;
    QComboBox * const centerComboBox;

    QCheckBox * const ignorePageMarginsCheckBox;

    QGroupBox * const previewGroup;
    QFrame * const previewFrame;

    QDialogButtonBox * const dialogButtonBox;

    explicit UI(PrintDialog *printDialog)
        : printDialog(printDialog)
        , CONSTRUCT_OBJECT(tabWidget, QTabWidget, (printDialog))
        , CONSTRUCT_OBJECT(generalTabFrame, QFrame, (printDialog))
        , CONSTRUCT_OBJECT(imageSettingsTabFrame, QFrame, (printDialog))
        , CONSTRUCT_OBJECT(printerSelectGroup, QGroupBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(printerSelectComboBox, QComboBox, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerNameHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerNameLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerDescriptionHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerDescriptionLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerDefaultHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerDefaultLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerRemoteHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerRemoteLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerLocationHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerLocationLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerMakeAndModelHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerMakeAndModelLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerStateHeaderLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printerStateLabel, QLabel, (printerSelectGroup))
        , CONSTRUCT_OBJECT(pageGroup, QGroupBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(portraitRadioButton, QRadioButton, (pageGroup))
        , CONSTRUCT_OBJECT(landscapeRadioButton, QRadioButton, (pageGroup))
        , CONSTRUCT_OBJECT(autoRotateCheckBox, QCheckBox, (pageGroup))
        , CONSTRUCT_OBJECT(pageSetupButton, QPushButton, (pageGroup))
        , CONSTRUCT_OBJECT(detailsGroup, QGroupBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(copiesLabel, QLabel, (detailsGroup))
        , CONSTRUCT_OBJECT(copiesSpinBox, QSpinBox, (detailsGroup))
        , CONSTRUCT_OBJECT(colorModeLabel, QLabel, (detailsGroup))
        , CONSTRUCT_OBJECT(colorModeComboBox, QComboBox, (detailsGroup))
        , CONSTRUCT_OBJECT(sizeGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(widthLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(widthSpinBox, QSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(heightLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(heightSpinBox, QSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(sizeUnitsComboBox, QComboBox, (sizeGroup))
        , CONSTRUCT_OBJECT(xResolutionLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(xResolutionSpinBox, QSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(yResolutionLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(yResolutionSpinBox, QSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(keepAspectCheckBox, QCheckBox, (sizeGroup))
        , CONSTRUCT_OBJECT(resolutionUnitsComboBox, QComboBox, (sizeGroup))
        , CONSTRUCT_OBJECT(loadDefaultsButton, QPushButton, (sizeGroup))
        , CONSTRUCT_OBJECT(positionGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(leftLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(leftSpinBox, QSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(rightLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(rightSpinBox, QSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(topLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(topSpinBox, QSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(bottomLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(bottomSpinBox, QSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(centerLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(centerComboBox, QComboBox, (positionGroup))
        , CONSTRUCT_OBJECT(ignorePageMarginsCheckBox, QCheckBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(previewGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(previewFrame, QFrame, (previewGroup))
        , CONSTRUCT_OBJECT(dialogButtonBox, QDialogButtonBox, (printDialog))
    {
        QGridLayout *printerInfoLayout = new QGridLayout();
        printerInfoLayout->addWidget(printerNameHeaderLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerNameLabel, 0, 1, Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDescriptionHeaderLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDescriptionLabel, 1, 1, Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDefaultHeaderLabel, 2, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerDefaultLabel, 2, 1, Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerRemoteHeaderLabel, 3, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerRemoteLabel, 3, 1, Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerLocationHeaderLabel, 4, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerLocationLabel, 4, 1, Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerMakeAndModelHeaderLabel, 5, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerMakeAndModelLabel, 5, 1, Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerStateHeaderLabel, 6, 0, Qt::AlignLeft | Qt::AlignVCenter);
        printerInfoLayout->addWidget(printerStateLabel, 6, 1, Qt::AlignVCenter);
        printerInfoLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 1);

        QVBoxLayout *printerSelectLayout = new QVBoxLayout(printerSelectGroup);
        printerSelectLayout->addWidget(printerSelectComboBox);
        printerSelectLayout->addLayout(printerInfoLayout);

        QGridLayout *pageLayout = new QGridLayout(pageGroup);
        pageLayout->addWidget(landscapeRadioButton, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
        pageLayout->addWidget(portraitRadioButton, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        pageLayout->addWidget(autoRotateCheckBox, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
        pageLayout->addWidget(pageSetupButton, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
        pageLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 2);

        QGridLayout *detailsLayout = new QGridLayout(detailsGroup);
        detailsLayout->addWidget(copiesLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
        detailsLayout->addWidget(copiesSpinBox, 0, 1, Qt::AlignVCenter);
        detailsLayout->addWidget(colorModeLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        detailsLayout->addWidget(colorModeComboBox, 1, 1, Qt::AlignVCenter);
        detailsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 2);

        QGridLayout *generalTabLayout = new QGridLayout(generalTabFrame);
        generalTabLayout->addWidget(printerSelectGroup, 0, 0, 1, 2);
        generalTabLayout->addWidget(pageGroup, 1, 0, 1, 1);
        generalTabLayout->addWidget(detailsGroup, 1, 1, 1, 1);

        QGridLayout *sizeLayout = new QGridLayout(sizeGroup);
        sizeLayout->addWidget(widthLabel, 0, 0, Qt::AlignVCenter);
        sizeLayout->addWidget(widthSpinBox, 0, 1, Qt::AlignVCenter);
        sizeLayout->addWidget(heightLabel, 1, 0, Qt::AlignVCenter);
        sizeLayout->addWidget(heightSpinBox, 1, 1, Qt::AlignVCenter);
        sizeLayout->addWidget(sizeUnitsComboBox, 0, 3, 2, 1, Qt::AlignVCenter);
        sizeLayout->addWidget(xResolutionLabel, 2, 0, Qt::AlignVCenter);
        sizeLayout->addWidget(xResolutionSpinBox, 2, 1, Qt::AlignVCenter);
        sizeLayout->addWidget(yResolutionLabel, 3, 0, Qt::AlignVCenter);
        sizeLayout->addWidget(yResolutionSpinBox, 3, 1, Qt::AlignVCenter);
        sizeLayout->addWidget(keepAspectCheckBox, 2, 2, 2, 1, Qt::AlignVCenter | Qt::AlignLeft);
        sizeLayout->addWidget(resolutionUnitsComboBox, 2, 3, 2, 1, Qt::AlignVCenter);
        sizeLayout->addWidget(loadDefaultsButton, 4, 0, 1, 4, Qt::AlignVCenter);
        sizeLayout->addItem(new QSpacerItem(72, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 1, 4, 1);

        QGridLayout *positionLayout = new QGridLayout(positionGroup);
        positionLayout->addWidget(leftLabel, 0, 0, Qt::AlignVCenter);
        positionLayout->addWidget(leftSpinBox, 0, 1, Qt::AlignVCenter);
        positionLayout->addWidget(rightLabel, 0, 2, Qt::AlignVCenter);
        positionLayout->addWidget(rightSpinBox, 0, 3, Qt::AlignVCenter);
        positionLayout->addWidget(topLabel, 1, 0, Qt::AlignVCenter);
        positionLayout->addWidget(topSpinBox, 1, 1, Qt::AlignVCenter);
        positionLayout->addWidget(bottomLabel, 1, 2, Qt::AlignVCenter);
        positionLayout->addWidget(bottomSpinBox, 1, 3, Qt::AlignVCenter);
        positionLayout->addWidget(centerLabel, 2, 0, Qt::AlignVCenter);
        positionLayout->addWidget(centerComboBox, 2, 1, 1, 3, Qt::AlignVCenter);
        positionLayout->addItem(new QSpacerItem(72, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 1, 2, 1);
        positionLayout->addItem(new QSpacerItem(72, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 3, 2, 1);

        previewFrame->setFixedSize(200, 200);
        previewFrame->setFrameStyle(QFrame::StyledPanel);

        QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
        previewLayout->addWidget(previewFrame);
        previewLayout->addStretch();

        QGridLayout *imageSettingsTabLayout = new QGridLayout(imageSettingsTabFrame);
        imageSettingsTabLayout->addWidget(sizeGroup, 0, 0, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(positionGroup, 1, 0, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(ignorePageMarginsCheckBox, 2, 0, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(previewGroup, 0, 1, 2, 1);

        tabWidget->addTab(generalTabFrame, QString());
        tabWidget->addTab(imageSettingsTabFrame, QString());

        dialogButtonBox->addButton(QDialogButtonBox::Ok);
        dialogButtonBox->addButton(QDialogButtonBox::Cancel);

        QVBoxLayout *layout = new QVBoxLayout(printDialog);
        layout->addWidget(tabWidget);
        layout->addWidget(dialogButtonBox);
        layout->addStretch();

        retranslate();
    }

    void retranslate()
    {
        printerSelectGroup->setTitle(qApp->translate("PrintDialog", "Printer"));
        printerNameHeaderLabel->setText(qApp->translate("PrintDialog", "Name:"));
        printerDescriptionHeaderLabel->setText(qApp->translate("PrintDialog", "Description:"));
        printerDefaultHeaderLabel->setText(qApp->translate("PrintDialog", "Default:"));
        printerRemoteHeaderLabel->setText(qApp->translate("PrintDialog", "Remote:"));
        printerLocationHeaderLabel->setText(qApp->translate("PrintDialog", "Location:"));
        printerMakeAndModelHeaderLabel->setText(qApp->translate("PrintDialog", "Make and model:"));
        printerStateHeaderLabel->setText(qApp->translate("PrintDialog", "State:"));

        pageGroup->setTitle(qApp->translate("PrintDialog", "Page"));
        portraitRadioButton->setText(qApp->translate("PrintDialog", "Portrait"));
        landscapeRadioButton->setText(qApp->translate("PrintDialog", "Landscape"));
        autoRotateCheckBox->setText(qApp->translate("PrintDialog", "Auto-rotate"));
        pageSetupButton->setText(qApp->translate("PrintDialog", "Page setup"));

        detailsGroup->setTitle(qApp->translate("PrintDialog", "Copies"));
        copiesLabel->setText(qApp->translate("PrintDialog", "Copies:"));
        colorModeLabel->setText(qApp->translate("PrintDialog", "Color Mode:"));

        sizeGroup->setTitle(qApp->translate("PrintDialog", "Size"));
        widthLabel->setText(qApp->translate("PrintDialog", "Width:"));
        heightLabel->setText(qApp->translate("PrintDialog", "Height:"));
        xResolutionLabel->setText(qApp->translate("PrintDialog", "X Resolution:"));
        yResolutionLabel->setText(qApp->translate("PrintDialog", "Y Resolution:"));
        loadDefaultsButton->setText(qApp->translate("PrintDialog", "Load Defaults"));

        positionGroup->setTitle(qApp->translate("PrintDialog", "Position"));
        leftLabel->setText(qApp->translate("PrintDialog", "Left:"));
        rightLabel->setText(qApp->translate("PrintDialog", "Right:"));
        topLabel->setText(qApp->translate("PrintDialog", "Top:"));
        bottomLabel->setText(qApp->translate("PrintDialog", "Bottom:"));
        centerLabel->setText(qApp->translate("PrintDialog", "Center:"));

        ignorePageMarginsCheckBox->setText(qApp->translate("PrintDialog", "Ignore Page Margins"));

        previewGroup->setTitle(qApp->translate("PrintDialog", "Preview"));

        tabWidget->setTabText(tabWidget->indexOf(generalTabFrame), qApp->translate("PrintDialog", "General"));
        tabWidget->setTabText(tabWidget->indexOf(imageSettingsTabFrame), qApp->translate("PrintDialog", "Image Settings"));

        dialogButtonBox->button(QDialogButtonBox::Ok)->setText(qApp->translate("PrintDialog", "Print"));
        dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(qApp->translate("PrintDialog", "Cancel"));
    }
};

#endif // PRINT_DIALOG_P_H_INCLUDED
