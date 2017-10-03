
#include "mobilegt_util.h"
/*
 * 处理TUN interface接收到的来自Internet的数据,然后转发回给手机客户端,采用多线程处理机制
 * 
 * 1. 接收到的是明文数据,依据报文的目的TUN_IP找到对应的手机客户端地址
 * 2. 加密数据,然后通过tunnel socket发送给对应的手机客户端
 *  
 */

int tunDataProcess(PacketPool & tunReceiver_packetPool, int socketfd_tunnel) {
	const string FUN_NAME = "tunDataProcess";
	const int WARN_THRESHOLD = 10000; //微秒
	EncryptDecryptHelper * aes_helper = EncryptDecryptHelper::getInstance();
	struct timeval sTime, eTime;
	long exeTime;
	char encryptedPacket[2048];
	PeerClientTable * ptr_peerClientTable = PeerClientTable::getInstance();
	while (true) {
		PacketNode* pkt_node = tunReceiver_packetPool.consume();
		if (pkt_node == NULL) {
			if (SYSTEM_EXIT)
				break;
			//阻塞等待通知有数据?
			//休眠然后重试
			this_thread::sleep_for(chrono::milliseconds(100)); //std::this_thread;std::chrono;
			continue;
		}
		//process pkt_node里面的数据
		char* packet = pkt_node->ptr;
		int packet_len = pkt_node->pkt_len;
		if (packet_len > 0) {
			byte lb [4];
			intToByte(packet_len, lb);
			if (OPEN_DEBUGLOG)
				log(log_level::DEBUG, FUN_NAME, "process internet response data packet. packet length is:" + to_string(packet_len));

			/*
			16进制输出加密前报文内容
			for(int i=0;i<4;i++)
				cout << hex << (0xFF & lb[i]) << " ";
			cout << endl;
			for(int i=0;i<length;i++)
				cout << hex << (0xFF & packet[i]) << " ";
			cout << endl;
			 */
			//encrypt packet;
			string strCiphertext = "";
			gettimeofday(&sTime, NULL);
			CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aes_helper->aesEncryption, aes_helper->iv);
			gettimeofday(&eTime, NULL);
			exeTime = (eTime.tv_sec - sTime.tv_sec)*1000000 + (eTime.tv_usec - sTime.tv_usec); //exeTime 单位是微秒
			if (exeTime > WARN_THRESHOLD)
				log(log_level::WARN, FUN_NAME, "encrypt init exeTime:" + to_string(exeTime));
			gettimeofday(&sTime, NULL);
			CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(strCiphertext), CryptoPP::BlockPaddingSchemeDef::ZEROS_PADDING);
			stfEncryptor.Put(lb, 4);
			stfEncryptor.Put((byte*) (packet), packet_len);
			stfEncryptor.MessageEnd();

			int iCipherTextSize = strCiphertext.size();
			//logger << "\tencrypted packet length: " << dec << iCipherTextSize << " packet:" << endl;
			encryptedPacket[0] = 1;
			//cout << hex << (0xFF & encryptedPacket[0]) << " ";
			for (int i = 0; i < iCipherTextSize; i++) {
				encryptedPacket[i + 1] = static_cast<byte> (strCiphertext[i]);
				//   std::cout <<hex << (0xFF & encryptedPacket[i+1]) << " ";
			}
			gettimeofday(&eTime, NULL);
			exeTime = (eTime.tv_sec - sTime.tv_sec)*1000000 + (eTime.tv_usec - sTime.tv_usec); //exeTime 单位是微秒
			if (exeTime > WARN_THRESHOLD)
				log(log_level::WARN, FUN_NAME, "encrypt exeTime:" + to_string(exeTime));
			//依据tun ip找到需要发送的目的客户端internet ip和port
			if (OPEN_DEBUGLOG)
				log(log_level::DEBUG, FUN_NAME, "find node peer. peer tun ip is:" + pkt_node->pkt_tunAddr);
			PeerClient * peerClient = ptr_peerClientTable->getPeerClientByTunIP(pkt_node->pkt_tunAddr);
			if (peerClient != NULL) {
				peerClient->increaseDataPktCount_recv();
				if (OPEN_DEBUGLOG)
					log(log_level::DEBUG, FUN_NAME, "finded internet ip & port:" + peerClient->getPeer_internet_ip() + ":" + to_string(peerClient->getPeer_internet_port()));
			} else {
				log(log_level::ERROR, FUN_NAME, "cannot find internet ip & port for peer tun ip:" + pkt_node->pkt_tunAddr);
				tunReceiver_packetPool.consumeCompleted(pkt_node); //设置处理数据完毕
				continue;
			}
			sockaddr_in peer_cli_addr;
			memset(&peer_cli_addr, 0, sizeof (peer_cli_addr));
			peer_cli_addr.sin_family = AF_INET;
			peer_cli_addr.sin_port = htons(peerClient->getPeer_internet_port());
			inet_pton(AF_INET, peerClient->getPeer_internet_ip().c_str(), &peer_cli_addr.sin_addr);
			if (OPEN_DEBUGLOG)
				log(log_level::DEBUG, FUN_NAME, "send encryptedPacket to socketfd_tunnel. packet size:" + to_string(iCipherTextSize + 1));
			int size = sendto(socketfd_tunnel, encryptedPacket, iCipherTextSize + 1, MSG_NOSIGNAL, (sockaddr *) & peer_cli_addr, sizeof (peer_cli_addr));
		}
		tunReceiver_packetPool.consumeCompleted(pkt_node);
	}
	stringstream ss;
	ss << this_thread::get_id();
	log(log_level::FATAL, FUN_NAME, "thread[" + ss.str() + "] exit!");
	return 0;
}