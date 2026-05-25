#if !defined(ILM_QT_THREAD_SEMAPHORE_H_INCLUDED)
#define ILM_QT_THREAD_SEMAPHORE_H_INCLUDED

#include <QSemaphore>

#include "IlmBaseConfig.h"
#include "IlmThreadNamespace.h"

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_ENTER

class Semaphore
{
   Q_DISABLE_COPY(Semaphore)

public:
   explicit Semaphore(unsigned int value = 0);
   virtual ~Semaphore();

   void wait();
   bool tryWait();
   void post();
   int value() const;

private:
   QSemaphore m_semaphore;
};

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_EXIT

#endif // ILM_QT_THREAD_SEMAPHORE_H_INCLUDED
