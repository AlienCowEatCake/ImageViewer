#if !defined(ILM_QT_THREAD_POOL_H_INCLUDED)
#define ILM_QT_THREAD_POOL_H_INCLUDED

#include <QMutex>
#include <QRunnable>
#include <QThreadPool>
#include <QWaitCondition>

#include "IlmThreadNamespace.h"

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_ENTER

class Task;
class ThreadPool;

class TaskGroup
{
    Q_DISABLE_COPY(TaskGroup)
    friend class Task;

public:
    TaskGroup();
    ~TaskGroup();

private:
    void incrementCounter();
    void decrementCounter();

private:
    int m_counter;
    QMutex m_mutex;
    QWaitCondition m_cond;
};

class Task
{
    Q_DISABLE_COPY(Task)
    friend class ThreadPool;

public:
    explicit Task(TaskGroup *g);
    virtual ~Task();

    virtual void execute() = 0;

    TaskGroup *group();

private:
    QRunnable *runnable();

private:
    class TaskImpl : public QRunnable
    {
    public:
        explicit TaskImpl(Task *task);
        ~TaskImpl();

        void run();

    private:
        Task *m_task;
    };
    TaskImpl *m_runnable;
    TaskGroup *m_group;
};

class ThreadPool
{
    Q_DISABLE_COPY(ThreadPool)

public:
    explicit ThreadPool(unsigned numThreads = 0);
    explicit ThreadPool(QThreadPool *pool);
    virtual ~ThreadPool();

    int numThreads() const;
    void setNumThreads(int count);
    void addTask(Task *task);
    static ThreadPool &globalThreadPool();
    static void addGlobalTask(Task *task);

protected:
    QThreadPool *m_threadPool;
    bool m_isExternal;
};

ILMTHREAD_INTERNAL_NAMESPACE_HEADER_EXIT

#endif // ILM_QT_THREAD_POOL_H_INCLUDED
