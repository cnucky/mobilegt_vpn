/*

 */
#ifndef MOBILEGT_UTIL_H
#define MOBILEGT_UTIL_H

#include <linux/if_tun.h>

#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <csignal>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ratio>
#include <unordered_map>  //std::unordered_map
#include <vector>    //std::vector
#include <queue>                //std::queue
#include <thread>               //std::thread
#include <mutex>                //std::mutex, std::unique_lock
#include <condition_variable>   //std::condition_variable
using namespace std;

#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/filters.h"

#include "logger.h"

extern string PROC_DIR;
extern string secret;
extern bool SYSTEM_EXIT;

void intToByte(int i, byte *bytes, int size = 4);

int bytesToInt(const byte * bytes, int size = 4);

//// c++11引入了to_string方法,该模板方法可被替换掉
//// 模板函数申明与定义放在一起（模板编译模式：分离模式、包含模式）

template <typename Type>
string numToString(Type num) {
	stringstream ss;
	string s;
	ss << num;
	ss >> s;
	return s;
}

/*
 * 解密/加密辅助类
 * 
 */
class EncryptDecryptHelper {
public:

	static EncryptDecryptHelper * getInstance() {
		if (single_Instance == NULL) //判断是否第一次调用  
			single_Instance = new EncryptDecryptHelper();
		return single_Instance;
	}

	//设置aes 密钥, default: 16byte,128bit
	byte key[CryptoPP::AES::DEFAULT_KEYLENGTH] = {'t', 'e', 's', 't', '0', 'r', 'y', 'w', 'a', 'n', 'g', '0', '0', '0', '0', '0'};
	//设置初始向量组, 可以默认随机，但是一旦指定，加密和解密必须使用相同的初始向量组,16byte
	byte iv[CryptoPP::AES::BLOCKSIZE] = {'t', 'o', 'y', 'v', 'p', 'n', 's', 'e', 'r', 'v', 'e', 'r', '0', '0', '0', '0'};
	CryptoPP::AESEncryption aesEncryption;
	CryptoPP::AESDecryption aesDecryption;

private:
	EncryptDecryptHelper();
	static EncryptDecryptHelper * single_Instance;
};

/*
 * 手机客户端连接历史记录表/tun私有地址分配记录表
 * deviceId,tun_addr,internet_addr,internet_port,timestamp
 * 
 * 采集到的pcap数据报文具有tun_addr和报文时间,匹配规则是报文属于报文时间比timestamp大且离timestamp最近的deviceId对应的手机客户端程序
 * 
 */

/*
 * TUN接口地址池,发送给手机客户端的私有地址
 * 每个手机客户端首先发送连接请求,vpnsever验证通过后分配一个私有地址
 * 该地址未来用于识别从TUN接口抓取到的数据报文属于哪个客户端手机的数据
 * 对应同一手机,尽量分配相同的私有地址
 * (需配合历史分配记录,对于某个deviceId若有多个不同的ip地址对应,使用最近一次的ip地址)
 * **********************************************
 * **** 目前只实现一个IP固定只给一个deviceId ****
 * **********************************************
 */
class TunIPAddrPool {
public:
	//// 为deviceId分配一个tun地址
	string assignTunIPAddr(string deviceId);

	//// 查询分配给deviceId的tun地址
	string queryTunIPAddrByDeviceId(string deviceId);

	//// 初始化tun地址池
	//// 在IP地址3种主要类型里，各保留了3个区域作为私有地址，其地址范围如下：  
	//// A类地址：10.0.0.0～10.255.255.255
	//// B类地址：172.16.0.0～172.31.255.255
	//// C类地址：192.168.0.0～192.168.255.255
	//// 举例：
	////     TunIPAddrPool("192.168.77.0","255,255.255.0") 可支持(2^8-2)2百多个客户端同时使用
	////     TunIPAddrPool("172.16.0.0","255.255.0.0") 可支持(2^16-2)6万多个客户端同时使用
	////     TunIPAddrPool("10.77.0.0","255.255.0.0") 可支持(2^16-2)6万多个客户端同时使用
	////     TunIPAddrPool("10.0.0.0","255.0.0.0") 可支持(2^24-2)1千万多个客户端同时使用
	//TunIPAddrPool(string netaddr, string netmask); //该方法未实现,系统固定使用了10.77.0.0/255.255.0.0初始化

	TunIPAddrPool(string assign_ip_recorder, CacheLogger & cLogger); //对象初始化时读取历史分配记录,系统固定使用了10.77.0.0/255.255.0.0初始化

