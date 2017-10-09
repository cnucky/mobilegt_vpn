/*
新版vpnserver的主程序,负责启动vpnserver服务
完成的功能：
1. 解析送入的参数
2. 启动服务
2.1 打开TUN接口;创建监听socketfd
2.2 启动主线程处理socketfd
2.3 启动主线程处理TUN接口
2.4 启动线程池处理数据
2.5 等待主线程退出,并循环检查是否需通知主线程及线程池退出
3. 退出服务
 */

#include "mobilegt_util.h"
#include "mobilegt_vpnserver.h"
#include "get_config.h"

using namespace std;

/*
 * 解析命令参数,启动vpn服务
 */
string LOG_DIR = "log/";
string logfileNameBase = "mobilegt_vpn_";
int loground[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int cLoground = 0;
string currentLogFile;
log_level LOG_LEVEL_SET = log_level::FATAL;
ofstream logger;
string secret = "test0";
bool SYSTEM_EXIT = false;
bool OPEN_DEBUGLOG = false;

string PROC_DIR = "proc/";

void usage(char **argv);
static int get_tunnel(const char *port);
static int get_tun_interface(const char * name);
int main(int argc, char **argv) {
	const string FUN_NAME = "main";
	if (argc < 3) {
		usage(argv);
	}
	//// 解析启动参数
	map<string, string> conf_m;
	ReadConfig(argv[2], conf_m);

	//// 测试打印参数配置
	PrintConfig(conf_m);
	auto f = conf_m.find("RUNNING_TAG");
	string tagfileName = "mobilegt.tag"; //0表示终止运行,1表示正在运行
	if (f == conf_m.end()) {
		log(log_level::FATAL, FUN_NAME, "NOT config RUNNING_TAG!!!!");
	} else {
		tagfileName = f->second;
		log(log_level::INFO, FUN_NAME, "INIT set tagfile completed. RUNNING_TAG file is:" + tagfileName);
	}
	ofstream tagFile((PROC_DIR + tagfileName).c_str(), ios::out);
	tagFile << 1 << endl; //运行标识文件置为1
	tagFile.close();

	logfileNameBase += conf_m["logfileNameBase"];
	string str_LOGLEVEL = conf_m["LOG_LEVEL"];
	LOG_LEVEL_SET = getLogLevel(str_LOGLEVEL);
	if (LOG_LEVEL_SET == log_level::DEBUG)
		OPEN_DEBUGLOG = true;
	//// append方法或+运算均可以,之前编译出错是由于函数模板分离编译所致,并非string+运算使用错误
	//// currentLogFile = logfileNameBase.append(".").append(numToString(loground[cLoground]));	
	currentLogFile = logfileNameBase + "." + numToString(loground[cLoground]); //第一个日志文件为$logfileNameBase.0
	checkLogger();

	f = conf_m.find("SECRET");
	if (f == conf_m.end()) {
		log(log_level::FATAL, FUN_NAME, "NOT config SECRET!!!!");
	} else {
		secret = f->second;
		log(log_level::INFO, FUN_NAME, "INIT set SECRET completed.");
	}

	this_thread::sleep_for(chrono::milliseconds(500)); //// 暂停500毫秒
	//// 初始化历史tun_ip分配记录
	string assign_ip_recorder = conf_m["assign_ip_recorder"];
	TunIPAddrPool tunip_pool(assign_ip_recorder);

	this_thread::sleep_for(chrono::milliseconds(500)); //// 暂停500毫秒
	//// 启动tunnel接口监听线程
	//// 根据配置文件VPN_PORT设置,启动TunnelReceiver线程监听该端口
	////
	string tunnel_port = "8800";
	f = conf_m.find("VPN_PORT");
	if (f == conf_m.end()) {
		log(log_level::FATAL, FUN_NAME, "NOT config VPN_PORT!!!!");
	} else {
		tunnel_port = f->second;
		log(log_level::INFO, FUN_NAME, "INIT set tunnel_port : " + tunnel_port);
	}

	int socketfd_tunnel = get_tunnel(tunnel_port.c_str());
	//int max_node_number=200;
	PacketPool tunnel_recv_packetPool;
	std::thread thread_tunnel_recv(tunnelReceiver, socketfd_tunnel, std::ref(tunnel_recv_packetPool));
	stringstream ss;
	ss << thread_tunnel_recv.get_id();
	log(log_level::INFO, FUN_NAME, "start thread_tunnel_recv. tunnel_port is:" + tunnel_port
			+ ". thread[" + ss.str() + "]");
	//thread_tunnel_recv.detach();

	//// 启动TUN接口监听线程
	//// 根据配置文件TUN_NAME设置,启动TunReceiver线程
	////
	string tun_ifname = "tun0";
	f = conf_m.find("TUN_NAME");
	if (f == conf_m.end()) {
		log(log_level::FATAL, FUN_NAME, "NOT config TUN_NAME!!!!");
	} else {
		tun_ifname = f->second;
	}
	int fd_tun_interface = get_tun_interface(tun_ifname.c_str());
	PacketPool tunIF_recv_packetPool;
	std::thread thread_tunIF_recv(tunReceiver, fd_tun_interface, std::ref(tunIF_recv_packetPool));
	ss.clear();
	ss.str("");
	ss << thread_tunIF_recv.get_id();
	log(log_level::INFO, FUN_NAME, "start thread_tunIF_recv. tun_ifname is:" + tun_ifname
			+ ". thread[" + ss.str() + "]");
	//thread_tunIF_recv.detach();

	////
	//// 启动多个线程处理两个缓冲池池里面的数据
	//// tunnel_recv_packetPool
	//// tunIF_recv_packetPool
	////
	vector<thread> vec_thread_tunnel_dataProcess;
	for (int i = 0; i < 1; i++) {

		//// 启动线程处理tunnel socket接收到的数据,接收到的数据通过tun接口转发到internet
		//// 或者是初次连接数据,通过socket将tunnel配置信息数据发给客户端
		//// 
		std::thread thread_tunnel_dataProcess(tunnelDataProcess, std::ref(tunnel_recv_packetPool), fd_tun_interface, socketfd_tunnel, std::ref(tunip_pool), std::ref(conf_m));
		ss.clear();
		ss.str("");
		ss << thread_tunnel_dataProcess.get_id();
		log(log_level::INFO, FUN_NAME, "start thread_tunnel_dataProcess. thread[" + ss.str() + "]");
		vec_thread_tunnel_dataProcess.push_back(std::move(thread_tunnel_dataProcess));
		//thread_tunnel_dataProcess.detach();
	}
	//// tunIF_recv_packetPool
	////
	vector<thread> vec_thread_tunIF_dataProcess;
	for (int i = 0; i < 1; i++) {
		//// 启动线程处理tun接口接收的数据,tun接收到的数据通过socket发给客户端
		//// 
		std::thread thread_tunIF_dataProcess(tunDataProcess, std::ref(tunIF_recv_packetPool), socketfd_tunnel);
		ss.clear();
		ss.str("");
		ss << thread_tunIF_dataProcess.get_id();
		log(log_level::INFO, FUN_NAME, "start thread_tunIF_dataProcess. thread[" + ss.str() + "]");
		vec_thread_tunIF_dataProcess.push_back(std::move(thread_tunIF_dataProcess)); //NOTE: 必须使用move,线程对象不能拷贝只能移动
		//thread_tunIF_dataProcess.detach();
	}

	//// 主线程进入循环判断
	while (!SYSTEM_EXIT) {
		//检查系统退出文件标识
		ifstream inTagFile((PROC_DIR + tagfileName).c_str());
		string line;
		while (getline(inTagFile, line)) {
			if (line == "0") {
				log(log_level::FATAL, FUN_NAME, "VPN server prepare exit.");
				SYSTEM_EXIT = true;
				break;
			}
		}
		inTagFile.close();
		this_thread::sleep_for(chrono::seconds(5)); //休眠5秒再次检查
	}
	// 检查到系统退出标识,等待其它线程运行终止,然后vpn server退出, 清理输出后关闭vpn server运行.
	//this_thread::sleep_for(chrono::seconds(5)); //休眠5秒,等待其它线程退出再退出
	thread_tunnel_recv.join();
	thread_tunIF_recv.join();
	for (auto &thread : vec_thread_tunnel_dataProcess)
		if (thread.joinable())
			thread.join();
	for (auto &thread : vec_thread_tunIF_dataProcess)
		if (thread.joinable())
			thread.join();
	PeerClientTable * ptr_peerClientTable = PeerClientTable::getInstance();
	unordered_map<string, PeerClient *> umap_tunip_client = ptr_peerClientTable->getUmap_tunip_client();
	log(log_level::FATAL, FUN_NAME, "Peer_deviceId:Peer_tun_ip:DataPktCount_send(CmdPktCount_send):DataPktCount_recv(CmdPktCount_recv)");

	for (auto iter = umap_tunip_client.begin(); iter != umap_tunip_client.end(); iter++) {
		PeerClient * ptr_pc = iter->second;
		log(log_level::FATAL, FUN_NAME, ptr_pc->getPeer_deviceId() + ":" + ptr_pc->getPeer_tun_ip() + ":"
				+ to_string(ptr_pc->getDataPktCount_send()) + "(" + to_string(ptr_pc->getCmdPktCount_send()) + "):"
				+ to_string(ptr_pc->getDataPktCount_recv()) + "(" + to_string(ptr_pc->getCmdPktCount_recv()) + ")");
	}
	log(log_level::FATAL, FUN_NAME, "VPN server exit.");
	logger.close();
}

//// 
//// 启动服务端口socket
////
static int get_tunnel(const char * tunnel_port) {

	// We use an IPv4 socket.
	int fd_tunnel_socket = socket(AF_INET, SOCK_DGRAM, 0);
	int flag = 1;
	setsockopt(fd_tunnel_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof (flag));
	flag = 0;

	// Accept packets received on any local address.
	sockaddr_in addr;
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(tunnel_port));

	// Call bind(2) in a loop since Linux does not have SO_REUSEPORT.
	while (bind(fd_tunnel_socket, (sockaddr *) & addr, sizeof (addr))) {
		if (errno != EADDRINUSE) {
			return -1;
		}
		usleep(100000);
	}
	char claddrStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr.sin_addr, claddrStr, INET_ADDRSTRLEN);

	//// 旧版程序一个客户端只有一个主进程同时处理tunnel和TUN的数据，需要设置为非阻塞模式，否则阻塞在tunnel接收则无法接收处理TUN的数据
	// Put the tunnel into non-blocking mode.
	//fcntl(tunnel, F_SETFL, O_NONBLOCK);

	return fd_tunnel_socket;
}

//// 
//// 
static int get_tun_interface(const char * tun_name) {
	//int interface = open("/dev/net/tun", O_RDWR | O_NONBLOCK);
	string FUN_NAME = "get_tun_interface";
	int fd_interface_tun = open("/dev/net/tun", O_RDWR);
	ifreq ifr;
	memset(&ifr, 0, sizeof (ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, tun_name, sizeof (ifr.ifr_name));

	if (ioctl(fd_interface_tun, TUNSETIFF, &ifr)) {
		log(log_level::FATAL, FUN_NAME, "Cannot get TUN interface");
		exit(1);
	}

	return fd_interface_tun;
}

//// 
////
void usage(char **argv) {

	printf("Usage: %s <config file name>\n"
			"\n"
			"config file name:\n"
			"  -f <config file name> \n"
			"Note that TUN interface needs to be configured properly\n"
			"BEFORE running this program. For more information, please\n"
			"read the comments in the source code.\n\n", argv[0]);
	exit(1);
}