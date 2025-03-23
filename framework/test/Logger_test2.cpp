#define __INIT_SYMBOL
#include <iostream>
#include "logger.hpp"
#include <dlfcn.h>
//gpp17d -Iinclude ./test/Logger_test2.cpp ./src/Logger.cpp ./src/Thread_Pool.cpp

int main()
{
    Logger* l = Singleton<Logger, std::string>::GetInstance("log.txt");
    l->Log(Logger::ERROR, "ayo");

    return 0;
}

