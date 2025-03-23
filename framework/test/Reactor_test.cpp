#include <iostream>
#include <functional>
#include <sys/socket.h>
#include <unistd.h>
#include <cassert>
#include <thread>
#include <chrono>

#include "reactor.hpp"

void TestAddAndRemoveReadHandler()
{
    Reactor reactor;
    bool readCalled = false;
    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    reactor.Add(fds[0], Reactor::READ, [&readCalled, &fds]() {
        readCalled = true;
        char buf[1];
        read(fds[0], buf, 1);
        });

    // Write to the other end to trigger the read handler
    assert(write(fds[1], "x", 1) == 1);

    // Run the reactor in a separate thread
    std::thread reactorThread([&reactor]() {
        reactor.Run();
        });

    // Allow some time for the reactor to process the event
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    reactor.Stop();
    reactorThread.join();

    assert(readCalled);

    // Remove the read handler and ensure it is not called again
    readCalled = false;
    reactor.Remove(fds[0], Reactor::READ);

    assert(write(fds[1], "y", 1) == 1);

    reactorThread = std::thread([&reactor]() {
        reactor.Run();
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    reactor.Stop();
    reactorThread.join();

    assert(!readCalled);

    close(fds[0]);
    close(fds[1]);

    std::cout << "TestAddAndRemoveReadHandler passed!" << std::endl;
}

void TestAddAndRemoveWriteHandler()
{
    Reactor reactor;
    bool writeCalled = false;
    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    reactor.Add(fds[0], Reactor::WRITE, [&writeCalled]() {
        writeCalled = true;
        });

    // Run the reactor in a separate thread
    std::thread reactorThread([&reactor]() {
        reactor.Run();
        });

    // Allow some time for the reactor to process the event
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    reactor.Stop();
    reactorThread.join();

    assert(writeCalled);

    // Remove the write handler and ensure it is not called again
    writeCalled = false;
    reactor.Remove(fds[0], Reactor::WRITE);

    reactorThread = std::thread([&reactor]() {
        reactor.Run();
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    reactor.Stop();
    reactorThread.join();

    assert(!writeCalled);

    close(fds[0]);
    close(fds[1]);

    std::cout << "TestAddAndRemoveWriteHandler passed!" << std::endl;
}

void TestMultipleHandlers()
{
    Reactor reactor;
    bool readCalled = false;
    bool writeCalled = false;
    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    reactor.Add(fds[0], Reactor::READ, [&readCalled, &fds]() {
        readCalled = true;
        char buf[1];
        read(fds[0], buf, 1);
        });

    reactor.Add(fds[0], Reactor::WRITE, [&writeCalled]() {
        writeCalled = true;
        });

    // Write to the other end to trigger the read handler
    assert(write(fds[1], "x", 1) == 1);

    // Run the reactor in a separate thread
    std::thread reactorThread([&reactor]() {
        reactor.Run();
        });

    // Allow some time for the reactor to process the event
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    reactor.Stop();
    reactorThread.join();

    assert(readCalled);
    assert(writeCalled);

    close(fds[0]);
    close(fds[1]);

    std::cout << "TestMultipleHandlers passed!" << std::endl;
}

void TestRemoveNextCallback()
{
    Reactor reactor;
    bool firstCallbackCalled = false;
    bool secondCallbackCalled = false;
    int fds[2];
    assert(socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0);

    // Add the second callback first
    reactor.Add(fds[0], Reactor::READ, [&secondCallbackCalled]() {
        secondCallbackCalled = true;
        });

    // Add the first callback that removes the second one
    reactor.Add(fds[0], Reactor::READ, [&firstCallbackCalled, &secondCallbackCalled, &reactor, &fds]() {
        firstCallbackCalled = true;
        reactor.Remove(fds[0], Reactor::READ);
        });

    // Write to the other end to trigger the read handler
    assert(write(fds[1], "x", 1) == 1);

    // Run the reactor in a separate thread
    std::thread reactorThread([&reactor]() {
        reactor.Run();
        });

    // Allow some time for the reactor to process the event
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    reactor.Stop();
    reactorThread.join();

    assert(firstCallbackCalled);
    assert(!secondCallbackCalled);

    close(fds[0]);
    close(fds[1]);

    std::cout << "TestRemoveNextCallback passed!" << std::endl;
}

int main()
{
    TestAddAndRemoveReadHandler();
    TestAddAndRemoveWriteHandler();
    TestMultipleHandlers();
    TestRemoveNextCallback();

    std::cout << "All tests passed!" << std::endl;

    return 0;
}