#!/bin/bash
killall tty_net
make clean
make
./tty_net tty_net.conf &
sleep 1
./tty_net status