	//// 保存tun地址分配历史记录
	//// #分配时间\t设备ID号\t分配的IP
	void hibernateHistoryLog(string logFileName_pattern); //该方法无需实现,每次分配tunip后都会将分配记录写入文件。deviceId->tun_private_addr存在固定的一一对应关系
private:
	string pool_netaddr = "10.77.0.0";
	string pool_netmask = "255.255.0.0";
	string ip_prefix = "10.77";
	int ip_index1 = 0; //记录当前可分配的IP最后一位,初始化时从文件读取然后+1，若无初始文件则初始为10.77.*.1
	const int IP_INDEX1_MAX = 254;
	int ip_index2 = 0; //记录当前可分配的IP倒数第二位,初始为0即10.77.0.*
	const int IP_INDEX2_MAX = 255;
	//// ##ip_prefix.#ip_index2.#ip_index1\t#deviceId
	string assign_ip_recorder; //记录已分配的IP记录文件,类初始化时读取这个文件初始化历史分配情况
	unordered_map<string, string> umap_tunip_deviceId; //缓存已分配的IP记录
	unordered_map<string, string> umap_deviceId_tunip; //缓存已分配的IP记录
	bool exceedScope = false;
	mutex mtx_tunaddr_pool;
	CacheLogger & cLogger;
};

/*
 * 客户端节点类,每个类对象对应一个手机客户端记录:
 * 1. 手机设备deviceId
 * 2. 客户端结点的实际互联网地址和端口
 * 3. 分配给客户端结点的TUN地址
 * 4. 最近一次的连接时间(或者是有接收报文的时间?)
 * 5. 已接收到的报文计数和已发送给该客户端结点的报文计数
 */
class PeerClient {
private:

	string peer_deviceId;
	string peer_tun_ip;
	string peer_internet_ip;
	int peer_internet_port;
	//system_clock::time_point firstConnectTime;
	std::chrono::system_clock::time_point recentConnectTime;
	int cmdPktCount_recv = 0;
	int cmdPktCount_send = 0;
	int dataPktCount_recv = 0;
	int dataPktCount_send = 0;
	mutex mtx_peerClient;
	mutex mtx_recentConnectTime;
	mutex mtx_cmdPktCount_recv;
	mutex mtx_cmdPktCount_send;
	mutex mtx_dataPktCount_recv;
	mutex mtx_dataPktCount_send;
public:
	string getPeer_deviceId();
	string getPeer_tun_ip();
	string getPeer_internet_ip();
	int getPeer_internet_port();
	void setPeer_deviceId(string deviceId);
	void setPeer_internet_ip(string internet_ip);
	void setPeer_internet_port(int internet_port);

	PeerClient(string deviceId, string tun_addr, string internet_addr, int internet_port);
	PeerClient();
	//void setFirstConnectTime();
	void refreshRecentConnectTime();
	std::chrono::system_clock::time_point getRecentConnectTime();
	void increaseDataPktCount_recv(int count = 1);
	void increaseDataPktCount_send(int count = 1);
	int getDataPktCount_send() const;
	int getDataPktCount_recv() const;
	void increaseCmdPktCount_recv(int count = 1);
	void increaseCmdPktCount_send(int count = 1);
	int getCmdPktCount_send() const;
	int getCmdPktCount_recv() const;
	void resetDataCount();
};

/**
////单实例模式模板
class Singleton:
{
	// 其它成员
public:
	static Singleton &GetInstance(){
		static Singleton instance;
		return instance;
	}
	//或者使用返回指针的形式
	static Singleton *GetInstance(){
		static Singleton instance;
		return &instance;
	}
private:
	Singleton(){};
	Singleton(const Singleton&);
	Singleton & operate = (const Singleton&);
	//Singleton(const Singleton&);和Singleton & operate = (const Singleton&);函数
	//我们声明成私用的，并且只声明不实现。从而实现禁止类拷贝和类赋值
	//
}
 */

/*
 * 
 * 客户端记录表，不再使用单实例类模式
 * 该类对象用于维护系统当前所有连接的客户端记录
 * 使用字典结构,key为分配给该手机客户端的tun接口地址,value为PeerClient对象指针
 * 设置了mtx_peerClientTable作为多线程并发操作锁，避免同时操作
 * #include <unordered_map>
 * 
 */
class PeerClientTable {
public:

	//static PeerClientTable * getInstance();

	PeerClient * getPeerClientByTunIP(string tun_ip);

	//int addPeerClient(string tun_ip, PeerClient * peerClient);

	int addPeerClient(string deviceId, string tun_ip, string internet_ip, int internet_port);

	int deletePeerClient(string tun_ip);

	bool checkPeerInternetIPandPort(string internet_ip, int port);
	~PeerClientTable();

	unordered_map<string, PeerClient*> getUmap_tunip_client() const;
	PeerClientTable(CacheLogger & cLogger);
private:

