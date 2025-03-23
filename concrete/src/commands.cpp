#include <iostream>
#define __INIT_SYMBOL
#include <functional>  // std::function

#include "commands.hpp"
#include "async_injection.hpp"
#include "response_manager.hpp"

char* packets_data;
nbd_reply reply;
extern void* __data;

#ifdef WORDS_BIGENDIAN
u_int64_t ntohll(u_int64_t a) {
    return a;
}
#else
u_int64_t ntohll(u_int64_t a) {
    u_int32_t lo = a & 0xffffffff;
    u_int32_t hi = a >> 32U;
    lo = ntohl(lo);
    hi = ntohl(hi);
    return ((u_int64_t) lo) << 32U | hi;
}
#endif
#define htonll ntohll

static int ReadAll(int fd, char* buf, size_t count)
{
    ssize_t bytes_read;
    while (count > 0)
    {
        bytes_read = read(fd, buf, count);
        if (bytes_read <= 0)
        {
            throw std::runtime_error("Read error");
        }
        buf += bytes_read;
        count -= bytes_read;
    }
    return 0;
}

static int WriteAll(int fd, const char* buf, size_t count)
{
    ssize_t bytes_written;
    while (count > 0)
    {
        bytes_written = write(fd, buf, count);
        if (bytes_written <= 0)
        {
            throw std::runtime_error("Write error");
        }
        buf += bytes_written;
        count -= bytes_written;
    }
    return 0;
}

static void SendReadData(minion_dest_t md);
static void SendWriteData(minion_dest_t md, void* _data);
std::mutex NBDWriteCommand::m_write_mutex;

//NBDData implementation

void FirstNBDreads(int m_sockfd)
{
    ssize_t bytes_read;
    CommandType m_key;
    nbd_request m_request;
    nbd_reply m_reply;
    void* m_chunk;

    m_reply.magic = htonl(NBD_REPLY_MAGIC);
    m_reply.error = htonl(0);
    //bytes_read = read(m_sockfd, &request, sizeof(request));
    // if (bytes_read <= 0)
    // {
    //     throw std::runtime_error("Failed to read request");
    // }
    NBDWriteCommand::m_write_mutex.lock();
    if((bytes_read = read(m_sockfd, &m_request, sizeof(m_request))) > 0)
    {
        assert(bytes_read == sizeof(m_request));
        memcpy(m_reply.handle, m_request.handle, sizeof(m_reply.handle));
        m_reply.error = htonl(0);

        size_t m_len = ntohl(m_request.len);
        size_t m_from = ntohll(m_request.from);
        assert(m_request.magic == htonl(NBD_REQUEST_MAGIC));
        
        switch(ntohl(m_request.type))
            {
                case NBD_CMD_READ:
                    m_chunk = malloc(m_len);
                    fprintf(stderr, "Got NBD_CMD_READ\n");
                    WriteAll(m_sockfd, reinterpret_cast<char*>(&m_reply), sizeof(nbd_reply));
                    memcpy(m_chunk, (char *)__data + m_from, m_len);
                    WriteAll(m_sockfd, reinterpret_cast<char*>(m_chunk), m_len);
                    free(m_chunk);
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;

                case NBD_CMD_WRITE:
                    m_chunk = malloc(m_len);
                    fprintf(stderr, "Got NBD_CMD_WRITE\n");
                    ReadAll(m_sockfd, reinterpret_cast<char*>(m_chunk), m_len);
                    WriteAll(m_sockfd, reinterpret_cast<char*>(&m_reply), sizeof(nbd_reply));
                    memcpy((char *)__data + m_from, m_chunk, m_len);
                    free(m_chunk);
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;

                case NBD_CMD_DISC:
                    fprintf(stderr, "Got NBD_CMD_DISC\n");
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;
    #ifdef NBD_FLAG_SEND_FLUSH
                case NBD_CMD_FLUSH:
                    fprintf(stderr, "Got NBD_CMD_FLUSH\n");
                    WriteAll(m_sockfd, reinterpret_cast<char*>(&m_reply), sizeof(nbd_reply));
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;
    #endif
    #ifdef NBD_FLAG_SEND_TRIM
                case NBD_CMD_TRIM:
                    fprintf(stderr, "Got NBD_CMD_TRIM\n");
                    WriteAll(m_sockfd, reinterpret_cast<char*>(&m_reply), sizeof(nbd_reply));
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;
    #endif
                default:
                    throw std::runtime_error("Unknown NBD command");
            }

    }
    else
    {
        fprintf(stderr, "failed read request\n");
    }
}

