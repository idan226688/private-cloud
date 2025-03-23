#define __INIT_SYMBOL
#include "singleton.hpp"
#include "thread_pool.hpp"

// g++ -Iinclude -fPIC -shared -o libsingleton.so ./src/Thread_Pool_Single.cpp

extern "C"
{
    ThreadPool* tp = Singleton<ThreadPool>::GetInstance();
}
