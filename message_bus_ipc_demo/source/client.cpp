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


const char* CHAT_CLIENT_NAME = "ChatClient";

//====================================================================================================
// Declaration
//====================================================================================================
class ChatClient {
public:
    ChatClient();
    void startInteractiveSession();

private:
    MessageClient client;
    int curr_msg_number;
    bool onMessage(uint32_t &id, char *data, uint32_t &size);
};

//====================================================================================================
// Definition
//====================================================================================================
ChatClient::ChatClient() : curr_msg_number(1) {
    // register to the MessageHub as "ChatClient" and run listener in a separate thread
    auto thread_func = [this]() {client.initializeAndListenMemberFunc(this, &ChatClient::onMessage, CHAT_CLIENT_NAME);};
    std::thread t = std::thread(thread_func);
    t.detach();
}

void ChatClient::startInteractiveSession() {
    // read user input from terminal and send to other clients, over and over again
    printf("You can start typing now...\n");
    string s;
    do {
        getline(cin, s);
        client.waitForClient(CHAT_CLIENT_NAME); // wait until there is some ChatClient to send the msg to
        client.send(50, s.c_str(), s.length() + 1, CHAT_CLIENT_NAME); // +1 for null
    } while (s != "exit");
}

bool ChatClient::onMessage(uint32_t &id, char *data, uint32_t &size) {
    switch (id) {
    case ID_CLIENT_SAYS_HELLO:
        //printf("%s connected\n", data);
        break;

    case ID_CLIENT_SAYS_GOODBYE:
        //printf("%s disconnected\n", data);
        break;

    default:
        // print incoming message and return true to continue the listener work
        printf("[%d] %s\n", curr_msg_number, data);
        curr_msg_number++;
        break;

    }
    return true;
}


//====================================================================================================
// Program entry point
//====================================================================================================
int main(int argc, char** argv) {
    ChatClient client;
    client.startInteractiveSession();
    return 0;
}
