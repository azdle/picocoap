#!/bin/sh

modality sut delete picocoap-client --force

make clean
cmake .
make

modality sut create .
modality sut use picocoap-client

modality session open $1 picocoap-client
modality session use $1

./picocoap-client &

pccpid=$!

sleep 0.5

modality mutate "send-skip@CLIENT_MAIN_PROCESS_PROBE" send-skip=true

wait $pccpid

modality session mutation list

sleep 2

modality session close $1
