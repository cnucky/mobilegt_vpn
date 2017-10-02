
#include "mobilegt_util.h"

/*
处理接收来自手机客户端的数据,然后转发给TUN interface,采用多线程机制并发处理
1. 解密接收到数据客户端数据(加密数据)
2. 转发

 */
int build_parameters(char *parameters, int size, const string tun_ip, const map<string, string> & conf_m);
int tunnelDataProcess(PacketPool & tunnelReceiver_packetPool, int fd_tun_interface, int socketfd_tunnel, TunIPAddrPool & tunip_pool, const map<string, string> & conf_m) {
	const string FUN_NAME = "tunnelDataProcess";
	EncryptDecryptHelper * aes_helper = EncryptDecryptHelper::getInstance();
	struct timeval sTime, eTime;
	long exeTime;
	char decryptedPacket[2048];
	PeerClientTable * ptr_peerClientTable = PeerClientTable::getInstance();
	while (true) {
		PacketNode* pkt_node = tunnelReceiver_packetPool.consume();
		if (pkt_node == NULL) {
			//阻塞等待通知有数据
			//std::unique_lock <std::mutex> lck(mtx);
			//tunnelReceiver_packetPool.produce_cv.wait(lck);
			//休眠然后重试
			this_thread::sleep_for(chrono::milliseconds(100)); //std::this_thread;std::chrono;
			continue;
		}
		//process pkt_node里面的数据
		char* packet = pkt_node->ptr;
		int packet_len = pkt_node->pkt_len;
		if (packet[0] != 0) {
			log(log_level::DEBUG, FUN_NAME, "process data packet.pkt_length:" + to_string(packet_len));
			gettimeofday(&sTime, NULL);
			CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aes_helper->aesDecryption, aes_helper->iv);
			gettimeofday(&eTime, NULL);
			exeTime = (eTime.tv_sec - sTime.tv_sec)*1000000 + (eTime.tv_usec - sTime.tv_usec); //exeTime 单位是微秒
			if (exeTime > 1000)
				log(log_level::WARN, FUN_NAME, "decrypt init exeTime:" + to_string(exeTime));

			//// decrypt packet;
			std::string strDecryptedText = "";
			//stfDecryptor.Put((byte*)(strCiphertext.c_str() ),strCiphertext.length());
			//logger << "\nrecv cipher packet from tunnel length:" << dec << length << " packet:" << endl;
			/*
			for(int i=0;i<length;i++)
				cout <<hex<< (0xFF & static_cast<byte>(packet[i])) << " ";
			cout << endl;
			 */
			gettimeofday(&sTime, NULL);
			CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(strDecryptedText), CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING);
			stfDecryptor.Put((byte*) packet + 1, packet_len - 1);
			stfDecryptor.MessageEnd();

			int iDecryptedTextSize = strDecryptedText.size();
			byte lb[4];
			for (int i = 0; i < 4; i++) {
				lb[i] = static_cast<byte> (strDecryptedText[i]);
				//std::cout << hex << (0xFF & static_cast<byte>(strDecryptedText[i])) << " ";
			}

			int datalength = bytesToInt(lb);

			log(log_level::DEBUG, FUN_NAME, "plain packet length-data length:" + to_string(iDecryptedTextSize) + "-" + to_string(datalength) + " length packet:");
			/*
			for(int i=0;i<4;i++) {
			//lb[i]=static_cast<byte>(strDecryptedText[i]);
			std::cout << hex << (0xFF & static_cast<byte>(strDecryptedText[i])) << " ";
			}
			cout << endl;
			 */
			for (int i = 0; i < datalength; i++) {
				decryptedPacket[i] = static_cast<byte> (strDecryptedText[i + 4]);
				//	std::cout << hex << (0xFF & static_cast<byte>(strDecryptedText[i+4])) << " ";
			}
			// Write the incoming packet to the fd_tun_interfacd.
			gettimeofday(&eTime, NULL);
			exeTime = (eTime.tv_sec - sTime.tv_sec)*1000000 + (eTime.tv_usec - sTime.tv_usec); //exeTime 单位是微秒
			if (exeTime > 1000)
				log(log_level::INFO, FUN_NAME, "decrypt exeTime:" + to_string(exeTime));
			log(log_level::DEBUG, FUN_NAME, "write to fd_tun_interface. decryptedPacket datalength is:" + to_string(datalength));
			write(fd_tun_interface, decryptedPacket, datalength);
		} else {
			//不是数据报文,而是建立建立连接命令报文,检查约定的密钥是否正确,正确则记录该客户端,并向该客户端发送响应报文
			//手机客户端命令报文数据格式为[0][sharedsecret][:][deviceId]
			log(log_level::DEBUG, FUN_NAME, "process cmd packet. pkt_length:" + to_string(packet_len));
			string str_recv_pkt1, str_recv_deviceId, str_recv_secret;
			str_recv_pkt1.append(&packet[1], packet_len - 1); //skip packet[0]
			string secret_delimiter = ":";
			string secret = "test0";
			str_recv_secret = str_recv_pkt1.substr(0, str_recv_pkt1.find(secret_delimiter));
			str_recv_deviceId = str_recv_pkt1.substr(str_recv_pkt1.find(secret_delimiter) + 1);
			log(log_level::DEBUG, FUN_NAME, "recv_secret is[" + str_recv_secret + "]. recvDeviceId is [" + str_recv_deviceId + "] deviceid length:" + to_string(str_recv_deviceId.length()));
			if (str_recv_secret == secret) {
				//验证通过,记录客户端IP,端口信息等
				string deviceId = str_recv_deviceId;
				string tun_ip = tunip_pool.assignTunIPAddr(deviceId);
				string internet_ip = pkt_node->pkt_internetAddr;
				int internet_port = pkt_node->pkt_internetPort;
				//PeerClient * peerClient = new PeerClient(deviceId, tun_ip, internet_ip, internet_port);
				//ptr_peerClientTable->addPeerClient(tun_ip, peerClient);
				ptr_peerClientTable->addPeerClient(deviceId, tun_ip, internet_ip, internet_port);
				//验证通过,发送响应报文,包括tun私有地址,dns地址等信息,发送三次
				char parameters[1024];
				build_parameters(parameters, sizeof (parameters), tun_ip, conf_m);
				sockaddr_in peer_cli_addr;
				memset(&peer_cli_addr, 0, sizeof (peer_cli_addr));
				peer_cli_addr.sin_family = AF_INET;
				peer_cli_addr.sin_port = htons(pkt_node->pkt_internetPort);
				inet_pton(AF_INET, pkt_node->pkt_internetAddr.c_str(), &peer_cli_addr.sin_addr);
				for (int i = 0; i < 3; ++i) {
					sendto(socketfd_tunnel, parameters, sizeof (parameters), MSG_NOSIGNAL, (sockaddr *) & peer_cli_addr, sizeof (peer_cli_addr));
				}
			}
		}
		tunnelReceiver_packetPool.consumeCompleted(pkt_node);
	}
	return 0;
}

