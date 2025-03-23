#include <thread>
#include <cassert>
#include <chrono>
#include <memory>
#include <iostream>

#include "scheduler.hpp"


class SchedulerTask : public Scheduler::ISchedulerTask
{
public:
    SchedulerTask(START_TIME start_time) :Scheduler::ISchedulerTask(start_time)
    {
    }
    void Run()
    {
        std::cout << "Task Scheduler RUN1\n";
    }
};

class SchedulerTask1 : public Scheduler::ISchedulerTask
{
public:
    SchedulerTask1(START_TIME start_time) :Scheduler::ISchedulerTask(start_time)
    {
    }
    void Run()
    {
        std::cout << "Task Scheduler RUN2\n";
    }
};

class SchedulerTask2 : public Scheduler::ISchedulerTask
{
public:
    SchedulerTask2(START_TIME start_time) :Scheduler::ISchedulerTask(start_time)
    {
    }
    void Run()
    {
        std::cout << "Task Scheduler RUN3\n";
    }
};

class MockSchedulerTask : public Scheduler::ISchedulerTask
{
public:
    MockSchedulerTask(START_TIME start_time) : ISchedulerTask(start_time), executed(false) {}

    void Run() override
    {
        executed = true;
    }

    bool HasExecuted() const
    {
        return executed;
    }

private:
    bool executed;
};

using namespace std::chrono_literals;

void BasicTest()
{
    Scheduler* sch = Singleton<Scheduler>::GetInstance();
    SchedulerTask task1(std::chrono::system_clock::now() + 10s);
    SchedulerTask1 task2(std::chrono::system_clock::now() + 1s);
    SchedulerTask2 task3(std::chrono::system_clock::now() + 5s);
    sch->AddTask(std::make_shared<SchedulerTask>(task1));
    sch->AddTask(std::make_shared<SchedulerTask1>(task2));
    sch->AddTask(std::make_shared<SchedulerTask2>(task3));

    std::this_thread::sleep_for(12s);
}


void TestAddTask_EmptyQueue()
{
    Scheduler* scheduler = Singleton<Scheduler>::GetInstance();
    auto start_time = std::chrono::system_clock::now() + std::chrono::seconds(1);
    auto task1 = std::make_shared<MockSchedulerTask>(start_time);

    scheduler->AddTask(task1);

    // Check if the task is scheduled
    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for the task to be executed
    assert(task1->HasExecuted() && "Task was not executed!");

    std::cout << "TestAddTask_EmptyQueue passed!" << std::endl;
}

void TestAddTask_MultipleTasks()
{
    Scheduler* scheduler = Singleton<Scheduler>::GetInstance();
    auto now = std::chrono::system_clock::now();

    auto task1 = std::make_shared<MockSchedulerTask>(now + std::chrono::seconds(2));
    auto task2 = std::make_shared<MockSchedulerTask>(now + std::chrono::seconds(1));

    scheduler->AddTask(task1);
    scheduler->AddTask(task2);

    // Wait for tasks to be executed
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Task2 should be executed first, followed by Task1
    assert(task2->HasExecuted() && "Task2 was not executed!");
    assert(task1->HasExecuted() && "Task1 was not executed!");

    std::cout << "TestAddTask_MultipleTasks passed!" << std::endl;
}

void TestNoTasks()
{
    Scheduler* scheduler = Singleton<Scheduler>::GetInstance();

    (void)scheduler; // avoid compiler warning

    std::this_thread::sleep_for(std::chrono::seconds(2)); // Wait for any unintended executions

    // No assertions needed; just ensure no crashes or errors occur
    std::cout << "TestNoTasks passed!" << std::endl;
}

int main()
{
    BasicTest();

    std::cout << "Basic test passed!" << std::endl;

    TestAddTask_EmptyQueue();
    TestAddTask_MultipleTasks();
    TestNoTasks();

    std::cout << "All tests passed!" << std::endl;

    return 0;
}
