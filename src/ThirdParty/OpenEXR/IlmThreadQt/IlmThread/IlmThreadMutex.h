#if !defined(ILM_QT_THREAD_MUTEX_H_INCLUDED)
#define ILM_QT_THREAD_MUTEX_H_INCLUDED

#include <QMutex>

#include "IlmBaseConfig.h"
#include "IlmThreadNamespace.h"

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_ENTER

class Mutex : public QMutex
{
    Q_DISABLE_COPY(Mutex)

public:
    Mutex();
    ~Mutex();
};

class Lock
{
    Q_DISABLE_COPY(Lock)

public:
    Lock(const Mutex &m, bool autoLock = true);
    ~Lock();

    void acquire();
    void release();
    bool locked() const;

private:
    Mutex &m_mutex;
    bool m_locked;
};

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_EXIT

#endif // ILM_QT_THREAD_MUTEX_H_INCLUDED
