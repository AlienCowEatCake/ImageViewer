/*
   Copyright (C) 2023 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <mach-o/dyld.h>
#include "Utils/InfoUtils.h"

namespace {

typedef uint32_t dyld_platform_t;

struct dyld_build_version_t
{
    dyld_platform_t platform;
    uint32_t version;
};

} // namespace

extern "C"
__attribute__ ((visibility ("hidden")))
bool _availability_version_check(uint32_t count, dyld_build_version_t versions[])
{
    for(uint32_t i = 0; i < count; ++i)
    {
        if(versions[i].platform != PLATFORM_MACOS)
            continue;

        const int major = (versions[i].version >> 16) & 0xff;
        const int minor = (versions[i].version >>  8) & 0xff;
        const int patch = (versions[i].version      ) & 0xff;
        return InfoUtils::MacVersionGreatOrEqual(major, minor, patch ? patch : -1);
    }
    return false;
}

