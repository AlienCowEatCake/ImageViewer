/*
   Copyright (C) 2018-2026 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_GLOBAL_H_INCLUDED)
#define QTUTILS_GLOBAL_H_INCLUDED

#include <cstddef>

#include <QtGlobal>

#if (QT_VERSION < QT_VERSION_CHECK(4, 6, 0))

static inline bool qFuzzyIsNull(double d)
{
    return qAbs(d) <= 0.000000000001;
}

static inline bool qFuzzyIsNull(float f)
{
    return qAbs(f) <= 0.00001f;
}

#endif

#if !defined (QT_HAS_ATTRIBUTE)
#   if defined (__has_attribute)
#       define QT_HAS_ATTRIBUTE(x) __has_attribute(x)
#   else
#       define QT_HAS_ATTRIBUTE(x) 0
#   endif
#endif

#if !defined (QT_HAS_CPP_ATTRIBUTE)
#   if defined (__has_cpp_attribute)
#       define QT_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#   else
#       define QT_HAS_CPP_ATTRIBUTE(x) 0
#   endif
#endif

#if !defined (QT_HAS_INCLUDE)
#   if defined (__has_include)
#       define QT_HAS_INCLUDE(x) __has_include(x)
#   else
#       define QT_HAS_INCLUDE(x) 0
#   endif
#endif

#if !defined (QT_HAS_INCLUDE_NEXT)
#   if defined (__has_include_next)
#       define QT_HAS_INCLUDE_NEXT(x) __has_include_next(x)
#   else
#       define QT_HAS_INCLUDE_NEXT(x) 0
#   endif
#endif

#if !defined (Q_FALLTHROUGH)
#   if defined (__cplusplus)
#       if QT_HAS_CPP_ATTRIBUTE(clang::fallthrough)
#           define Q_FALLTHROUGH() [[clang::fallthrough]]
#       elif QT_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#           define Q_FALLTHROUGH() [[gnu::fallthrough]]
#       elif QT_HAS_CPP_ATTRIBUTE(fallthrough)
#           define Q_FALLTHROUGH() [[fallthrough]]
#       endif
#   endif
#   if !defined (Q_FALLTHROUGH)
#       if (defined (Q_CC_GNU) && ((Q_CC_GNU + 0) >= 700)) && !defined (Q_CC_INTEL)
#           define Q_FALLTHROUGH() __attribute__((fallthrough))
#       else
#           define Q_FALLTHROUGH() (void)0
#       endif
#   endif
#endif

#if !defined (Q_DECL_OVERRIDE)
#   if defined (__cplusplus) && (__cplusplus >= 201103L)
#       define Q_DECL_OVERRIDE override
#   else
#       define Q_DECL_OVERRIDE
#   endif
#endif

#if !defined (Q_DECL_FINAL)
#   if defined (__cplusplus) && (__cplusplus >= 201103L)
#       define Q_DECL_FINAL final
#   else
#       define Q_DECL_FINAL
#   endif
#endif

#if !defined (Q_NULLPTR)
#   if defined (__cplusplus) && (__cplusplus >= 201103L)
#       define Q_NULLPTR nullptr
#   else
#       define Q_NULLPTR NULL
#   endif
#endif

#if !defined (Q_LIKELY)
#   if defined (__GNUC__)
#       define Q_LIKELY(x) __builtin_expect(!!(x), true)
#   else
#       define Q_LIKELY(x) (x)
#   endif
#endif

#if !defined (Q_UNLIKELY)
#   if defined (__GNUC__)
#       define Q_UNLIKELY(x) __builtin_expect(!!(x), false)
#   else
#       define Q_UNLIKELY(x) (x)
#   endif
#endif


#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define Qt_SkipEmptyParts (Qt::SkipEmptyParts)
#else
#define Qt_SkipEmptyParts (QString::SkipEmptyParts)
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#define QImage_rgbSwap(image) ((image).rgbSwap())
#else
#define QImage_rgbSwap(image) ((image) = (image).rgbSwapped())
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
#define QImage_convertTo(image, format) ((image).convertTo((format)))
#else
#define QImage_convertTo(image, format) ((image) = (image).convertToFormat((format)))
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))
#define QImage_flipped(image, orient) ((image).flipped((orient)))
#else
#define QImage_flipped(image, orient) ((image).mirrored(((orient) & Qt::Horizontal) != 0, ((orient) & Qt::Vertical) != 0))
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6, 9, 0))
#define QImage_flip(image, orient) ((image).flip((orient)))
#elif (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#define QImage_flip(image, orient) ((image).mirror(((orient) & Qt::Horizontal) != 0, ((orient) & Qt::Vertical) != 0))
#else
#define QImage_flip(image, orient) ((image) = QImage_flipped((image), (orient)))
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 2, 0))
#define QFileInfo_exists(file) (QFileInfo::exists((file)))
#else
#define QFileInfo_exists(file) (QFileInfo((file)).exists())
#endif

#endif // QTUTILS_GLOBAL_H_INCLUDED

