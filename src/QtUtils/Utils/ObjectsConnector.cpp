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

#include "ObjectsConnector.h"
#include <cassert>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <QObject>
#include <QPointer>

namespace ObjectsConnector {

typedef std::pair<QPointer<QObject>, std::string>   ObjectInfo;
typedef std::list<ObjectInfo>                       ObjectInfoList;
typedef std::map<std::string, ObjectInfoList>       ObjectInfoListMap;

namespace {

ObjectInfoListMap emitters;
ObjectInfoListMap receivers;

} // namespace

/// @brief Регистрирация объекта, испускающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] emitter - объект, испускающий сигнал
/// @param[in] signal - сигнал, испускаемый объектом
void RegisterEmitter(const char *id, QObject *emitter, const char *signal)
{
    assert(emitter);
    assert(id && std::string(id).length() > 0);
    assert(signal && signal[0] == '2');

    const std::string id_str = std::string(id);
    const std::string signal_str = std::string(signal);

    emitters[id_str].push_back(std::make_pair(QPointer<QObject>(emitter), signal_str));

    ObjectInfoListMap::iterator list = receivers.find(id_str);
    if(list == receivers.end())
        return;

    for(ObjectInfoList::iterator it = list->second.begin(); it != list->second.end(); ++it)
    {
        if(!it->first.isNull())
            QObject::connect(emitter, signal, it->first.data(), it->second.c_str());
    }
}

/// @brief Регистрирация объекта, принимающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] receiver - объект, принимающий сигнал
/// @param[in] slot - слот, в который должен прилететь сигнал
void RegisterReceiver(const char *id, QObject *receiver, const char *slot)
{
    assert(receiver);
    assert(id && std::string(id).length() > 0);
    assert(slot && slot[0] == '1');

    const std::string id_str = std::string(id);
    const std::string slot_str = std::string(slot);

    receivers[id_str].push_back(std::make_pair(QPointer<QObject>(receiver), slot_str));

    ObjectInfoListMap::iterator list = emitters.find(id_str);
    if(list == emitters.end())
        return;

    for(ObjectInfoList::iterator it = list->second.begin(); it != list->second.end(); ++it)
    {
        if(!it->first.isNull())
            QObject::connect(it->first.data(), it->second.c_str(), receiver, slot);
    }
}

/// @brief Разрегистрирация объекта, испускающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] emitter - объект, испускающий сигнал
/// @param[in] signal - сигнал, испускаемый объектом
void UnregisterEmitter(const char *id, QObject *emitter, const char *signal)
{
    assert(emitter);
    assert(id && std::string(id).length() > 0);
    assert(signal && signal[0] == '2');

    const std::string id_str = std::string(id);
    const std::string signal_str = std::string(signal);

    const ObjectInfo objectInfo = std::make_pair(QPointer<QObject>(emitter), signal_str);
    assert(emitters.find(id_str) != emitters.end());
    assert(std::find(emitters[id_str].begin(), emitters[id_str].end(), objectInfo) != emitters[id_str].end());
    emitters[id_str].erase(std::find(emitters[id_str].begin(), emitters[id_str].end(), objectInfo));

    ObjectInfoListMap::iterator list = receivers.find(id_str);
    if(list == receivers.end())
        return;

    for(ObjectInfoList::iterator it = list->second.begin(); it != list->second.end(); ++it)
    {
        if(!it->first.isNull())
            QObject::disconnect(emitter, signal, it->first.data(), it->second.c_str());
    }
}

/// @brief Разрегистрирация объекта, принимающего сигнал
/// @param[in] id - идентификатор сигнала
/// @param[in] receiver - объект, принимающий сигнал
/// @param[in] slot - слот, в который должен прилететь сигнал
void UnregisterReceiver(const char *id, QObject *receiver, const char *slot)
{
    assert(receiver);
    assert(id && std::string(id).length() > 0);
    assert(slot && slot[0] == '1');

    const std::string id_str = std::string(id);
    const std::string slot_str = std::string(slot);

    const ObjectInfo objectInfo = std::make_pair(QPointer<QObject>(receiver), slot_str);
    assert(receivers.find(id_str) != receivers.end());
    assert(std::find(receivers[id_str].begin(), receivers[id_str].end(), objectInfo) != receivers[id_str].end());
    receivers[id_str].erase(std::find(receivers[id_str].begin(), receivers[id_str].end(), objectInfo));

    ObjectInfoListMap::iterator list = emitters.find(id_str);
    if(list == emitters.end())
        return;

    for(ObjectInfoList::iterator it = list->second.begin(); it != list->second.end(); ++it)
    {
        if(!it->first.isNull())
            QObject::disconnect(it->first.data(), it->second.c_str(), receiver, slot);
    }
}

} // namespace ObjectsConnector

