/*
   Copyright (C) 2011-2025 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_WORKAROUNDS_H_INCLUDED)
#define QTUTILS_WORKAROUNDS_H_INCLUDED

#include <QString>

namespace Workarounds {

/// @brief Initialize resources of static QtUtils library
void InitQtUtilsResources();

/// @brief Apply locale specific font hacks for Windows and Wine
/// @param[in] language - current language
/// @note Function should be called on GUI language changes
void FontsFix(const QString &language);

/// @brief Automatically scale GUI on high DPI screens
/// @note Function should be called before creating QApplication instance
/// @attention Does nothing before Qt 5.4 because scaling is not supported
/// @attention Does nothing since Qt 6.0 because scaling is already active
void HighDPIFix();

/// @brief Check if app is running with X11 forwarding over SSH
/// @return true - X11 forwarding over SSH is active, false - otherwise
bool IsRemoteSession();

/// @brief Override unsupported QT_QPA_PLATFORMTHEME and QT_STYLE_OVERRIDE
void StyleFix();

/// @brief Override all environment variables by applying values from their
/// prefixed versions
/// @param[in] prefix - app specific prefix
/// @details Some users systems may have some global environment variables like
/// QT_SCALE_FACTOR_ROUNDING_POLICY=Floor for another buggy apps. Thus, launch
/// of application with scalable interface will become impossible without local
/// environment variables, what can cause difficulties on some systems like
/// Windows. With this function, such users can set another app specific global
/// environment variable APPPREFIX_QT_SCALE_FACTOR_ROUNDING_POLICY=PassThrough
/// and get scalable interface or achieve another necessary results
void ApplyEnvVarOverrides(const char *prefix);

} // Workarounds

#endif // QTUTILS_WORKAROUNDS_H_INCLUDED

