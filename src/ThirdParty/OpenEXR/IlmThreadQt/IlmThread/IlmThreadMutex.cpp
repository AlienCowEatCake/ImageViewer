#include "IlmThreadMutex.h"

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_ENTER

Mutex::Mutex()
{}

Mutex::~Mutex()
{}

Lock::Lock(const Mutex &m, bool autoLock)
    : m_mutex(const_cast<Mutex&>(m))
    , m_locked(false)
{
    if(autoLock)
        acquire();
}

Lock::~Lock()
{
    if(m_locked)
        m_mutex.unlock();
}

void Lock::acquire()
{
    m_mutex.lock();
    m_locked = true;
}

void Lock::release()
{
    m_mutex.unlock();
    m_locked = false;
}

bool Lock::locked() const
{
    return m_locked;
}

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_EXIT
