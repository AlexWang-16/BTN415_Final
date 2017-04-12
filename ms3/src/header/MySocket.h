#ifndef MYSOCKET_H
#define MYSOCKET_H
#define NOMINMAX

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <thread>
#include <chrono>
#include <limits>

enum  SocketType { CLIENT, SERVER };
enum  ConnectionType { TCP, UDP };
const int DEFAULT_SIZE = 128;

class MySocket {

protected:
	int version_num1, version_num2;
	int port;
	std::string IPAddr;
	WSADATA wsa_data;
	char *Buffer;

	SOCKET welcomeSocket, connectionSocket;

	struct sockaddr_in svrAddr;

	SocketType mySocket;
	ConnectionType connectionType;

	bool bTCPConnect = false;		//True if connection is established

	int maxSize;

public:
	//constructor and destructor
	MySocket(SocketType socketType, std::string ipAddress, 
    unsigned int portNumber, ConnectionType connectionType, 
    unsigned int bufferSize);
	~MySocket();

	//getter functions
	int getPort();
	std::string getIPAddr();
	SocketType getType();
	ConnectionType getConnectionType();
	
	//setter functions
	void setIPAddr(std::string ipAddress);
	void setPort(int portNumber);
	void setType(SocketType socketType);

	//socket functions
	void start_DLLS();
	SOCKET initializeTcpSocket();
	SOCKET initializeUdpSocket();
	void bindSocket();
	void listenSocket();
  void acceptConnection();

	//connection and transmission functions
	void connectTCP();
	void disconnectTCP();
	int getData(char* recvBuffer);
	void sendData(const char* data, int dataSize);

};

void commandThread(std::string ip, int port);
void telemetryThread(std::string ipAddress, int port);
#endif MYSOCKET_H