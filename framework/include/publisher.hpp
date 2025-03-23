#ifndef __PUBLISHER_HPP__
#define __PUBLISHER_HPP__

#include <vector>     // std::vector
#include <functional> // std::functional
#include <mutex>      //mutex
#include <algorithm> //find

template <typename NOTIFICATION>
class ISubscriber;

template <typename NOTIFICATION>
class Publisher
{
public:
        explicit Publisher() = default;
        ~Publisher();
        
        void Subscribe(ISubscriber<NOTIFICATION>* subscriber);
        void Unsubscribe(ISubscriber<NOTIFICATION>* subscriber);
        void Notify(const NOTIFICATION& notification);
        
        
        Publisher(const Publisher& other) = delete;
        Publisher& operator=(const Publisher& other) = delete;
        Publisher(Publisher&& other) = delete;
        Publisher& operator=(Publisher&& other) = delete;
        
private:
    std::mutex unsubscribe_mutex;
    std::vector<ISubscriber<NOTIFICATION>*> m_subscribers;
}; // Class Publisher

template <typename NOTIFICATION>
class ISubscriber
{
public:
    explicit ISubscriber(Publisher<NOTIFICATION>* publisher = nullptr);
    virtual ~ISubscriber() = 0;
    virtual void OnDeathFunction();
    
private:
    friend class Publisher<NOTIFICATION>;
    Publisher<NOTIFICATION>* m_publisher;
    void OnPublisherDeath();
    virtual void Update(const NOTIFICATION& notification) = 0;
    
}; // Class ISubscriber

template <typename NOTIFICATION>
inline Publisher<NOTIFICATION>::~Publisher()
{
    for (const auto& x: m_subscribers) 
    {
        x->OnPublisherDeath();
    }
}

template <typename NOTIFICATION>
inline void Publisher<NOTIFICATION>::Subscribe(ISubscriber<NOTIFICATION> *subscriber)
{
    auto to_add = std::find(m_subscribers.begin(), m_subscribers.end(), subscriber);
    if(to_add != m_subscribers.end())
    {
        return;
    }
    m_subscribers.push_back(subscriber);
}

template <typename NOTIFICATION>
inline void Publisher<NOTIFICATION>::Unsubscribe(ISubscriber<NOTIFICATION> *subscriber) 
{
    std::lock_guard<std::mutex> lock(unsubscribe_mutex);
    auto to_delete = find(m_subscribers.begin(), m_subscribers.end(), subscriber);
    
    if(to_delete != m_subscribers.end())
    {
        m_subscribers.erase(to_delete);
    }
}

template <typename NOTIFICATION>
inline void Publisher<NOTIFICATION>::Notify(const NOTIFICATION &notification)
{
    std::lock_guard<std::mutex> lock(unsubscribe_mutex);
    for (const auto& x: m_subscribers) 
    {
        x->Update(notification);
    }
}

template <typename NOTIFICATION>
inline void ISubscriber<NOTIFICATION>::OnPublisherDeath()
{
    m_publisher = nullptr;
}

template <typename NOTIFICATION>
inline ISubscriber<NOTIFICATION>::ISubscriber(Publisher<NOTIFICATION> *publisher) : m_publisher(publisher)
{
    if(nullptr != publisher)
    {
        publisher->Subscribe(this);
    }
}

template <typename NOTIFICATION>
inline ISubscriber<NOTIFICATION>::~ISubscriber(){}

template <typename NOTIFICATION>
inline void ISubscriber<NOTIFICATION>::OnDeathFunction()
{
    m_publisher->Unsubscribe(this);
}

#endif // __PUBLISHER_HPP__