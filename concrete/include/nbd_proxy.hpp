#ifndef __NBD_PROXY__
#define __NBD_PROXY__

#include <cstring>
#include <string>
#include <fcntl.h>
#include <linux/nbd.h>
#include <netinet/in.h>
#include <cassert>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <thread>
#include <signal.h>
#include <stdexcept>
#include <memory>

#include "framework_interfaces.hpp"
#include "commands.hpp"


void* __data;
static int nbd_dev_to_disconnect = -1;

static void disconnect_nbd(int signal)
{
    if (nbd_dev_to_disconnect != -1)
    {
        if (ioctl(nbd_dev_to_disconnect, NBD_DISCONNECT) == -1)
        {
            std::cerr << "Failed to request disconnect on NBD device" << std::endl;
        }
        else
        {
            nbd_dev_to_disconnect = -1;
            std::cerr << "Successfully requested disconnect on NBD device" << std::endl;
        }
    }
}

class NBDProxy : public IInputProxy<CommandType>
{
public:
    NBDProxy(std::string path, size_t size);
    NBDProxy(std::string path, size_t blksize, size_t numBlocks);
    ~NBDProxy();

    ilrd::ITaskData<CommandType>* GetTaskData() override;
    int GetFD();

private:
    void SetNBD();
    void InitNBD();
    void SetupSignalHandling();

    int nbd_fd;
    int m_fd[2];
    std::string m_path;
    size_t m_size;
};


inline NBDProxy::NBDProxy(std::string path, size_t size)
    : m_path(path), m_size(size)
{
    SetNBD();
    SetupSignalHandling();
    
    if (ioctl(nbd_fd, NBD_SET_SIZE, size) == -1)
    {
        throw std::runtime_error("Failed to set NBD size");
    }
    __data = malloc(size);
    InitNBD();
}


inline NBDProxy::NBDProxy(std::string path, size_t blksize, size_t numBlocks)
    : m_path(path)
{
    SetNBD();

    if (ioctl(nbd_fd, NBD_SET_BLKSIZE, blksize) == -1)
    {
        throw std::runtime_error("Failed to set block size");
    }

    if (ioctl(nbd_fd, NBD_SET_SIZE_BLOCKS, numBlocks) == -1)
    {
        throw std::runtime_error("Failed to set size in blocks");
    }
    __data = malloc(blksize * numBlocks);
    InitNBD();
}


inline NBDProxy::~NBDProxy()
{
    ioctl(nbd_fd, NBD_DISCONNECT);

    close(nbd_fd);
    close(m_fd[0]);
    close(m_fd[1]);
}


inline ilrd::ITaskData<CommandType>* NBDProxy::GetTaskData()
{
    NBDData* data = new NBDData(m_fd[0]); 
    if(data == NULL)
    {
        std::cout<< "NBDData is NULL" << std::endl;
    }
    return data;
}


inline int NBDProxy::GetFD()
{
    return m_fd[0];
}


inline void NBDProxy::SetNBD()
{
    nbd_fd = open(m_path.c_str(), O_RDWR);
    std::cout<< "nbd_fd = " <<nbd_fd << std::endl;
    if (-1 == nbd_fd)
    {
        throw std::runtime_error("Failed to open NBD device");
    }
    
    int flags = fcntl(nbd_fd, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::runtime_error("Failed to get file descriptor flags");
    }

    if (fcntl(nbd_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        throw std::runtime_error("Failed to set file descriptor to non-blocking");
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, m_fd) == -1)
    {
        throw std::runtime_error("Failed to create socket pair");
    }
}


inline void NBDProxy::InitNBD()
{
    int flags;
    if (ioctl(nbd_fd, NBD_CLEAR_SOCK) == -1)
    {
        throw std::runtime_error("Failed to clear socket");
    }
    if (ioctl(nbd_fd, NBD_SET_SOCK, m_fd[1]) == -1)
    {
        throw std::runtime_error("Failed to set socket");
    }

    #if defined NBD_SET_FLAGS
        flags = 0;
    #endif
    #if defined NBD_FLAG_SEND_TRIM
        flags |= NBD_FLAG_SEND_TRIM;
    #endif
    #if defined NBD_FLAG_SEND_FLUSH
        flags |= NBD_FLAG_SEND_FLUSH;
    #endif

    if (flags != 0 && ioctl(nbd_fd, NBD_SET_FLAGS, flags) == -1)
    {
        throw std::runtime_error("Failed to set flags");
    }

    std::thread DO_IT_Thread([this](){
// Unmask the SIGINT and SIGTERM signals for the main thread (or signal handler thread)
sigset_t sigset;
if (sigfillset(&sigset) != 0 || pthread_sigmask(SIG_SETMASK, &sigset, NULL) != 0)
{
    throw std::runtime_error("Failed to set signal mask in the worker thread");
}

try 
{
    if (ioctl(nbd_fd, NBD_DO_IT) == -1)
    {
        throw std::runtime_error("Failed to execute NBD_DO_IT");
    }
}
catch (...)
{
    ioctl(nbd_fd, NBD_CLEAR_QUE);
    ioctl(nbd_fd, NBD_CLEAR_SOCK);
    throw;
}
});

DO_IT_Thread.detach();
}
// NBDProxy implementation end

inline void NBDProxy::SetupSignalHandling()
{
    struct sigaction act;
    act.sa_handler = disconnect_nbd;
    act.sa_flags = SA_RESTART;

    if (sigemptyset(&act.sa_mask) != 0 ||
        sigaddset(&act.sa_mask, SIGINT) != 0 ||
        sigaddset(&act.sa_mask, SIGTERM) != 0)
    {
        throw std::runtime_error("Failed to prepare signal mask");
    }

    if (sigaction(SIGINT, &act, nullptr) != 0 ||
        sigaction(SIGTERM, &act, nullptr) != 0)
    {
        throw std::runtime_error("Failed to register signal handlers");
    }

    nbd_dev_to_disconnect = nbd_fd;

    // Unmask SIGINT and SIGTERM for the main thread
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);

    if (pthread_sigmask(SIG_UNBLOCK, &sigset, nullptr) != 0)
    {
        throw std::runtime_error("Failed to unblock signals in the main thread");
    }
}

#endif // __NBD_PROXY__
