/**
 *   @file: hub.cpp
 *
 *   @date: Feb 22, 2017
 * @author: Mateusz Midor
 */

#include <iostream>
#include "MessageHub.h"

using namespace std;
using namespace messagebusipc;


//====================================================================================================
// Program entry point
//====================================================================================================
int main() {
    // run MessageHub in current thread (blocking run)
    MessageHub::runAndForget(false);
    return 0;
}
