#include <iostream>
#include <thread>
#include <unistd.h>  
#include <cassert>

#include "thread_pool.hpp"
#include "factory.hpp"

void test(void)
{
   std::cout << "SHALOM" << std::endl;
}

class SimpleTask : public ThreadPool::ITask
{
public:
    SimpleTask(int& counter) : m_counter(counter) {}
    void Run() override
    {
        ++m_counter;
    }

private:
    int& m_counter;
};

void STests()
{
    std::cout << "start\n";
    std::shared_ptr<ThreadPool::FunctionTask> ft = std::shared_ptr<ThreadPool::FunctionTask>(new ThreadPool::FunctionTask(test));
   // std::shared_ptr<ThreadPool::FutureTask<int>> ft2(new ThreadPool::FutureTask<int>(test2));
    ThreadPool t(4);
    // int ThreadPool::HIGH = 10;

    Factory<char*, ThreadPool::ITask> f;


    for (size_t i = 0; i < 20; i++)
    {
        f.Add("simple", SimpleTask)   
    }
    
    // std::cout << "before resume\n";
    std::cout << "before_pause\n";

    t.Pause();
    t.Resume();
    std::cout << "after_pause\n";

    std::cout << "destruct\n";
}


void StressTest()
{
    for (int i = 0; i < 5000; ++i)
    {
        // testThreadPool();
        for (int j = 0; j < 5000; ++j)
        {
            STests();
        }
        std::cout << "test num : " << i * 50 << std::endl;
    }
    std::cout << "All tests passed successfully!" << std::endl;
}

int main()
{
    StressTest();

    return 0;
}