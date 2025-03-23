#include "minion_proxy.hpp"

#include <stdlib.h> //exit
#include <string.h> //strlen
#include <iostream> //cout
#include <arpa/inet.h> //inet_pton
#include <unistd.h> //close
#include <sstream>

static int CreateSocketUDP() 
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        throw std::runtime_error("Failed to create socket");
    }
    return sockfd;
}

static void SetupAddress(struct sockaddr_in *addr, const char *ip, int port) 
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) 
    {
        throw std::runtime_error("Invalid address to connect");
    }
}

static void SendUDPMessage(int sockfd, const struct sockaddr_in *addr, const char *message, size_t len) 
{
    sendto(sockfd, message, len, 0, (const struct sockaddr*)addr, sizeof(*addr)); 
}

static void ReceiveUDPMessage(int sockfd, struct sockaddr_in *addr, char *buffer, size_t size) 
{
    socklen_t addrLen = sizeof(*addr);
    int n = recvfrom(sockfd, buffer, size, 0, (struct sockaddr*)addr, &addrLen);
    if (-1 == n)
    {
        throw std::runtime_error("recvfrom failed");
    }
    std::cout<< "Recieved: "<< buffer << std::endl;
}

/* a quote from Beej: Remember, if you connect() a datagram socket, you can then simply 
use send() and recv() for all your transactions. The socket itself is 
still a datagram socket and the packets still use UDP, but the socket 
interface will automatically add the destination and source information for you.
*/

std::unordered_map<size_t, MinionProxy*> MinionProxy::m_instances;
std::mutex MinionProxy::m_mutex;

MinionProxy* MinionProxy::GetInstance(const std::string &ip, int port)
{
    size_t key = GenerateKey(ip, port);
    auto tmp = m_instances.find(key);

   if (tmp == m_instances.end())
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        MinionProxy* ret = m_instances[key];
        if (ret == nullptr)
        {
            ret = new MinionProxy(ip, port);
            std::atomic_thread_fence(std::memory_order_release);
            m_instances[key] = ret;
            atexit(MinionProxy::delete_instances);
        }
    }
    return m_instances[key];
}

void MinionProxy::delete_instances()
{
    for (auto pair : m_instances)
    {
        delete pair.second;
    }
    m_instances.clear(); // Clear the map    delete m_instance.load();
}

MinionProxy::MinionProxy(const std::string &ip, int port) : m_socket(CreateSocketUDP()), m_ip(ip), m_port(port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    SetupAddress(&m_addr, ip.c_str(), port);
    int opt = 1;

    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) 
    {
        throw std::runtime_error("Failed to set socket options");
    }

    // Bind the socket to the IP and port
    // if (bind(m_socket, (struct sockaddr*)&m_addr, sizeof(m_addr)) < 0) 
    // {
    //     throw std::runtime_error("Failed to bind socket");
    // }
}

MinionProxy::~MinionProxy()
{
    close(m_socket);
}

void MinionProxy::SendMessage(char *buffer, size_t size)
{
    SendUDPMessage(m_socket, &m_addr, buffer, size);
}

void MinionProxy::RecieveMessage(char* out_param, size_t size)
{
    ReceiveUDPMessage(m_socket, &m_addr, out_param, size);
}

int MinionProxy::GetFD()
{
    return m_socket;
}
std::string MinionProxy::GetIP()
{
    return m_ip;
}
int MinionProxy::GetPort()
{
    return m_port;
}
size_t MinionProxy::GenerateKey(const std::string &ip, int port)
{
    return std::hash<std::string>{}(ip + std::to_string(port));
}