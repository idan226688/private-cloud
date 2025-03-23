#ifndef __WAITABLE_QUEUE_HPP__
#define __WAITABLE_QUEUE_HPP__

#include <queue>                   //std::queue
#include <chrono>                  // std::chrono
#include <mutex>                  // std::timed_mutex, std::lock_guard, std::unique_lock
#include <condition_variable>     //std::conditinal_variable

/* 
Concepts:
- T should be copyable and assignable. 
- CONTAINER should support an API identical to std::queue's API: push(), pop(), front(), empty().
*/
template <typename T, typename CONTAINER = std::queue<T>>
class WaitableQueue
{
public:
     WaitableQueue() = default;
     WaitableQueue(const WaitableQueue& other) = delete;
     WaitableQueue& operator=(const WaitableQueue& other) = delete;
     WaitableQueue(WaitableQueue&& other) = delete;
     WaitableQueue& operator=(WaitableQueue&& other) = delete;
     ~WaitableQueue() = default;
     
     void Pop(T& outparam);
     bool Pop(T& outparam, const std::chrono::milliseconds& timeout);
     void Push(const T& item);
     bool IsEmpty() const;
    
private:
    CONTAINER m_queue; 
    std::condition_variable_any m_cond;
	mutable std::timed_mutex m_mutex;
    
}; // end of class WaitableQueue

template <typename T, typename CONTAINER>
inline void WaitableQueue<T, CONTAINER>::Pop(T &outparam)
{
    std::unique_lock<std::timed_mutex> lock(m_mutex);
    m_cond.wait(lock, [this] {return !m_queue.empty();});
    outparam = m_queue.front();
    m_queue.pop();
}

template <typename T, typename CONTAINER>
inline bool WaitableQueue<T, CONTAINER>::Pop(T& outparam, const std::chrono::milliseconds& timeout)
{
    std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
    auto time_to_aquire = std::chrono::steady_clock::now() + timeout;
    lock.try_lock_until(time_to_aquire);

    if (false == m_cond.wait_until(lock, time_to_aquire, [this]{return (false == m_queue.empty());})) 
    {
        return false;
    }

    outparam = m_queue.front();
    m_queue.pop();
    return true;
}

template <typename T, typename CONTAINER>
inline void WaitableQueue<T, CONTAINER>::Push(const T &item)
{
    {
        std::lock_guard<std::timed_mutex> lock(m_mutex);
        m_queue.push(item);
    }
    m_cond.notify_one();
}

template <typename T, typename CONTAINER>
inline bool WaitableQueue<T, CONTAINER>::IsEmpty() const
{
    std::lock_guard<std::timed_mutex> lock(m_mutex);
    return m_queue.empty();
}

#endif //__WAITABLE_QUEUE_HPP__