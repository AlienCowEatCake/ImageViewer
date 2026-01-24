/*
   Copyright (C) 2017-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#include <cassert>

#include <QLocale>
#include <QString>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
#include <QCollator>
#endif

#include "Global.h"

#if defined (Q_OS_WIN)
#include <Windows.h>
#endif

namespace StringUtils {

#if defined (Q_OS_WIN)
namespace {

class ShlwapiHelper
{
    Q_DISABLE_COPY(ShlwapiHelper)

private:
    typedef int(WINAPI *StrCmpLogicalW_t)(PCWSTR, PCWSTR);

public:
    ShlwapiHelper()
        : m_hShlwapi(LoadLibraryA("shlwapi.dll"))
        , m_StrCmpLogicalW_f(m_hShlwapi ? reinterpret_cast<StrCmpLogicalW_t>(GetProcAddress(m_hShlwapi, "StrCmpLogicalW")) : Q_NULLPTR)
    {}

    ~ShlwapiHelper()
    {
        if(m_hShlwapi)
            FreeLibrary(m_hShlwapi);
    }

    bool canCompareNumeric() const
    {
        return m_StrCmpLogicalW_f;
    }

    int doCompareNumeric(const QString &s1, const QString &s2) const
    {
        PCWSTR psz1 = reinterpret_cast<PCWSTR>(s1.constData());
        PCWSTR psz2 = reinterpret_cast<PCWSTR>(s2.constData());
        return callStrCmpLogicalW(psz1, psz2);
    }

private:
    int callStrCmpLogicalW(PCWSTR psz1, PCWSTR psz2) const
    {
        assert(canCompareNumeric());
        return m_StrCmpLogicalW_f(psz1, psz2);
    }

private:
    HMODULE m_hShlwapi;
    StrCmpLogicalW_t m_StrCmpLogicalW_f;
};

} // namespace
#endif

#if !defined (Q_OS_MAC) && (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
namespace {

class QCollatorHelper
{
    Q_DISABLE_COPY(QCollatorHelper)

public:
    QCollatorHelper()
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
        : m_collator(QLocale::system().collation())
#else
        : m_collator(QLocale::system())
#endif
    {
        m_collator.setNumericMode(true);
        m_collator.setCaseSensitivity(Qt::CaseInsensitive);
        m_canCompareNumeric = compareNumericImpl(QString::number(9), QString::number(10)) < 0;
    }

    bool canCompareNumeric() const
    {
        return m_canCompareNumeric;
    }

    int doCompareNumeric(const QString &s1, const QString &s2) const
    {
        assert(canCompareNumeric());
        return compareNumericImpl(s1, s2);
    }

private:
    int compareNumericImpl(const QString &s1, const QString &s2) const
    {
        return m_collator.compare(s1, s2);
    }

private:
    QCollator m_collator;
    bool m_canCompareNumeric;
};

} // namespace
#endif

#if !defined (Q_OS_MAC)
bool PlatformNumericLessThan(const QString &s1, const QString &s2)
{
#if defined (Q_OS_WIN)
    static const ShlwapiHelper shlwapiHelper;
    if(shlwapiHelper.canCompareNumeric())
        return shlwapiHelper.doCompareNumeric(s1, s2) < 0;
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
    static const QCollatorHelper qcollatorHelper;
    if(qcollatorHelper.canCompareNumeric())
        return qcollatorHelper.doCompareNumeric(s1, s2) < 0;
#endif
    return NumericLessThan(s1, s2, true);
}
#endif

bool NumericLessThan(const QString &s1, const QString &s2, bool localeDependent)
{
    QString sl1 = s1.normalized(QString::NormalizationForm_C);
    QString sl2 = s2.normalized(QString::NormalizationForm_C);
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    if(localeDependent)
    {
        QLocale locale = QLocale::system();
        sl1 = locale.toLower(sl1);
        sl2 = locale.toLower(sl2);
    }
    else
#endif
    {
        sl1 = sl1.toLower();
        sl2 = sl2.toLower();
    }
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
    if(localeDependent)
        return QString::localeAwareCompare(s1, s2) < 0;
    return s1 < s2;
}

} // namespace StringUtils

