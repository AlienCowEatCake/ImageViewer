/*
   Copyright (C) 2011-2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

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

#if !defined (QTUTILS_OBJECTSCONNECTOR_H_INCLUDED)
#define QTUTILS_OBJECTSCONNECTOR_H_INCLUDED

class QObject;

namespace ObjectsConnector {

/// @brief Регистрирация объекта, испускающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] emitter - объект, испускающий сигнал
/// @param[in] signal - сигнал, испускаемый объектом
void RegisterEmitter(const char *id, QObject *emitter, const char *signal);

/// @brief Регистрирация объекта, принимающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] receiver - объект, принимающий сигнал
/// @param[in] slot - слот, в который должен прилететь сигнал
void RegisterReceiver(const char *id, QObject *receiver, const char *slot);

/// @brief Разрегистрирация объекта, испускающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] emitter - объект, испускающий сигнал
/// @param[in] signal - сигнал, испускаемый объектом
void UnregisterEmitter(const char *id, QObject *emitter, const char *signal);

/// @brief Разрегистрирация объекта, принимающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] receiver - объект, принимающий сигнал
/// @param[in] slot - слот, в который должен прилететь сигнал
void UnregisterReceiver(const char *id, QObject *receiver, const char *slot);

} // namespace ObjectsConnector

#endif // QTUTILS_OBJECTSCONNECTOR_H_INCLUDED

