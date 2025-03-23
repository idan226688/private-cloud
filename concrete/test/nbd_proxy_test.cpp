#include <iostream>

#define __INIT_SYMBOL

#include "nbd_proxy.hpp" // nbd proxy header
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

void TestNbd()
{
    const std::string &dll_folder = "./include";

    std::map<std::pair<int, Reactor::Mode>, IInputProxy<CommandType>*> callbacks;

    // callback
    NBDProxy* nbd_proxy = new NBDProxy("/dev/nbd0", 4096, 256 * 1024);

    callbacks[std::make_pair(nbd_proxy->GetFD(), Reactor::Mode::READ)] = nbd_proxy;

    std::map<CommandType, std::function<ICommand*(ITaskData<CommandType>*)>> creators;

    creators[READ] = CreateNbdReadCommand;
    creators[WRITE] = CreateNbdWriteCommand;
    creators[DISCONNECT] = CreateNbdDiscCommand;
    creators[TRIM] = CreateNbdTrimCommand;
    creators[FLUSH] = CreateNbdFlushCommand;

    Framework<CommandType> framework(dll_folder, callbacks, creators);

    std::cout << "Run Framework" << std::endl;
    framework.Start();
    std::cout << "Test Nbd" << std::endl;
}

int main()
{
    TestNbd();
    return 0;
}