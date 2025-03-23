#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include <string> //std::string
#include <boost/uuid/uuid.hpp> //uid
#include <boost/uuid/uuid_generators.hpp> //uid 
#include <boost/uuid/uuid_io.hpp> //uid
#include <atomic> //std::atomic_int
#include <linux/nbd.h> //nbd
#include <iostream> //cout
#include <mutex> //mutex 

#include "framework_interfaces.hpp"
#include "minion_proxy.hpp"
#include "raid_manager.hpp"


#define PACKET_SIZE 65000

enum CommandType
{
    READ,
    WRITE,
    RESPONSE,
    DISCONNECT,
    FLUSH,
    TRIM
};

typedef struct nbd_data // struct instead of functions in NBDData
{
    int sockfd;
    size_t from;
    size_t len;
}nbd_data_t;

typedef struct packet_header // sends to minion
{
    size_t len;
    size_t offset;
    CommandType type;
    int packet_id;
}packet_header_t;

typedef struct reply_header // receives from minion
{
    size_t len;
    int packet_id;
    int error;
    CommandType type;
}reply_header_t;

class NBDData : public ITaskData<CommandType>
{
public:
    NBDData(int socket_fd);
    ~NBDData();

    CommandType GetKey() const override;
    nbd_data_t GetData();

private:
    int m_sockfd;
    size_t m_from;
    size_t m_len;
    CommandType m_key;
    nbd_request m_request;
    void* m_chunk;
};

// class NBDWriteCommand

class NBDWriteCommand : public ICommand
{
public:
    NBDWriteCommand(ITaskData<CommandType>* data);

    void Run() override;
    static std::mutex m_write_mutex;

private:
    ITaskData<CommandType>* m_data;

};// Class NBDWriteCommand

// Class NBDReadCommand

class NBDReadCommand : public ICommand
{
public:
    NBDReadCommand(ITaskData<CommandType>* data);

    static std::mutex m_read_mutex;

    void Run() override;

private:
    ITaskData<CommandType>* m_data;

}; // Class NBDReadCommand

// DISCONNECT, FLUSH and TRIM operations does almost nothing, so it should be done  
// in NBDData, so the following commands are empty:

// Class NBDDiscCommand

class NBDDiscCommand : public ICommand 
{
public:
    NBDDiscCommand(ITaskData<CommandType>* data);

    void Run() override;

private:
    ITaskData<CommandType>* m_data;

}; // Class NBDDiscCommand

// Class NBDFlushCommand

class NBDFlushCommand : public ICommand 
{
public:
    NBDFlushCommand(ITaskData<CommandType>* data);

    void Run() override;

private:
    ITaskData<CommandType>* m_data;

}; // Class NBDFlushCommand

// Class NBDTrimCommand

class NBDTrimCommand : public ICommand 
{
public:
    NBDTrimCommand(ITaskData<CommandType>* data);

    void Run() override;

private:
    ITaskData<CommandType>* m_data;

}; // Class NBDTrimCommand

// Class ResponseData

class ResponseData : public ITaskData<CommandType>
{
    public:
        ResponseData(int fd);
        virtual CommandType GetKey() const;
        char* GetResponsePacket();

    private:
        int m_fd;
        char* m_packet;
}; // Class ResponseData

// Class ResponseCommand

class ResponseCommand : public ICommand
{
public:
    ResponseCommand(ITaskData<CommandType>* data);

    void Run() override;
private:
    ITaskData<CommandType>* m_data;
};// Class ResponseData

#endif //__COMMANDS_HPP__