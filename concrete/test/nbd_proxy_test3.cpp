// #include <iostream>
// #include "nbd_proxy.hpp"
// #include "framework.hpp"

// using namespace ilrd;

// static char* data;

// template <typename KEY>
// class NBDReadCommand : public ICommand
// {
// public:
//     NBDReadCommand(ilrd::ITaskData<KEY>* task):m_task(task)
//     {
//     }
//     void Run()
//     {
//         NBDData<KEY>* task = dynamic_cast<NBDData<KEY>*>(m_task);
        
//         char logdata[1000];
//         sprintf(logdata,"Entered ReadCommand with arguements\nSocketFD = %d\nLen = %ld\nOffset = %ld\nReplyMagic = %u",task->GetSockFD(),task->GetLen(),task->GetOffset(),task->GetReply().magic);
//         std::string log= logdata;
//         logger->Log(Logger::DEBUG,log);

//         char *buffer = task->GetBuffer();
//         memcpy(buffer ,data + task->GetOffset(),task->GetLen());
//         task->GetProxy()->SendReply(task,buffer);

//         delete[] buffer;
        
//     }
// private:
//     ITaskData<KEY>* m_task;
// };

// template <typename KEY>
// class NBDWriteCommand : public ICommand
// {
// public:
//     NBDWriteCommand(ilrd::ITaskData<KEY>* task):m_task(task)
//     {
//     }
//     void Run()
//     {
//         char logdata[1000];
//         NBDData<KEY>* task = dynamic_cast<NBDData<KEY>*>(m_task);
//         sprintf(logdata,"Entered ReadCommand with arguements\nSocketFD = %d\nLen = %ld\nOffset = %ld\nReplyMagic = %u",task->GetSockFD(),task->GetLen(),task->GetOffset(),task->GetReply().magic);
//         std::string log= logdata;
//         logger->Log(Logger::DEBUG,log);

//         char* buffer = task->GetBuffer();
//         ReadAll(task->GetSockFD(), buffer, task->GetLen());
//         memcpy(data + task->GetOffset(),buffer,task->GetLen());
//         task->GetProxy()->SendReply(task);


//         delete[] buffer;
//     }
// private:
//     ITaskData<KEY>* m_task;
// };

// template <typename KEY>
// class NBDFlushCommand : public ICommand
// {
// public:
//     NBDFlushCommand(ilrd::ITaskData<KEY>* task):m_task(task)
//     {
//     }
//     void Run()
//     {
//         char logdata[1000];
//         NBDData<KEY>* task = dynamic_cast<NBDData<KEY>*>(m_task);
//         sprintf(logdata,"Entered ReadCommand with arguements\nSocketFD = %d\nLen = %ld\nOffset = %ld\nReplyMagic = %u",task->GetSockFD(),task->GetLen(),task->GetOffset(),task->GetReply().magic);
//         std::string log= logdata;
//         logger->Log(Logger::DEBUG,log);

//         task->GetProxy()->SendReply(task);


//         delete[] task->GetBuffer();
//     }
// private:
//     ITaskData<KEY>* m_task;
// };

// template <typename KEY>
// class NBDDisconnectCommand : public ICommand
// {
// public:
//     NBDDisconnectCommand(ilrd::ITaskData<KEY>* task):m_task(task)
//     {
//     }
//     void Run()
//     {
//         char logdata[1000];
//         NBDData<KEY>* task = dynamic_cast<NBDData<KEY>*>(m_task);
//         sprintf(logdata,"Entered ReadCommand with arguements\nSocketFD = %d\nLen = %ld\nOffset = %ld\nReplyMagic = %u",task->GetSockFD(),task->GetLen(),task->GetOffset(),task->GetReply().magic);
//         std::string log= logdata;
//         logger->Log(Logger::DEBUG,log);


//         delete[] task->GetBuffer();

//     }
// private:
//     ITaskData<KEY>* m_task;
// };

// template <typename KEY>
// class NBDTrimCommand : public ICommand
// {
// public:
//     NBDTrimCommand(ilrd::ITaskData<KEY>* task):m_task(task)
//     {
//     }
//     void Run()
//     {
//         char logdata[1000];
//         NBDData<KEY>* task = dynamic_cast<NBDData<KEY>*>(m_task);
//         sprintf(logdata,"Entered ReadCommand with arguements\nSocketFD = %d\nLen = %ld\nOffset = %ld\nReplyMagic = %u",task->GetSockFD(),task->GetLen(),task->GetOffset(),task->GetReply().magic);
//         std::string log= logdata;
//         logger->Log(Logger::DEBUG,log);

//         task->GetProxy()->SendReply(task);

//         delete[] task->GetBuffer();
//     }
// private:
//     ITaskData<KEY>* m_task;
// };

// template <typename KEY>
// ilrd::ICommand* ReadCreator(ilrd::ITaskData<KEY>* task)
// {
//     return new NBDReadCommand<KEY>(task);
// }

// template <typename KEY>
// ilrd::ICommand* WriteCreator(ilrd::ITaskData<KEY>* task)
// {
//     return new NBDWriteCommand<KEY>(task);
// }
// template <typename KEY>
// ilrd::ICommand* FlushCreator(ilrd::ITaskData<KEY>* task)
// {
//     return new NBDFlushCommand<KEY>(task);
// }
// template <typename KEY>
// ilrd::ICommand* DisconnectCreator(ilrd::ITaskData<KEY>* task)
// {
//     return new NBDDisconnectCommand<KEY>(task);
// }
// template <typename KEY>
// ilrd::ICommand* TrimCreator(ilrd::ITaskData<KEY>* task)
// {
//     return new NBDTrimCommand<KEY>(task);
// }


// int main()
// {
//     try
//     {
//         std::map<std::pair<int, ilrd::Reactor::Mode>,  ilrd::IInputProxy<int>*> callbackMap;
//         std::map<int,std::function<ilrd::ICommand*(ilrd::ITaskData<int>*)>> creatorMap;
//         NBDProxy<int> proxy("/dev/nbd0",128*1024*1024);

//         data = new char[128*1024*1024];
//         std::memset(data,5,128*1024*1024);
        
//         creatorMap[NBD_CMD_READ] = ReadCreator<int>;
//         creatorMap[NBD_CMD_WRITE] = WriteCreator<int>;
//         creatorMap[NBD_CMD_FLUSH] = FlushCreator<int>;
//         creatorMap[NBD_CMD_DISC] = DisconnectCreator<int>;
//         creatorMap[NBD_CMD_TRIM] = TrimCreator<int>;
//         callbackMap[{proxy.GetFD(),ilrd::Reactor::Mode::READ}] = &proxy;
        
//         ilrd::Framework<int> framework("/home/plugins",callbackMap,creatorMap);

//         framework.Start();

//         delete[] data;
//     }
//     catch(const std::exception& e)
//     {
//         std::cerr << e.what() << '\n';
//     }
    


//     return 0;

// }
