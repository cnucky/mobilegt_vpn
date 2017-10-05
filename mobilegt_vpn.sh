#/bin/bash

usage() {
	echo "example:"
	echo "$0 start"
	echo "$0 stop"
}

if [ ! $# -eq 1 ];then
	usage
	exit 0
fi

if [ $1 == "start" ];then
	echo "start mobilegt vpn server..."

	PROG_SRC_DIR="/home/rywang/.netbeans/remote/222.16.4.76/lenovo-pc-Windows-x86_64/D/GitHub/mobilegt_vpn/dist/Debug/GNU-Linux"
	PROG_DST_DIR="/home/rywang/vpnserver"
	CMD_CP="/bin/cp"

	echo "$CMD_CP $PROG_SRC_DIR/mobilegt_vpn $PROG_DST_DIR/"
	#$CMD_CP $PROG_SRC_DIR/mobilegt_vpn $PROG_DST_DIR/

	echo "$PROG_DST_DIR/mobilegt_vpn -f mobilegt_vpn.cfg &"
	#$PROG_DST_DIR/mobilegt_vpn -f mobilegt_vpn.cfg &
	
	echo "start mobilegt vpn server completed."

elif [ $1 == "stop" ];then
	echo "stop mobilegt vpn server..."

	tunnel_IP="222.16.4.76"
	tunnel_PORT="8000"
	TUN_INTERFACE="tun0"
	CMD_NC="/bin/nc"
	CMD_PING="/bin/ping"
	echo 0 > mobilegt.tag
	sleep 3
	$CMD_NC -vuz $tunnel_IP $tunnel_PORT
	sleep 3
	$CMD_PING -c 1 -W 1 -I $TUN_INTERFACE $tunnel_IP

	echo "stop mobilegt vpn server completed."

else
	usage
fi

