#include "../header/MySocket.h"

MySocket::MySocket(SocketType sockType, std::string ipAddr, unsigned int port, ConnectionType connectType, unsigned int size) {
	this->version_num1 = 2;
	this->version_num2 = 2;
	this->start_DLLS();

	SetType(sockType);
	SetIPAddr(ipAddr);
	SetPort(port);
	connectionType = connectType;

	if (0 < size && size <= std::numeric_limits<int>::max()) {
		Buffer = new char[size];
		this->MaxSize = size;
	}
	else {
		Buffer = new char[DEFAULT_SIZE];
		this->MaxSize = DEFAULT_SIZE;
	}

	if (this->GetConnectionType() == TCP) {
		if (this->GetType() == SERVER) {
			this->WelcomeSocket = this->initialize_tcp_socket();
			this->bind_socket();
			this->listen_socket();
			this->accept_connection();
		}
		else {
			this->ConnectionSocket = this->initialize_tcp_socket();
		}
	}
	else if (this->GetConnectionType() == UDP) {
		this->WelcomeSocket = this->initialize_udp_socket();
		if (this->GetType() == SERVER) {
			this->bind_socket();
		}
	}
}

void MySocket::ConnectTCP()
{
	std::cout << "Trying to connect to the server" << std::endl;

	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET; //Address family type internet
	SvrAddr.sin_port = htons(this->GetPort()); //port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str()); //IP address
	while (!this->bTCPConnect) {
		if ((connect(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		else {
			std::cout << "Connection Established" << std::endl;
			this->bTCPConnect = true;
		}
	}
}

void MySocket::DisconnectTCP()
{
	if (this->GetType() == SERVER) {
		closesocket(this->ConnectionSocket);
		closesocket(this->WelcomeSocket);
		WSACleanup();
	}
	else {
		closesocket(this->ConnectionSocket);
		WSACleanup();
	}
}

void MySocket::SendData(const char* data, int dataSize)
{
	if (this->connectionType == TCP) {
		//we transmit dataSize + 1 length so the \0 will be included
		send(this->ConnectionSocket, data, dataSize + 1, 0);
	}
	else if (this->connectionType == UDP) {
		//we transmit dataSize + 1 length so the \0 will be included
		sendto(this->ConnectionSocket, data, dataSize + 1, 0,
			(struct sockaddr *)&SvrAddr, sizeof(SvrAddr));
	}
}

int MySocket::GetData(char * data)
{
	if (this->connectionType == TCP) {
		recv(this->ConnectionSocket, data, this->MaxSize, 0);
	}
	else if (this->connectionType == UDP) {
		struct sockaddr_in CltAddr; //A struct that holds client IP and port info
		CltAddr.sin_family = AF_INET;
		CltAddr.sin_port = htons(this->port);
		CltAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str());
		int addr_len = sizeof(CltAddr);
		recvfrom(this->WelcomeSocket, data, this->MaxSize, 0, (struct sockaddr *) &CltAddr, &addr_len);
	}
	return this->MaxSize;
}

std::string MySocket::GetIPAddr()
{
	return this->IPAddr;
}

void MySocket::SetIPAddr(std::string ipAdd)
{
	this->IPAddr = ipAdd;
}

void MySocket::SetPort(int p)
{
	this->port = p;
}

int MySocket::GetPort()
{
	return this->port;
}

SocketType MySocket::GetType()
{
	return this->mySocket;
}

void MySocket::SetType(SocketType sockType)
{
	this->mySocket = sockType;
}

ConnectionType MySocket::GetConnectionType()
{
	return this->connectionType;
}

void MySocket::SetConnectionType(ConnectionType connType)
{
	this->connectionType = connType;
}

void MySocket::start_DLLS()
{
	if ((WSAStartup(MAKEWORD(this->version_num1, this->version_num2), &this->wsa_data)) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		std::cin.get();
		exit(0);
	}
}

SOCKET MySocket::initialize_tcp_socket()
{
	SOCKET LocalSocket;
	LocalSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (LocalSocket == INVALID_SOCKET) {
		WSACleanup();
		std::cout << "Could not initialize socket" << std::endl;
		std::cin.get();
		exit(0);
	}
	return LocalSocket;
}

SOCKET MySocket::initialize_udp_socket()
{
	SOCKET LocalSocket;
	LocalSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (LocalSocket == INVALID_SOCKET) {
		WSACleanup();
		std::cout << "Could not initialize socket" << std::endl;
		std::cin.get();
		exit(0);
	}
	return LocalSocket;
}

void MySocket::bind_socket()
{
	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET; //Address family type internet
	SvrAddr.sin_port = htons(this->GetPort()); //port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str()); //IP address
	if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)))
		== SOCKET_ERROR) {
		closesocket(this->WelcomeSocket);
		WSACleanup();
		std::cout << "Could not bind to the socket" << std::endl;
		std::cin.get();
		exit(0);
	}
}

void MySocket::listen_socket()
{
	if (listen(this->WelcomeSocket, 1) == SOCKET_ERROR) {
		closesocket(this->WelcomeSocket);
		WSACleanup();
		std::cout << "Could not listen to the provided socket." << std::endl;
		std::cin.get();
		exit(0);
	}
	else {
		std::cout << "Waiting for client connection" << std::endl;
	}
}

void MySocket::accept_connection() {
	if ((this->ConnectionSocket = accept(this->WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
		closesocket(this->WelcomeSocket);
		WSACleanup();
		std::cout << "Could not accept incoming connection." << std::endl;
		std::cin.get();
		exit(0);
	}
	else {
		std::cout << "Connection Accepted" << std::endl;
	}
}

MySocket::~MySocket()
{
	delete[] this->Buffer;
}
