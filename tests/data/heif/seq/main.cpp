/*
  heif-enc -o seq_rep_inf.heif -S --fps=10 --repetitions infinite --hevc -L $(seq 0 20 | sed 's|$|.png|' | xargs)
  heif-enc -o seq_rep_1.heif -S --fps=10 --repetitions 1 --hevc -L $(seq 0 20 | sed 's|$|.png|' | xargs)
  heif-enc -o seq_rep_3.heif -S --fps=10 --repetitions 3 --hevc -L $(seq 0 20 | sed 's|$|.png|' | xargs)
*/

#include <QGuiApplication>
#include <QImage>
#include <QImageWriter>
#include <QMetaEnum>
#include <QPainter>

QImage drawSampleImage(int index)
{
    QImage img(68, 64, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setPen(Qt::NoPen);
    switch (index)
    {
    case 20:
        p.setBrush(QColor(127, 127, 127, 191));
        p.drawEllipse(QRect(52, 39, 13, 13));
    case 19:
        p.setBrush(QColor(127, 127, 127, 63));
        p.drawEllipse(QRect(39, 26, 13, 13));
    case 18:
        p.setBrush(QColor(0, 0, 255, 127));
        p.drawEllipse(QRect(52, 26, 13, 13));
    case 17:
        p.setBrush(QColor(0, 255, 0, 127));
        p.drawEllipse(QRect(52, 13, 13, 13));
    case 16:
        p.setBrush(QColor(255, 0, 0, 127));
        p.drawEllipse(QRect(52, 0, 13, 13));
    case 15:
        p.setBrush(QColor(63, 63, 63, 127));
        p.drawEllipse(QRect(39, 39, 13, 13));
    case 14:
        p.setBrush(QColor(127, 127, 127, 127));
        p.drawEllipse(QRect(26, 39, 13, 13));
    case 13:
        p.setBrush(QColor(191, 191, 191, 127));
        p.drawEllipse(QRect(13, 39, 13, 13));
    case 12:
        p.setBrush(QColor(255, 255, 255, 127));
        p.drawEllipse(QRect(0, 39, 13, 13));
    case 11:
        p.setBrush(Qt::darkGray);
        p.drawEllipse(QRect(26, 26, 13, 13));
    case 10:
        p.setBrush(Qt::gray);
        p.drawEllipse(QRect(13, 26, 13, 13));
    case 9:
        p.setBrush(Qt::lightGray);
        p.drawEllipse(QRect(0, 26, 13, 13));
    case 8:
        p.setBrush(Qt::white);
        p.drawEllipse(QRect(39, 13, 13, 13));
    case 7:
        p.setBrush(Qt::blue);
        p.drawEllipse(QRect(26, 13, 13, 13));
    case 6:
        p.setBrush(Qt::green);
        p.drawEllipse(QRect(13, 13, 13, 13));
    case 5:
        p.setBrush(Qt::red);
        p.drawEllipse(QRect(0, 13, 13, 13));
    case 4:
        p.setBrush(Qt::black);
        p.drawEllipse(QRect(39, 0, 13, 13));
    case 3:
        p.setBrush(Qt::yellow);
        p.drawEllipse(QRect(26, 0, 13, 13));
    case 2:
        p.setBrush(Qt::magenta);
        p.drawEllipse(QRect(13, 0, 13, 13));
    case 1:
        p.setBrush(Qt::cyan);
        p.drawEllipse(QRect(0, 0, 13, 13));
    default:
        break;
    }
    return img;
}

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    for(int i = 0; i <= 20; ++i)
    {
        const QString name = QString::fromLatin1("%1.png").arg(i);
        const QImage img = drawSampleImage(i);
        QImageWriter w;
        w.setFileName(name);
        w.write(img);
    }

    return 0;
}
