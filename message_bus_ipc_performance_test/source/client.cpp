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

#include <iostream>
#include <chrono>

using namespace std;
using namespace messagebusipc;


const int TERMINATE_RECEIVER = 999;


class Timer
{
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return std::chrono::duration_cast<second_>
            (clock_::now() - beg_).count(); }

private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1> > second_;
    std::chrono::time_point<clock_> beg_;
};





bool callback(uint32_t &id, char *data, uint32_t &size) {
//    printf("messageId: %d, size: %d\n", id, size);
    return (id != TERMINATE_RECEIVER);
}

void sendBunchOfMessages( MessageClient& client, int num_messages, int msg_size_in_kb) {
    char *data = new char[msg_size_in_kb * 1024];

    for (int i = 0; i < num_messages; i++)
        client.send(50, data, msg_size_in_kb * 1024);

    // terminate clients
    client.send(TERMINATE_RECEIVER, nullptr, 0);

    delete[] data;
}

void runAsReceiver(std::thread& t) {
    printf("Run as message receiver. "
            "You can run as sender by providing num messages and msg size in KB eg. ./client_performance 100 1\n");
    t.join();
}

void runAsSender(MessageClient &client, std::thread &t, char** argv) {
    // run as sender. has num messages and message size in KB params
    sleep(1); // let the hub and receivers start
    int num_messages = atoi(argv[1]);
    int message_size_in_kb = atoi(argv[2]);
    Timer timer;


    timer.reset();
    sendBunchOfMessages(client, num_messages, message_size_in_kb);
    double elapsed = timer.elapsed();


    printf("*********************************************************\n");
    printf("%d messages of %dKB transmitted in %f seconds\n", num_messages, message_size_in_kb, elapsed);
    printf("*********************************************************\n");
    t.detach();
}

int main(int argc, char** argv) {
    MessageClient client;
    std::thread t([&client]() {client.initializeAndListen(callback);});

    if (argc != 3)
        runAsReceiver(t);
    else
        runAsSender(client, t, argv);

    return 0;
}
