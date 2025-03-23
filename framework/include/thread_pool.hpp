#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__

#include <thread> //thread
#include <unordered_map> //map
#include <atomic> //atomic vars
#include <condition_variable> //conditional variables
#include <mutex> //mutex
#include "waitable_queue.hpp" // waitable queue
#include "priority_queue.hpp" // priority queue
#include <functional> // std::function
#include "singleton.hpp" // singleton

class ThreadPool
{
public:
    enum Priority
    {
        LOW,
        MED,
        HIGH
    };
    
    ~ThreadPool() noexcept;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    ThreadPool(ThreadPool&& other) = default;
    ThreadPool& operator=(ThreadPool&& other) = default;
    
    void SetNumOfThreads(size_t num_threads); //throw(std::system_error); 

    class ITask
    {
    public:
        virtual void Run() = 0;
        virtual ~ITask() = default;
    }; // Class ITask
    
    void Add(std::shared_ptr<ITask> task, Priority priority = LOW); //throw(std::system_error)
    void Pause() noexcept;
    void Resume() noexcept;
    void Stop() noexcept;

    class FunctionTask : public ITask
    {
    public:
        explicit FunctionTask(std::function<void()> func) noexcept : m_func(func){}
        void Run() override {m_func();}

    private:
        std::function<void()> m_func;
    }; // // Class FunctionTask

private:
    friend class Singleton<ThreadPool>;
    friend class Singleton<ThreadPool, size_t>;

    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
    class pause_task : public ThreadPool::ITask
    {
        public:
            pause_task(ThreadPool& pool) : m_pool(pool) {}
            void Run();

        private:
            ThreadPool& m_pool;    

    }; // Class pause_task

    class stop_thread : public ThreadPool::ITask
    {
        public:
            stop_thread(ThreadPool& pool) : m_pool(pool) {}
            void Run();

        private:
            ThreadPool& m_pool;    

    }; // Class stop_thread

    typedef std::pair<std::shared_ptr<ITask>, Priority> TPTask_t;
    class PriorityCompare
    {
    public:
        bool operator()(const TPTask_t& lhs, const TPTask_t& rhs)
        {
            return (lhs.second < rhs.second);
        }
    }; // Class PriorityCompare

    std::unordered_map<std::thread::id, std::thread> m_threads; 

    WaitableQueue<TPTask_t, PriorityQueue<TPTask_t, std::vector<TPTask_t>, PriorityCompare>> m_task_queue;

    size_t m_num_threads;
    std::atomic<bool> m_stopFlag;
    std::atomic<bool> m_paused;
    std::condition_variable m_pauseCondition;
    std::mutex m_pauseMutex;

    static void WorkerFunc(ThreadPool* pool);
}; // Class ThreadPool

#endif //__THREAD_POOL_HPP__
