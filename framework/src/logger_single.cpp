#define __INIT_SYMBOL
#include "logger.hpp"

//g++ -Iinclude -fPIC -shared -o libsingleton_logger.so ./src/Logger_Single.cpp

extern "C"
{
    Logger* ls = Singleton<Logger, std::string>::GetInstance("log.txt");
}
