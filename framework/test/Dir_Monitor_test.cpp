#include <iostream>
#include <unistd.h>

#include "dir_monitor.hpp"

int main()
{
    const std::string dirPath = ".";
    DirMonitor dm(dirPath);
    DllLoader dl;

    dm.Subscribe(&dl);

    dm.RunMonitor();


    sleep(10);


    return 0;
}
