#include <QGuiApplication>
#include <QImage>
#include <QImageWriter>
#include <QMetaEnum>
#include <QPainter>

QImage drawSampleImage(QImage::Format format)
{
    QImage img(65, 53, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::cyan);
    p.drawEllipse(QRect(0, 0, 13, 13));
    p.setBrush(Qt::magenta);
    p.drawEllipse(QRect(13, 0, 13, 13));
    p.setBrush(Qt::yellow);
    p.drawEllipse(QRect(26, 0, 13, 13));
    p.setBrush(Qt::black);
    p.drawEllipse(QRect(39, 0, 13, 13));
    p.setBrush(Qt::red);
    p.drawEllipse(QRect(0, 13, 13, 13));
    p.setBrush(Qt::green);
    p.drawEllipse(QRect(13, 13, 13, 13));
    p.setBrush(Qt::blue);
    p.drawEllipse(QRect(26, 13, 13, 13));
    p.setBrush(Qt::white);
    p.drawEllipse(QRect(39, 13, 13, 13));
    p.setBrush(Qt::lightGray);
    p.drawEllipse(QRect(0, 26, 13, 13));
    p.setBrush(Qt::gray);
    p.drawEllipse(QRect(13, 26, 13, 13));
    p.setBrush(Qt::darkGray);
    p.drawEllipse(QRect(26, 26, 13, 13));
    p.setBrush(QColor(255, 255, 255, 127));
    p.drawEllipse(QRect(0, 39, 13, 13));
    p.setBrush(QColor(191, 191, 191, 127));
    p.drawEllipse(QRect(13, 39, 13, 13));
    p.setBrush(QColor(127, 127, 127, 127));
    p.drawEllipse(QRect(26, 39, 13, 13));
    p.setBrush(QColor(63, 63, 63, 127));
    p.drawEllipse(QRect(39, 39, 13, 13));
    p.setBrush(QColor(255, 0, 0, 127));
    p.drawEllipse(QRect(52, 0, 13, 13));
    p.setBrush(QColor(0, 255, 0, 127));
    p.drawEllipse(QRect(52, 13, 13, 13));
    p.setBrush(QColor(0, 0, 255, 127));
    p.drawEllipse(QRect(52, 26, 13, 13));
    p.setBrush(QColor(127, 127, 127, 63));
    p.drawEllipse(QRect(39, 26, 13, 13));
    p.setBrush(QColor(127, 127, 127, 191));
    p.drawEllipse(QRect(52, 39, 13, 13));
    return img.convertToFormat(format);
}

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    const QList<QImage::Format> formats = QList<QImage::Format>()
                                          << QImage::Format_Mono
                                          << QImage::Format_MonoLSB
                                          << QImage::Format_Indexed8
                                          << QImage::Format_RGB32
                                          << QImage::Format_ARGB32
                                          << QImage::Format_ARGB32_Premultiplied
                                          << QImage::Format_RGB16
                                          << QImage::Format_ARGB8565_Premultiplied
                                          << QImage::Format_RGB666
                                          << QImage::Format_ARGB6666_Premultiplied
                                          << QImage::Format_RGB555
                                          << QImage::Format_ARGB8555_Premultiplied
                                          << QImage::Format_RGB888
                                          << QImage::Format_RGB444
                                          << QImage::Format_ARGB4444_Premultiplied
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
                                          << QImage::Format_RGBX8888
                                          << QImage::Format_RGBA8888
                                          << QImage::Format_RGBA8888_Premultiplied
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                                          << QImage::Format_BGR30
                                          << QImage::Format_A2BGR30_Premultiplied
                                          << QImage::Format_RGB30
                                          << QImage::Format_A2RGB30_Premultiplied
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))
                                          << QImage::Format_Alpha8
                                          << QImage::Format_Grayscale8
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
                                          << QImage::Format_RGBX64
                                          << QImage::Format_RGBA64
                                          << QImage::Format_RGBA64_Premultiplied
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
                                          << QImage::Format_Grayscale16
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
                                          << QImage::Format_BGR888
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 2, 0))
                                          << QImage::Format_RGBX16FPx4
                                          << QImage::Format_RGBA16FPx4
                                          << QImage::Format_RGBA16FPx4_Premultiplied
                                          << QImage::Format_RGBX32FPx4
                                          << QImage::Format_RGBA32FPx4
                                          << QImage::Format_RGBA32FPx4_Premultiplied
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
                                          << QImage::Format_CMYK8888
#endif
        ;

    for(QList<QImage::Format>::ConstIterator it = formats.constBegin(); it != formats.constEnd(); ++it)
    {
        const QString nameTemplate = QString::fromLatin1("%1_%2.tiff").arg(QMetaEnum::fromType<QImage::Format>().valueToKey(*it));
        const QImage img = drawSampleImage(*it);
        QImageWriter w;
        w.setCompression(0);
        w.setFileName(nameTemplate.arg(QString::fromLatin1("NoCompression")));
        w.write(img);
        w.setCompression(1);
        w.setFileName(nameTemplate.arg(QString::fromLatin1("LzwCompression")));
        w.write(img);
    }

    return 0;
}
