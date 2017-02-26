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

bool callback(uint32_t &id, char *data, uint32_t &size) {
    printf("%s\n", data);
    return true;
}

void sendBunchOfMessages( MessageClient& client, char** argv) {
    for (int i = 0; i < 10; i++) {
        client.send(50, argv[1], strlen(argv[1]) + 1); // +1 for null
        sleep(1);
    }
}

void startInteractiveSession(MessageClient& client) {
    string s;
    do {
        getline(cin, s);
        client.send(50, s.c_str(), s.length() + 1);
    } while (s != "exit");
}

int main(int argc, char** argv) {
    MessageClient client;

    std::thread t([&client]() {client.initializeAndListen(callback);});
    t.detach();

    if (argc > 1)
        sendBunchOfMessages(client, argv);
    else
        startInteractiveSession(client);

    return 0;
}
