#ifndef __PRIORITY_QUEUE_HPP__
#define __PRIORITY_QUEUE_HPP__

#include <queue> // std::queue

template <typename T, typename CONTAINER = std::vector<T>,typename COMPARISON = std::less<typename CONTAINER::value_type>>
class PriorityQueue : private std::priority_queue<T,CONTAINER, COMPARISON>
{
     using priority_queue = std::priority_queue<T, CONTAINER, COMPARISON>;
public:
     PriorityQueue() = default;
     PriorityQueue(const PriorityQueue& other) = default;
     PriorityQueue& operator=(const PriorityQueue& other) = default;
     PriorityQueue(PriorityQueue&& other) = default;
     PriorityQueue& operator=(PriorityQueue&& other) = default;
     ~PriorityQueue() = default;
     
    using priority_queue::push;
    using priority_queue::pop;
    using priority_queue::empty;

    inline const T& front() const;

}; // End of class PriorityQueue

template <typename T, typename CONTAINER ,typename COMPARISON>
const T& PriorityQueue<T,CONTAINER, COMPARISON>::front() const
{
    return priority_queue::top();
}

#endif //__PRIORITY_QUEUE_HPP__