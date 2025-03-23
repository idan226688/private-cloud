#include <iostream>
#include "nbd_proxy2.hpp"

int main()
{
    try
    {
        std::string nbdDevicePath = "/dev/nbd0";
        size_t blockSize = 4096;
        size_t numBlocks = 256 * 1024; 

        ilrd::NBDProxy nbdProxy(nbdDevicePath, blockSize, numBlocks);
        ilrd::NBDData data(nbdProxy.GetFD());
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
