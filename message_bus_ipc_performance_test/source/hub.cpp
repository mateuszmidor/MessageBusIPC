/**
 *   @file: hub.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include "MessageHub.h"

using namespace messagebusipc;


int main() {
    MessageHub::runAndForget(false);
    return 0;
}
