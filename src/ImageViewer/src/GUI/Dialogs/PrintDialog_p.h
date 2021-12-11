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
#include <QDebug>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGraphicsItem>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyleOptionGraphicsItem>
#include <QTabWidget>
#include <QTransform>
#include <QVBoxLayout>

#include "Utils/ObjectsUtils.h"

#include "Decoders/GraphicsItemFeatures/IGrabImage.h"

class PrintPreviewWidget : public QWidget
{
public:
    PrintPreviewWidget(QWidget *parent = Q_NULLPTR)
        : QWidget(parent)
        , m_graphicsItem(Q_NULLPTR)
        , m_rotateAngle(0)
    {}

    void setGraphicsItem(QGraphicsItem *item, int rotateAngle, const Qt::Orientations &flipOrientations)
    {
        m_graphicsItem = item;
        m_rotateAngle = rotateAngle;
        m_flipOrientations = flipOrientations;
        update();
    }

    void setPaperRect(const QRectF &rect)
    {
        m_paperRect = rect;
        update();
    }

    void setPageRect(const QRectF &rect)
    {
        m_pageRect = rect;
        update();
    }

    void setItemRect(const QRectF &rect)
    {
        m_itemRect = rect;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        QWidget::paintEvent(event);
        const qreal paperOffset = 10;
        const qreal xscale = (width() - paperOffset) / m_paperRect.width();
        const qreal yscale = (height() - paperOffset) / m_paperRect.height();
        const qreal scale = qMin(xscale, yscale);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(0, 0, width(), height(), Qt::black);
        painter.translate(width() / 2, height() / 2);
        painter.scale(scale, scale);
        painter.translate(-m_paperRect.width() / 2, -m_paperRect.height() / 2);
        const qreal paperShadowOffset = 2 / scale;
        painter.fillRect(m_paperRect.adjusted(paperShadowOffset, paperShadowOffset, paperShadowOffset, paperShadowOffset), Qt::darkGray);
        painter.fillRect(m_paperRect, Qt::white);
        if(m_graphicsItem)
        {
            painter.save();
            const QRectF boundingRect = m_graphicsItem->boundingRect();
            const QRectF rotatedBoundingRect = QTransform()
                    .translate(boundingRect.center().x(), boundingRect.center().y())
                    .rotate(m_rotateAngle)
                    .translate(-boundingRect.center().x(), -boundingRect.center().y())
                    .mapRect(boundingRect);
            painter.translate(m_itemRect.x(), m_itemRect.y());
            painter.scale(m_itemRect.width() / rotatedBoundingRect.width(), m_itemRect.height() / rotatedBoundingRect.height());
            painter.translate(-rotatedBoundingRect.x(), -rotatedBoundingRect.y());
            painter.translate(rotatedBoundingRect.center().x(), rotatedBoundingRect.center().y());
            painter.rotate(m_rotateAngle);
            painter.translate(-rotatedBoundingRect.center().x(), -rotatedBoundingRect.center().y());
            if(m_flipOrientations & Qt::Horizontal)
            {
                painter.scale(-1, 1);
                painter.translate(-boundingRect.width(), 0);
            }
            if(m_flipOrientations & Qt::Vertical)
            {
                painter.scale(1, -1);
                painter.translate(0, -boundingRect.height());
            }
            if(IGrabImage *itemWithGrabImage = dynamic_cast<IGrabImage*>(m_graphicsItem))
            {
                QImage image = itemWithGrabImage->grabImage();
                const QTransform worldTransform = painter.worldTransform();
                const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
                const QImage scaledImage = image.scaled(deviceRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if(!scaledImage.isNull())
                    image = scaledImage;
                else
                    qWarning() << "Image scaling failed, target size =" << deviceRect.size();
                painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
            }
            else
            {
                QStyleOptionGraphicsItem options;
                options.exposedRect = boundingRect;
                m_graphicsItem->paint(&painter, &options);
            }
            painter.restore();
        }
        else
        {
            painter.fillRect(m_itemRect, QBrush(Qt::green, Qt::Dense4Pattern));
        }
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter.drawRect(m_itemRect);
        painter.setPen(QPen(Qt::black, 1, Qt::DotLine));
        painter.drawRect(m_itemRect);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter.drawRect(m_pageRect);
        painter.setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter.drawRect(m_pageRect);
    }

private:
    QGraphicsItem *m_graphicsItem;
    int m_rotateAngle;
    Qt::Orientations m_flipOrientations;
    QRectF m_paperRect;
    QRectF m_pageRect;
    QRectF m_itemRect;
};

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

    QGroupBox * const miscGroup;
    QLabel * const copiesLabel;
    QSpinBox * const copiesSpinBox;
    QLabel * const colorModeLabel;
    QComboBox * const colorModeComboBox;

    QGroupBox * const sizeGroup;
    QLabel * const widthLabel;
    QDoubleSpinBox * const widthSpinBox;
    QLabel * const heightLabel;
    QDoubleSpinBox * const heightSpinBox;
    QComboBox * const sizeUnitsComboBox;
    QLabel * const xResolutionLabel;
    QDoubleSpinBox * const xResolutionSpinBox;
    QLabel * const yResolutionLabel;
    QDoubleSpinBox * const yResolutionSpinBox;
    QLabel * const keepAspectLabelTop;
    QLabel * const keepAspectLabelBottom;
    QCheckBox * const keepAspectCheckBox;
    QComboBox * const resolutionUnitsComboBox;
    QPushButton * const loadDefaultsButton;

