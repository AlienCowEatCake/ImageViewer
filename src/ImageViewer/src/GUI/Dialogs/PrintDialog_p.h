/*
   Copyright (C) 2021-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cmath>

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGraphicsItem>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QStyleOptionGraphicsItem>
#include <QTabWidget>
#include <QTransform>
#include <QVBoxLayout>

#include "Utils/IsOneOf.h"
#include "Utils/Logging.h"
#include "Utils/ObjectsUtils.h"

#include "Decoders/GraphicsItemFeatures/IGrabImage.h"
#include "Decoders/GraphicsItemFeatures/IGrabScaledImage.h"
#include "Decoders/GraphicsItemFeatures/ITransformationMode.h"

class PrintEffect
{
    Q_DISABLE_COPY(PrintEffect)

public:
    PrintEffect()
        : m_legacyRenderer(false)
        , m_grayscale(false)
        , m_brightness(0)
        , m_contrast(0)
        , m_exposure(0)
    {}

    ~PrintEffect()
    {}

    bool legacyRenderer() const
    {
        return m_legacyRenderer;
    }

    void setLegacyRenderer(bool legacyRenderer)
    {
        m_legacyRenderer = legacyRenderer;
    }

    void setGrayscale(bool grayscale)
    {
        m_grayscale = grayscale;
    }

    void setBrightness(int level)
    {
        m_brightness = level;
    }

    void setContrast(int level)
    {
        m_contrast = level;
    }

    void setExposure(int level)
    {
        m_exposure = level;
    }

    void apply(QImage &image) const
    {
        if(m_legacyRenderer)
            return;

        if(!m_grayscale && !m_brightness && !m_contrast && !m_exposure)
            return;

        const bool useRgbTransform = m_brightness || m_contrast;
        int rgbTransform[256];
        if(useRgbTransform)
        {
            for(int i = 0; i < 256; ++i)
            {
                int c = i;
                if(m_brightness)
                {
                    c = qBound(0, c + m_brightness, 255);
                }
                if(m_contrast)
                {
                    // https://ie.nitk.ac.in/blog/2020/01/19/algorithms-for-adjusting-brightness-and-contrast-of-an-image/
                    const qreal f = (259.0 * (m_contrast + 255.0)) / (255.0 * (259.0 - m_contrast));
                    c = qBound(0, static_cast<int>(f * (c - 128) + 128), 255);
                }
                rgbTransform[i] = c;
            }
        }

        const bool useVTransform = m_exposure;
        int vTransform[256];
        if(useVTransform)
        {
            for(int i = 0; i < 256; ++i)
            {
                int v = i;
                if(m_exposure)
                {
                    // https://habr.com/ru/post/268115/
                    v = qBound(0, v + static_cast<int>(std::sin(v * 0.01255) * m_exposure), 255);
                }
                vTransform[i] = v;
            }
        }

        if(!IsOneOf(image.format(), QImage::Format_RGB32, QImage::Format_ARGB32))
            QImage_convertTo(image, image.hasAlphaChannel() ? QImage::Format_ARGB32 : QImage::Format_RGB32);

        for(int j = 0, height = image.height(); j < height; ++j)
        {
            QRgb *line = reinterpret_cast<QRgb*>(image.scanLine(j));
            for(int i = 0, width = image.width(); i < width; ++i)
            {
                QRgb &pixel = line[i];
                const int a = qAlpha(pixel);
                if(useRgbTransform)
                {
                    pixel = qRgba(rgbTransform[qRed(pixel)], rgbTransform[qGreen(pixel)], rgbTransform[qBlue(pixel)], a);
                }
                if(useVTransform)
                {
                    QColor color(pixel);
                    color = color.toHsv();
                    int h = 0, s = 0, v = 0;
                    color.getHsv(&h, &s, &v);
                    color.setHsv(h, s, vTransform[v]);
                    color = color.toRgb();
                    int r = 0, g = 0, b = 0;
                    color.getRgb(&r, &g, &b);
                    pixel = qRgba(r, g, b, a);
                }
                if(m_grayscale)
                {
                    const int c = qGray(pixel);
                    pixel = qRgba(c, c, c, a);
                }
            }
        }
    }

private:
    bool m_legacyRenderer;
    bool m_grayscale;
    int m_brightness;
    int m_contrast;
    int m_exposure;
};

class PrintPreviewWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PrintPreviewWidget)

Q_SIGNALS:
    void geometryChangeRequested(const QRectF &newGeometry);

private:
    enum Position
    {
        POS_NONE,
        POS_LEFT,
        POS_RIGHT,
        POS_TOP,
        POS_BOTTOM,
        POS_TOP_LEFT,
        POS_TOP_RIGHT,
        POS_BOTTOM_LEFT,
        POS_BOTTOM_RIGHT,
        POS_INSIDE
    };

public:
    PrintPreviewWidget(QWidget *parent = Q_NULLPTR)
        : QWidget(parent)
        , m_graphicsItem(Q_NULLPTR)
        , m_rotateAngle(0)
        , m_operation(POS_NONE)
        , m_editable(false)
    {
        setAttribute(Qt::WA_MouseTracking);
    }

    ~PrintPreviewWidget()
    {}

    void setGraphicsItem(QGraphicsItem *item, int rotateAngle, const Qt::Orientations &flipOrientations)
    {
        m_graphicsItem = item;
        m_rotateAngle = rotateAngle;
        m_flipOrientations = flipOrientations;
        update();
    }

    void setEditable(bool editable)
    {
        m_editable = editable;
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

    void setLegacyRenderer(bool legacyRenderer)
    {
        m_printEffect.setLegacyRenderer(legacyRenderer);
        update();
    }

    void setGrayscale(bool grayscale)
    {
        m_printEffect.setGrayscale(grayscale);
        update();
    }

    void setBrightness(int level)
    {
        m_printEffect.setBrightness(level);
        update();
    }

    void setContrast(int level)
    {
        m_printEffect.setContrast(level);
        update();
    }

    void setExposure(int level)
    {
        m_printEffect.setExposure(level);
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE
    {
        QWidget::paintEvent(event);
        const qreal scale = this->scale();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.fillRect(0, 0, width(), height(), Qt::black);
        if(m_paperRect.isEmpty())
            return;
        painter.translate(width() / 2, height() / 2);
        painter.scale(scale, scale);
        painter.translate(-m_paperRect.width() / 2, -m_paperRect.height() / 2);
        const qreal paperShadowOffset = 2 / scale;
        painter.fillRect(m_paperRect.adjusted(paperShadowOffset, paperShadowOffset, paperShadowOffset, paperShadowOffset), Qt::darkGray);
        painter.fillRect(m_paperRect, Qt::white);
        if(m_graphicsItem)
        {
            painter.save();
            painter.setClipRect(m_paperRect);
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
            if(!m_printEffect.legacyRenderer())
            {
                const QTransform worldTransform = painter.worldTransform();
                const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
                Qt::TransformationMode transformationMode = Qt::SmoothTransformation;
                if(QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(m_graphicsItem))
                    transformationMode = pixmapItem->transformationMode();
                if(ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(m_graphicsItem))
                    transformationMode = itemWithTransformationMode->transformationMode();
                const qreal scaleFactor = qMax(deviceRect.width() / rotatedBoundingRect.width(), deviceRect.height() / rotatedBoundingRect.height());
                bool applyEffectsBeforeTransform = true;
                QImage image;
                if(IGrabScaledImage *itemWithGrabScaledImage = dynamic_cast<IGrabScaledImage*>(m_graphicsItem))
                {
                    image = itemWithGrabScaledImage->grabImage(qMax(scaleFactor, static_cast<qreal>(1.0)));
                    applyEffectsBeforeTransform = scaleFactor >= 1.0;
                }
                else if(IGrabImage *itemWithGrabImage = dynamic_cast<IGrabImage*>(m_graphicsItem))
                {
                    image = itemWithGrabImage->grabImage();
                    applyEffectsBeforeTransform = scaleFactor >= 1.0;
                }
                else
                {
                    QSize size = (boundingRect.size() * scaleFactor).toSize();
                    image = QImage(size, QImage::Format_ARGB32_Premultiplied);
                    while(image.isNull() && !size.isEmpty())
                    {
                        LOG_WARNING() << LOGGING_CTX << "Image rendering failed, target size =" << size.width() << "x" << size.height();
                        size = QSize(size.width() / 2, size.height() / 2);
                        image = QImage(size, QImage::Format_ARGB32_Premultiplied);
                    }
                    image.fill(Qt::transparent);
                    QPainter painterLocal;
                    painterLocal.begin(&image);
                    painterLocal.scale(image.width() / boundingRect.width(), image.height() / boundingRect.height());
                    painterLocal.translate(-boundingRect.x(), -boundingRect.y());
                    painterLocal.setRenderHint(QPainter::Antialiasing, transformationMode == Qt::SmoothTransformation);
                    painterLocal.setRenderHint(QPainter::TextAntialiasing, transformationMode == Qt::SmoothTransformation);
                    painterLocal.setRenderHint(QPainter::SmoothPixmapTransform, transformationMode == Qt::SmoothTransformation);
                    QStyleOptionGraphicsItem options;
                    options.exposedRect = boundingRect;
                    m_graphicsItem->paint(&painterLocal, &options);
                    painterLocal.end();
                }
                if(applyEffectsBeforeTransform)
                    m_printEffect.apply(image);
                if(m_flipOrientations)
                    QImage_flip(image, m_flipOrientations);
                QSize unrotatedDeviceSize = deviceRect.size();
                if(m_rotateAngle)
                {
                    QTransform transform;
                    transform.rotate(-m_rotateAngle);
                    unrotatedDeviceSize = transform.mapRect(deviceRect).size();
                }
                if(transformationMode == Qt::SmoothTransformation && unrotatedDeviceSize.width() != image.width() && unrotatedDeviceSize.height() != image.height())
                {
                    const QImage scaledImage = image.scaled(unrotatedDeviceSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    if(!scaledImage.isNull())
                        image = scaledImage;
                    else
                        LOG_WARNING() << LOGGING_CTX << "Image scaling failed, target size =" << unrotatedDeviceSize.width() << "x" << unrotatedDeviceSize.height();
                }
                if(!applyEffectsBeforeTransform)
                    m_printEffect.apply(image);
                painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
            }
            else
            {
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
                if(IGrabScaledImage *itemWithGrabScaledImage = dynamic_cast<IGrabScaledImage*>(m_graphicsItem))
                {
                    const QTransform worldTransform = painter.worldTransform();
                    const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
                    const qreal scaleFactor = qMax(qMax(deviceRect.width() / rotatedBoundingRect.width(), deviceRect.height() / rotatedBoundingRect.height()), static_cast<qreal>(1.0));
                    QImage image = itemWithGrabScaledImage->grabImage(scaleFactor);
                    Qt::TransformationMode transformationMode = Qt::SmoothTransformation;
                    if(QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(m_graphicsItem))
                        transformationMode = pixmapItem->transformationMode();
                    if(ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(m_graphicsItem))
                        transformationMode = itemWithTransformationMode->transformationMode();
                    if(transformationMode == Qt::SmoothTransformation && deviceRect.width() < image.width() && deviceRect.height() < image.height())
                    {
                        const QImage scaledImage = image.scaled(deviceRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        if(!scaledImage.isNull())
                            image = scaledImage;
                        else
                            LOG_WARNING() << LOGGING_CTX << "Image scaling failed, target size =" << deviceRect.width() << "x" << deviceRect.height();
                    }
                    painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
                }
                else if(IGrabImage *itemWithGrabImage = dynamic_cast<IGrabImage*>(m_graphicsItem))
                {
                    QImage image = itemWithGrabImage->grabImage();
                    const QTransform worldTransform = painter.worldTransform();
                    const QRect deviceRect = worldTransform.mapRect(boundingRect).toAlignedRect();
                    Qt::TransformationMode transformationMode = Qt::SmoothTransformation;
                    if(QGraphicsPixmapItem *pixmapItem = dynamic_cast<QGraphicsPixmapItem*>(m_graphicsItem))
                        transformationMode = pixmapItem->transformationMode();
                    if(ITransformationMode *itemWithTransformationMode = dynamic_cast<ITransformationMode*>(m_graphicsItem))
                        transformationMode = itemWithTransformationMode->transformationMode();
                    if(transformationMode == Qt::SmoothTransformation && deviceRect.width() < image.width() && deviceRect.height() < image.height())
                    {
                        const QImage scaledImage = image.scaled(deviceRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                        if(!scaledImage.isNull())
                            image = scaledImage;
                        else
                            LOG_WARNING() << LOGGING_CTX << "Image scaling failed, target size =" << deviceRect.width() << "x" << deviceRect.height();
                    }
                    painter.drawImage(worldTransform.inverted().mapRect(deviceRect), image);
                }
                else
                {
                    QStyleOptionGraphicsItem options;
                    options.exposedRect = boundingRect;
                    m_graphicsItem->paint(&painter, &options);
                }
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

    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        QWidget::mouseMoveEvent(event);
        if(m_paperRect.isEmpty() || m_itemRect.isEmpty() || !m_editable)
            return;

        if(m_operation == POS_NONE)
            updateCursor(event->pos());
        else
            applyGeometryChanges(event->pos());
    }

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        QWidget::mousePressEvent(event);
        if(m_paperRect.isEmpty() || m_itemRect.isEmpty() || !m_editable)
            return;

        m_operation = currentPosition(event->pos());
        m_startOffset = event->pos() - transform().map(m_itemRect.center()).toPoint();
        updateCursor(event->pos());
    }

    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE
    {
        QWidget::mouseReleaseEvent(event);
        if(m_paperRect.isEmpty() || m_itemRect.isEmpty() || !m_editable)
            return;

        if(m_operation != POS_NONE)
        {
            applyGeometryChanges(event->pos());
            m_operation = POS_NONE;
            updateCursor(event->pos());
        }
    }

private:
    qreal scale() const
    {
        const qreal paperOffset = 10;
        const qreal xscale = (width() - paperOffset) / m_paperRect.width();
        const qreal yscale = (height() - paperOffset) / m_paperRect.height();
        return qMin(xscale, yscale);
    }

    QTransform transform() const
    {
        const qreal scale = this->scale();
        QTransform t;
        t.translate(width() / 2, height() / 2);
        t.scale(scale, scale);
        t.translate(-m_paperRect.width() / 2, -m_paperRect.height() / 2);
        return t;
    }

    bool isNear(int lhs, int rhs) const
    {
        return qAbs(lhs - rhs) < 4;
    }

    bool isNear(const QPoint &lhs, const QPoint &rhs) const
    {
        return isNear((lhs - rhs).manhattanLength(), 0);
    }

    Position currentPosition(const QPoint &localPos) const
    {
        const QRect itemRect = transform().mapRect(m_itemRect).toAlignedRect();
        if(isNear(localPos, itemRect.topLeft()))
            return POS_TOP_LEFT;
        if(isNear(localPos, itemRect.topRight()))
            return POS_TOP_RIGHT;
        if(isNear(localPos, itemRect.bottomLeft()))
            return POS_BOTTOM_LEFT;
        if(isNear(localPos, itemRect.bottomRight()))
            return POS_BOTTOM_RIGHT;
        if(isNear(localPos.x(), itemRect.left()) && qBound(itemRect.top(), localPos.y(), itemRect.bottom()) == localPos.y())
            return POS_LEFT;
        if(isNear(localPos.x(), itemRect.right()) && qBound(itemRect.top(), localPos.y(), itemRect.bottom()) == localPos.y())
            return POS_RIGHT;
        if(isNear(localPos.y(), itemRect.top()) && qBound(itemRect.left(), localPos.x(), itemRect.right()) == localPos.x())
            return POS_TOP;
        if(isNear(localPos.y(), itemRect.bottom()) && qBound(itemRect.left(), localPos.x(), itemRect.right()) == localPos.x())
            return POS_BOTTOM;
        if(itemRect.contains(localPos, true))
            return POS_INSIDE;
        return POS_NONE;
    }

    void updateCursor(const QPoint &localPos)
    {
        switch(currentPosition(localPos))
        {
        case POS_LEFT:
        case POS_RIGHT:
            setCursor(Qt::SizeHorCursor);
            break;
        case POS_TOP:
        case POS_BOTTOM:
            setCursor(Qt::SizeVerCursor);
            break;
        case POS_TOP_LEFT:
        case POS_BOTTOM_RIGHT:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case POS_TOP_RIGHT:
        case POS_BOTTOM_LEFT:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case POS_INSIDE:
            if(m_operation == POS_INSIDE)
                setCursor(Qt::ClosedHandCursor);
            else
                setCursor(Qt::OpenHandCursor);
            break;
        default:
            unsetCursor();
            break;
        }
    }

    void applyGeometryChanges(const QPoint &localPos)
    {
        const QTransform transform = this->transform();
        QRectF itemRect = transform.mapRect(m_itemRect);
        switch(m_operation)
        {
        case POS_LEFT:
        {
            itemRect.setLeft(qMin(static_cast<qreal>(localPos.x()), itemRect.right() - 1));
            break;
        }
        case POS_RIGHT:
        {
            itemRect.setRight(qMax(static_cast<qreal>(localPos.x()), itemRect.left() + 1));
            break;
        }
        case POS_TOP:
        {
            itemRect.setTop(qMin(static_cast<qreal>(localPos.y()), itemRect.bottom() - 1));
            break;
        }
        case POS_BOTTOM:
        {
            itemRect.setBottom(qMax(static_cast<qreal>(localPos.y()), itemRect.top() + 1));
            break;
        }
        case POS_TOP_LEFT:
        {
            QRectF newRect = itemRect;
            newRect.setTopLeft(QPointF(qMin(static_cast<qreal>(localPos.x()), newRect.right() - 1), qMin(static_cast<qreal>(localPos.y()), newRect.bottom() - 1)));
            QSizeF newSize = itemRect.size();
            newSize.scale(newRect.size(), Qt::KeepAspectRatio);
            newRect.setSize(newSize);
            newRect.moveBottomRight(itemRect.bottomRight());
            itemRect = newRect;
            break;
        }
        case POS_BOTTOM_RIGHT:
        {
            QRectF newRect = itemRect;
            newRect.setBottomRight(QPointF(qMax(static_cast<qreal>(localPos.x()), newRect.left() + 1), qMax(static_cast<qreal>(localPos.y()), newRect.top() + 1)));
            QSizeF newSize = itemRect.size();
            newSize.scale(newRect.size(), Qt::KeepAspectRatio);
            newRect.setSize(newSize);
            newRect.moveTopLeft(itemRect.topLeft());
            itemRect = newRect;
            break;
        }
        case POS_TOP_RIGHT:
        {
            QRectF newRect = itemRect;
            newRect.setTopRight(QPointF(qMax(static_cast<qreal>(localPos.x()), newRect.left() + 1), qMin(static_cast<qreal>(localPos.y()), newRect.bottom() - 1)));
            QSizeF newSize = itemRect.size();
            newSize.scale(newRect.size(), Qt::KeepAspectRatio);
            newRect.setSize(newSize);
            newRect.moveBottomLeft(itemRect.bottomLeft());
            itemRect = newRect;
            break;
        }
        case POS_BOTTOM_LEFT:
        {
            QRectF newRect = itemRect;
            newRect.setBottomLeft(QPointF(qMin(static_cast<qreal>(localPos.x()), newRect.right() - 1), qMax(static_cast<qreal>(localPos.y()), newRect.top() + 1)));
            QSizeF newSize = itemRect.size();
            newSize.scale(newRect.size(), Qt::KeepAspectRatio);
            newRect.setSize(newSize);
            newRect.moveTopRight(itemRect.topRight());
            itemRect = newRect;
            break;
        }
        case POS_INSIDE:
        {
            itemRect.moveCenter(localPos - m_startOffset);
            break;
        }
        default:
            break;
        }
        Q_EMIT geometryChangeRequested(transform.inverted().mapRect(itemRect));
    }

private:
    QGraphicsItem *m_graphicsItem;
    int m_rotateAngle;
    Qt::Orientations m_flipOrientations;
    QRectF m_paperRect;
    QRectF m_pageRect;
    QRectF m_itemRect;
    Position m_operation;
    QPoint m_startOffset;
    PrintEffect m_printEffect;
    bool m_editable;
};

struct PrintDialog::UI
{
    PrintDialog * const printDialog;
    QTabWidget * const tabWidget;

    QFrame * const generalTabFrame;
    QFrame * const imageSettingsTabFrame;
    QFrame * const effectsTabFrame;

    QGroupBox * const printerSelectGroup;
    QComboBox * const printerSelectComboBox;
    QPushButton * const printDialogButton;
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
    QCheckBox * const ignorePaperBoundsCheckBox;

    QGroupBox * const previewGroup;
    PrintPreviewWidget * const previewWidget;

    QGroupBox * const effectsControlGroup;
    QLabel * const brightnessLabel;
    QSlider * const brightnessSlider;
    QSpinBox * const brightnessSpinBox;
    QLabel * const contrastLabel;
    QSlider * const contrastSlider;
    QSpinBox * const contrastSpinBox;
    QLabel * const exposureLabel;
    QSlider * const exposureSlider;
    QSpinBox * const exposureSpinBox;
    QCheckBox * const grayscaleCheckBox;
    QCheckBox * const legacyRendererCheckBox;

    QGroupBox * const effectsPreviewGroup;
    PrintPreviewWidget * const effectsPreviewWidget;

    QDialogButtonBox * const dialogButtonBox;

    explicit UI(PrintDialog *printDialog)
        : printDialog(printDialog)
        , CONSTRUCT_OBJECT(tabWidget, QTabWidget, (printDialog))
        , CONSTRUCT_OBJECT(generalTabFrame, QFrame, (printDialog))
        , CONSTRUCT_OBJECT(imageSettingsTabFrame, QFrame, (printDialog))
        , CONSTRUCT_OBJECT(effectsTabFrame, QFrame, (printDialog))
        , CONSTRUCT_OBJECT(printerSelectGroup, QGroupBox, (generalTabFrame))
        , CONSTRUCT_OBJECT(printerSelectComboBox, QComboBox, (printerSelectGroup))
        , CONSTRUCT_OBJECT(printDialogButton, QPushButton, (printerSelectGroup))
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
        , CONSTRUCT_OBJECT(ignorePaperBoundsCheckBox, QCheckBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(previewGroup, QGroupBox, (imageSettingsTabFrame))
        , CONSTRUCT_OBJECT(previewWidget, PrintPreviewWidget, (previewGroup))
        , CONSTRUCT_OBJECT(effectsControlGroup, QGroupBox, (effectsTabFrame))
        , CONSTRUCT_OBJECT(brightnessLabel, QLabel, (effectsControlGroup))
        , CONSTRUCT_OBJECT(brightnessSlider, QSlider, (effectsControlGroup))
        , CONSTRUCT_OBJECT(brightnessSpinBox, QSpinBox, (effectsControlGroup))
        , CONSTRUCT_OBJECT(contrastLabel, QLabel, (effectsControlGroup))
        , CONSTRUCT_OBJECT(contrastSlider, QSlider, (effectsControlGroup))
        , CONSTRUCT_OBJECT(contrastSpinBox, QSpinBox, (effectsControlGroup))
        , CONSTRUCT_OBJECT(exposureLabel, QLabel, (effectsControlGroup))
        , CONSTRUCT_OBJECT(exposureSlider, QSlider, (effectsControlGroup))
        , CONSTRUCT_OBJECT(exposureSpinBox, QSpinBox, (effectsControlGroup))
        , CONSTRUCT_OBJECT(grayscaleCheckBox, QCheckBox, (effectsControlGroup))
        , CONSTRUCT_OBJECT(legacyRendererCheckBox, QCheckBox, (effectsControlGroup))
        , CONSTRUCT_OBJECT(effectsPreviewGroup, QGroupBox, (effectsTabFrame))
        , CONSTRUCT_OBJECT(effectsPreviewWidget, PrintPreviewWidget, (effectsPreviewGroup))
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

        QGridLayout *printerSelectLayout = new QGridLayout(printerSelectGroup);
        printerSelectLayout->addWidget(printerSelectComboBox, 0, 0, Qt::AlignVCenter);
        printerSelectLayout->addWidget(printDialogButton, 0, 1, Qt::AlignVCenter);
        printerSelectLayout->addLayout(printerInfoLayout, 1, 0, 1, 2);
        printerSelectLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 0);

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
        previewWidget->setEditable(true);

        QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
        previewLayout->addWidget(previewWidget);
        previewLayout->addStretch();

        QGridLayout *imageSettingsTabLayout = new QGridLayout(imageSettingsTabFrame);
        imageSettingsTabLayout->addWidget(sizeGroup, 0, 0, 1, 2, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(positionGroup, 1, 0, 1, 2, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(ignorePageMarginsCheckBox, 2, 0, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(ignorePaperBoundsCheckBox, 2, 1, Qt::AlignTop);
        imageSettingsTabLayout->addWidget(previewGroup, 0, 2, 2, 1);

        brightnessSlider->setOrientation(Qt::Horizontal);
        contrastSlider->setOrientation(Qt::Horizontal);
        exposureSlider->setOrientation(Qt::Horizontal);

        QGridLayout *effectsControlLayout = new QGridLayout(effectsControlGroup);
        effectsControlLayout->addWidget(brightnessLabel, 0, 0, 1, 2);
        effectsControlLayout->addWidget(brightnessSlider, 1, 0, 1, 1);
        effectsControlLayout->addWidget(brightnessSpinBox, 1, 1, 1, 1);
        effectsControlLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 2, 0, 1, 2);
        effectsControlLayout->addWidget(contrastLabel, 3, 0, 1, 2);
        effectsControlLayout->addWidget(contrastSlider, 4, 0, 1, 1);
        effectsControlLayout->addWidget(contrastSpinBox, 4, 1, 1, 1);
        effectsControlLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 5, 0, 1, 2);
        effectsControlLayout->addWidget(exposureLabel, 6, 0, 1, 2);
        effectsControlLayout->addWidget(exposureSlider, 7, 0, 1, 1);
        effectsControlLayout->addWidget(exposureSpinBox, 7, 1, 1, 1);
        effectsControlLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 8, 0, 1, 2);
        effectsControlLayout->addWidget(grayscaleCheckBox, 9, 0, 1, 2);
        effectsControlLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 10, 0, 1, 2);
        effectsControlLayout->addWidget(legacyRendererCheckBox, 11, 0, 1, 2);
        effectsControlLayout->addItem(new QSpacerItem(64, 1, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1, 8, 1);

        effectsPreviewWidget->setFixedSize(200, 200);

        QVBoxLayout *effectsPreviewLayout = new QVBoxLayout(effectsPreviewGroup);
        effectsPreviewLayout->addWidget(effectsPreviewWidget);
        effectsPreviewLayout->addStretch();

        QHBoxLayout *effectsTabLayout = new QHBoxLayout(effectsTabFrame);
        effectsTabLayout->addWidget(effectsControlGroup);
        effectsTabLayout->addWidget(effectsPreviewGroup);

        tabWidget->addTab(generalTabFrame, QString());
        tabWidget->addTab(imageSettingsTabFrame, QString());
        tabWidget->addTab(effectsTabFrame, QString());

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
        printDialogButton->setText(qApp->translate("PrintDialog", "\xe2\x80\xa6"));
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
        autoRotateCheckBox->setText(qApp->translate("PrintDialog", "Auto-Rotate"));
        pageSetupButton->setText(qApp->translate("PrintDialog", "Page Setup"));

        miscGroup->setTitle(qApp->translate("PrintDialog", "Misc"));
        copiesLabel->setText(qApp->translate("PrintDialog", "Copies:"));
        colorModeLabel->setText(qApp->translate("PrintDialog", "Color Mode:"));

        sizeGroup->setTitle(qApp->translate("PrintDialog", "Size"));
        widthLabel->setText(qApp->translate("PrintDialog", "Width:"));
        heightLabel->setText(qApp->translate("PrintDialog", "Height:"));
        xResolutionLabel->setText(qApp->translate("PrintDialog", "X Resolution:"));
        yResolutionLabel->setText(qApp->translate("PrintDialog", "Y Resolution:"));
        loadDefaultsButton->setText(qApp->translate("PrintDialog", "Load Defaults"));
        if(sizeGroup->layoutDirection() == Qt::RightToLeft)
        {
            keepAspectLabelTop->setText(QString::fromLatin1("<b>&#9581;</b>"));
            keepAspectLabelBottom->setText(QString::fromLatin1("<b>&#9584;</b>"));
        }
        else
        {
            keepAspectLabelTop->setText(QString::fromLatin1("<b>&#9582;</b>"));
            keepAspectLabelBottom->setText(QString::fromLatin1("<b>&#9583;</b>"));
        }

        positionGroup->setTitle(qApp->translate("PrintDialog", "Position"));
        leftLabel->setText(qApp->translate("PrintDialog", "Left:"));
        rightLabel->setText(qApp->translate("PrintDialog", "Right:"));
        topLabel->setText(qApp->translate("PrintDialog", "Top:"));
        bottomLabel->setText(qApp->translate("PrintDialog", "Bottom:"));
        centerLabel->setText(qApp->translate("PrintDialog", "Center:"));

        ignorePageMarginsCheckBox->setText(qApp->translate("PrintDialog", "Ignore Page Margins"));
        ignorePaperBoundsCheckBox->setText(qApp->translate("PrintDialog", "Ignore Paper Bounds"));

        previewGroup->setTitle(qApp->translate("PrintDialog", "Preview"));

        effectsControlGroup->setTitle(qApp->translate("PrintDialog", "Effects", "Effects"));
        brightnessLabel->setText(qApp->translate("PrintDialog", "Brightness:", "Effects"));
        contrastLabel->setText(qApp->translate("PrintDialog", "Contrast:", "Effects"));
        exposureLabel->setText(qApp->translate("PrintDialog", "Exposure:", "Effects"));
        grayscaleCheckBox->setText(qApp->translate("PrintDialog", "Grayscale", "Effects"));
        legacyRendererCheckBox->setText(qApp->translate("PrintDialog", "Use Legacy Rendering Algorithm", "Effects"));

        effectsPreviewGroup->setTitle(qApp->translate("PrintDialog", "Preview", "Effects"));

        tabWidget->setTabText(tabWidget->indexOf(generalTabFrame), qApp->translate("PrintDialog", "General"));
        tabWidget->setTabText(tabWidget->indexOf(imageSettingsTabFrame), qApp->translate("PrintDialog", "Image Settings"));
        tabWidget->setTabText(tabWidget->indexOf(effectsTabFrame), qApp->translate("PrintDialog", "Effects"));

        dialogButtonBox->button(QDialogButtonBox::Ok)->setText(qApp->translate("PrintDialog", "Print"));
        dialogButtonBox->button(QDialogButtonBox::Cancel)->setText(qApp->translate("PrintDialog", "Cancel"));
    }
};

#endif // PRINT_DIALOG_P_H_INCLUDED
