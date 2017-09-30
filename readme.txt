sudo apt-get install tshark
sudo apt-get install libcrypto++-dev

cat /etc/resolv.conf

首先确认vpnserver程序所在的目录为：
	/home/ubuntu/vpnserver
，否则需修改启动脚本里面的VPNSERVER_HOME变量值为准确目录

1.启动vpn服务
num代表一个数字，&代表后台运行，启动前先使用ps -ef|grep ToyVpnServer检查是否有对应的ToyVpn服务运行检查是否有对应的ToyVpn服务运行
./startVpn_encrypt.sh num &
如
./startVpn_encrypt.sh 0 &      //在tun0上提供vpn服务
./startVpn_encrypt.sh 1 &      //在tun1上提供vpn服务
./startVpn_encrypt.sh 2 &      //在tun2上提供vpn服务

2.清除制定vpn服务
num代表一个数字,
./stopVpn.sh num
如
./stopVpn.sh 0           //清除在tun0上的vpn服务
./stopVpn.sh 1           //清除在tun1上的vpn服务

清除所有vpn服务使用
./stopVpn.sh all

