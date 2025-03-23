#include <iostream>
#include "logger.hpp"
#include <dlfcn.h>
//gpp17d -Iinclude ./test/Logger_test.cpp ./src/Logger.cpp ./src/Thread_Pool.cpp -lsingleton_logger -L. -Wl,--rpath=.

int main()
{
    void* handle = dlopen("./libsingleton_logger.so", RTLD_LAZY);
    Logger* l = Singleton<Logger, std::string>::GetInstance("log.txt");
    dlsym(handle, "ZN4ilrd9SingletonINS_6LoggerEJNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEEEE11GetInstanceES7");
    if (!handle)
    {
        std::cerr << "Cannot open library: " << dlerror() << std::endl;
        return 1;
    }

    dlclose(handle);

    return 0;
}