NBDData::NBDData(int socket_fd) 
    : m_sockfd(socket_fd)
{
    // static auto start = std::chrono::high_resolution_clock::now();
    // static auto duration = std::chrono::seconds(20);
    // static bool free_flag = false;

    // if(std::chrono::high_resolution_clock::now() - start < duration)
    // {
    //     m_key = READ;
    //     FirstNBDreads(socket_fd);
    //     return;
    // }
    // std::cout << "after first reads" << std::endl;
    // if(free_flag == false)
    // {
    //     free(__data);
    //     free_flag = true;
    // }

    ssize_t bytes_read;
    reply.magic = htonl(NBD_REPLY_MAGIC);
    reply.error = htonl(0);
    //bytes_read = read(m_sockfd, &request, sizeof(request));
    // if (bytes_read <= 0)
    // {
    //     throw std::runtime_error("Failed to read request");
    // }
    //NBDWriteCommand::m_write_mutex.unlock();

    if((bytes_read = read(m_sockfd, &m_request, sizeof(m_request))) > 0)
    {
        assert(bytes_read == sizeof(m_request));
        memcpy(reply.handle, m_request.handle, sizeof(reply.handle));
        reply.error = htonl(0);

        m_len = ntohl(m_request.len);
        m_from = ntohll(m_request.from);
        assert(m_request.magic == htonl(NBD_REQUEST_MAGIC));
        
        switch(ntohl(m_request.type))
            {
                case NBD_CMD_READ:
                    fprintf(stderr, "IN NBDDATA READ\n");
                    
                    m_key = READ;
                    //m_chunk = malloc(m_len);
                    // fprintf(stderr, "Got NBD_CMD_READ\n");
                    // WriteAll(m_sockfd, reinterpret_cast<char*>(&m_reply), sizeof(struct nbd_reply));
                    // memcpy(m_chunk, (char *)data + m_from, m_len);
                    // WriteAll(m_sockfd, reinterpret_cast<char*>(m_chunk), m_len);
                    // free(m_chunk);
                    break;

                case NBD_CMD_WRITE:
                    fprintf(stderr, "IN NBDDATA WRITE\n");
                    //m_chunk = malloc(m_len);
                    m_key = WRITE;
                    // fprintf(stderr, "Got NBD_CMD_WRITE\n");
                    // ReadAll(m_sockfd, reinterpret_cast<char*>(m_chunk), m_len);
                    // WriteAll(m_sockfd, reinterpret_cast<char*>(&m_reply), sizeof(struct nbd_reply));
                    // memcpy((char *)data + m_from, m_chunk, m_len);
                    // free(m_chunk);
                    break;

                case NBD_CMD_DISC:
                    m_key = DISCONNECT;
                    fprintf(stderr, "Got NBD_CMD_DISC\n");
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;
    #ifdef NBD_FLAG_SEND_FLUSH
                case NBD_CMD_FLUSH:
                    m_key = FLUSH;
                    fprintf(stderr, "Got NBD_CMD_FLUSH\n");
                    WriteAll(m_sockfd, reinterpret_cast<char*>(&reply), sizeof(struct nbd_reply));
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;
    #endif
    #ifdef NBD_FLAG_SEND_TRIM
                case NBD_CMD_TRIM:
                    m_key = TRIM;
                    fprintf(stderr, "Got NBD_CMD_TRIM\n");
                    WriteAll(m_sockfd, reinterpret_cast<char*>(&reply), sizeof(struct nbd_reply));
                    NBDWriteCommand::m_write_mutex.unlock();
                    break;
    #endif
                default:
                    throw std::runtime_error("Unknown NBD command");
            }

    }
    else
    {
        fprintf(stderr, "failed read request\n");
    }
}

NBDData::~NBDData()
{
}

CommandType NBDData::GetKey() const
{
    return m_key;
}

nbd_data_t NBDData::GetData()
{
    nbd_data_t ret = {m_sockfd, m_from, m_len};
    return ret;
}

// NBDWriteCommand implementation

NBDWriteCommand::NBDWriteCommand(ITaskData<CommandType> *data) : m_data(data)
{
}