    QGroupBox * const positionGroup;
    QLabel * const leftLabel;
    QDoubleSpinBox * const leftSpinBox;
    QLabel * const rightLabel;
    QDoubleSpinBox * const rightSpinBox;
    QLabel * const topLabel;
    QDoubleSpinBox * const topSpinBox;
    QLabel * const bottomLabel;
    QDoubleSpinBox * const bottomSpinBox;
    QLabel * const centerLabel;
    QComboBox * const centerComboBox;

    QCheckBox * const ignorePageMarginsCheckBox;

    QGroupBox * const previewGroup;
    PrintPreviewWidget * const previewWidget;

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
        , CONSTRUCT_OBJECT(miscGroup, QGroupBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(copiesLabel, QLabel, (miscGroup))
        , CONSTRUCT_OBJECT(copiesSpinBox, QSpinBox, (miscGroup))
        , CONSTRUCT_OBJECT(colorModeLabel, QLabel, (miscGroup))
        , CONSTRUCT_OBJECT(colorModeComboBox, QComboBox, (miscGroup))
        , CONSTRUCT_OBJECT(sizeGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(widthLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(widthSpinBox, QDoubleSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(heightLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(heightSpinBox, QDoubleSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(sizeUnitsComboBox, QComboBox, (sizeGroup))
        , CONSTRUCT_OBJECT(xResolutionLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(xResolutionSpinBox, QDoubleSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(yResolutionLabel, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(yResolutionSpinBox, QDoubleSpinBox, (sizeGroup))
        , CONSTRUCT_OBJECT(keepAspectLabelTop, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(keepAspectLabelBottom, QLabel, (sizeGroup))
        , CONSTRUCT_OBJECT(keepAspectCheckBox, QCheckBox, (sizeGroup))
        , CONSTRUCT_OBJECT(resolutionUnitsComboBox, QComboBox, (sizeGroup))
        , CONSTRUCT_OBJECT(loadDefaultsButton, QPushButton, (sizeGroup))
        , CONSTRUCT_OBJECT(positionGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(leftLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(leftSpinBox, QDoubleSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(rightLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(rightSpinBox, QDoubleSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(topLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(topSpinBox, QDoubleSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(bottomLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(bottomSpinBox, QDoubleSpinBox, (positionGroup))
        , CONSTRUCT_OBJECT(centerLabel, QLabel, (positionGroup))
        , CONSTRUCT_OBJECT(centerComboBox, QComboBox, (positionGroup))
        , CONSTRUCT_OBJECT(ignorePageMarginsCheckBox, QCheckBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(previewGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(previewWidget, PrintPreviewWidget, (previewGroup))
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

        QGridLayout *detailsLayout = new QGridLayout(miscGroup);
        detailsLayout->addWidget(copiesLabel, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
        detailsLayout->addWidget(copiesSpinBox, 0, 1, Qt::AlignVCenter);
        detailsLayout->addWidget(colorModeLabel, 1, 0, Qt::AlignLeft | Qt::AlignVCenter);
        detailsLayout->addWidget(colorModeComboBox, 1, 1, Qt::AlignVCenter);
        detailsLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding), 2, 2);

        QGridLayout *generalTabLayout = new QGridLayout(generalTabFrame);
        generalTabLayout->addWidget(printerSelectGroup, 0, 0, 1, 2);
        generalTabLayout->addWidget(pageGroup, 1, 0, 1, 1);
        generalTabLayout->addWidget(miscGroup, 1, 1, 1, 1);

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
        sizeLayout->addWidget(keepAspectLabelTop, 2, 2, Qt::AlignVCenter | Qt::AlignLeft);
        sizeLayout->addWidget(keepAspectLabelBottom, 3, 2, Qt::AlignVCenter | Qt::AlignLeft);
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

        previewWidget->setFixedSize(200, 200);

        QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
        previewLayout->addWidget(previewWidget);
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

        miscGroup->setTitle(qApp->translate("PrintDialog", "Misc"));
        copiesLabel->setText(qApp->translate("PrintDialog", "Copies:"));
        colorModeLabel->setText(qApp->translate("PrintDialog", "Color Mode:"));

        sizeGroup->setTitle(qApp->translate("PrintDialog", "Size"));
        widthLabel->setText(qApp->translate("PrintDialog", "Width:"));
        heightLabel->setText(qApp->translate("PrintDialog", "Height:"));
        xResolutionLabel->setText(qApp->translate("PrintDialog", "X Resolution:"));
        yResolutionLabel->setText(qApp->translate("PrintDialog", "Y Resolution:"));
        loadDefaultsButton->setText(qApp->translate("PrintDialog", "Load Defaults"));
        keepAspectLabelTop->setText(qApp->translate("PrintDialog", "<b>&#9582;</b>"));
        keepAspectLabelBottom->setText(qApp->translate("PrintDialog", "<b>&#9583;</b>"));

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
