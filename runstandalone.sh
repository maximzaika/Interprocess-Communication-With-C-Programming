#!/bin/bash
#BASH SCRIPT FOR RUNNING SOCKET SERVER PROGRAM IN A STANDALONE ENVIRONMENT

#please run as root user
if [[ $EUID -ne 0 ]]; then
    echo "This script must be run using sudo"
    exit 1
fi

killall -u root hiped 2> /dev/null; rm /run/hipe.socket 2> /dev/null;
xinit /usr/local/sbin/hiped --css /etc/hipe-files/hipe.css --fill \
    -- :1 vt8 &
BGPID=$!

until ls /run/hipe.socket 2> /dev/null > /dev/null; do
    echo "Waiting for back-end to come up..."
    sleep 0.5
done
chmod 766 /run/hipe.socket
sleep 0.5

echo "*** PLEASE USE [host key]+F7/F8 TO SWITCH BETWEEN CONSOLES ***"

##########
# Please modify the following value to assign the command needed to run
# your socket server program:
#
COMMAND="iol -- ./server"

sudo -u student bash -c "\
    export HIPE_KEYFILE=/run/hipe.hostkey;\
    export HIPE_SOCKET=/run/hipe.socket;\
    $COMMAND"

echo "SHUTTING DOWN..."
kill -2 $BGPID
