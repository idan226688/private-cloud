#include <sys/select.h> //select
#include <iostream> //cerr
#include <unistd.h> // inotify

#include "dir_monitor.hpp" // dir monitor header

DirMonitor::DirMonitor(const std::string &dirPath) : m_dirPath(dirPath), m_publisher(new Publisher<std::string>)
{
    m_init_fd = inotify_init();

    if (m_init_fd == -1)
    {
        throw std::runtime_error("inotify_init failed");
    }

    m_dir_fd = inotify_add_watch(m_init_fd, dirPath.c_str(), IN_CREATE | IN_DELETE | IN_MODIFY);

    if (m_dir_fd == -1)
    {
        close(m_init_fd);
        throw std::runtime_error("inotify_add_watch failed");
    }
}

void DirMonitor::RunMonitor()
{
    m_stopFlag = false;
    m_monitorThread = std::thread(&DirMonitor::RunMonitorThread, this);
}

void DirMonitor::Subscribe(DllLoader *loader)
{
    m_publisher->Subscribe(loader);
}

DirMonitor::~DirMonitor() noexcept
{
    m_stopFlag = true;
    inotify_rm_watch(m_init_fd, m_dir_fd);

    if (m_monitorThread.joinable())
    {
        m_monitorThread.join();
    }

    delete m_publisher;
}

void DirMonitor::RunMonitorThread()
{
    fd_set fds;
    int ret;

    while (false == m_stopFlag)
    {
        FD_ZERO(&fds);
        FD_SET(m_init_fd, &fds);

        ret = select(m_init_fd + 1, &fds, nullptr, nullptr, nullptr);

        if (ret < 0)
        {
            std::cerr << "select error" << std::endl;
            break;
        }

        if (FD_ISSET(m_init_fd, &fds))
        {
            ssize_t length = read(m_init_fd, buffer, sizeof(buffer));
            
            if (length < 0)
            {
                std::cerr << "read error" << std::endl;
                break;
            }

            char* ptr = buffer;
            struct inotify_event* event = (struct inotify_event*)ptr;

            for (; ptr < buffer + length; ptr += sizeof(struct inotify_event) + event->len)
            {
                if (0 < event->len)
                {
                    std::string message;

                    if (event->mask & IN_CREATE)
                    {
                        message += "Created: " + m_dirPath + "/" + std::string(event->name);
                    }
                    else if (event->mask & IN_DELETE)
                    {
                        message += "Deleted: " + m_dirPath + "/" + std::string(event->name);
                    }
                    else if (event->mask & IN_MODIFY)
                    {
                        message += "Modified: " + m_dirPath + "/" + std::string(event->name);
                    }
                    m_publisher->Notify(message);
                }
            }
        }
    }
}