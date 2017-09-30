
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
    PID=`ps -ef | grep $CMD_NAME | grep $IF_NAME | awk '{print $2}'`
    PID2=`ps -ef | grep $CMD_NAME2 | grep $IF_NAME | awk '{print $2}'`
else
    PID=`ps -ef | grep $CMD_NAME | awk '{print $2}'`
    PID2=`ps -ef | grep $CMD_NAME2 | awk '{print $2}'`
fi

if [ -z "$PID2" ];then
    echo no PID2 NEED kill.
else
    echo kill PID2:$PID2
    #cat password | sudo -S kill -9 $PID2
    sudo kill -9 $PID2
fi

if [ -z "$PID" ];then
    echo no PID NEED kill.
else
    echo kill PID:$PID
    #cat password | sudo -S kill -9 $PID
    sudo kill -9 $PID
fi

