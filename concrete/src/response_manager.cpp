#include "response_manager.hpp"

#define RESPONSE_PORT 8888 // the port the minions will send to

ResponseManager::ResponseManager()
{
    m_response_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_response_fd < 0)
    {
        throw std::runtime_error("response manager creating socket failed");
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(RESPONSE_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_response_fd, (sockaddr *)(&serverAddr), sizeof(serverAddr)) < 0)
    {
        close(m_response_fd);
        throw std::runtime_error("response manager binding socket failed");
    }
}

void ResponseManager::AddPacket(char *packet)
{
    packet_header_t* header = (packet_header_t*)packet;
    char* buffer = NULL;
    std::cout << "adding packet: " << header->packet_id << std::endl;

    if(header->type == READ)
    {
        buffer = new char[sizeof(packet_header_t)];
        memcpy(buffer, packet, sizeof(packet_header_t));
        for (size_t i = 0; i <= header->len / PACKET_SIZE; i++)
        {
            m_receive_packets[i] = std::make_pair(nullptr, SENT);
        }
        m_packets[header->packet_id] = std::make_pair(buffer, SENT);
    }
    else
    {
        m_packets[header->packet_id] = std::make_pair(packet, SENT);
    }
}


void *ResponseManager::GetPacket(int packet_id)
{
    std::cout << "getting packet: " << packet_id << std::endl;
    return m_packets[packet_id].first;
}


void ResponseManager::RemovePacket(int packet_id)
{
    std::cout << "removed packet: " << packet_id << std::endl;
    //delete[] m_packets[packet_id].first;
    
    m_packets.erase(packet_id);
}


void ResponseManager::ChangePacketStatus(int packet_id, PacketStatus status)
{
    std::cout << "changing sent packet" << packet_id << " to status: " << status << std::endl;
    m_packets[packet_id].second = status;
}

void ResponseManager::ChangeRecievedPacketStatus(int packet_id, PacketStatus status)
{
    std::cout << "changing recieved packet" << packet_id << " to status: " << status << std::endl;
    m_packets[packet_id].second = status;
}

PacketStatus ResponseManager::GetPacketStatus(int packet_id)
{
    return m_packets[packet_id].second;
}

int ResponseManager::AllPacketsRecieved()
{
    for(auto packet : m_receive_packets)
    {
        if(packet.second.second != SUCCESS)
        {
            return false;
        }
    }
    m_receive_packets.clear();
    return true;
}

void ResponseManager::SetNBDFD(int sockfd)
{
    m_send_nbd_sockfd = sockfd;
}

int ResponseManager::GetNBDFD()
{
    return m_send_nbd_sockfd;
}

int ResponseManager::GetFD()
{
    return m_response_fd;
}

const ITaskData<CommandType>* ResponseManager::GetTaskData()
{
    return new ResponseData(m_response_fd);
}
