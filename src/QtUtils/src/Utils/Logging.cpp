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

#include "Logging.h"

#include <cassert>

LoggingContextHelper::LoggingContextHelper(const char * const file, const char * const line, const char * const func)
{
    // Memory allocation is prohibited here
    const size_t bufferSize = sizeof(buffer);
    size_t i = 0;
    buffer[i++] = '[';
    for(const char *c = file; *c != '\0' && i < bufferSize; ++c)
        buffer[i++] = *c;
    if(i < bufferSize)
        buffer[i++] = ':';
    for(const char *c = line; *c != '\0' && i < bufferSize; ++c)
        buffer[i++] = *c;
    if(i < bufferSize)
        buffer[i++] = ':';
    for(const char *c = func; *c != '\0' && i < bufferSize; ++c)
        buffer[i++] = *c;
    if(i < bufferSize)
        buffer[i++] = ']';
    if(i < bufferSize)
        buffer[i++] = '\0';
    assert(i < bufferSize);
    buffer[bufferSize - 1] = '\0';
}

const char *LoggingContextHelper::cutFileName(const char *const file)
{
    // Memory allocation is prohibited here
    const char *fileName = file;
    for(const char *c = file; c[0] != '\0'; ++c)
        if(c[0] == '/' || c[0] == '\\')
            fileName = c + 1;
    return fileName;
}

const char *LoggingContextHelper::cutFuncName(const char *const func)
{
    // Memory allocation is prohibited here
    const char *funcName = func;
    for(const char *c = func; c[0] != '\0' && c[1] != '\0'; ++c)
        if(c[0] == ':' && c[1] == ':')
            funcName = c + 2;
    return funcName;
}

