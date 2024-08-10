/*
   Copyright (C) 2017-2024 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include "StringUtils.h"

#include <QString>

namespace StringUtils {

bool NumericLessThan(const QString &s1, const QString &s2)
{
    const QString sl1 = s1.toLower(), sl2 = s2.toLower();
    QString::ConstIterator it1 = sl1.constBegin(), it2 = sl2.constBegin();
    for(; it1 != sl1.constEnd() && it2 != sl2.constEnd(); ++it1, ++it2)
    {
        QChar c1 = *it1, c2 = *it2;
        if(c1.isNumber() && c2.isNumber())
        {
            QString num1, num2;
            while(c1.isNumber())
            {
                num1.append(c1);
                if((++it1) == sl1.constEnd())
                    break;
                c1 = *it1;
            }
            while(c2.isNumber())
            {
                num2.append(c2);
                if((++it2) == sl2.constEnd())
                    break;
                c2 = *it2;
            }
            if(num1.length() < num2.length())
                return true;
            if(num1.length() > num2.length())
                return false;
            if(num1 != num2)
                return num1 < num2;
            if(it1 == sl1.constEnd() || it2 == sl2.constEnd())
                break;
            --it1;
            --it2;
        }
        else if(c1 != c2)
        {
            return c1 < c2;
        }
    }
    if(it1 == sl1.constEnd() && it2 != sl2.constEnd())
        return true;
    if(it1 != sl1.constEnd() && it2 == sl2.constEnd())
        return false;
    return s1 < s2;
}

} // namespace StringUtils

