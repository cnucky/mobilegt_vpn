/*

 */
#include "mobilegt_util.h"

//// 使用网上copy的代码处理字符串
#include "get_config.h"

//int转换为字节bytes
void intToByte(int i, byte *bytes, int size /*= 4*/) {
	//byte[] bytes = new byte[4];
	memset(bytes, 0, sizeof (byte) * size);
	bytes[3] = (byte) (0xff & i);
	bytes[2] = (byte) ((0xff00 & i) >> 8);
	bytes[1] = (byte) ((0xff0000 & i) >> 16);
	bytes[0] = (byte) ((0xff000000 & i) >> 24);
}

//bytes转换int
int bytesToInt(const byte * bytes, int size /*= 4*/) {
	int addr = bytes[3] & 0xFF;
	addr |= ((bytes[2] << 8) & 0xFF00);
	addr |= ((bytes[1] << 16) & 0xFF0000);
	addr |= ((bytes[0] << 24) & 0xFF000000);
	return addr;
}

//EncryptDecryptHelper
//必须加上初始化,否则编译会在使用到该成员变量的地方提示undefine reference to single_Instance的错误
EncryptDecryptHelper * EncryptDecryptHelper::single_Instance = NULL;
EncryptDecryptHelper::EncryptDecryptHelper() {
	//构造函数是私有的  
	aesEncryption.SetKey(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
	aesDecryption.SetKey(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
}

//// 
//// 日志级别类型转换处理:字符串到enum类型处理
////
log_level getLogLevel(string str_loglevel) {
	log_level ll = log_level::DEBUG;
	if (str_loglevel == "DEBUG")
		ll = log_level::DEBUG;
	else if (str_loglevel == "INFO")
		ll = log_level::INFO;
	else if (str_loglevel == "WARN")
		ll = log_level::WARN;
	else if (str_loglevel == "ERROR")
		ll = log_level::ERROR;
	else if (str_loglevel == "FATAL")
		ll = log_level::FATAL;

	return ll;
}

//// 
//// 日志方法用到的currentLogFile也是个全局变量
//// 全局变量,日志操作相关的两个锁变量
//// 
mutex mtx_log; //mtx.lock(),mtx.unlock()
mutex mtx_checklog;
void log(log_level ll, string fun_name, string log_str, bool checkLogFile) {
	//如果配置文件设定日志输出级别为WARN,则ll级别为DEBUG和INFO的信息不会输出
	if (ll >= LOG_LEVEL_SET) {
		mtx_log.lock();
		auto logTime = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		//std::put_time(std::localtime(&logTime), "%F %T");
		cout << std::put_time(std::localtime(&logTime), "%F %T") << " [" << fun_name << "] ";
		logger << std::put_time(std::localtime(&logTime), "%F %T") << " [" << fun_name << "] ";
		switch (ll) {
			case DEBUG:
				cout << "DEBUG: ";
				logger << "DEBUG: ";
				break;
			case INFO:
				cout << "INFO: ";
				logger << "INFO: ";
				break;
			case WARN:
				cout << "WARN: ";
				logger << "WARN: ";
				break;
			case ERROR:
				cout << "ERROR: ";
				logger << "ERROR: ";
				break;
			case FATAL:
				cout << "FATAL: ";
				logger << "FATAL: ";
				break;
		}
		cout << log_str << endl;
		logger << log_str << endl;
		mtx_log.unlock();
		if (checkLogFile)
			checkLogger(currentLogFile);
	}
}

//// 
//// 该方法检测日志文件是过大,过大则round robin处理
//// 每个日志文件最大10*1024*1024即10M,超过则写入下一个日志文件
//// 
void checkLogger(string currentLogFile) {
	mtx_checklog.lock();
	const string FUN_NAME = "mobilegt_util-->checkLogger";
	const bool CHECK_LOGGER = false; //必须设置为false,否则log()----checklogger()----log()死递归了
	if (!logger.is_open()) {
		logger.open(currentLogFile.c_str(), ios::trunc | ios::out);
		log(log_level::INFO, FUN_NAME, "openfile:" + currentLogFile, CHECK_LOGGER);
	}
	struct stat buf;
	if (stat(currentLogFile.c_str(), &buf) < 0)
		log(log_level::ERROR, FUN_NAME, "stat file failed:" + currentLogFile, CHECK_LOGGER);
	if (buf.st_size > (10 * 1024 * 1024)) {
		logger.close();
		cLoground++;
		cLoground %= 10;
		currentLogFile = logfileNameBase + "." + to_string(loground[cLoground]);
		logger.open(currentLogFile.c_str(), ios::trunc | ios::out);
		log(log_level::INFO, FUN_NAME, "open new file" + currentLogFile, CHECK_LOGGER);
	}
	mtx_checklog.unlock();
}

//// 
//// class PacketPool
//// 构造函数
//// 报文缓冲池, 构造初始化构建分配POOL_NODE_MAX_NUMBER个缓冲节点用于存放数据
PacketPool::PacketPool() {
	const string FUN_NAME = "PacketPool";
	log(log_level::DEBUG, FUN_NAME, "packet pool constructor.");
	for (int i = 0; i < POOL_NODE_MAX_NUMBER; i++) {
		ptr_pktNodePool[i] = new PacketNode(i);
		queue_producer.push(i); // 即:queue_producer.push(ptr_pktNodePool[i]->index);
	}

}

//// 
//// class PacketPool
//// 析构函数,回收构造是分配的POOL_NODE_MAX_NUMBER个缓冲报文节点
//// 
PacketPool::~PacketPool() {
	for (int i = 0; i < POOL_NODE_MAX_NUMBER; i++) {
		delete ptr_pktNodePool[i];
	}
}

//// 得到一个用于接收网络数据报文的节点
PacketNode* PacketPool::produce() {
	int nodeIndex = -1;
	mtx_queue_producer.lock();
	if (!queue_producer.empty()) {
		nodeIndex = queue_producer.front();
		queue_producer.pop();
	}
	mtx_queue_producer.unlock();
	if (nodeIndex >= 0)
		return ptr_pktNodePool[nodeIndex];
	else
		return NULL;
}

//// 结点已接收完网络数据报文,加入待处理队列
void PacketPool::produceCompleted(PacketNode * const pkt_node) {
	mtx_queue_consumer.lock();
	queue_consumer.push(pkt_node->index);
	mtx_queue_consumer.unlock();
	produce_cv.notify_all();
}
//// 结点接收的数据为空或用于接收数据的结点不需要接收数据,回收结点用于下次接收
void PacketPool::produceWithdraw(PacketNode * const pkt_node) {
	PacketPool::consumeCompleted(pkt_node);
}

//// 得到一个需要处理(即,已接收网络数据报文)的节点
PacketNode* PacketPool::consume() {
	int nodeIndex = -1;
	mtx_queue_consumer.lock();
	if (!queue_consumer.empty()) {
		nodeIndex = queue_consumer.front();
		queue_consumer.pop();
	}
	mtx_queue_consumer.unlock();
	if (nodeIndex >= 0)
		return ptr_pktNodePool[nodeIndex];
	else
		return NULL;
}

//// 节点数据处理完毕,加入待处理队列
void PacketPool::consumeCompleted(PacketNode * const pkt_node) {
	pkt_node->pkt_len = 0;
	mtx_queue_producer.lock();
	queue_producer.push(pkt_node->index);
	mtx_queue_producer.unlock();
	consume_cv.notify_all(); // 条件变量,唤醒所有线程.
}


//// 
TunIPAddrPool::TunIPAddrPool(string netaddr, string netmask) : pool_netaddr(netaddr), pool_netmask(netmask) {
	//// 未实现
}

//// 
//// 构造函数
//// 读取assign_ip_recorder文件，初始化历史分配情况，保证同样的deviceId与分配的IP有一一对应关系
//// 
TunIPAddrPool::TunIPAddrPool(string assign_ip_recorder) : assign_ip_recorder(assign_ip_recorder) {
	mtx_tunaddr_pool.lock();
	const string FUN_NAME = "TunIPAddrPool->TunIPAddrPool";
	log(log_level::DEBUG, FUN_NAME, "initial TunIPAddrPool");
	ifstream infile(assign_ip_recorder.c_str());
	if (!infile) {
		cout << "file open error.[" << assign_ip_recorder << "]" << endl;
	}
	//// ##ip_prefix.#ip_index2.#ip_index1=#deviceId
	//// #开头为注释
	//// 10.77.0.1=deviceId-1
	//// 10.77.0.2=deviceId-2
	//// ......
	string line, tunip, deviceId;
	while (getline(infile, line)) {
		if (AnalyseLine(line, tunip, deviceId)) {
			umap_deviceId_tunip[deviceId] = tunip;
			umap_tunip_deviceId[tunip] = deviceId;
			int new_index1, new_index2;
			auto pos = tunip.rfind(".");
			std::stringstream ss;
			ss << tunip.substr(pos + 1);
			ss >> new_index1;
			ip_index1 = ip_index1 >= new_index1 ? ip_index1 : new_index1;

			string t = tunip.substr(0, pos);
			pos = t.rfind(".");
			ss.clear();
			ss << t.substr(pos + 1);
			ss >> new_index2;
			ip_index2 = ip_index2 >= new_index2 ? ip_index2 : new_index2;
		}
	}
	ip_index1++; //所有已分配的ip再加1为第一个可分配IP
	if (ip_index1 > IP_INDEX1_MAX) {
		ip_index2++;
		ip_index1 = 1;
	}
	if (ip_index2 == IP_INDEX2_MAX && ip_index1 == IP_INDEX1_MAX) {
		log(log_level::FATAL, FUN_NAME, "exceed ip_index1 & ip_index2. current ip_index1:" + to_string(ip_index1) + " ip_index2:" + to_string(ip_index2));
	}
	infile.close();
	log(log_level::DEBUG, FUN_NAME, "initial ip_index1:" + to_string(ip_index1) + " ip_index2:" + to_string(ip_index2));
	mtx_tunaddr_pool.unlock();
}

//// 
void TunIPAddrPool::hibernateHistoryLog(string logFileName) {
	//未实现
}

////
//// TunIPAddrPool
//// 每次分配新的tunip给新的deviceId后立即将分配记录写入分配记录文件
//// 
string TunIPAddrPool::assignTunIPAddr(string deviceId) {
	const string FUN_NAME = "TunIPAddrPool::assignTunIPAddr";
	mtx_tunaddr_pool.lock();
	string tun_ip;
	auto f = umap_deviceId_tunip.find(deviceId);
	if (f == umap_deviceId_tunip.end()) {
		log(log_level::DEBUG, FUN_NAME, "cannot find a assigned tun ip for deviceId[" + deviceId + "]");
		//// #ip_prefix.#ipindex2.#ipindex1
		//// example: 10.77.0.1 10.77.0.2 10.77.0.3 ...
		string newip = "";
		if (!exceedScope) {
			newip = ip_prefix + "." + to_string(ip_index2) + "." + to_string(ip_index1);
			log(log_level::DEBUG, FUN_NAME, "assigned tun_ip is:[" + newip + "] current ip_index1:" + to_string(ip_index1) + " ip_index2:" + to_string(ip_index2));
			ip_index1++; //所有已分配的ip再加1为第一个可分配IP
			if (ip_index1 > IP_INDEX1_MAX) {
				ip_index2++;
				ip_index1 = 1;
			}
			if (ip_index2 == IP_INDEX2_MAX && ip_index1 == IP_INDEX1_MAX) {
				log(log_level::FATAL, FUN_NAME, "exceed ip_index1 & ip_index2. current ip_index1:" + to_string(ip_index1) + " ip_index2:" + to_string(ip_index2));
				exceedScope = true;
			}

			umap_deviceId_tunip[deviceId] = newip;
			umap_tunip_deviceId[newip] = deviceId;
			//// update assign_ip_recorder
			ofstream outfile(assign_ip_recorder.c_str(), ios::app);
			streampos sp = outfile.tellp();
			if (sp <= 0)
				outfile << "##ip_prefix.#ip_index2.#ip_index1=#deviceId" << endl;
			outfile << newip << "=" << deviceId << endl;
			outfile.close();
		} else {
			log(log_level::ERROR, FUN_NAME, "cannot assign tun_ip. current ip_index1:" + to_string(ip_index1) + " ip_index2:" + to_string(ip_index2));
		}
		tun_ip = newip;
	} else {
		log(log_level::DEBUG, FUN_NAME, "find a assigned tun ip.");
		tun_ip = f->second;
	}
	mtx_tunaddr_pool.unlock();
	return tun_ip;
}

//// 
//// 依据deviceId查询对应分配的tunip
//// 
string TunIPAddrPool::queryTunIPAddrByDeviceId(string deviceId) {
	string tun_ip;
	auto f = umap_deviceId_tunip.find(deviceId);
	if (f == umap_deviceId_tunip.end()) {
		;
	} else {
		tun_ip = f->second;
	}
	return tun_ip;
}


//// 
//// class PeerClientTable
//// 
PeerClientTable * PeerClientTable::single_Instance = NULL;
PeerClientTable::PeerClientTable() {
	//构造函数是私有的  

}

//// 析构函数 
PeerClientTable::~PeerClientTable() {
	mtx_peerClientTable.lock();
	const string FUN_NAME = "~PeerClientTable";
	log(log_level::DEBUG, FUN_NAME, "destructor clear all peer client object.");
	if (single_Instance == NULL) {
		for (auto iter = umap_tunip_client.begin(); iter != umap_tunip_client.end(); iter++) {
			PeerClient * p_peerClient = iter->second;
			delete p_peerClient;
		}
		delete single_Instance;
	}
	umap_tunip_client.clear();
	umap_internetip_client.clear();
	mtx_peerClientTable.unlock();
}

//// 
PeerClientTable * PeerClientTable::getInstance() {
	if (single_Instance == NULL) //判断是否第一次调用  
		single_Instance = new PeerClientTable();
	return single_Instance;
}

//// 
PeerClient * PeerClientTable::getPeerClientByTunIP(string tun_ip) {
	if (umap_tunip_client.find(tun_ip) == umap_tunip_client.end()) {
		return NULL;
	} else {
		return umap_tunip_client[tun_ip];
	}
}

//int PeerClientTable::addPeerClient(string tun_ip, PeerClient * p_peerClient) {
//	int result = 0;
//	umap_tunip_client[tun_ip] = p_peerClient;
//	umap_internetip_client[p_peerClient->getPeer_internet_ip()] = p_peerClient;
//	result = 1;
//	return result;
//}
int PeerClientTable::addPeerClient(string deviceId, string tun_ip, string internet_ip, int internet_port) {
	mtx_peerClientTable.lock();
	PeerClient* ptr_oldClient = getPeerClientByTunIP(tun_ip);
	int result = 0;
	if (ptr_oldClient == NULL) {
		PeerClient * p_peerClient = new PeerClient(deviceId, tun_ip, internet_ip, internet_port);
		p_peerClient->refreshRecentConnectTime();
		umap_tunip_client[tun_ip] = p_peerClient;
		umap_internetip_client[internet_ip] = p_peerClient;
		result = 1;
	} else {
		ptr_oldClient->refreshRecentConnectTime();
		ptr_oldClient->setPeer_deviceId(deviceId);
		ptr_oldClient->setPeer_internet_ip(internet_ip);
		ptr_oldClient->setPeer_internet_port(internet_port);
		ptr_oldClient->resetDataCount();
	}
	mtx_peerClientTable.unlock();
	return result;
}

//// 
int PeerClientTable::deletePeerClient(string tun_ip) {
	mtx_peerClientTable.lock();
	const string FUN_NAME = "deletePeerClient";
	int result = 0;
	auto iter1 = umap_tunip_client.find(tun_ip);
	log(log_level::DEBUG, FUN_NAME, "find tun_ip:" + tun_ip);
	if (iter1 != umap_tunip_client.end()) {
		PeerClient * p_peerClient = umap_tunip_client[tun_ip];
		string internet_ip = p_peerClient->getPeer_internet_ip();
		auto iter2 = umap_internetip_client.find(internet_ip);
		if (iter2 != umap_internetip_client.end()) {
			umap_internetip_client.erase(iter2);
		}
		umap_tunip_client.erase(iter1);
		log(log_level::DEBUG, FUN_NAME, "begin delete p_peerClient");
		delete p_peerClient;
		result = 1;
	}
	mtx_peerClientTable.unlock();
	return result;
}

////
bool PeerClientTable::checkPeerInternetIPandPort(string internet_ip, int port) {
	const string FUN_NAME = "checkPeerInternetIPandPort";
	bool succeed = false;
	log(log_level::DEBUG, FUN_NAME, "begin check." + internet_ip + ":" + to_string(port));
	if (umap_internetip_client.find(internet_ip) != umap_internetip_client.end()) {
		log(log_level::DEBUG, FUN_NAME, "finded " + internet_ip + ".begin check time and port");
		PeerClient * p_peerClient = umap_internetip_client[internet_ip];

		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point recent = p_peerClient->getRecentConnectTime();
		//// 30minute * 60second/minute
		//// 最近连接时间不超过30分钟
		int tt = std::chrono::duration_cast<std::chrono::seconds>(now - recent).count();
		int finded_port = p_peerClient->getPeer_internet_port();
		log(log_level::DEBUG, FUN_NAME, "finded port is:" + to_string(finded_port) + ".time duration seconds is:" + to_string(tt));
		if (tt <= 30 * 60) {
			if (p_peerClient->getPeer_internet_port() == port) {
				succeed = true;
				p_peerClient->refreshRecentConnectTime();
				p_peerClient->increaseDataPktCount_send();
			}
		}
	}
	return succeed;
}

//// 
//// class PeerClient
////
PeerClient::PeerClient() {

}
PeerClient::PeerClient(string deviceId, string tun_addr, string internet_addr, int internet_port) : peer_deviceId(deviceId), peer_tun_ip(tun_addr), peer_internet_ip(internet_addr), peer_internet_port(internet_port) {

}
string PeerClient::getPeer_deviceId() {

	return peer_deviceId;
}

//// 
string PeerClient::getPeer_tun_ip() {

	return peer_tun_ip;
}

//// 
string PeerClient::getPeer_internet_ip() {

	return peer_internet_ip;
}

//// 
int PeerClient::getPeer_internet_port() {

	return peer_internet_port;
}

//// 
void PeerClient::setPeer_deviceId(string deviceId) {
	mtx_peerClient.lock();
	peer_deviceId = deviceId;
	mtx_peerClient.unlock();
}

//// 
void PeerClient::setPeer_internet_ip(string internet_ip) {
	mtx_peerClient.lock();
	peer_internet_ip = internet_ip;
	mtx_peerClient.unlock();
}

//// 
void PeerClient::setPeer_internet_port(int internet_port) {
	mtx_peerClient.lock();
	peer_internet_port = internet_port;
	mtx_peerClient.unlock();
}

//// 
void PeerClient::refreshRecentConnectTime() {
	mtx_peerClient.lock();
	recentConnectTime = std::chrono::system_clock::now();
	mtx_peerClient.unlock();
}

//// 
std::chrono::system_clock::time_point PeerClient::getRecentConnectTime() {

	return recentConnectTime;
}

//// 
void PeerClient::increaseDataPktCount_recv() {
	mtx_peerClient.lock();
	dataPktCount_recv++;
	mtx_peerClient.unlock();
}

//// 
void PeerClient::increaseDataPktCount_send() {
	mtx_peerClient.lock();
	dataPktCount_send++;
	mtx_peerClient.unlock();
}

//// 
int PeerClient::getDataPktCount_send() const {

	return dataPktCount_send;
}

//// 
int PeerClient::getDataPktCount_recv() const {
	return dataPktCount_recv;
}
void PeerClient::increaseCmdPktCount_send() {
	mtx_peerClient.lock();
	cmdPktCount_send++;
	mtx_peerClient.unlock();
}
void PeerClient::increaseCmdPktCount_recv() {
	mtx_peerClient.lock();
	cmdPktCount_recv
	mtx_peerClient.unlock();
}
int PeerClient::getCmdPktCount_send() const {
	return cmdPktCount_send;
}
int PeerClient::getCmdPktCount_recv() const {
	return cmdPktCount_recv;
}
void PeerClient::resetDataCount() {
	mtx_peerClient.lock();
	dataPktCount_send = 0;
	dataPktCount_recv = 0;
	cmdPktCount_send = 0;
	cmdPktCount_recv = 0;
	mtx_peerClient.unlock();
}