	//static PeerClientTable *single_Instance;
	//一个tunip对应一个客户端,不同的客户端使用不同的deviceId,系统依据deviceId分配不同的tunip
	unordered_map<string, PeerClient *> umap_tunip_client;
	//internetip_port对应唯一的客户端, internetip_port=internet_ip:internet_port
	//多个客户端可能会使用同样的internetip(例如:当多个客户端通过无线路由器上网)
	unordered_map<string, PeerClient *> umap_internetip_port_client;
	mutex mtx_peerClientTable;
	CacheLogger & cLogger;
};

/**
 * 处理时间跟踪
 * 
 */
class ProcessTimeTrack {
public:
	string str_track_description; //跟踪点描述
	std::chrono::system_clock::time_point timestamp; //跟踪点的时间戳

};

/**
 * 缓冲池由称为报文节点的多个缓冲节点组成.报文节点缓冲存放接收到的具体网络数据报文.每个报文节点在缓冲池中有唯一的索引编号.
 * 缓冲池的设计为维护两个队列:一个消费者队列,一个生产者队列.生产者队列存放了可用于缓存接收网络数据的的节点队列.生产结束后将对应的节点放入消费者队列;消费者队列存放了已经接收网络数据的节点队列.消费结束后将消费的节点放入生产者队列.
 * produce()方法得到一个空的报文节点用于待接收网络数据.
 * produceCompleted(const pktNode *)接收完网络数据后调用该方法标识该报文节点已缓存数据,报文节点放入消费者队列.
 * consume()方法得到一个已缓存报文的节点,然后其它程序课处理该报文节点缓存的数据.
 * consumeCompleted(const pktNode *)报文节点缓存的数据处理完毕后调用该方法标识缓存的数据已处理,节点放入生产者队列.
 * 数据缓冲池的设计
 */
class PacketNode {
public:
	char * ptr; //字符数组,存放具体接收到的网络数据报文
	const int MAX_LEN = 2048; //每个节点可存放数据报文的最大长度
	int pkt_len; //网络数据报文长度
	const int index; //记录本节点在缓冲池中的索引编号,一旦初始化则不能再修改
	std::chrono::system_clock::time_point timestamp; //记录节点数据报文接收时间标记
	string pkt_tunAddr; //对于tun_interface接收的数据需记录目的地址(tun_interface地址),后续处理报文需依据这个地址找到客户端internet地址
	string pkt_internetAddr; //记录节点数据报文来源IP地址,只用于记录手机客户端的ip
	int pkt_internetPort; //记录节点数据报文来源端口,只用于记录手机客户端的端口
	std::chrono::microseconds getPktNodeDurationMicroseconds(); //获取报文数据到当前时间的时间间隔,便于观察一个报文从接收到发送的处理时间延迟
	PacketNode(int nodeIndex);
	vector<ProcessTimeTrack> vec_process_TT;
	~PacketNode();
	void addProcessTimeTrack(string track_description); //添加一个处理时间跟踪记录
	string getStrProcessTimeTrack(); //打印输出处理时间跟踪记录
	void clear(); //清除节点数据
};

//// 
//// 必须考虑线程安全性<----
////

class PacketPool {
public:
	const static int POOL_NODE_MAX_NUMBER = 2000; //缓冲池最大节点数目
	PacketPool(CacheLogger & cLogger);
	~PacketPool();
	PacketNode* produce(); //得到一个用于接收网络数据报文的节点
	void produceCompleted(PacketNode * const pkt_node); //节点已接收完网络数据报文,加入待处理队列
	void produceWithdraw(PacketNode * const pkt_node); //接收到的报文无需处理,回收空闲节点
	PacketNode* consume(); //得到一个需要处理(即,已接收网络数据报文)的节点
	void consumeCompleted(PacketNode * const pkt_node); //节点数据处理完毕,加入可接收数据的结点队列
	int getRemainInConsumer();
	int getReaminInProducer();
	void terminateProcess();

private:
	PacketNode* ptr_pktNodePool[POOL_NODE_MAX_NUMBER];
	mutex mtx_queue_producer; //mtx***.lock(),mtx***.unlock()
	mutex mtx_queue_consumer;
	queue<int> queue_producer; //可接收数据的结点索引队列,记录当前可用于接收网络数据报文结点链表,需接收数据时从链表中取下一个结点用于接收数据. 接收完数据后结点放入queue_consumer.
	queue<int> queue_consumer; //待处理的结点索引队列,记录当前已接收到数据的结点链表.数据处理线程从链表中取下一个结点处理，处理完毕后将结点放回queue_producer.
	std::condition_variable consume_cv;
	std::condition_variable produce_cv;
	std::mutex mtx_consume_cv;
	std::mutex mtx_produce_cv;
	CacheLogger & cLogger;
};


#endif
