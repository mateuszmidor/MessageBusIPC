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
    MessageHub::runAndForget(true);

    cout << "MessageHub running in own thread. Press any key to exit..." << endl;
    cin.get();
    cout << "MessageHub exit" << endl;

    return 0;
}
