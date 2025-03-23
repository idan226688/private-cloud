#ifndef __REACTOR_HPP__
#define __REACTOR_HPP__

#include <unordered_map> // std::unordered_map
#include <functional> // std::functional
#include <atomic> // std::atomic
#include <sys/select.h> //select
#include <singleton.hpp>

namespace ilrd
{

class Reactor
{
    using Callback = std::function<void()>;
public:
    enum Mode
    {
        READ,
        WRITE
    };
    Reactor();
    ~Reactor() noexcept;
    Reactor(const Reactor& other) = delete;
    Reactor& operator=(const Reactor& other) = delete;
    Reactor(Reactor&& other) = delete;
    Reactor& operator=(Reactor&& other) = delete;
    
    void Add(int fd, Mode mode, const Callback& callback);
    void Remove(int fd, Mode mode);
    void Run();
    void Stop();
    
private:
    friend class Singleton<Reactor>;
    std::unordered_map<int, Callback> m_read_handlers;
    std::unordered_map<int, Callback> m_write_handlers;
    fd_set m_read_fds_master;
    fd_set m_write_fds_master;
    std::atomic<bool> m_isRunning;
    int m_alert_pipe[2];
    int m_max_fd;
};// class Reactor


#endif //__REACTOR_HPP__