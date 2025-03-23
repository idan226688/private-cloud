#include <iostream>
#include <thread>
#include <chrono>
#include <cstring> // for std::strlen
#include <unistd.h> //close

#define __INIT_SYMBOL
#include "singleton.hpp"
#include "minion_proxy.hpp"
#include "commands.hpp"
#include "response_manager.hpp"

#define BUFFER_SIZE 4096

void ServerFunction()
{
    int port = 8080;
    char* buffer = new char[70000];
    //ilrd::packet_header_t header;
    //ilrd::ResponseManager* rm = ilrd::Singleton<ilrd::ResponseManager>::GetInstance();

    // Setup server socket and address
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        throw std::runtime_error("Failed to create socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        throw std::runtime_error("Failed to bind socket");
    }
    int i = 0;
    // Wait for a message
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t addr_len = sizeof(client_addr);
    //int n = recvfrom(sockfd, buffer, sizeof(ilrd::packet_header_t) + 19, 0, (struct sockaddr *)&client_addr, &addr_len);
    while(recvfrom(sockfd, buffer, 70000, 0, (struct sockaddr *)&client_addr, &addr_len))
    {
        //std::cout << "Server received: " << header.offset << std::endl;
        std::cout << "Server received: " << buffer[24] << std::endl;
        while(i < 70000 && buffer[i] != 'S')
        {
            ++i;
        }
        std::cout << i << std::endl;
        i = 0;
        //header = *((ilrd::packet_header_t*)buffer);
        //std::cout << "changing packet:" <<header.packet_id << std::endl;
        //rm->ChangePacketStatus(header.packet_id, ilrd::SUCCESS);
    }

    // Send response back
    const char *response = "Message received";
    sendto(sockfd, response, std::strlen(response), 0, (const struct sockaddr *)&client_addr, addr_len);

    close(sockfd);
}

void ClientFunction()
{
    std::string ip = "127.0.0.1";
    int port = 5000;

    auto client = ilrd::MinionProxy::GetInstance(ip, port);

    // Send message to server
    char message[] = "Hello from client!";
    client->SendMessage(message, strlen(message));

    // Receive response from server
    char response[BUFFER_SIZE] = {0};
    client->RecieveMessage(response, BUFFER_SIZE);

    std::cout << "Client received: " << response << std::endl;
}

void ClientCommandFunction()
{
    std::string ip = "127.0.0.1";
    int port = 8080;
    //ilrd::MinionProxy client(ip, port);

    // Send message to server
    char message[70000] = "Hello from client!";
    message[64976] = 'S';
    //client.SendMessage(message);
    
    // ilrd::WriteCommand wc(ip, port, message, sizeof(message), strlen(message));
    // wc.Run();
    //auto mp = ilrd::Singleton<ilrd::MinionProxy, const std::string, int>::GetInstance(ip, port);

    //ilrd::SendReadData sd(ip, port, message, 70000, 50);
    //ilrd::SendWriteCommand rc(&sd);
    //rc.Run();

    // Receive response from server
    //char response[BUFFER_SIZE] = {0};
    //client.RecieveMessage(response, BUFFER_SIZE);

    //std::cout << "Client received: " << response << std::endl;
}

int main()
{
    std::thread serverThread(ServerFunction);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // // Start the clientss
    //ClientFunction();
    ClientCommandFunction();

    // // Wait for the server to finish
    serverThread.join();



    return 0;
}
