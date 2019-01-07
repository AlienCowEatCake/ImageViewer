/*
   Copyright (C) 2019 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (WA_SHIFT_NEGATIVE_IGNORED_GCC) && defined (__GNUC__) && (__GNUC__ >= 6)
#define WA_SHIFT_NEGATIVE_IGNORED_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-negative-value"
#endif

#if !defined (WA_SHIFT_NEGATIVE_IGNORED_CLANG) && defined (__clang__) && ( \
    (defined (__apple_build_version__) && (__clang_major__ >= 7)) || \
    (!defined (__apple_build_version__) && ((__clang_major__ > 3) || (__clang_major__ == 3 && __clang_minor__ >= 7))) )
#define WA_SHIFT_NEGATIVE_IGNORED_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-negative-value"
#endif

