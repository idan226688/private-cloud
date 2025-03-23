#ifndef __FRAMEWORK_HPP__
#define __FRAMEWORK_HPP__


#include <map>                      // std::map
#include <functional>               // std::function

#define __INIT_SYMBOL              // handleton "permit"
#include "singleton.hpp"            // ilrd::Singleton<T>
#include "thread_pool.hpp"          // ilrd::ThreadPool, ilrd::ThreadPool::IThreadTask
#include "factory.hpp"              // ilrd::Factory
#include "logger.hpp"               // ilrd::Logger
#include "reactor.hpp"              // ilrd::Reactor
#include "dir_monitor.hpp"          // ilrd::DirMonitor
#include "framework_interfaces.hpp" // ICommand, IInputProxy, ITaskData

template <class KEY>
class Framework
{
public:
    using CallbackMap = std::map<std::pair<int, Reactor::Mode>, IInputProxy<KEY>*>;

    using CreateFunc = std::function<ICommand* (ITaskData<KEY>*)>;

    using CreatorsMap = std::map<KEY, CreateFunc>;

    Framework(const std::string& pluginsFolderPath, const CallbackMap& eventsCallbacks, const CreatorsMap& creators);
    ~Framework() noexcept;
    Framework& operator=(const Framework& other) = delete;
    Framework(const Framework& other) = delete;

    void Start();

private:
    ThreadPool* m_threadpool;
    Reactor* m_reactor;
    DirMonitor m_pluginsMonitor;
    Factory<KEY, ICommand, ITaskData<KEY>*>* m_factory;
    Logger* m_logger;

    class TPTask : public ThreadPool::ITask
    {
    public:
        explicit TPTask(ITaskData<KEY>* taskData);
        void Run() override;
    private:
        ITaskData<KEY>* m_taskData;
    };

}; // class Framework



template<class KEY>
inline Framework<KEY>::Framework(const std::string& pluginsFolderPath, const CallbackMap& eventsCallbacks, const CreatorsMap& creators)
    : m_threadpool(Singleton<ThreadPool>::GetInstance()),
    m_reactor(Singleton<Reactor>::GetInstance()),
    m_pluginsMonitor(pluginsFolderPath),
    m_factory(Singleton<Factory<KEY, ICommand, ITaskData<KEY>*>>::GetInstance()),
    m_logger(Singleton<Logger, std::string>::GetInstance("log.txt"))
{
    //add callbacks to reactor
    for (const auto& callback : eventsCallbacks)
    {
        int fd = callback.first.first;
        Reactor::Mode mode = callback.first.second;
        IInputProxy<KEY>* proxy = callback.second; // KEY = CommandType, when using my concrete

        m_reactor->Add(fd, mode,
            [proxy, this] 
            {
                auto taskData = proxy->GetTaskData();
                std::shared_ptr<TPTask> task(new TPTask(const_cast<ITaskData<KEY>*>(taskData)));
                m_threadpool->Add(task);
            }
        );
    }

    //add creators functions to factory
    for (const auto& creator : creators)
    {
        m_factory->Add(creator.first, creator.second);
    }


    // m_pluginsMonitor.Subscribe(DllLoader);

}

template<class KEY>
inline Framework<KEY>::~Framework() noexcept
{
    m_logger->Log(Logger::INFO, "Framework shuts down...");

}

template<class KEY>
inline void Framework<KEY>::Start()
{
    m_logger->Log(Logger::INFO, "Framework is starting...");

    m_reactor->Run();
}

template<class KEY>
inline Framework<KEY>::TPTask::TPTask(ITaskData<KEY>* taskData)
    : m_taskData(taskData)
{
}

template<class KEY>
inline void Framework<KEY>::TPTask::Run()
{
    try
    {
        auto factory = Singleton<Factory<KEY, ICommand, ITaskData<KEY>*>>::GetInstance();
        auto command = factory->Create(m_taskData->GetKey(), m_taskData);
        command->Run();
    }
    catch (const std::exception& e)
    {
        Logger* logger = Singleton<Logger, std::string>::GetInstance("log.txt");
        logger->Log(Logger::ERROR, std::string("Failed to create command for key. ") + e.what());
    }


}


#endif //__FRAMEWORK_HPP__
