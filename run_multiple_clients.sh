#!/bin/bash

for i in {1..5}; do  ./build/message_bus_ipc_demo/client "hello$i" &    done
