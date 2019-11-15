#include "IlmThreadPool.h"

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_ENTER

TaskGroup::TaskGroup()
    : m_counter(0)
{}

TaskGroup::~TaskGroup()
{
    m_mutex.lock();
    while(m_counter > 0)
        m_cond.wait(&m_mutex);
    m_mutex.unlock();
}

void TaskGroup::incrementCounter()
{
    m_mutex.lock();
    m_counter++;
    m_mutex.unlock();
}

void TaskGroup::decrementCounter()
{
    m_mutex.lock();
    m_counter--;
    m_mutex.unlock();
    m_cond.wakeAll();
}

Task::Task(TaskGroup *g)
    : m_runnable(new TaskImpl(this))
    , m_group(g)
{
    m_group->incrementCounter();
}

Task::~Task()
{
    m_group->decrementCounter();
}

TaskGroup *Task::group()
{
    return m_group;
}

QRunnable *Task::runnable()
{
    return m_runnable;
}

Task::TaskImpl::TaskImpl(Task *task)
    : m_task(task)
{}

Task::TaskImpl::~TaskImpl()
{
    delete m_task;
}

void Task::TaskImpl::run()
{
    m_task->execute();
}

ThreadPool::ThreadPool(unsigned numThreads)
    : m_threadPool(new QThreadPool)
    , m_isExternal(false)
{
    setNumThreads(static_cast<int>(numThreads));
}

ThreadPool::ThreadPool(QThreadPool *pool)
    : m_threadPool(pool)
    , m_isExternal(true)
{}

ThreadPool::~ThreadPool()
{
    if(m_isExternal)
        return;
    m_threadPool->waitForDone();
    m_threadPool->deleteLater();
}

int ThreadPool::numThreads() const
{
    return m_threadPool->maxThreadCount();
}

void ThreadPool::setNumThreads(int count)
{
    m_threadPool->setMaxThreadCount(count);
}

void ThreadPool::addTask(Task *task)
{
    m_threadPool->start(task->runnable());
}

ThreadPool &ThreadPool::globalThreadPool()
{
    static ThreadPool globalPool(QThreadPool::globalInstance());
    return globalPool;
}

void ThreadPool::addGlobalTask(Task *task)
{
    globalThreadPool().addTask(task);
}

ILMTHREAD_INTERNAL_NAMESPACE_SOURCE_EXIT
