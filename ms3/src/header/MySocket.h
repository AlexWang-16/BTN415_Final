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

	SOCKET WelcomeSocket, ConnectionSocket;

	struct sockaddr_in SvrAddr;

	SocketType mySocket;
	ConnectionType connectionType;

	bool bTCPConnect = false;		//True if connection is established

	int MaxSize;

public:
	//constructor and destructor
	MySocket(SocketType socketType, std::string ipAddress, 
    unsigned int portNumber, ConnectionType connectionType, 
    unsigned int bufferSize);
	~MySocket();

	//getter functions
	int GetPort();
	std::string GetIPAddr();
	SocketType GetType();
	ConnectionType GetConnectionType();
	
	//setter functions
	void SetIPAddr(std::string ipAddress);
	void SetPort(int portNumber);
	void SetType(SocketType socketType);

	//socket functions
	void start_DLLS();
	SOCKET initialize_tcp_socket();
	SOCKET initialize_udp_socket();
	void bind_socket();
	void listen_socket();
	void accept_connection();

	//connection and transmission functions
	void ConnectTCP();
	void DisconnectTCP();
	int GetData(char* recvBuffer);
	void SendData(const char* data, int dataSize);

};

void commandThread(std::string ip, int port);
void telemetryThread(std::string ipAddress, int port);
#endif MYSOCKET_H