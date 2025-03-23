#include <iostream>
#include "async_injection.hpp" // AsyncInjection


using CLOCK = std::chrono::system_clock;


AsyncInjection::AsyncInjection(const ACTION& action, std::chrono::milliseconds interval)
    : m_action(std::move(action)), m_interval(interval)
{
    auto scheduler = Singleton<Scheduler>::GetInstance();

    AsyncTask task(this);
    scheduler->AddTask(std::make_shared<AsyncTask>(task));
}

AsyncInjection::AsyncTask::AsyncTask(AsyncInjection* async)
    : ISchedulerTask(CLOCK::now() + async->m_interval), m_asyncInjection(async)
{
    //std::cout << "AsyncTask created, time to run: " << this->m_start_time.time_since_epoch().count() << '\n';
}

void AsyncInjection::AsyncTask::Run()
{
    bool res = m_asyncInjection->m_action();
    //std::cout << "running asynctask address: "<< this << '\n';
    if (res)
    {
        delete m_asyncInjection;
    }
    else
    {
        auto scheduler = Singleton<Scheduler>::GetInstance();
        auto task = std::shared_ptr<AsyncTask>(new AsyncTask(m_asyncInjection));
        scheduler->AddTask(task);
    }
}
