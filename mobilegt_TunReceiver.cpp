#include "mobilegt_util.h"

/*
 * tun_interface接收到的数据为互联网响应给客户端的数据
 * 
 * 不断接收发送给tun_interface的数据
 * (解析出目的IP,目的地址是tun的地址,依据这个地址查询客户端连接记录表可得到对应客户端的目的IP和端口)这个工作交给后续其它线程处理?
 * 
 * 将数据放入待发送缓冲池中
 * 触发其它线程处理缓冲池中的数据
 * PacketPool
 * 
 * 
 */

/**
 * 
 * @param fd_tun_interface
 * @return 
 */
int tunReceiver(int fd_tun_interface, PacketPool & tunIF_recv_packetPool) {
	//PacketPool packetPool; //报文缓冲池
	const string FUN_NAME = "tunReceiver";
	while (true) {
		// Read the packet from the TUN interface.
		// 来自该接口的流量为互联网响应流量
		//packet来自缓冲池的空节点
		log(log_level::DEBUG, FUN_NAME, "produce TUN recv node");
		PacketNode* pkt_node = tunIF_recv_packetPool.produce(); //得到一个空闲节点
		if (pkt_node == NULL) {
			//没有空闲节点
			//告警,缓冲池满,没有多余的空闲节点
			//暂停100毫秒？
			this_thread::sleep_for(chrono::milliseconds(100)); //std::this_thread;std::chrono;
			continue; //进入下一次循环而不进入读取网络数据报文的阻塞调用
		}
		char* packet = pkt_node->ptr;
		int packet_len = pkt_node->MAX_LEN;
		log(log_level::DEBUG, FUN_NAME, "pkt_node index is:" + to_string(pkt_node->index) + ". read(fd_tun_interface) data to node");
		int length = read(fd_tun_interface, packet, packet_len);
		pkt_node->pkt_len = length;
		log(log_level::DEBUG, FUN_NAME, "recv fd_tun_interface length:" + to_string(length));
		bool dropPacket = true; //是否丢弃报文
		if (length > 0) {
			//解析packet包含的目的IP地址packet[16-19](源地址为packet[12-15],参考IPv4报文头格式前20字节)
			struct in_addr addr_dst;
			memcpy((void*) &addr_dst.s_addr, &packet[16], sizeof (in_addr));
			char IPdotdec[20];
			inet_ntop(AF_INET, (void *) &addr_dst, IPdotdec, 16);
			string ip_tun = IPdotdec;
			pkt_node->pkt_tunAddr = ip_tun;
			log(log_level::DEBUG, FUN_NAME, "recv fd_tun_interface data to " + ip_tun);
			//置条件变量为有数据到达，触发其它线程将缓存池中的数据发送给对应的客户端
			dropPacket = false;
		}
		if (dropPacket) {
			tunIF_recv_packetPool.produceWithdraw(pkt_node);
		} else {
			tunIF_recv_packetPool.produceCompleted(pkt_node);
		}
	}

	return 0;
}