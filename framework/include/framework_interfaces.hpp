#ifndef __FRAMEWORK_INTERFACES_HPP
#define __FRAMEWORK_INTERFACES_HPP

class ICommand
{
public:
    virtual ~ICommand() noexcept = default;

    virtual void Run() = 0;

}; // class ICommand

template <class KEY>
class ITaskData
{
public:
    virtual ~ITaskData() noexcept = default;

    virtual KEY GetKey() const = 0;

}; // class ITaskData

template <class KEY>
class IInputProxy
{
public:
    virtual ~IInputProxy() noexcept = default;

    virtual const ITaskData<KEY>* GetTaskData() = 0;
}; // class IInputProxy


#endif // __FRAMEWORK_INTERFACES_HPP
