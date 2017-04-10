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

	bool bTCPConnect = false;

	int MaxSize;

public:
	MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);

	void ConnectTCP();
	void DisconnectTCP();
	void SendData(const char*, int);
	int GetData(char*);
	std::string GetIPAddr();
	void SetIPAddr(std::string);
	void SetPort(int);
	int GetPort();
	SocketType GetType();
	void SetType(SocketType);
	ConnectionType GetConnectionType();
	void SetConnectionType(ConnectionType connType);
	void start_DLLS();
	SOCKET initialize_tcp_socket();
	SOCKET initialize_udp_socket();
	void bind_socket();
	void listen_socket();
	void accept_connection();
	~MySocket();
};


#endif MYSOCKET_H