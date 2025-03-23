#include <iostream>
#include <unistd.h>

#include "framework.hpp"
#include "framework_interfaces.hpp"

class TaskDataPipe : public ITaskData<int>
{
public:
    explicit TaskDataPipe(int key, const std::string& data) : m_key(key), m_data(data) {}

    int GetKey() const override
    {
        return m_key;
    }

    std::string GetData()
    {
        return m_data;
    }

private:
    int m_key;
    std::string m_data;

}; //class TaskDataPipe

class PipeInputProxy : public IInputProxy<int>
{
public:
    PipeInputProxy(int fd) : m_fd(fd) {}

    const ITaskData<int>* GetTaskData() override
    {
        char buffer[100];
        ssize_t bytesRead = read(m_fd, buffer, sizeof(buffer) - 1);
        buffer[bytesRead] = '\0';

        return new TaskDataPipe(1, std::string(buffer));
    }

private:
    int m_fd;

}; //class PipeInputProxy

class StdinInputProxy : public IInputProxy<int>
{
public:
    StdinInputProxy(int fd) : m_fd(fd) {}

    const ITaskData<int>* GetTaskData() override
    {
        char buffer[100];
        ssize_t bytesRead = read(m_fd, buffer, sizeof(buffer) - 1);
        buffer[bytesRead] = '\0';

        return new TaskDataPipe(2, std::string(buffer));
    }

private:
    int m_fd;

}; //class PipeInputProxy

class PrintCommand : public ICommand
{
public:
    PrintCommand(std::string arg) : m_arg(arg) {}

    void Run() override
    {
        std::cout << "Executing command: " << m_arg << std::endl;
    }

private:
    std::string m_arg;

}; //class PrintCommand

class StopCommand : public ICommand
{
public:
    StopCommand(std::string arg) : m_arg(arg) {}

    void Run() override
    {
        if (m_arg.compare("quit"))
        {
            std::cout << "Executing Stop command. " << std::endl;
            Reactor* reactor = Singleton<Reactor>::GetInstance();
            reactor->Stop();
        }
    }

private:
    std::string m_arg;

}; //class PrintCommand


static void PipeTest()
{
    int pipefds[2];
    if (-1 == pipe(pipefds))
    {
        std::cerr << "Failed to create pipe" << std::endl;
        return;
    }

    std::string pluginFolder("plugins");

    PipeInputProxy* inputProxy = new PipeInputProxy(pipefds[0]);
    StdinInputProxy* stdin_inputProxy = new StdinInputProxy(STDIN_FILENO);

    Framework<int>::CallbackMap callBacks = {
        {{pipefds[0], Reactor::READ}, inputProxy},
        {{STDIN_FILENO, Reactor::READ}, stdin_inputProxy},
    };

    Framework<int>::CreatorsMap creators = {
        {1, [](ITaskData<int>* data)
        {
            return new PrintCommand((dynamic_cast<TaskDataPipe*>(data))->GetData());;
        }},
        {2, [](ITaskData<int>* data)
        {
            return new StopCommand((dynamic_cast<TaskDataPipe*>(data))->GetData());
        }}
    };

    Framework<int> framework(pluginFolder, callBacks, creators);

    std::thread thread([pipefds](){
        sleep(3);
        write(pipefds[1], "hello", 5);
        });

    framework.Start();

}

int main()
{
    PipeTest();

    return 0;
}
