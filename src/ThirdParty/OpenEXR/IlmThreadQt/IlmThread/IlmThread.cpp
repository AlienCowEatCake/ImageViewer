#include "IlmThread.h"

#include <QString>

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_ENTER

bool supportsThreads()
{
    return true;
}

Thread::Thread()
{
    setObjectName(QString::fromLatin1("IlmThreadQt"));
}

Thread::~Thread()
{
    if(isRunning())
        wait();
}

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_EXIT
