#EXAMPLE: 
#  ./startVpn_encrypt.sh 0
if [ ! $# -eq 1 ];then
    echo "example:"
    echo "$0 num &"
    echo "$0 0 &"
    echo "$0 1 &"
    exit 0
fi


VPNSERVER_HOME=/home/ubuntu/vpnserver
cd $VPNSERVER_HOME
g++ ToyVpnServer_encrypt.cpp -o ToyVpnServer_encrypt -lcryptopp

sleep 3

CaptureCMD=$VPNSERVER_HOME/checkCapture.sh
IF_NAME=eth0
NUM=$1
#echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward

ADDR_PREFIX=10.77
#cat password | sudo -S sysctl -w net.ipv4.ip_forward=1
#cat password | sudo -S iptables -t nat -A POSTROUTING -s 10.0.0.0/8 -o $IF_NAME -j MASQUERADE
 sudo sysctl -w net.ipv4.ip_forward=1
 sudo iptables -t nat -A POSTROUTING -s $ADDR_PREFIX.$NUM.0/24 -o $IF_NAME -j MASQUERADE

TUN_NAME=tun$NUM
TUN_ADDR=$ADDR_PREFIX.$NUM.1
TUN_DSTADDR=$ADDR_PREFIX.$NUM.2
PRIVATE_ADDR=$ADDR_PREFIX.$NUM.2
#DNS_ADDR=202.38.193.33
#DNS_ADDR=8.8.8.8
DNS_ADDR=172.31.0.2
SECRET=test0
VPN_PORT=8000

#cat password | sudo -S ip tuntap del dev $TUN_NAME mode tun
#cat password | sudo -S ip tuntap add dev $TUN_NAME mode tun
#cat password | sudo -S ifconfig $TUN_NAME $TUN_ADDR dstaddr $TUN_DSTADDR up
#cat password | sudo -S ./ToyVpnServer $TUN_NAME $VPN_PORT $SECRET -m 1400 -a $PRIVATE_ADDR 32 -d $DNS_ADDR -r 0.0.0.0 0
sudo ip tuntap del dev $TUN_NAME mode tun
sudo ip tuntap add dev $TUN_NAME mode tun
sudo ifconfig $TUN_NAME $TUN_ADDR dstaddr $TUN_DSTADDR up
sudo ./ToyVpnServer_encrypt $TUN_NAME $VPN_PORT $SECRET $CaptureCMD -m 1400 -a $PRIVATE_ADDR 32 -d $DNS_ADDR -r 0.0.0.0 0