void NBDWriteCommand::Run()
{
    NBDWriteCommand::m_write_mutex.lock();
    NBDData* nd = dynamic_cast<NBDData*>(m_data);
    nbd_data_t ndt = nd->GetData();
    char* chunk = (char*)malloc(ndt.len);
    char* curr_chunk = chunk;
    fprintf(stderr, "Got NBD_CMD_WRITE\n");
    ReadAll(ndt.sockfd, chunk, ndt.len); // what the user writes, this should be sent to the minion


    //WriteAll(ndt.sockfd, reinterpret_cast<char*>(&reply), sizeof(nbd_reply)); // the response manager should handle this, it will send the reply to the minion, using global variable that will be changed and replyed from response command
    //memcpy((char *)data + ndt.from, chunk, ndt.len); // this data should be from the minion itself, and not the local data saved in the global variable data that we write and read from

    RaidManager* rm = Singleton<RaidManager, std::vector<MinionProxy*>, size_t, size_t>::GetInstance(std::vector<MinionProxy*>(), 0, 0);
    std::cout << "ndt.len = " << ndt.len << " ndt.from = " << ndt.from << std::endl;
    std::vector<minion_dest_t> mdv = rm->GetDest(ndt.from, ndt.len);

    std::cout << "sending to write " << ndt.len << " bytes, " << "from offset " << ndt.from << std::endl;

    for(auto& md : mdv)
    {
        std::cout << "sending to write to the port " << md.mp->GetPort() << " len = " << md.len << " offset = " << md.offset << std::endl;
        SendWriteData(md, curr_chunk);
        if(md.is_backup == false)
        {
            curr_chunk += md.len;
        }
    }
    NBDWriteCommand::m_write_mutex.unlock();
    free(chunk);
}

// NBDReadCommand implementation

std::mutex NBDReadCommand::m_read_mutex;

NBDReadCommand::NBDReadCommand(ITaskData<CommandType>* _data) : m_data(_data)
{
}

void NBDReadCommand::Run()
{
    // static auto start = std::chrono::high_resolution_clock::now();
    // static auto duration = std::chrono::seconds(20);

    // if(std::chrono::high_resolution_clock::now() - start < duration)
    // {
    //     return;
    // }
    NBDWriteCommand::m_write_mutex.lock();
    NBDData* nd = dynamic_cast<NBDData*>(m_data);
    nbd_data_t ndt = nd->GetData();
    //char* chunk = (char*)malloc(ndt.len);
    packets_data = (char*)malloc(ndt.len); // to handle all response packets

    fprintf(stderr, "Got NBD_CMD_READ\n");
    //NBDWriteCommand::m_write_mutex.lock(); // this lock stucks the select on the reactor, causing it to not wake up on responses from minions, so i need to make it read all current requests from nbd and send it to the minions, or just read the initial nbd requests from the nbd when we connect it
    //WriteAll(ndt.sockfd, reinterpret_cast<char*>(&reply), sizeof(nbd_reply));
    //memcpy(chunk, (char *)packets_data + ndt.from, ndt.len);
    //WriteAll(ndt.sockfd, reinterpret_cast<char*>(chunk), ndt.len);
    //NBDWriteCommand::m_write_mutex.unlock();
    RaidManager* rm = Singleton<RaidManager, std::vector<MinionProxy*>, size_t, size_t>::GetInstance(std::vector<MinionProxy*>(), 0, 0);
    std::vector<minion_dest_t> mdv = rm->GetDest(ndt.from, ndt.len);

    for(auto& md : mdv)
    {
        if(md.is_backup == false)
        {
            SendReadData(md);
        }
    }
    //free(chunk);
}

static void CreateAsync(packet_header_t header, MinionProxy* mp)
{
        auto action = [header, mp]() -> bool
        {
            auto rm = Singleton<ResponseManager>::GetInstance();

            std::cout << "checking packet: " << header.packet_id << "ip: " << mp->GetIP() << "port: " << mp->GetPort() << std::endl;
            if (rm->GetPacketStatus(header.packet_id) == SUCCESS)
            {
                rm->RemovePacket(header.packet_id);
                return true;
            }

            void* packet = rm->GetPacket(header.packet_id);
            if(packet == NULL)
            {
                std::cout << "packet is NULL wtf" << std::endl;
                return true;
            }
            packet_header_t* ph = static_cast<packet_header_t*>(packet);
            std::cout << "sending packet: " << ph->packet_id << " of size = " << ph->len << " in offset " << ph->offset <<  "to ip: " << mp->GetIP() << "port: " << mp->GetPort() << std::endl;
            if(ph->type == READ)
            {
                mp->SendMessage(static_cast<char*>(packet), sizeof(header));
            }
            else
            {
                mp->SendMessage(static_cast<char*>(packet), ph->len + sizeof(header));
            }
            return false;
        };
        AsyncInjection* ai = new AsyncInjection(action, std::chrono::milliseconds(2000));
        (void)ai;
}


