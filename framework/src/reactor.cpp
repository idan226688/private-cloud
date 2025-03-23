#include <iostream> //cerr
#include <unistd.h> //pipe
#include <fcntl.h> //fcntl

#include "reactor.hpp" //reactor

static void empty_func(){}

Reactor::Reactor() : m_isRunning(false) , m_max_fd(0)
{
    if (pipe(m_alert_pipe) == -1)
    {
        throw std::runtime_error("pipe creation failed");
    }

    fcntl(m_alert_pipe[0], F_GETFL, 0);

    FD_ZERO(&m_read_fds_master);
    FD_ZERO(&m_write_fds_master);
    
    m_read_handlers[m_alert_pipe[0]] = empty_func;
}

Reactor::~Reactor()
{
    close(m_alert_pipe[0]);
    close(m_alert_pipe[1]);
}

void Reactor::Add(int fd, Mode mode, const Callback &callback)
{
    if(mode == READ)
    {
        m_read_handlers[fd] = callback;
        FD_SET(fd, &m_read_fds_master);
    }
    else
    {
        m_write_handlers[fd] = callback;
        FD_SET(fd, &m_write_fds_master);
    }
    m_max_fd = std::max(m_max_fd, fd);
    write(m_alert_pipe[1], "add", 4);
    std::cout << "added fd: " << fd << std::endl;
}

void Reactor::Remove(int fd, Mode mode)
{
    if(mode == READ)
    {
        m_read_handlers.erase(fd);
        if(FD_ISSET(fd, &m_read_fds_master))
        {
            m_read_handlers[fd] = empty_func;
        }
        else
        {
            FD_CLR(fd, &m_read_fds_master);
        }
    }
    else
    {
        m_write_handlers.erase(fd);
        FD_CLR(fd, &m_write_fds_master);
        if(FD_ISSET(fd, &m_write_fds_master))
        {
            m_read_handlers[fd] = empty_func;
        }
        else
        {
            FD_CLR(fd, &m_write_fds_master);
        }
    }
    write(m_alert_pipe[1], "remove", 6);
}

void Reactor::Run()
{
    m_isRunning = true;
    fd_set read_fds;
    fd_set write_fds;

    int selected;

    FD_SET(m_alert_pipe[0], &m_read_fds_master);
    m_max_fd = std::max(m_max_fd, m_alert_pipe[0]);

    while(true == m_isRunning)
    {
        read_fds = m_read_fds_master;
        write_fds = m_write_fds_master;

        selected = select(m_max_fd + 1, &read_fds, &write_fds, nullptr, nullptr);

        if (selected < 0)
        {
            std::cerr << "select error" << std::endl;
            break;
        }
        if (FD_ISSET(m_alert_pipe[0], &read_fds))
        {
            char buffer[10];
            read(m_alert_pipe[0], buffer, sizeof(buffer));
            continue;
        }

        for (int i = 0; i <= m_max_fd; i++)
        {
            if(FD_ISSET(i, &read_fds))
            {
                std::cout << "woke up on read fd: " << i << std::endl;
                m_read_handlers[i]();
            }
            if(FD_ISSET(i, &write_fds))
            {
                std::cout << "woke up on write fd: " << i << std::endl;
                m_write_handlers[i]();
            }
        }

    }
}

void Reactor::Stop()
{
    m_isRunning = false;
    write(m_alert_pipe[1], "stop", 4);
}


