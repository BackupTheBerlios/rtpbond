#!/bin/bash
killall rtpbondd
make clean
make
./rtpbondd rtpbondd.conf &
top && killall rtpbondd
