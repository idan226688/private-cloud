#ifndef __SCHEDULER_HPP__
#define __SCHEDULER_HPP__

#include <chrono> // std::chrono
#include <time.h> // timer_t
#include <signal.h> // sigevent
#include <mutex>  // std::mutex  

#define __INIT_SYMBOL
#include "singleton.hpp"      // singleton header
#include "waitable_queue.hpp" // waitable_queue header
#include "priority_queue.hpp" // priority_queue header

using START_TIME = std::chrono::time_point<std::chrono::system_clock>;
using namespace std::chrono_literals;

class Scheduler
{
public:
    class ISchedulerTask
    {
    public:
        ISchedulerTask(START_TIME start_time);
        virtual void Run() = 0;
        virtual ~ISchedulerTask() = default;

        START_TIME m_start_time;
    };

    void AddTask(const std::shared_ptr<ISchedulerTask> task);

    Scheduler(const Scheduler& other) = delete;
    Scheduler& operator=(const Scheduler& other) = delete;
    Scheduler(Scheduler&& other) = delete;
    Scheduler& operator=(Scheduler&& other) = delete;

private:
    timer_t m_timer;
    std::shared_ptr<ISchedulerTask> m_nextTask;
    std::recursive_mutex m_queue_lock;

    class TaskCompare
    {
    public:
        bool operator()(const std::shared_ptr<ISchedulerTask>& lhs, const std::shared_ptr<ISchedulerTask>& rhs)
        {
            return (lhs->m_start_time > rhs->m_start_time);
        }
    }; // PriorityCompare

    using TaskQueue = WaitableQueue<std::shared_ptr<ISchedulerTask>, PriorityQueue<std::shared_ptr<ISchedulerTask>, std::vector<std::shared_ptr<ISchedulerTask>>, TaskCompare>>;

    TaskQueue m_task_queue;
    friend Singleton<Scheduler>;
    Scheduler(); // throws std::bad_alloc
    ~Scheduler() noexcept;
    static void TimerRoutine(union sigval);
    void TimerInit(); // throw TimerException
    void SetTimer(const itimerspec& timer_data); // throw TimerException
    void StopTimer(); // throw TimerException
    void UpdateTimer(const START_TIME& due_time); // throw TimerException



}; // Scheduler class

#endif
