#include <iostream> // cout 
#include <thread> //thread

#include "publisher.hpp" // publisher header

template <typename NOTIFICATION>
class Subscriber : public ISubscriber<NOTIFICATION>
{
    public:
        Subscriber(Publisher<NOTIFICATION>* publisher) : ISubscriber<NOTIFICATION>(publisher){}
        ~Subscriber(){std::cout << "dead" << std::endl;}
        void Update(const NOTIFICATION &notification) override
        {
            notification.display();
        }
};

class Message
{
    public:
        explicit Message(const std::string& text) : m_text(text){}
        void display() const
        {
            std::cout << m_text << std::endl;
        }
    private:
        const std::string m_text;
};

void notify_pub(Publisher<Message>* pub)
{
    Message m("helo");
    while(1)
    {
        pub->Notify(m);
    }
}

void add_remove_subscriber(Publisher<Message>* pub)
{
    while(1)
    {
        Subscriber<Message> subsrciber(pub);
        Message m("added subscriber");
        pub->Notify(m);
        pub->Unsubscribe(&subsrciber);
        pub->Subscribe(&subsrciber);
        pub->Unsubscribe(&subsrciber);
    }
}

void TestPublisher()
{
    Publisher<Message> publisher;

    //Subscriber<Message> subsrciber1(&publisher);
    //Subscriber<Message> subsrciber2(&publisher);
    //Subscriber<Message> subsrciber3(&publisher);
    std::thread t(notify_pub, &publisher);
    std::thread tt(add_remove_subscriber, &publisher);


    Message m("ayoo");
    t.join();
    tt.join();
    publisher.Notify(m);
    while(1);
}

int main()
{
    TestPublisher();
    return 0;
}
