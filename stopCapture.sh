
#example:
#    ./stopCapture tun0

if [ ! $# -eq 1 ];then
    echo "example:"
    echo "$0 tun0"
    exit 0
fi

IF_NAME=$1
CMD_NAME="tshark"
CMD_NAME2="dumpcap"
#DEVICE_ID=$2

if [ $1 != "all" -a $1 != "ALL" -a $1 != "All" ];then
    #ps -ef | grep $CMD_NAME | grep $IF_NAME
    PID=`ps -ef | grep $CMD_NAME | grep $IF_NAME | grep -v "grep" | awk '{print $2}'`
    PID2=`ps -ef | grep $CMD_NAME2 | grep $IF_NAME | grep -v "grep" | awk '{print $2}'`
else
    PID=`ps -ef | grep $CMD_NAME | grep -v "grep" | awk '{print $2}'`
    PID2=`ps -ef | grep $CMD_NAME2 | grep -v "grep" | awk '{print $2}'`
fi

if [ -z "$PID2" ];then
    echo "    NO running $CMD_NAME2 process."
else
    echo "    kill $CMD_NAME2 PID2:$PID2"
    #cat password | sudo -S kill -9 $PID2
    kill -9 $PID2
fi

if [ -z "$PID" ];then
    echo "    NO running $CMD_NAME process."
else
    echo "    kill $CMD_NAME PID:$PID"
    #cat password | sudo -S kill -9 $PID
    kill -9 $PID
fi

