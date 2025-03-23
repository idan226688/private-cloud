// receives path to the block device, the minion id and port

/* gpp17d -Iinclude -I../framework/include ./src/commands.cpp ./src/minion_proxy.cpp 
./src/raid_manager.cpp ../framework/src/scheduler.cpp ./src/response_manager.cpp 
../framework/src/async_injection.cpp ./test/minion_test.cpp -o minion */

// loop devices must be initialized with size of minions

// sudo ./minion /dev/loop14 1 5000 
// sudo ./minion /dev/loop15 2 5001
// sudo ./minion /dev/loop16 3 5002
// sudo ./minion /dev/loop17 4 5003
// sudo ./minion /dev/loop18 5 5004
// sudo ./minion /dev/loop19 6 5005

#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <stdlib.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

void* __data = NULL;
#include "commands.hpp"

using namespace ilrd;

int minion = 0;
int port = 0;

int writeToDevice(int fd, off_t offset, size_t size, const char* data)
{
    ssize_t written = 0;
    const size_t block_size = 4096;  // Set block size to 4096 bytes

    while (size > 0)
    {
        // Calculate the current block to write, ensuring we don't exceed the remaining size
        size_t bytes_to_write = (size < block_size) ? size : block_size;

        std::cout << "minion " << minion << " writing at offset " << offset 
                  << ", " << bytes_to_write << " bytes" << std::endl;

        written = pwrite(fd, data, bytes_to_write, offset);
        if (written <= 0)
        {
            std::cerr << "Error writing to minion " << minion << " at offset " << offset << std::endl;
            return 1;
        }

        data += written;
        size -= written;
        offset += written;  // Increment the offset by the number of bytes written
    }

    return 0;
}


int readFromDevice(int fd, off_t offset, size_t size, char* buffer)
{
    ssize_t bytesRead = 0;

    while (size > 0)
    {
        bytesRead = pread(fd, buffer, size, offset);
        if (bytesRead <= 0)
        {
            throw std::runtime_error("Read error");
        }
        buffer += bytesRead;
        size -= bytesRead;
    }

    return 0;
}

void SendMessage(int fd, char *buffer, size_t size)
{
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(8888);  // Sending to port 8888 (response port)
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);  // IP is 127.0.0.1

    // Sending the message to 127.0.0.1:8888
    if (sendto(fd, buffer, size, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) == -1)
    {
        throw std::runtime_error("Failed to send UDP message");
    }
    std::cout << "minion " << minion << " sent message to 127.0.0.1:8888 " << std::endl;

}

void ReceiveMessage(int fd, char *buffer, size_t size)
{
    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(src_addr);
    memset(&src_addr, 0, sizeof(src_addr));

    ssize_t bytes_received = recvfrom(fd, buffer, size, 0, (struct sockaddr*)&src_addr, &addrlen);
    if (bytes_received < 0)
    {
        throw std::runtime_error("Failed to receive UDP message");
    }

    std::cout << "Received message from " << inet_ntoa(src_addr.sin_addr) << ":" << ntohs(src_addr.sin_port) << std::endl;
}

int main(int argc, char* argv[])
{
    packet_header_t request_header;
    reply_header_t reply_header;
    if (argc != 4)
    {
        std::cerr << "Please provide 3 arguments: device path, minion id, port" << std::endl;
        return EXIT_FAILURE;
    }

    std::string device = argv[1];
    minion = atoi(argv[2]);
    port = atoi(argv[3]);

    std::cout << "minion " << minion << " listening on port " << port << std::endl;

    char* request = new char[PACKET_SIZE];
    char* reply = new char[PACKET_SIZE];

    int fd_device = open(device.c_str(), O_RDWR);
    if (fd_device < 0)
    {
        std::cerr << "Failed to open device " << device << std::endl;
        return EXIT_FAILURE;
    }

    // Create UDP socket for sending and receiving messages
    int fd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_udp < 0)
    {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return EXIT_FAILURE;
    }

    int reuse = 1;
    if (setsockopt(fd_udp, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "setsockopt(SO_REUSEPORT) failed!" << std::endl;
        close(fd_udp);
        return 1;
    }

    // Bind the socket to the port specified in the arguments for receiving messages
    struct sockaddr_in recv_addr;
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(port);
    recv_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all available interfaces

    if (bind(fd_udp, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) < 0)
    {
        std::cerr << "Failed to bind UDP socket to port " << port << std::endl;
        return EXIT_FAILURE;
    }

    // tests the first read requests

    // auto start = std::chrono::high_resolution_clock::now(); 
    // auto duration = std::chrono::seconds(60); 

    // while (std::chrono::high_resolution_clock::now() - start < duration) 
    // {
    //     ReceiveMessage(fd_udp, request, PACKET_SIZE);  // Receive a request
    //     memcpy(&request_header, request, sizeof(request_header));
    //     reply_header.error = 0;
    //     reply_header.len = request_header.len;
    //     reply_header.packet_id = request_header.packet_id;
    //     reply_header.type = request_header.type;
    //     memcpy(reply, &reply_header, sizeof(reply_header));
    //     SendMessage(fd_udp, reply, reply_header.len + sizeof(reply_header));
    //     std::cout << "minion " << minion << " sending packet id " << request_header.packet_id << ", " << "of len = " << request_header.len << std::endl;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }

    while (true)
    {
        ReceiveMessage(fd_udp, request, PACKET_SIZE);  // Receive a request
        memcpy(&request_header, request, sizeof(request_header));

        if (request_header.type == READ)
        {
            // Read data from the device and prepare a reply
            reply_header.error = readFromDevice(fd_device, request_header.offset, request_header.len, reply + sizeof(reply_header));
            reply_header.len = request_header.len;
            reply_header.packet_id = request_header.packet_id;
            reply_header.type = request_header.type;
            memcpy(reply, &reply_header, sizeof(reply_header));
            std::cout << "minion " << minion << " sending packet id " << reply_header.packet_id << ", " << "of len = " << reply_header.len << "error = " << reply_header.error << std::endl;
            SendMessage(fd_udp, reply, reply_header.len + sizeof(reply_header));  // Send the reply
        }
        else
        {
            // Write data to the device
            std::cout << "minion " << minion << " got: packet id = " << request_header.packet_id << "len = " << request_header.len << "offset = " << request_header.offset << "type = " << request_header.type << "packet_id = " << request_header.packet_id << std::endl;
            reply_header.error = writeToDevice(fd_device, request_header.offset, request_header.len, request + sizeof(request_header));
            reply_header.len = request_header.len;
            reply_header.packet_id = request_header.packet_id;
            reply_header.type = request_header.type;

            // Send the response
            SendMessage(fd_udp, (char*)&reply_header, sizeof(reply_header));
        }
    }

    close(fd_device);
    close(fd_udp);
    delete[] request;
    delete[] reply;

    return 0;
}
