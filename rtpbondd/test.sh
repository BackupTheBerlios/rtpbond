#!/bin/bash
killall rtpbondd
make clean
make
./rtpbondd rtpbondd.conf &
sleep 1
./rtpbondd status
