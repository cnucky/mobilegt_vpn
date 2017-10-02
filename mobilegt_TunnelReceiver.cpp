#include "mobilegt_util.h"
/*
 * tunnel接收到的数据是来自客户端的请求数据,
 * 1. 连接验证命令数据,验证约定的密钥,然后将TUN私有地址、DNS信息、路由信息等配置信息返回手机客户端
 * 2. 连接成功后的访问请求数据,这些访问互联网请求数据需通过TUN接口转发出去internet
 * 3. 具体的1,2的数据处理由其它线程完成,此处主要只负责完成数据的接收
 *  
 * 不断接收发送给某特定端口的(某个tunnel)的数据
 * 记录发送端的IP
 * 检查是否为恶意客户端？加入简单防御攻击措施？
 * 如果是命令数据，放入待检测连接缓冲池，触发其它线程检查约定密钥，记录发送端IP
 * 如果是普通数据，检查是否为通过验证的发送端，若是则放入缓冲池，触发其它线程处理缓冲池中的数据
 * 如果是未通过连接验证的发送端，丢弃数据(是否需要通知客户端重新连接验证？以及未来需要添加简单防御攻击措施)
 * 
 */
int tunnelReceiver(int fd_tunnel, PacketPool & tunnel_recv_packetPool) {
	const string FUN_NAME = "tunnelReceiver";
	sockaddr_in peer_addr;
	socklen_t addr_len;
	//PacketPool packetPool; //报文缓冲池
	PeerClientTable * ptr_peerClientTable = PeerClientTable::getInstance();
	while (true) {
		// Read the incoming packet from the tunnel.
		// packet来自缓冲池的空节点
		log(log_level::DEBUG, FUN_NAME, "produce tunnel recv node");
		PacketNode* pkt_node = tunnel_recv_packetPool.produce(); //得到一个空闲节点		
		if (pkt_node == NULL) {
			//没有空闲节点
			//告警,缓冲池满,没有多余的空闲节点
			//暂停100毫秒
			log(log_level::WARN, FUN_NAME, "tunnel_recv_packetPool.produce() is NULL.");
			this_thread::sleep_for(chrono::milliseconds(100)); //std::this_thread;std::chrono;
			continue; //进入下一次循环而不进入读取网络数据报文的阻塞调用
		}
		char * packet = pkt_node->ptr;
		int packet_len = pkt_node->MAX_LEN;
		log(log_level::DEBUG, FUN_NAME, "pkt_node index is:" + to_string(pkt_node->index) + ". recvfrom(fd_tunnel) data to node");
		int length = recvfrom(fd_tunnel, packet, packet_len, 0, (sockaddr *) & peer_addr, &addr_len);
		pkt_node->pkt_len = length;
		bool dropPacket = true; //是否丢弃报文
		if (length > 0) {
			// control messages, which start with zero. data message, which start with nonzero
			char IPdotdec[20];
			string ip = inet_ntop(AF_INET, (void *) &peer_addr.sin_addr, IPdotdec, 16);
			int port = ntohs(peer_addr.sin_port);
			log(log_level::DEBUG, FUN_NAME, "recv fd_tunnel length:" + to_string(length) + " from " + ip + ":" + to_string(port));
			pkt_node->pkt_internetAddr = ip;
			pkt_node->pkt_internetPort = port;
			if (packet[0] == 0) {
				//process control messages
				//接收线程只负责接收数据，由数据处理线程检查约定密钥,正确才发送响应报文,记录该客户端和实际互联网地址对应关系
				//检查客户端地址是否在黑名单中
				//TODO:黑名单限制暂无实现

				dropPacket = false;
			} else {
				//process data messages
				//检查客户端地址是否已在记录表中,不在则丢弃报文
				if (ptr_peerClientTable->checkPeerInternetIPandPort(ip, port))
					dropPacket = false;
			}
		}
		if (dropPacket) {
			log(log_level::WARN, FUN_NAME, "drop Packet.");
			tunnel_recv_packetPool.produceWithdraw(pkt_node);
		} else {
			tunnel_recv_packetPool.produceCompleted(pkt_node);
		}
	}
	return 0;
}