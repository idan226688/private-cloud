#include <iostream>
#include <thread>
#include <unistd.h>  
#include <cassert>
#include "thread_pool.hpp"

std::mutex vector_mutex;

class SimpleTask : public ThreadPool::ITask
{
public:
    SimpleTask(int& counter) : m_counter(counter) {}
    void Run() override
    {
        ++m_counter;
    }

private:
    int& m_counter;
};

void TestBasicTaskExecution()
{
    int counter = 0;
    ThreadPool pool(4);

    auto task = std::make_shared<SimpleTask>(counter);
    pool.Add(task, ThreadPool::MED);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(counter == 1);

    std::cout << "TestBasicTaskExecution passed." << std::endl;
}

void TestMultipleTaskExecution()
{
    int counter = 0;
    ThreadPool pool(4);

    auto task = std::make_shared<SimpleTask>(counter);
    for (int i = 0; i < 10; ++i)
    {
        pool.Add(task, ThreadPool::MED);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    assert(counter == 10);

    std::cout << "TestMultipleTaskExecution passed." << std::endl;
}

class PriorityTask : public ThreadPool::ITask
{
public:
    PriorityTask(int id, std::vector<int>& log) : m_id(id), m_log(log) {}
    void Run() override
    {
        std::unique_lock<std::mutex> lock(vector_mutex);

        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "Task " << m_id << " is running." << std::endl;
        m_log.push_back(m_id);
    }

private:
    int m_id;
    std::vector<int>& m_log;
};

void TestPriorityExecution()
{
    std::vector<int> log;
    ThreadPool pool(4);

    pool.Add(std::make_shared<PriorityTask>(2, log), ThreadPool::HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.Add(std::make_shared<PriorityTask>(3, log), ThreadPool::MED);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pool.Add(std::make_shared<PriorityTask>(1, log), ThreadPool::LOW);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    assert(log.size() == 3);
    assert(log[0] == 2);
    assert(log[1] == 3);
    assert(log[2] == 1);

    std::cout << "TestPriorityExecution passed." << std::endl;
}

void TestPauseResume()
{
    int counter = 0;
    ThreadPool pool(4);

    auto task = std::make_shared<SimpleTask>(counter);
    pool.Pause();
    pool.Add(task, ThreadPool::MED);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    assert(counter == 0);

    pool.Resume();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(counter == 1);

    std::cout << "TestPauseResume passed." << std::endl;
}


class CountThreads : public ThreadPool::ITask
{
public:
    CountThreads(std::atomic<int>& counter, std::unordered_map<std::thread::id, std::thread::id>& threads, std::mutex& mutex_count_threads)
        : m_counter(counter), m_threads(threads), m_mutex_count_threads(mutex_count_threads) {}
    void Run() override
    {
        std::unique_lock<std::mutex> lock(m_mutex_count_threads);
        {
            if (m_threads.end() == m_threads.find(std::this_thread::get_id()))
            {
                m_counter++;

                m_threads.insert({ std::this_thread::get_id(), std::this_thread::get_id() });
            }
        }
        for (volatile int i = 0; i < 10000; ++i) {}
    }

private:
    std::atomic<int>& m_counter;
    std::unordered_map<std::thread::id, std::thread::id>& m_threads;
    std::mutex& m_mutex_count_threads;
};

void TestDynamicThreadCountAdjustment()
{
    std::unordered_map<std::thread::id, std::thread::id>threads;
    std::atomic<int> counter(0);
    std::mutex mutex_count_threads;
    ThreadPool pool(10);

    auto task = std::make_shared<CountThreads>(counter, threads, mutex_count_threads);
    for (int i = 0; i < 100; ++i)
    {
        pool.Add(task, ThreadPool::Priority::MED);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    assert(counter.load() == 10);

    counter.store(0);
    threads.clear();

    // Decrease the number of threads
    pool.SetNumOfThreads(5);
    std::cout << "------------------" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (int i = 0; i < 100; ++i)
    {
        pool.Add(task, ThreadPool::Priority::MED);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    assert(counter.load() == 5);

    counter.store(0);
    threads.clear();

    // Increase the number of threads
    pool.SetNumOfThreads(15);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (int i = 0; i < 100; ++i)
    {
        pool.Add(task, ThreadPool::Priority::MED);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    assert(counter.load() == 15);
    

    std::cout << "TestDynamicThreadCountAdjustment passed." << std::endl;
}

void TestStop()
{
    int counter = 0;
    ThreadPool pool(4);

    auto task = std::make_shared<SimpleTask>(counter);
    pool.Add(task, ThreadPool::MED);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    pool.Stop();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    assert(counter == 1); // Only one task should be executed before stopping

    std::cout << "TestStop passed." << std::endl;
}

int main()
{
    TestBasicTaskExecution();
    TestMultipleTaskExecution();
    TestPriorityExecution();
    TestPauseResume();
    TestDynamicThreadCountAdjustment();
    TestStop();
    return 0;
}

