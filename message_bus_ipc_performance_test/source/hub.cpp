/**
 *   @file: hub.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include "MessageHub.h"

using namespace messagebusipc;


int main() {
    MessageHub hub;
    hub.runAndForget();
    return 0;
}
