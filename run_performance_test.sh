#!/bin/bash

#perf record -g ./build/message_bus_ipc_performance_test/hub_performancetest&
./build/message_bus_ipc_performance_test/hub_performancetest&

sleep 1

./build/message_bus_ipc_performance_test/client_performancetest&
./build/message_bus_ipc_performance_test/client_performancetest&
./build/message_bus_ipc_performance_test/client_performancetest&
./build/message_bus_ipc_performance_test/client_performancetest 100000 1

sleep 1

killall hub_performancetest
