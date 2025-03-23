#include <iostream> // std::cerr
#include <stdexcept> // std::runtime_error
#include <dlfcn.h> // dlopen, dlclose

#include "dll_loader.hpp" // dll loader

DllLoader::~DllLoader() noexcept
{
    OnDeathFunction();
}

void DllLoader::Update(const std::string& path)
{
    if (path.find("Created: ") != std::string::npos)
    {
        std::string dll_path = path.substr(9);

        void* handle = dlopen(dll_path.c_str(), RTLD_LAZY);
        
        if (!handle)
        {
            std::cerr << "Error loading DLL: " << dlerror() << std::endl;
            return;
        }

        m_open_dll.push_back(handle);
        std::cout << "DLL loaded: " << dll_path << std::endl;
    }

    else if (path.find("Deleted: ") != std::string::npos)
    {
        std::cout << "File deleted: " << path.substr(9) << std::endl;
    }

    else if (path.find("Modified: ") != std::string::npos)
    {
        std::string dll_path = path.substr(10);

        void* handle = dlopen(dll_path.c_str(), RTLD_LAZY);
        if (!handle)
        {
            std::cerr << "Error loading DLL: " << dlerror() << std::endl;
            return;
        }

        if (dlclose(handle) != 0)
        {
            std::cerr << "Error closing DLL: " << dlerror() << std::endl;
        }

        handle = dlopen(dll_path.c_str(), RTLD_LAZY);
        std::cout << "File modified: " << path.substr(10) << std::endl;
    }
}

void DllLoader::OnDeathFunction()
{
    for (auto handle : m_open_dll)
    {
        std::cout << "closing" << std::endl;
        if (dlclose(handle) != 0)
        {
            std::cerr << "Error closing DLL: " << dlerror() << std::endl;
        }
    }
    std::cout << "closed" << std::endl;
    m_open_dll.clear();
}