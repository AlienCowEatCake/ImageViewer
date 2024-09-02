#if !defined(ILM_QT_THREAD_H_INCLUDED)
#define ILM_QT_THREAD_H_INCLUDED

#include <QThread>

#include "IlmBaseConfig.h"
#include "IlmThreadNamespace.h"

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_ENTER

bool supportsThreads();

class Thread : public QThread
{
    Q_DISABLE_COPY(Thread)

public:
    Thread();
    ~Thread();
};

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_EXIT

#endif // ILM_QT_THREAD_H_INCLUDED
