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
#define LOG_WARNING qWarning

class LoggingContextHelper
{
public:
    LoggingContextHelper(const char *const file, const char *const line, const char *const func);
    inline const char *toString() const { return buffer; }

    static const char *cutFileName(const char *const file);
    static const char *cutFuncName(const char *const func);

private:
    const char *const file;
    const char *const line;
    const char *const func;
    char buffer[256];
};

#define LOGGING_STRINGIZE_NX(X) #X
#define LOGGING_STRINGIZE(X) LOGGING_STRINGIZE_NX(X)

#if defined (__FILE_NAME__)
#define LOGGING_FILE (__FILE_NAME__)
#elif defined (__FILE__)
#define LOGGING_FILE (LoggingContextHelper::cutFileName(__FILE__))
#else
#define LOGGING_FILE ""
#endif

#if defined (__LINE__)
#define LOGGING_LINE LOGGING_STRINGIZE(__LINE__)
#else
#define LOGGING_LINE ""
#endif

//#define LOGGING_FUNC (LoggingContextHelper::cutFuncName(__func__))
#define LOGGING_FUNC (LoggingContextHelper::cutFuncName(__FUNCTION__))
//#define LOGGING_FUNC (__PRETTY_FUNCTION__)
//#define LOGGING_FUNC (__FUNCSIG__)

#define LOGGING_CTXS(S)     (LoggingContextHelper(LOGGING_FILE, LOGGING_LINE, (S)).toString())
#define LOGGING_CTXQS(QS)   (LOGGING_CTXQS((QS).toLocal8Bit().data()))
#define LOGGING_CTX         (LOGGING_CTXS(LOGGING_FUNC))

#endif // QTUTILS_LOGGING_H_INCLUDED

