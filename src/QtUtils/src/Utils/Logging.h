/*
   Copyright (C) 2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `QtUtils' library.

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

#if !defined (QTUTILS_LOGGING_H_INCLUDED)
#define QTUTILS_LOGGING_H_INCLUDED

#include <QDebug>

#define LOG_DEBUG   qDebug
#define LOG_INFO    qDebug
#define LOG_WARNING qWarning
#define LOG_ERROR   qWarning

#if defined (__FILE_NAME__)
#define LOGGING_FILE QString::fromLatin1(__FILE_NAME__)
#elif defined (__FILE__)
#define LOGGING_FILE QString::fromLatin1(__FILE__).section(QChar::fromLatin1('/'), -1).section(QChar::fromLatin1('\\'), -1)
#else
#define LOGGING_FILE QString()
#endif

#if defined (__LINE__)
#define LOGGING_LINE QString::fromLatin1("%1").arg(__LINE__)
#else
#define LOGGING_LINE QString()
#endif

#if defined (__func__)
#define LOGGING_FUNC QString::fromLatin1(__func__)
#elif defined (__FUNCTION__)
#define LOGGING_FUNC QString::fromLatin1(__FUNCTION__)
#elif defined (__PRETTY_FUNCTION__)
#define LOGGING_FUNC QString::fromLatin1(__PRETTY_FUNCTION__)
#elif defined (__FUNCSIG__)
#define LOGGING_FUNC QString::fromLatin1(__FUNCSIG__)
#else
#define LOGGING_FUNC QString()
#endif

#define LOGGING_CTXQS(QS)   QString::fromLatin1("[%1:%2:%3]:").arg((LOGGING_FILE), (LOGGING_LINE), (QS)).toLatin1().data()
#define LOGGING_CTXS(S)     LOGGING_CTXQS(QString::fromLatin1(S))
#define LOGGING_CTX         LOGGING_CTXQS(LOGGING_FUNC)

#endif // QTUTILS_LOGGING_H_INCLUDED

