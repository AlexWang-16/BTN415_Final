#include "../header/MySocket.h"

MySocket::MySocket(SocketType sockType, std::string ipAddr, unsigned int port, ConnectionType connectType, unsigned int size) {
	this->version_num1 = 2;
	this->version_num2 = 2;
	this->start_DLLS();

	SetType(sockType);
	SetIPAddr(ipAddr);
	SetPort(port);
	connectionType = connectType;
	if (size > 0) {
		Buffer = new char[size];
	}
	else {
		Buffer = new char[DEFAULT_SIZE];
	}

	if (this->GetConnectionType() == TCP) {
		this->WelcomeSocket = this->initialize_tcp_socket();
		if (this->GetType() == SERVER) {	
			this->bind_socket();
			this->listen_socket();
			this->ConnectTCP();
		//	this->accept_connection();
		}
		else {
			this->ConnectToTCPServer();
		}
		
	}
	else if (this->GetConnectionType() == UDP) {
		this->WelcomeSocket = this->initialize_udp_socket();
	}
}

void MySocket::ConnectTCP()
{
	//if (this)
	this->accept_connection();

}


/*
bool tcp_node::three_way_handshake(tcp_node *server, channel * wireless){
	send_signal(server, this, wireless, "SYN");
	send_signal(this, server, wireless, "SYN/ACK");
	send_signal(server, this, wireless, "ACK");
	return true;
}

bool tcp_node::four_way_handshake(tcp_node *server, channel * wireless){
	send_signal(server, this, wireless, "FIN");
	send_signal(this, server, wireless, "ACK/FIN");
	send_signal(this, server, wireless, "FIN");
	send_signal(server, this, wireless, "ACK/FIN");
return true;
}

*/


void MySocket::DisconnectTCP()
{
	closesocket(ConnectionSocket);
	closesocket(WelcomeSocket);

}

void MySocket::SendData(const char *, int)
{
	//TODO
}

int MySocket::GetData(char *)
{
	//TODO
	return 0;
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
	if (this->GetType() == SERVER) {
		if (this->GetConnectionType() == UDP) {
			struct sockaddr_in SvrAddr;
			SvrAddr.sin_family = AF_INET;
			SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str());
			SvrAddr.sin_port = htons(this->GetPort());
			if (bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr,
				sizeof(SvrAddr)) == SOCKET_ERROR)
			{
				closesocket(this->WelcomeSocket);
				WSACleanup();
				return;
			}
		}
		else if (this->GetConnectionType() == TCP) {
			struct sockaddr_in SvrAddr;
			SvrAddr.sin_family = AF_INET; //Address family type internet
			SvrAddr.sin_port = htons(this->GetPort()); //port (host to network conversion)
			SvrAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str()); //IP address
			if ((bind(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not bind to the socket" << std::endl;
				std::cin.get();
				exit(0);
			}
		}
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

void MySocket::accept_connection()
{
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

void MySocket::ConnectToTCPServer() {
	std::cout << "Trying to connect to the server" << std::endl;

	bool connected = false;
	struct sockaddr_in SvrAddr;
	SvrAddr.sin_family = AF_INET; //Address family type internet
	SvrAddr.sin_port = htons(this->GetPort()); //port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str()); //IP address
	while (!connected) {
		if ((connect(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
		else {
			std::cout << "Connection Established" << std::endl;
			connected = true;
		}
	}
}

//receives messages from the connection_socket
char * MySocket::receive_message() {
	if (GetType() == SERVER) {
	memset(Buffer, 0, 128);
	recv(this->ConnectionSocket, this->Buffer, sizeof(this->Buffer), 0);
	return this->Buffer;
	}
	else if (GetType() == CLIENT) {
		memset(Buffer, 0, 128);
		recv(this->WelcomeSocket, this->Buffer, sizeof(this->Buffer), 0);
		return this->Buffer;
	}
	
}

//sends messages to the connection_socket
void MySocket::send_message(char * tx_buffer) {
	if (GetType() == SERVER) {
		send(this->ConnectionSocket, tx_buffer, strlen(tx_buffer), 0);
	}
	else {
		send(this->WelcomeSocket, tx_buffer, strlen(tx_buffer), 0);
	}
}


MySocket::~MySocket()
{
	closesocket(this->WelcomeSocket);
	WSACleanup();
}
