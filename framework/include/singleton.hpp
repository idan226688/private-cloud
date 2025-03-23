#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

#include <mutex>         // std::mutex
#include <atomic>        // std::atomic
#include <cstdlib>       // atexit

#define DEAD_BEEF 0xDEADBEEF

namespace ilrd
{

template <typename T, typename... ARGS>
class Singleton
{
public:
    Singleton() = delete;
    ~Singleton() = delete;
    Singleton(const Singleton& other) = delete;
    Singleton& operator=(const Singleton& other) = delete;
    Singleton(Singleton&& other) noexcept = delete;
    Singleton& operator=(Singleton&& other) noexcept = delete;

    static T* GetInstance(ARGS... args);
    static void delete_instance();

private:
    static std::atomic<T*> m_instance;
    static std::mutex creation_lock;
};
#ifdef __INIT_SYMBOL


template <typename T, typename... ARGS>
std::atomic<T*> Singleton<T, ARGS...>::m_instance{nullptr};

template <typename T, typename... ARGS>
std::mutex Singleton<T, ARGS...>::creation_lock;

template <typename T, typename... ARGS>
inline T* Singleton<T, ARGS...>::GetInstance(ARGS... args)
{
    T* tmp = m_instance.load(std::memory_order_acquire);
    if (tmp == nullptr)
    {
        std::lock_guard<std::mutex> lock(creation_lock);
        tmp = m_instance.load(std::memory_order_relaxed);
        if (tmp == nullptr)
        {
            tmp = new T(args...);
            std::atomic_thread_fence(std::memory_order_release);
            m_instance.store(tmp, std::memory_order_relaxed);
            atexit(Singleton::delete_instance);
        }
    }
    return tmp;
}

template <typename T, typename... ARGS>
inline void Singleton<T, ARGS...>::delete_instance()
{
    delete m_instance.load();
    m_instance.store((T*)DEAD_BEEF); // put dead_beef
}
#endif // __INIT_SYMBOL

#endif // __SINGLETON_HPP__
