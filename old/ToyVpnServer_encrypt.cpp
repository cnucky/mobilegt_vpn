
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctime>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include <net/if.h>
#include <linux/if_tun.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
using namespace std;

extern char * tunInterfaceName;

#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/filters.h"
using namespace CryptoPP;


template <typename Type>
string numToString(Type num)
{
	stringstream ss;
	string s;
	ss << num;
	ss >> s;
	return s;
}

string logfileNameBase="ToyVpnlog_";
int loground[10]={0,1,2,3,4,5,6,7,8,9};
int cLoground=0;
string currentLogFile;
ofstream logger;
void checkLogger(string currentLogFile)
{
	cout << "debug:" << currentLogFile << endl;
	if(!logger.is_open()) {
		logger.open(currentLogFile.c_str(),ios::trunc|ios::out);
		cout << "debug: openfile" << endl;
	}
	struct stat buf;
	if(stat(currentLogFile.c_str(), &buf)<0)
		cout << "debug: stat file failed:" << currentLogFile << endl;
	if(buf.st_size>(10*1024*1024)) {
		logger.close();
		cLoground++;
		cLoground %= 10;
		currentLogFile=logfileNameBase+"."+numToString(loground[cLoground]);
		logger.open(currentLogFile.c_str(),ios::trunc|ios::out);
	}	
}

char * cmdName=NULL;
int startCapture(const char * deviceId)
{
	//const char * cmdName="/home/ubuntu/vpnserver/checkCapture.sh";
	const char * argv0="checkCapture.sh";
	signal( SIGCHLD, SIG_IGN );     //!> 忽略产生僵尸进程 
	pid_t pid;
	if((pid=fork())<0) {
		logger << "fork error" << endl;
		return pid;
	}
	if(pid==0) {
		logger << " this child process, pid is [" << getpid() << "]\n";
		execlp(cmdName, argv0, tunInterfaceName, deviceId, NULL);
		exit(0);
	} else {
		logger << "father ok! " << "father pid is [" << getpid() << "]child pid is [" << pid << "]" << endl;
	}
	return pid;
}

static int get_interface(char *name)
{
	int interface = open("/dev/net/tun", O_RDWR | O_NONBLOCK);

	ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	if (ioctl(interface, TUNSETIFF, &ifr)) {
		logger << "Cannot get TUN interface" << endl;
		exit(1);
	}

	return interface;
}

char * recvDeviceId=NULL;//AES key

static int get_tunnel(char *port, char *secret)
{
	time_t cur_timer;
	time(&cur_timer);
	logger << "-------------------------\n" << ctime(&cur_timer) << " get tunnel:" << port << endl;
/*
	// We use an IPv6 socket to cover both IPv4 and IPv6.
	int tunnel = socket(AF_INET6, SOCK_DGRAM, 0);
	int flag = 1;
	setsockopt(tunnel, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	flag = 0;
	setsockopt(tunnel, IPPROTO_IPV6, IPV6_V6ONLY, &flag, sizeof(flag));

	// Accept packets received on any local address.
	sockaddr_in6 addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(atoi(port));
*/

	// We use an IPv4 socket.
	int tunnel = socket(AF_INET, SOCK_DGRAM, 0);
	int flag = 1;
	setsockopt(tunnel, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	flag = 0;


	// Accept packets received on any local address.
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));

	// Call bind(2) in a loop since Linux does not have SO_REUSEPORT.
	logger << "bind port:" << port << endl;
	while (bind(tunnel, (sockaddr *)&addr, sizeof(addr))) {
		if (errno != EADDRINUSE) {
			return -1;
		}
		usleep(100000);
	}
	char claddrStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET,&addr.sin_addr,claddrStr,INET_ADDRSTRLEN);
	logger << "bind port completed! " << claddrStr << ":" << port;
	logger << " secret is:" << secret << endl;
	// Receive packets till the secret matches.
	char packet[1024];	
	char key[]=":";
	socklen_t addrlen;
	char secret_deviceId[40];
	do {
		addrlen = sizeof(addr);
		int n = recvfrom(tunnel, packet, sizeof(packet), 0,
				(sockaddr *)&addr, &addrlen);
		if (n <= 0) {
			return -1;
		}
		packet[n] = 0;
		char IPdotdec[20];
		inet_ntop(AF_INET,(void *)&addr.sin_addr,IPdotdec,16);
		logger << "recv:" << &packet[1] << " from " << IPdotdec << ":" << ntohs(addr.sin_port) << endl;
		recvDeviceId=NULL;
		recvDeviceId=strpbrk(&packet[1],key);
		if(recvDeviceId!=NULL)
			logger << "recvDeviceId is " << recvDeviceId+1 << endl;
		strcpy(secret_deviceId,secret);
	} while (packet[0] != 0 || strcmp(strcat(secret_deviceId,(const char *)recvDeviceId), &packet[1]));

	// Connect to the client as we only handle one client at a time.
	
	inet_ntop(AF_INET,&addr.sin_addr,claddrStr,INET_ADDRSTRLEN);
	logger << "connect tunnel to " << claddrStr << ":" << ntohs(addr.sin_port) << endl;
	connect(tunnel, (sockaddr *)&addr, addrlen);

	// start capture
	
	const char * deviceId=recvDeviceId+1;
	logger << "startCapture " << deviceId << endl;
	startCapture(deviceId);
	return tunnel;
}

