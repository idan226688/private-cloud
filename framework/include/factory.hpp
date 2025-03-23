#ifndef __FACTORY_HPP__
#define __FACTORY_HPP__

#include <functional>   // std::function
#include <memory>       // std::shared_ptr
#include <unordered_map> // std::unordered_map
#include <stdexcept>        // std::runtime_error

#include "singleton.hpp" //singelton

template <typename KEY, typename IBASE, typename... ARGS>
class Factory
{
using CreateFunc = std::function<IBASE*(ARGS...)>;
public:


        void Add(KEY key, CreateFunc createFunction);
        std::shared_ptr<IBASE> Create(KEY key, ARGS... args);

private:
        friend class Singleton<Factory>;
        std::unordered_map<KEY, CreateFunc> m_commandMap;
        Factory();
        ~Factory();

}; // Class Factory

template <typename KEY, typename IBASE, typename... ARGS>
inline Factory<KEY, IBASE, ARGS...>::Factory()
{
}

template <typename KEY, typename IBASE, typename... ARGS>
inline void Factory<KEY, IBASE, ARGS...>::Add(KEY key, CreateFunc createFunction)
{
    m_commandMap[key] = createFunction;
}

template <typename KEY, typename IBASE, typename... ARGS>
inline std::shared_ptr<IBASE> Factory<KEY, IBASE, ARGS...>::Create(KEY key, ARGS... args)
{
    if (m_commandMap.find(key) == m_commandMap.end())
    {
        throw std::runtime_error("Constructor for key not found.");
    }
    return std::shared_ptr<IBASE>(m_commandMap[key](std::forward<ARGS>(args)...));
}

template <typename KEY, typename IBASE, typename... ARGS>
inline Factory<KEY, IBASE, ARGS...>::~Factory()
{
}

#endif // __FACTORY_HPP__