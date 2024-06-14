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

#if !defined (WA_SIGN_COMPARE_IGNORED_GCC) && defined (__GNUC__) && !defined (__clang__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#define WA_SIGN_COMPARE_IGNORED_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#if !defined (WA_SIGN_COMPARE_IGNORED_CLANG) && defined (__clang__) && defined (__has_warning)
#if __has_warning ("-Wsign-compare")
#define WA_SIGN_COMPARE_IGNORED_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#endif
#endif

