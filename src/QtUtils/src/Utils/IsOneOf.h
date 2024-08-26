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

#if !defined (QTUTILS_ISONEOF_H_INCLUDED)
#define QTUTILS_ISONEOF_H_INCLUDED

/**
 * @note Variadic parameter pack is not available before C++11. So we
 * can use this python3 script to generate required number of functions:
```
count = 32
for i in range(1, count, 1):
    str = 'template <typename T0'
    for j in range(1, i + 2, 1):
        str = str + ', typename T{0}'.format(j)
    str = str + '>'
    print(str)
    str = 'inline bool IsOneOf(const T0 &arg0'
    for j in range(1, i + 2, 1):
        str = str + ', const T{0} &arg{0}'.format(j)
    str = str + ')'
    print(str)
    print('{')
    str = '    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0'
    for j in range(2, i + 2, 1):
        str = str + ', T{0}'.format(j)
    str = str + '>(arg0'
    for j in range(2, i + 2, 1):
        str = str + ', arg{0}'.format(j)
    str = str + ');'
    print(str)
    print('}')
    print('')
```
 **/

template <typename T0, typename T1>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1)
{
    return arg0 == arg1;
}

#define ADD_SIGNED_UNSIGNED_COMPARE(UTYPE, STYPE, CTYPE) \
template<> \
inline bool IsOneOf(const UTYPE &arg0, const STYPE &arg1) \
{ \
    return arg1 >= 0 && IsOneOf<CTYPE, CTYPE>(static_cast<CTYPE>(arg0), static_cast<CTYPE>(arg1)); \
} \
template<> \
inline bool IsOneOf(const STYPE &arg0, const UTYPE &arg1) \
{ \
    return arg0 >= 0 && IsOneOf<CTYPE, CTYPE>(static_cast<CTYPE>(arg0), static_cast<CTYPE>(arg1)); \
}
ADD_SIGNED_UNSIGNED_COMPARE(unsigned char, char, unsigned char)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned char, signed char, unsigned char)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned char, signed short, signed short)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned char, signed int, signed int)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned char, signed long, signed long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned char, signed long long, signed long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned short, char, unsigned short)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned short, signed char, unsigned short)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned short, signed short, unsigned short)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned short, signed int, signed int)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned short, signed long, signed long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned short, signed long long, signed long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned int, char, unsigned int)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned int, signed char, unsigned int)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned int, signed short, unsigned int)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned int, signed int, unsigned int)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned int, signed long, signed long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned int, signed long long, signed long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long, char, unsigned long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long, signed char, unsigned long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long, signed short, unsigned long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long, signed int, unsigned long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long, signed long, unsigned long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long, signed long long, signed long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long long, char, unsigned long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long long, signed char, unsigned long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long long, signed short, unsigned long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long long, signed int, unsigned long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long long, signed long, unsigned long long)
ADD_SIGNED_UNSIGNED_COMPARE(unsigned long long, signed long long, unsigned long long)
#undef ADD_SIGNED_UNSIGNED_COMPARE

template <typename T0, typename T1, typename T2>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2>(arg0, arg2);
}

template <typename T0, typename T1, typename T2, typename T3>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3>(arg0, arg2, arg3);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4>(arg0, arg2, arg3, arg4);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5>(arg0, arg2, arg3, arg4, arg5);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6>(arg0, arg2, arg3, arg4, arg5, arg6);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7>(arg0, arg2, arg3, arg4, arg5, arg6, arg7);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26, const T27 &arg27)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26, arg27);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26, const T27 &arg27, const T28 &arg28)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26, arg27, arg28);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26, const T27 &arg27, const T28 &arg28, const T29 &arg29)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26, arg27, arg28, arg29);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26, const T27 &arg27, const T28 &arg28, const T29 &arg29, const T30 &arg30)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26, arg27, arg28, arg29, arg30);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26, const T27 &arg27, const T28 &arg28, const T29 &arg29, const T30 &arg30, const T31 &arg31)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26, arg27, arg28, arg29, arg30, arg31);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14, typename T15, typename T16, typename T17, typename T18, typename T19, typename T20, typename T21, typename T22, typename T23, typename T24, typename T25, typename T26, typename T27, typename T28, typename T29, typename T30, typename T31, typename T32>
inline bool IsOneOf(const T0 &arg0, const T1 &arg1, const T2 &arg2, const T3 &arg3, const T4 &arg4, const T5 &arg5, const T6 &arg6, const T7 &arg7, const T8 &arg8, const T9 &arg9, const T10 &arg10, const T11 &arg11, const T12 &arg12, const T13 &arg13, const T14 &arg14, const T15 &arg15, const T16 &arg16, const T17 &arg17, const T18 &arg18, const T19 &arg19, const T20 &arg20, const T21 &arg21, const T22 &arg22, const T23 &arg23, const T24 &arg24, const T25 &arg25, const T26 &arg26, const T27 &arg27, const T28 &arg28, const T29 &arg29, const T30 &arg30, const T31 &arg31, const T32 &arg32)
{
    return IsOneOf<T0, T1>(arg0, arg1) || IsOneOf<T0, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31, T32>(arg0, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24, arg25, arg26, arg27, arg28, arg29, arg30, arg31, arg32);
}

#endif // QTUTILS_ISONEOF_H_INCLUDED

