// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#ifndef RW_LOCK_HPP
#define RW_LOCK_HPP

#include <QReadWriteLock>

namespace Exiv2 {
        /*!
         @brief Class to provide a Read-Write Lock
        */
        class RWLock
        {
        public:
            //! constructor (acquires the lock)
            explicit RWLock()
            {
            }

            //! constructor (releases lock)
            ~RWLock()
            {
            }

            //! acquire rw lock
            void wrlock()
            {
                rwlock_.lockForWrite();
            }

            //! test to see if the rw lock can be acquired
            bool trywrlock()
            {
                return rwlock_.tryLockForWrite();
            }

            //! acquire rd lock
            void rdlock()
            {
                rwlock_.lockForRead();
            }

            //! test to see if the rd lock can be acquired
            bool tryrdlock()
            {
                return rwlock_.tryLockForRead();
            }

            //! release rw lock
            void unlock()
            {
                rwlock_.unlock();
            }

            //! unlock rd lock
            void rdunlock() { unlock(); }

            //! unlock rw lock
            void wrunlock() { unlock(); }

        private:
            //! the lock itself
            QReadWriteLock rwlock_;
        };

        /*!
         @brief Class to provide a ScopedReadLock.
         The lock is applied by the constructor and released by the destructor.
        */
        class ScopedReadLock
        {
        public:
            //! constructor - locks the object
            explicit ScopedReadLock(RWLock &rwlock):
                rwlock_(rwlock)
            {
                rwlock_.rdlock();
            }

            //! destructor - unlocks the object used in constructor
            ~ScopedReadLock() { rwlock_.rdunlock(); }

        private:
            //! object locked by the constructor (and released by destructor)
            RWLock &rwlock_;
        };

        /*!
         @brief Class to provide a ScopedWriteLock.
         The lock is applied by the constructor and released by the destructor.
        */
        class ScopedWriteLock
        {
        public:
            //! constructor - locks the object
            explicit ScopedWriteLock(RWLock &rwlock):
                rwlock_(rwlock)
            {
                rwlock_.wrlock();
            }

            //! destructor - unlocks the object used in constructor
            ~ScopedWriteLock() { rwlock_.wrunlock(); }

        private:
            //! object locked by the constructor (and released by destructor)
            RWLock &rwlock_;
        };
}

#endif // RW_LOCK_HPP
