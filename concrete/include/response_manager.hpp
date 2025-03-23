#ifndef __RESPONSE_MANAGER_HPP__
#define __RESPONSE_MANAGER_HPP__


#include <string> //std::string
#include <sys/types.h> // flags
#include <sys/socket.h> // socket
#include <netinet/in.h> //sockaddr_in
#include <unordered_map> //unordered map
#include <iostream> //cout

#include "singleton.hpp" //singleton
#include "commands.hpp" //commands
    
enum PacketStatus
{
    SENT,
    FAILED,
    SUCCESS
};

class ResponseManager : public IInputProxy<CommandType>
{
public:

    ~ResponseManager() = default;

    void AddPacket(char* packet);
    void* GetPacket(int packet_id);
    void RemovePacket(int packet_id);

    void ChangePacketStatus(int packet_id, PacketStatus status);
    void ChangeRecievedPacketStatus(int packet_id, PacketStatus status);
    PacketStatus GetPacketStatus(int packet_id);
    int AllPacketsRecieved();

    void SetNBDFD(int sockfd);
    int GetNBDFD();
    int GetFD();


    virtual const ITaskData<CommandType>* GetTaskData();
    
private:
    friend Singleton<ResponseManager>;
    ResponseManager();
    std::unordered_map<int, std::pair<char*, PacketStatus>> m_packets;
    std::unordered_map<int, std::pair<char*, PacketStatus>> m_receive_packets;

    int m_send_nbd_sockfd;
    int m_response_fd;

}; // Class ResponseManager


#endif //__MINION_PROXY_HPP__