static void build_parameters(char *parameters, int size, int argc, char **argv)
{
	// Well, for simplicity, we just concatenate them (almost) blindly.
	int offset = 0;
	for (int i = 5; i < argc; ++i) {
		char *parameter = argv[i];
		int length = strlen(parameter);
		char delimiter = ',';

		// If it looks like an option, prepend a space instead of a comma.
		if (length == 2 && parameter[0] == '-') {
			++parameter;
			--length;
			delimiter = ' ';
		}

		// This is just a demo app, really.
		if (offset + length >= size) {
			logger << "Parameters are too large";
			exit(1);
		}

		// Append the delimiter and the parameter.
		parameters[offset] = delimiter;
		memcpy(&parameters[offset + 1], parameter, length);
		offset += 1 + length;
	}

	// Fill the rest of the space with spaces.
	memset(&parameters[offset], ' ', size - offset);

	// Control messages always start with zero.
	parameters[0] = 0;
}

void  intToByte(int i,byte *bytes,int size = 4)
{
	 //byte[] bytes = new byte[4];
	memset(bytes,0,sizeof(byte) *  size);
	bytes[3] = (byte) (0xff & i);
	bytes[2] = (byte) ((0xff00 & i) >> 8);
	bytes[1] = (byte) ((0xff0000 & i) >> 16);
	bytes[0] = (byte) ((0xff000000 & i) >> 24);
	return ;
 }

//byte转int
 int bytesToInt(const byte * bytes,int size = 4) 
{
	int addr = bytes[3] & 0xFF;
	addr |= ((bytes[2] << 8) & 0xFF00);
	addr |= ((bytes[1] << 16) & 0xFF0000);
	addr |= ((bytes[0] << 24) & 0xFF000000);
	return addr;
 }

//-----------------------------------------------------------------------------

char * tunInterfaceName = NULL;

