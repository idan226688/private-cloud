#include <iostream>
#include <thread>
#include<unistd.h>  
#include <cassert>
#include "waitable_queue.hpp"

void BasicunitTest();
void MultiThreadTest();
void MultiThreadNotimeoutTest();

int main()
{
        std::cout << "Basic units:" << std::endl;
        BasicunitTest();

        std::cout << "Multi Thread time out Test:" << std::endl;
        MultiThreadTest();

        std::cout << "Multi Thread no time out Test:" << std::endl;
        MultiThreadNotimeoutTest();
}

void WriteToQue(WaitableQueue<int, std::queue<int>>& int_wq, int i, int id)
{
        int x = i + 10;

        for (; i < x; ++i)
        {
                int_wq.Push(i * 100 + id);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << "wait: " << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        for (i = 0; i < x; ++i)
        {
                int_wq.Push(i * 100 + id);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
}
void ReadFromQue(WaitableQueue<int, std::queue<int>>& int_wq, int id)
{

        int out_param = 0;
        int counter = 0;
        while (counter < 20)
        {

                ++counter;
                int_wq.Pop(out_param, std::chrono::milliseconds(1000));

                std::cout << "reader id: " << id << "_" << out_param << " " << counter << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
}

void WriteToQue1(WaitableQueue<int, std::queue<int>>& int_wq, int i, int id)
{
        int x = i + 10;

        for (; i < x; ++i)
        {
                int_wq.Push(i * 100 + id);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << "wait: " << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        for (i = 0; i < x; ++i)
        {
                int_wq.Push(i * 100 + id);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
}
void ReadFromQue1(WaitableQueue<int, std::queue<int>>& int_wq, int id)
{

        int out_param = 0;
        int counter = 0;
        while (counter < 20)
        {

                ++counter;
                int_wq.Pop(out_param);

                std::cout << "reader id: " << id << "_" << out_param << " " << counter << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
}

void MultiThreadTest()
{

        WaitableQueue<int, std::queue<int>> int_wq;
        std::thread t1(WriteToQue, std::ref(int_wq), 1, 1), t2(ReadFromQue, std::ref(int_wq), 1), t3(ReadFromQue, std::ref(int_wq), 2), t4(ReadFromQue, std::ref(int_wq), 3);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
}

void MultiThreadNotimeoutTest()
{

        WaitableQueue<int, std::queue<int>> int_wq;
        std::thread t1(WriteToQue1, std::ref(int_wq), 1, 1), t2(ReadFromQue1, std::ref(int_wq), 1), t3(ReadFromQue1, std::ref(int_wq), 2), t4(ReadFromQue1, std::ref(int_wq), 3);
        t1.join();
        t2.join();
        t3.join();
        t4.join();
}

void BasicunitTest()
{

        WaitableQueue<int, std::queue<int>> int_wq;
        int test = 4;
        int outparam = 0;
        int test_num = 1;


        std::cout << "TEST" << test_num++ << " Creating a queue and check if empty: ";
        if (int_wq.IsEmpty() == true)
        {
                std::cout << "PASSED" << std::endl;
        }
        else
        {
                std::cout << "Fail" << std::endl;
        }

        std::cout << "TEST" << test_num++ << " pushing item: ";
        int_wq.Push(test);
        if (int_wq.IsEmpty() == false)
        {
                std::cout << "PASSED" << std::endl;
        }
        else
        {
                std::cout << "Fail: didnt pushed an item" << std::endl;
        }

        std::cout << "TEST" << test_num++ << " pop item no timeout: ";
        int_wq.Pop(outparam);
        if (outparam == test)
        {
                std::cout << "PASSED" << std::endl;
        }
        else
        {
                std::cout << "Fail: out param is " << outparam << ", expected: " << test << std::endl;
        }

        std::cout << "      - Check empty: ";

        if (int_wq.IsEmpty() == true)
        {
                std::cout << "PASSED" << std::endl;
        }
        else
        {
                std::cout << "Fail: didnt pushed an item" << std::endl;
        }


        std::cout << "TEST" << test_num++ << " pop item from empty queue time out: ";
        std::chrono::milliseconds ms(3000);

        if (int_wq.Pop(outparam, ms) == false)
        {
                std::cout << "PASSED" << std::endl;
        }
        else
        {
                std::cout << "Fail: should wait for 3 sec" << std::endl;
        }

        std::cout << "TEST" << test_num++ << " pop item from queue time out: ";
        test = 6;
        int_wq.Push(test);
        if (int_wq.Pop(outparam, ms) == true && outparam == test)
        {
                std::cout << "PASSED" << std::endl;
        }
        else
        {
                std::cout << "Fail: should pop an item" << std::endl;
        }

}


template<typename T>
void producer(WaitableQueue<T>& q, T start, int count) 
{
    for (int i = 0; i < count; ++i) 
    {
        q.Push(start);
    }
}

template<typename T>
void consumer(WaitableQueue<T>& q, int totalItems) 
{
    T item;
    for (int i = 0; i < totalItems; ++i) 
    {
        q.Pop(item);
        std::cout << "Poopped: " << item << std::endl;
    }
}


void IntMultiThreadTest()
{
    WaitableQueue<int> q;
    std::thread cons(consumer<int>, std::ref(q), 100);
    std::thread prod1(producer<int>, std::ref(q), 0, 50);
    std::thread prod2(producer<int>, std::ref(q), 50, 50);

    prod1.join();
    prod2.join();
    cons.join();

    assert(q.IsEmpty());
    std::cout << "Int Multi-Threaded Test finished successfully, hurray!" << std::endl;
}

void StringMultiThreadTest()
{
    WaitableQueue<std::string> strQ;

    std::thread strCons(consumer<std::string>, std::ref(strQ), 100);
    std::thread strProd1(producer<std::string>, std::ref(strQ), std::string("String "), 50);
    std::thread strProd2(producer<std::string>, std::ref(strQ), std::string("String 50 "), 50);

    strProd1.join();
    strProd2.join();
    strCons.join();

    assert(strQ.IsEmpty());
    std::cout << "String Multi-Threaded Test finished successfully, hurray!" << std::endl;

}
