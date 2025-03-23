#include <iostream>

#define __INIT_SYMBOL

#include "master.hpp"
#include "framework.hpp" //framework

ICommand* CreateNbdReadCommand(ITaskData<CommandType>* data)
{
    return new NBDReadCommand(data);
}

ICommand* CreateNbdWriteCommand(ITaskData<CommandType>* data)
{
    return new NBDWriteCommand(data); //s
}

ICommand* CreateNbdDiscCommand(ITaskData<CommandType>* data)
{
    return new NBDDiscCommand(data);
}

ICommand* CreateNbdTrimCommand(ITaskData<CommandType>* data)
{
    return new NBDTrimCommand(data);
}

ICommand* CreateNbdFlushCommand(ITaskData<CommandType>* data)
{
    return new NBDFlushCommand(data);
}

ICommand* CreateResponseCommand(ITaskData<CommandType>* data)
{
    return new ResponseCommand(data);
}

void TestMaster()
{
    const std::string &dll_folder = "./include";
    std::string ip = "127.0.0.1";
    int port = 5000; // minion's source port
    int minion_count = 6;
    size_t num_blocks = 180000; // 737,280,000 bytes , 122,880,000 bytes each minion
    int minion_size = num_blocks * 4096 / minion_count;

    std::map<std::pair<int, Reactor::Mode>, IInputProxy<CommandType>*> callbacks;

    Singleton<ThreadPool,size_t>::GetInstance(30);
    NBDProxy* nbd_proxy = new NBDProxy("/dev/nbd0", 4096, num_blocks); 
    ResponseManager* rp = Singleton<ResponseManager>::GetInstance();
    rp->SetNBDFD(nbd_proxy->GetFD());
    std::vector<MinionProxy*> mpv(minion_count);
    for (int i = 0; i < minion_count; ++i)
    {
        mpv[i] = ilrd::MinionProxy::GetInstance(ip, port + i);
    }

    Singleton<RaidManager, std::vector<MinionProxy*>, size_t, size_t>::GetInstance(mpv, minion_size * 2, minion_count);

    callbacks[std::make_pair(nbd_proxy->GetFD(), Reactor::Mode::READ)] = nbd_proxy;    
    callbacks[std::make_pair(rp->GetFD(), Reactor::Mode::READ)] = rp;    

    std::map<CommandType, std::function<ICommand*(ITaskData<CommandType>*)>> creators;

    creators[READ] = CreateNbdReadCommand;
    creators[WRITE] = CreateNbdWriteCommand;
    creators[DISCONNECT] = CreateNbdDiscCommand;
    creators[TRIM] = CreateNbdTrimCommand;
    creators[FLUSH] = CreateNbdFlushCommand;
    creators[RESPONSE] = CreateResponseCommand;

    Framework<CommandType> framework(dll_folder, callbacks, creators);

    std::cout << "Run Framework" << std::endl;
    framework.Start();
    std::cout << "Test Nbd" << std::endl;
}

int main()
{
    TestMaster();
    return 0;
}