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

#include "AboutDialog.h"
#include "AboutDialog_p.h"

#include <QApplication>

#include "Decoders/DecodersManager.h"

namespace {

const char *ABOUT_PIXMAP_PATH     = ":/icon/icon_64.png";
const char *ABOUT_URL_STRING      = "https://fami.codefreak.ru/gitlab/peter/ImageViewer";
const char *ABOUT_LICENSE_STRIG   = "<a href=\"http://www.gnu.org/copyleft/gpl.html\">GNU GPL v3</a>";
const char *ABOUT_YEAR_STRING     = "2017";
const char *ABOUT_AUTHOR_STRING   = QT_TRANSLATE_NOOP("MainWindow", "Peter S. Zhigalov");
const char *ABOUT_EMAIL_STRING    = "peter.zhigalov@gmail.com";

QString getTextBrowserContent()
{
    QString result;

    QStringList availableDecoders = DecodersManager::getInstance().registeredDecoders();
    for(QStringList::Iterator it = availableDecoders.begin(); it != availableDecoders.end(); ++it)
    {
        const QString decoderPrefix = QString::fromLatin1("Decoder");
        if(it->startsWith(decoderPrefix))
            *it = it->mid(decoderPrefix.length()).toLower();
        else
            *it = it->toLower();
    }
    availableDecoders.sort();

    result.append(QString::fromLatin1("<b>%1</b> %2")
            .arg(qApp->translate("MainWindow", "Available image decoders:"))
            .arg(availableDecoders.join(QString::fromLatin1(", "))));
    return result;
}

} // namespace

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , m_ui(new UI(this))
{
    connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(close()));

    m_ui->iconLabel->setPixmap(qApp->translate("MainWindow", ABOUT_PIXMAP_PATH));
    setWindowTitle(qApp->translate("MainWindow", "About"));
    m_ui->titleLabel->setText(QString::fromLatin1("<h3>%1 v%2</h3>").arg(qApp->applicationName()).arg(qApp->applicationVersion()));
    m_ui->textLabel->setText(QString::fromLatin1(
                                 "<a href=\"%1\">%1</a><br>"
                                 "%2: %3<br><br>"
                                 "Copyright &copy; %4<br>"
                                 "%5 &lt;<a href=\"mailto:%6\">%6</a>&gt;"
                                 )
                             .arg(qApp->translate("MainWindow", ABOUT_URL_STRING))
                             .arg(qApp->translate("MainWindow", "License"))
                             .arg(qApp->translate("MainWindow", ABOUT_LICENSE_STRIG))
                             .arg(qApp->translate("MainWindow", ABOUT_YEAR_STRING))
                             .arg(qApp->translate("MainWindow", ABOUT_AUTHOR_STRING))
                             .arg(qApp->translate("MainWindow", ABOUT_EMAIL_STRING))
                             );
    m_ui->textBrowser->setHtml(getTextBrowserContent());
}

AboutDialog::~AboutDialog()
{}

void AboutDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    m_ui->textBrowser->setFixedWidth(width() - 2 * LAYOUT_MARGINS);
}
