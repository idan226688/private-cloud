#ifndef __ASYNC_INJECTION_HPP__
#define __ASYNC_INJECTION_HPP__

#include <functional>               // std::function
#include <chrono>                   // std::chrono::milliseconds
#include "framework_interfaces.hpp" // ICommand
#include "scheduler.hpp"            // ilrd::Scheduler

class AsyncInjection
{

public:
    using ACTION = std::function<bool()>;

    AsyncInjection(const ACTION& action, std::chrono::milliseconds interval);

    AsyncInjection(const AsyncInjection& other) = delete;
    AsyncInjection& operator=(const AsyncInjection& other) = delete;
    AsyncInjection(AsyncInjection&& other) noexcept = delete;
    AsyncInjection& operator=(AsyncInjection&& other) noexcept = delete;

private:
    class AsyncTask : public Scheduler::ISchedulerTask
    {
    public:
        AsyncTask(AsyncInjection* async);
        void Run();

    private:
        AsyncInjection* m_asyncInjection;
    }; // class AsyncTask


    ~AsyncInjection() = default;

    const ACTION m_action;
    std::chrono::milliseconds m_interval;

}; // class AsyncInjection

#endif //__ASYNC_INJECTION_HPP__
