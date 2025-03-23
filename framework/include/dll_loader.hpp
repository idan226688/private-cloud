#ifndef __DLL_LOADER_HPP__
#define __DLL_LOADER_HPP__

#include <string> //std::string
#include <vector> // vector
#include "publisher.hpp" //publisher


class DllLoader : public ISubscriber<std::string>
{
public:
    ~DllLoader() noexcept;
    void Update(const std::string& path) override;
    void OnDeathFunction() override;

private:
    std::vector<void*> m_open_dll;
}; //class DllLoader


#endif // __DLL_LOADER_HPP__