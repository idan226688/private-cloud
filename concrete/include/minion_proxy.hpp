#ifndef __MINION_PROXY_HPP__
#define __MINION_PROXY_HPP__
#include <string> //std::string
#include <sys/types.h> // flags
#include <sys/socket.h> // socket
#include <netinet/in.h> //sockaddr_in
#include <unordered_map> // map
#include <mutex> //mutex
#include <memory> //shared pointer

#include "framework_interfaces.hpp"
#include "commands.hpp"

class MinionProxy
{
public:
    static MinionProxy* GetInstance(const std::string& ip, int port);
    static void delete_instances();
    ~MinionProxy();

    void SendMessage(char* buffer, size_t size);
    void RecieveMessage(char* out_param, size_t size); // for later use maybe
    int GetFD();
    std::string GetIP();
    int GetPort();

private:
    MinionProxy(const std::string& ip, int port);
    int m_socket;
    struct sockaddr_in m_addr;
    static size_t GenerateKey(const std::string& ip, int port);
    std::string m_ip;
    int m_port;
    static std::unordered_map<size_t, MinionProxy*> m_instances;
    static std::mutex m_mutex;
}; // Class MinionProxy

#endif //__MINION_PROXY_HPP__