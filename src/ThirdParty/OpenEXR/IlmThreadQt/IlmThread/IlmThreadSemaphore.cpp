#include "IlmThreadSemaphore.h"

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_ENTER

Semaphore::Semaphore(unsigned int value)
    : m_semaphore(static_cast<int>(value))
{}

Semaphore::~Semaphore()
{}

void Semaphore::wait()
{
    m_semaphore.acquire();
}

bool Semaphore::tryWait()
{
    return m_semaphore.tryAcquire();
}

void Semaphore::post()
{
    m_semaphore.release();
}

int Semaphore::value() const
{
    return m_semaphore.available();
}

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_EXIT
