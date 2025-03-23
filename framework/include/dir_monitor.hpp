#ifndef __DIRMONITOR_HPP__
#define __DIRMONITOR_HPP__

#include <sys/inotify.h> // inotify
#include <string> //std::string
#include <atomic> //atomic vars
#include <thread> //thread

#include "publisher.hpp" // publisher header
#include "dll_loader.hpp" //dll loader

class DirMonitor 
{
public:
        explicit DirMonitor(const std::string& dirPath);
        void RunMonitor();
        void Subscribe(DllLoader* loader); 
        ~DirMonitor() noexcept;

private:
        void RunMonitorThread();
        std::atomic<bool> m_stopFlag;
        std::thread m_monitorThread;
        int m_init_fd;
        int m_dir_fd;
        const std::string& m_dirPath;
        char buffer[4096];
        Publisher<std::string>* m_publisher;
}; //Class DirMonitor


#endif //__DIRMONITOR_HPP__