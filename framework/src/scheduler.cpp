#include <cstring>
#include <iostream>

#include "scheduler.hpp"

class TimerException : public std::exception
{
public:
    explicit TimerException(const std::string& message) : m_message(message)
    {
    }

    const char *what() const noexcept
    {
        return m_message.c_str();
    }
private:
    std::string m_message;
}; // class TimerException

using CLOCK = std::chrono::system_clock;

itimerspec TimeDiff(START_TIME t1, START_TIME t2)
{
    auto duration = max(std::chrono::nanoseconds(1), t1 - t2);

    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    struct itimerspec its;
    its.it_value.tv_sec = duration_ns / 1000000000ULL;  // Convert nanoseconds to seconds
    its.it_value.tv_nsec = duration_ns % 1000000000ULL; // Remaining nanoseconds
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    return its;
}

void Scheduler::AddTask(const std::shared_ptr<ISchedulerTask> task)
{
    std::lock_guard<std::recursive_mutex> lk(m_queue_lock);

    if (nullptr == m_nextTask)
    {
        m_nextTask = task;
        UpdateTimer(m_nextTask->m_start_time);
    }
    else
    {
        if (m_nextTask->m_start_time < task->m_start_time)
        {
            m_task_queue.Push(task);
        }
        else
        {
            m_task_queue.Push(m_nextTask);
            m_nextTask = task;
            UpdateTimer(m_nextTask->m_start_time);
        }
    }
}
Scheduler::Scheduler() :m_nextTask(nullptr)
{
    TimerInit();
}

Scheduler::~Scheduler() noexcept
{
    timer_delete(m_timer);
}

void Scheduler::TimerRoutine(union sigval param)
{
    Scheduler* curr = static_cast<Scheduler*>(param.sival_ptr);

    std::lock_guard<std::recursive_mutex> lk(curr->m_queue_lock);

    if(curr->m_nextTask != nullptr)
    {
        if(curr->m_nextTask->m_start_time <= CLOCK::now())
        {
            curr->m_nextTask->Run();
            curr->m_nextTask = nullptr;
        }
        else
        {
            curr->UpdateTimer(curr->m_nextTask->m_start_time);
        }
    }

    if(!curr->m_task_queue.IsEmpty())
    {
        curr->m_task_queue.Pop(curr->m_nextTask);
        curr->UpdateTimer(curr->m_nextTask->m_start_time);
    }
    else
    {
        // if there are no more tasks, stop the timer until new tasks arrive
        curr->StopTimer();
        curr->m_nextTask = nullptr;
    }
}

void Scheduler::TimerInit()
{
    sigevent s_event;
    memset(&s_event, 0, sizeof(sigevent));
    s_event.sigev_notify = SIGEV_THREAD;
    s_event._sigev_un._sigev_thread._function = TimerRoutine;
    s_event.sigev_value.sival_ptr = this;

    if(timer_create(CLOCK_REALTIME, &s_event, &m_timer) == -1)
    {
        throw TimerException("timer_create failed!");
    }
}

void Scheduler::SetTimer(const itimerspec& timer_data)
{
    if(timer_settime(m_timer, 0, &timer_data, nullptr) == -1)
    {
        throw TimerException("timer_settime failed!");
    }
}

void Scheduler::UpdateTimer(const START_TIME& due_time)
{
    auto timer_data = TimeDiff(due_time, CLOCK::now());
    SetTimer(timer_data);
}

void Scheduler::StopTimer()
{
    SetTimer({{0, 0}, {0, 0}});
}

Scheduler::ISchedulerTask::ISchedulerTask(START_TIME start_time) :m_start_time(start_time)
{
}