int main(int argc, char **argv)
{
	if (argc < 6) {
		printf("Usage: %s <tunN> <port> <secret> <checkCaptureCMD> options...\n"
			   "\n"
			   "Options:\n"
			   "  -m <MTU> for the maximum transmission unit\n"
			   "  -a <address> <prefix-length> for the private address\n"
			   "  -r <address> <prefix-length> for the forwarding route\n"
			   "  -d <address> for the domain name server\n"
			   "  -s <domain> for the search domain\n"
			   "\n"
			   "Note that TUN interface needs to be configured properly\n"
			   "BEFORE running this program. For more information, please\n"
			   "read the comments in the source code.\n\n", argv[0]);
		exit(1);
	}

	//设置aes 密钥, default: 16byte,128bit
	byte key[AES::DEFAULT_KEYLENGTH] = {'t','e','s','t','0','r','y','w','a','n','g','0','0','0','0','0'};
	//设置初始向量组, 可以默认随机，但是一旦指定，加密和解密必须使用相同的初始向量组,16byte
	byte iv[AES::BLOCKSIZE] = {'t','o','y','v','p','n','s','e','r','v','e','r','0','0','0','0'};

	AESEncryption aesEncryption;
	aesEncryption.SetKey(key, AES::DEFAULT_KEYLENGTH);  
	//CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
	//等同于下面两句构建基于AES 算法的CBC模式
	//CBC_Mode<AES>::Encryption cbcEncryption;
	//cbcEncryption.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv);

	AESDecryption aesDecryption;
	aesDecryption.SetKey(key, AES::DEFAULT_KEYLENGTH);
	//CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	logfileNameBase+=argv[1];
	currentLogFile=logfileNameBase+"."+numToString(loground[cLoground]);
	checkLogger(currentLogFile);
	// Parse the arguments and set the parameters.
	char parameters[1024];
	logger << "build parameters" << endl;
	build_parameters(parameters, sizeof(parameters), argc, argv);

	// Get TUN interface.
	int interface = get_interface(argv[1]);
	tunInterfaceName = argv[1];
	cmdName=argv[4];
	// Wait for a tunnel.
	int tunnel;
	logger << "begin get tunnel:" << argv[1] << "," << argv[2] <<"," << argv[3] << endl;
	while ((tunnel = get_tunnel(argv[2], argv[3])) != -1) {
		logger << argv[1] << " : Here comes a new tunnel" << endl;

		// On UN*X, there are many ways to deal with multiple file
		// descriptors, such as poll(2), select(2), epoll(7) on Linux,
		// kqueue(2) on FreeBSD, pthread(3), or even fork(2). Here we
		// mimic everything from the client, so their source code can
		// be easily compared side by side.

		// Put the tunnel into non-blocking mode.
		fcntl(tunnel, F_SETFL, O_NONBLOCK);

		// Send the parameters several times in case of packet loss.
		for (int i = 0; i < 3; ++i) {
			//logger << "send parameter." << endl;
			send(tunnel, parameters, sizeof(parameters), MSG_NOSIGNAL);
		}

		// Allocate the buffer for a single packet.
		char packet[2048];
		char encryptedPacket[2048];
		char decryptedPacket[2048];
		// We use a timer to determine the status of the tunnel. It
		// works on both sides. A positive value means sending, and
		// any other means receiving. We start with receiving.
		int timer = 0;

struct timeval sTime, eTime;
long exeTime;
		// We keep forwarding packets till something goes wrong.
		while (true) {
		try {
			// Assume that we did not make any progress in this iteration.
			bool idle = true;
	
			// Read the outgoing packet from the input stream.
			int length = read(interface, packet, sizeof(packet));
			//printf("\ntest internet input length:%d",length);
			if (length > 0) {
		
				//system("date");
				//printf("\nINPUT PACKET==========================\n");
				//printPacket(packet);
				// There might be more outgoing packets.
				byte lb [4];
				intToByte(length,lb);
				//logger << "\n\trecv input packet from internet length:" << dec << length <<" length & packets:" << endl;
				/*
				for(int i=0;i<4;i++)
					cout << hex << (0xFF & lb[i]) << " ";
				cout << endl;
				for(int i=0;i<length;i++)
					cout << hex << (0xFF & packet[i]) << " ";
				cout << endl;
				*/
				//encrypt packet;
				//string plaintext(packet,0,length);
				string strCiphertext = "";
gettimeofday(&sTime, NULL);
				CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);
gettimeofday(&eTime, NULL);
exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec); //exeTime 单位是微秒
if(exeTime>1000)
cout << "encrypt init exeTime:" << exeTime << endl;
gettimeofday(&sTime, NULL);
				StreamTransformationFilter stfEncryptor(cbcEncryption, new StringSink(strCiphertext),BlockPaddingSchemeDef::ZEROS_PADDING);
				stfEncryptor.Put(lb,4);
				stfEncryptor.Put((byte*)(packet),length);
				//stfEncryptor.Put((byte*)(plaintext.c_str()),plaintext.length());
				stfEncryptor.MessageEnd();
		
				int iCipherTextSize = strCiphertext.size();
				//logger << "\tencrypted packet length: " << dec << iCipherTextSize << " packet:" << endl;
				encryptedPacket[0]=1; 
				//cout << hex << (0xFF & encryptedPacket[0]) << " ";		
				for( int i = 0; i<iCipherTextSize; i++) {
					encryptedPacket[i+1]=static_cast<byte>(strCiphertext[i]);
					//   std::cout <<hex << (0xFF & encryptedPacket[i+1]) << " ";
				}
gettimeofday(&eTime, NULL);
exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec); //exeTime 单位是微秒
if(exeTime>1000)
cout << "encrypt exeTime:" << exeTime << endl;
				//cout << endl;
				// Write the outgoing packet to the tunnel.
				//send(tunnel, packet, length, MSG_NOSIGNAL);
				int size=send(tunnel, encryptedPacket, iCipherTextSize+1, MSG_NOSIGNAL);
				//cout << "send packet length:" << dec << size;
				idle = false;

				// If we were receiving, switch to sending.
				if (timer < 1) {
					timer = 1;
				}
			}

			// Read the incoming packet from the tunnel.
			length = recv(tunnel, packet, sizeof(packet), 0);
			//printf("\ntest tunnel input length:%d",length);
			if (length == 0) {
				break;
			}
			if (length > 0) {
				// Ignore control messages, which start with zero.
				if (packet[0] != 0) {
gettimeofday(&sTime, NULL);
	 				CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);
gettimeofday(&eTime, NULL);
exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec); //exeTime 单位是微秒
if(exeTime>1000)
cout << "decrypt init exeTime:" << exeTime << endl;
					//decrypt packet;
					std::string strDecryptedText = "";
					//stfDecryptor.Put((byte*)(strCiphertext.c_str() ),strCiphertext.length());
					//logger << "\nrecv cipher packet from tunnel length:" << dec << length << " packet:" << endl;			
					/*
					for(int i=0;i<length;i++)
					cout <<hex<< (0xFF & static_cast<byte>(packet[i])) << " ";
					cout << endl;
					*/
gettimeofday(&sTime, NULL);
					StreamTransformationFilter stfDecryptor(cbcDecryption, new StringSink(strDecryptedText),BlockPaddingSchemeDef::ZEROS_PADDING);					
					stfDecryptor.Put((byte*)packet+1,length-1);
					stfDecryptor.MessageEnd();

					int iDecryptedTextSize = strDecryptedText.size();
					byte lb[4];
					for(int i=0;i<4;i++) {
						lb[i]=static_cast<byte>(strDecryptedText[i]);
						//std::cout << hex << (0xFF & static_cast<byte>(strDecryptedText[i])) << " ";
					}

					int datalength=bytesToInt(lb);

					//logger << "plain packet length-data length:" << dec << iDecryptedTextSize << "-" << datalength << " length packet:" << endl;
					/*
					for(int i=0;i<4;i++) {
					//lb[i]=static_cast<byte>(strDecryptedText[i]);
					std::cout << hex << (0xFF & static_cast<byte>(strDecryptedText[i])) << " ";
					}
					cout << endl;
					*/
					for( int i = 0; i<datalength; i++) {
						decryptedPacket[i]=static_cast<byte>(strDecryptedText[i+4]);
					//	std::cout << hex << (0xFF & static_cast<byte>(strDecryptedText[i+4])) << " ";
					}
					// Write the incoming packet to the output stream.
					//system("date");
					//printf("\nOUTPUT PACKET==========================\n");
					//printPacket(packet);
					//write(interface, packet, length);
gettimeofday(&eTime, NULL);
exeTime = (eTime.tv_sec-sTime.tv_sec)*1000000+(eTime.tv_usec-sTime.tv_usec); //exeTime 单位是微秒
if(exeTime>1000)
cout << "decrypt exeTime:" << exeTime << endl;
					write(interface, decryptedPacket, datalength);
				}

				// There might be more incoming packets.
				idle = false;

				// If we were sending, switch to receiving.
				if (timer > 0) {
					timer = 0;
				}
			}

			// If we are idle or waiting for the network, sleep for a
			// fraction of time to avoid busy looping.
			if (idle) {
				usleep(100000);

				// Increase the timer. This is inaccurate but good enough,
				// since everything is operated in non-blocking mode.
				timer += (timer > 0) ? 100 : -100;

				// We are receiving for a long time but not sending.
				// Can you figure out why we use a different value? :)
				if (timer < -16000) {
					// Send empty control messages.
					packet[0] = 0;
					for (int i = 0; i < 3; ++i) {
						send(tunnel, packet, 1, MSG_NOSIGNAL);
					}

					// Switch to sending.
					timer = 1;
				}

				// We are sending for a long time but not receiving.
				if (timer > 20000) {
					break;
				}
			}
		} catch(...) {
			logger << "exception" << endl;
			break;
		}
		}//end while
		logger << argv[1] << " : The tunnel is broken" << endl;
		close(tunnel);
	}
	logger << "Cannot create tunnels" << endl;
	exit(1);
}