//build_parameters()方法构建参数,将tun接口的配置数据发送给通过验证的客户端
//example:
//	char parameters[1024];
//	build_parameters(parameters, sizeof(parameters), argc, argv);
//	for (int i = 0; i < 3; ++i) {
//		//logger << "send parameter." << endl;
//		send(tunnel, parameters, sizeof(parameters), MSG_NOSIGNAL);
//	}
//参数主要有tun的地址,dns的地址,mtu
//sudo ./ToyVpnServer_encrypt $TUN_NAME $VPN_PORT $SECRET $CaptureCMD -m 1400 -a $PRIVATE_ADDR 32 -d $DNS_ADDR -r 0.0.0.0 0
//	"  -m <MTU> for the maximum transmission unit\n"
//	"  -a <address> <prefix-length> for the private address\n"
//	"  -r <address> <prefix-length> for the forwarding route\n"
//	"  -d <address> for the domain name server\n"

//m,<MTU> a,<PRIVATE_ADDR>,32 <d>,<DNS_ADDR> r,<0.0.0.0>,<0>
int build_parameters(char *parameters, int size, const string tun_ip, const map<string, string> & conf_m) {
	// Well, for simplicity, we just concatenate them (almost) blindly.
	int offset = 0;
	const string FUN_NAME = "build_parameters";
	string str_MTU, str_DNS;
	map<string, string>::const_iterator cit = conf_m.find("MTU");
	if (cit != conf_m.end()) {
		str_MTU = cit->second;
	} else {
		log(log_level::FATAL, FUN_NAME, "cannot found MTU configure");
	}
	cit = conf_m.find("DNS_ADDR");
	if (cit != conf_m.end()) {
		str_DNS = cit->second;
	} else {
		log(log_level::FATAL, FUN_NAME, "cannot found DNS_ADDR configure");
	}
	string str_ROUTE = "r,0.0.0.0,0";
	string delimiter = " ";
	string str_parameter = "m," + str_MTU + delimiter + "a," + tun_ip + ",32" + delimiter + "d," + str_DNS + delimiter + str_ROUTE;

	// Append the delimiter and the parameter.
	int length = str_parameter.length();
	memcpy(&parameters[offset + 1], str_parameter.c_str(), length);
	offset += 1 + length;
	// Fill the rest of the space with spaces.
	memset(&parameters[offset], ' ', size - offset);

	// Control messages always start with zero.
	parameters[0] = 0;
}