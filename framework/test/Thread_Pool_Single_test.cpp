#include <iostream>
#include <thread>
#include <unistd.h>  
#include <cassert>
#include "thread_Pool.hpp"
#include <dlfcn.h>
//gpp11d -Iinclude ./test/Thread_Pool_Single_test.cpp -lsingleton -L.

int main()
{
    void* handle = dlopen("./libsingleton.so", RTLD_LAZY);
    ThreadPool* st = Singleton<ThreadPool>::GetInstance();
    dlsym(handle, "tp");
    if (!handle)
    {
        std::cerr << "Cannot open library: " << dlerror() << std::endl;
        return 1;
    }

    dlclose(handle);

    return 0;
}

