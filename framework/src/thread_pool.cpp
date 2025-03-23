#include "thread_pool.hpp"
#include <iostream>

namespace thread_local_storage
{
    thread_local bool is_stopped = false;
}

ThreadPool::ThreadPool(size_t num_threads) : m_num_threads(num_threads), m_stopFlag(false), m_paused(false)
{
    if(num_threads == 0)
    {
        num_threads = 16;
    }
    
    for (size_t i = 0; i < num_threads; ++i)
    {
        std::thread new_thread(WorkerFunc, this);
        m_threads.insert({new_thread.get_id(), std::move(new_thread)});
    }
}

ThreadPool::~ThreadPool() noexcept
{
    m_stopFlag = true;
    Resume();
    std::shared_ptr<ThreadPool::ITask> st = std::make_shared<stop_thread>(*this);

    for (size_t i = 0; i < m_num_threads; ++i)
    {
        Add(st, Priority(LOW - 1));
    }

    for (auto& thread_pair : m_threads)
    {
        if (thread_pair.second.joinable())
        {
            thread_pair.second.join();
        }
    }
}

void ThreadPool::SetNumOfThreads(size_t num_threads)
{
    if(num_threads > m_num_threads)
    {
        for (size_t i = 0; i < num_threads - m_num_threads; ++i)
        {
            std::thread new_thread(WorkerFunc, this);
            m_threads.insert({new_thread.get_id(), std::move(new_thread)});
        }
    }

    else
    {
        std::shared_ptr<ThreadPool::ITask> st = std::make_shared<stop_thread>(*this);
        for (size_t i = 0; i < m_num_threads - num_threads; ++i)
        {
            Add(st, Priority(HIGH + 1));
        }
    }

    m_num_threads = num_threads;
}

void ThreadPool::WorkerFunc(ThreadPool *pool)
{
    while (false == pool->m_stopFlag && false == thread_local_storage::is_stopped)
    {
        TPTask_t curr_task;
        pool->m_task_queue.Pop(curr_task);

        if (curr_task.first != NULL)
        {
            try
            {
                curr_task.first->Run();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }
}

void ThreadPool::Add(std::shared_ptr<ITask> task, Priority priority)
{
    TPTask_t to_add = {task, priority};
    m_task_queue.Push(to_add);
}

void ThreadPool::Pause() noexcept
{    
    m_paused = true;
    std::shared_ptr<ThreadPool::ITask> pt = std::make_shared<pause_task>(*this);
    for (size_t i = 0; i < m_num_threads; ++i)
    {
        Add(pt, Priority(HIGH + 1));
    }
}
void ThreadPool::Resume() noexcept
{
    {
        std::lock_guard<std::mutex> lock(m_pauseMutex);
        m_paused = false;
    }

    m_pauseCondition.notify_all();
}

void ThreadPool::Stop() noexcept
{
    m_stopFlag = true;
    m_paused = false;
    m_pauseCondition.notify_all();
    std::shared_ptr<ThreadPool::ITask> st = std::make_shared<stop_thread>(*this);

    for (size_t i = 0; i < m_num_threads; ++i)
    {
        Add(st, Priority(HIGH + 2));
    }

    for (auto& thread_pair : m_threads)
    {
        if (thread_pair.second.joinable())
        {
            thread_pair.second.join();
        }
    }
}
void ThreadPool::stop_thread::Run()
{
    thread_local_storage::is_stopped = true;
}
void ThreadPool::pause_task::Run()
{
        std::unique_lock<std::mutex> lock(m_pool.m_pauseMutex);
        m_pool.m_pauseCondition.wait(lock, [&]() { return !m_pool.m_paused.load() || m_pool.m_stopFlag.load(); });
}