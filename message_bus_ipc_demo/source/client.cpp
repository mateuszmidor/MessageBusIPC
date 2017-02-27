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


class Client {
public:
    Client() {
        auto thread_func = [this](){ client.initializeAndListenMemberFunc(this, &Client::onMessage); };
        std::thread t = std::thread(thread_func);
        t.detach();
        n_prints = 0;
    }

    void sendBunchOfMessages(const char* str) {
        for (int i = 0; i < 10; i++) {
            client.send(50, str, strlen(str) + 1); // +1 for null
            sleep(1);
        }
    }

    void startInteractiveSession() {
        string s;
        do {
            getline(cin, s);
            client.send(50, s.c_str(), s.length() + 1); // +1 for null
        } while (s != "exit");
    }

private:
    MessageClient client;
    int n_prints;

    bool onMessage(uint32_t &id, char *data, uint32_t &size) {
        printf("%d. %s\n", n_prints, data);
        n_prints++;
        return true;
    }
};


int main(int argc, char** argv) {
    Client client;

    if (argc > 1)
        client.sendBunchOfMessages(argv[1]);
    else
        client.startInteractiveSession();

    return 0;
}
