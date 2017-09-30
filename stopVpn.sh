
#example:
#    ./stopVpn 0

if [ ! $# -eq 1 ];then
    echo "example:"
    echo "$0 num"
    echo "$0 0"
    echo "$0 1"
    exit 0
fi

TUN_NAME=tun$1
CMD_NAME="ToyVpnServer"
#DEVICE_ID=$2

if [ $1 != "all" -a $1 != "ALL" -a $1 != "All" ];then
    #ps -ef | grep $CMD_NAME | grep $TUN_NAME
    PID=`ps -ef | grep $CMD_NAME | grep $TUN_NAME | awk '{print $2}'`
    ./stopCapture.sh $TUN_NAME
else
    PID=`ps -ef | grep $CMD_NAME | awk '{print $2}'`
    ./stopCapture.sh all
fi

if [ -z "$PID" ];then
    echo no PID NEED kill.
else
    echo kill PID:$PID
    #cat password | sudo -S kill -9 $PID
    sudo kill -9 $PID
fi

#cat password | sudo -S ip tuntap del dev $TUN_NAME mode tun
sudo ip tuntap del dev $TUN_NAME mode tun

