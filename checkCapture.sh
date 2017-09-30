#!/bin/bash

#example:
#    ./checkCapture.sh tun0 4cd4ccc

if [ ! $# -eq 2 ];then
    echo "example:"
    echo "$0 tun0 4cd4ccc"
    exit 0
fi

IF_NAME=$1
DEVICE_ID=$2
CMD_NAME="tshark"
CMD_NAME2="dumpcap"

PID=`ps -ef | grep $CMD_NAME | grep $IF_NAME | grep $DEVICE_ID | awk '{print $2}'`
PID2=`ps -ef | grep $CMD_NAME2 | grep $IF_NAME | grep $DEVICE_ID | awk '{print $2}'`
DEVICE_PID=`ps -ef | grep $CMD_NAME | grep $DEVICE_ID | awk '{print $2}'`
DEVICE_PID2=`ps -ef | grep $CMD_NAME2 | grep $DEVICE_ID | awk '{print $2}'`
IF_PID=`ps -ef | grep $CMD_NAME | grep $IF_NAME | awk '{print $2}'`
IF_PID2=`ps -ef | grep $CMD_NAME2 | grep $IF_NAME | awk '{print $2}'`

FOUND_DUMPCAP=0
if [ -z "$PID2" -o -z "$PID" ];then
    if [ -n "$DEVICE_PID2" ]; then
        echo kill DEVICE_PID2:$DEVICE_PID2
        #cat password | sudo -S kill -9 $DEVICE_PID2
        sudo kill -9 $DEVICE_PID2
    fi
    if [ -n "$IF_PID2" ]; then
        echo kill IF_PID2:$IF_PID2
        #cat password | sudo -S kill -9 $IF_PID2
        sudo -S kill -9 $IF_PID2
    fi
    if [ -n "$DEVICE_PID" ]; then
        echo kill DEVICE_PID:$DEVICE_PID
        #cat password | sudo -S kill -9 $DEVICE_PID
        sudo kill -9 $DEVICE_PID
    fi
    if [ -n "$IF_PID" ]; then
        echo kill IF_PID:$IF_PID
        #cat password | sudo -S kill -9 $IF_PID
        sudo -S kill -9 $IF_PID
    fi
	echo ./startCapture.sh $IF_NAME $DEVICE_ID
   	./startCapture.sh $IF_NAME $DEVICE_ID &
else
   echo "FIND tshark & dumpcap process,DON'T NEED RECAPTURE."
fi