static void SendWriteData(minion_dest_t md, void* _data)
{
    packet_header_t header;
    char* packet = new char[PACKET_SIZE];
    size_t max_data_size = PACKET_SIZE - sizeof(header);
    auto rm = Singleton<ResponseManager>::GetInstance();
    
    header.packet_id = 0;
    header.offset = md.offset;
    header.len = std::min(md.len, max_data_size);
    header.type = WRITE;

    while(md.len >= max_data_size)
    {
        memcpy(packet, &header, sizeof(header));
        memcpy(packet + sizeof(header), _data, header.len);
        (md.mp)->SendMessage(packet, header.len + sizeof(header));
        rm->AddPacket(packet);

        CreateAsync(header, md.mp);

        _data = (char*)_data + header.len;
        md.len -= header.len;
        ++header.packet_id;
        header.offset += header.len;
        header.len = std::min(md.len, max_data_size);
    }

    if(md.len > 0)
    {
        memcpy(packet, &header, sizeof(header));
        memcpy(packet + sizeof(header), _data, md.len);
        (md.mp)->SendMessage(packet, md.len + sizeof(header));
        rm->AddPacket(packet);
        CreateAsync(header, md.mp);
    }

}

static void SendReadData(minion_dest_t md)
{
    packet_header_t header;
    static int pid = 0;
    auto rm = Singleton<ResponseManager>::GetInstance();
    
    header.packet_id = pid;
    header.offset = md.offset;
    header.len = md.len;
    header.type = READ;

    (md.mp)->SendMessage((char*)&header, sizeof(header));
    rm->AddPacket((char*)&header);
    
    CreateAsync(header, md.mp);
    ++pid;
}

// ResponseData implementation

ResponseData::ResponseData(int fd) : m_fd(fd)
{
    
    reply_header_t header;
    std::cout << "created response data" << std::endl;
    m_packet = new char[PACKET_SIZE];
    ssize_t rr = 0;
    rr = read(m_fd, m_packet, PACKET_SIZE);
    if(0 >= rr)
    {
        throw std::runtime_error("read response data error");
    }

    // if(0 >= read(m_fd, &header, sizeof(header)))
    // {
    //     throw std::runtime_error("read response header error");
    // }
    // std::cout << "didnt read1" << std::endl;
    memcpy(&header, m_packet, sizeof(header));


    std::cout << " packet id: " << header.packet_id << "read bytes: " << rr << " header len: " << header.len << std::endl;
    
}

CommandType ResponseData::GetKey() const
{
    return RESPONSE;
}

char *ResponseData::GetResponsePacket()
{
    return m_packet;
}// ResponseData implementation

// ResponseCommand implementation

ResponseCommand::ResponseCommand(ITaskData<CommandType> *data) : m_data(data) 
{
}

void ResponseCommand::Run()
{
    reply_header_t header;
    ResponseData* rd = dynamic_cast<ResponseData*>(m_data);
    char* reply_packet = rd->GetResponsePacket();
    static int offset = 0; // each packet's offset, to send all packet's data together

    std::cout << "entered response command" << std::endl;
    memcpy(&header, reply_packet, sizeof(header));

    auto rm = Singleton<ResponseManager>::GetInstance();

    if(header.error == 0)
    {
        if(header.type == READ)
        {
            rm->ChangeRecievedPacketStatus(header.packet_id, SUCCESS);
            memcpy(packets_data + offset, reply_packet + sizeof(header), header.len);
            delete[] reply_packet;
            if(1 == true) // if(rm->AllPacketsRecieved() == true), need to be fixed
            {
                rm->ChangePacketStatus(0, SUCCESS);
                std::cout << "all packets received" << std::endl;
                write(rm->GetNBDFD(), &reply, sizeof(nbd_reply)); // send the full reply from the minion(s) to the nbd (all packets)
                write(rm->GetNBDFD(), packets_data, offset + header.len); // send the full data from the minion(s) the the nbd (all packets)
                offset = 0;
                free(packets_data);
                NBDWriteCommand::m_write_mutex.unlock();
            }
            else
            {
                offset += header.len; //offset to the next packet data inside data variable, to send it all to the nbd with the reply 
            }
        }
        else if(header.type == WRITE)
        {
            rm->ChangePacketStatus(header.packet_id, SUCCESS);
            //NBDWriteCommand::m_write_mutex.lock();
            write(rm->GetNBDFD(), &reply, sizeof(nbd_reply)); // send the full reply from the minion(s) to the nbd (all packets)
            NBDWriteCommand::m_write_mutex.unlock();
        }
    }
    //NBDWriteCommand::m_write_mutex.unlock();
}

NBDDiscCommand::NBDDiscCommand(ITaskData<CommandType> *data) : m_data(data)
{
}

void NBDDiscCommand::Run()
{
}

NBDFlushCommand::NBDFlushCommand(ITaskData<CommandType> *data) : m_data(data)
{
}

void NBDFlushCommand::Run()
{
}

NBDTrimCommand::NBDTrimCommand(ITaskData<CommandType> *data) : m_data(data)
{
}

void NBDTrimCommand::Run()
{
    //delete m_data; maybe add this to all nbd commands
}
