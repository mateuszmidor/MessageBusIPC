/**
 *   @file: client.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <thread>
#include <iostream>
#include <string>
#include "MessageClient.h"

using namespace std;
using namespace messagebusipc;

//====================================================================================================
// Declaration
//====================================================================================================
class MessageBusTermianalExample {
public:
    MessageBusTermianalExample();
    void startInteractiveSession();

private:
    MessageClient client;
    int n_prints;
    bool onMessage(uint32_t &id, char *data, uint32_t &size);
};

//====================================================================================================
// Definition
//====================================================================================================
MessageBusTermianalExample::MessageBusTermianalExample() {
    auto thread_func = [this](){ client.initializeAndListenMemberFunc(this, &MessageBusTermianalExample::onMessage, "Terminal"); };
    std::thread t = std::thread(thread_func);
    t.detach();
    n_prints = 1;
    printf("You can start typing now...\n");
}

void MessageBusTermianalExample::startInteractiveSession() {
    string s;
    do {
        getline(cin, s);
        client.send(50, s.c_str(), s.length() + 1, "Terminal"); // +1 for null
    } while (s != "exit");
}

bool MessageBusTermianalExample::onMessage(uint32_t &id, char *data, uint32_t &size) {
    printf("[%d] %s\n", n_prints, data);
    n_prints++;
    return true;
}


//====================================================================================================
// Program entry point
//====================================================================================================
int main(int argc, char** argv) {
    MessageBusTermianalExample terminal;
    terminal.startInteractiveSession();

    return 0;
}
