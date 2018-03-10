/*
   Copyright (C) 2018 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "DelayCalculator.h"

namespace DelayCalculator {

int calculate(int originalDelayMs, Mode mode)
{
    switch(mode)
    {
    case MODE_CHROME:
        // https://bugs.chromium.org/p/chromium/issues/detail?id=454693
        if(originalDelayMs < 11)
            return 100;
        return originalDelayMs;
    default:
        break;
    }
    return originalDelayMs;
}

} // namespace DelayCalculator
