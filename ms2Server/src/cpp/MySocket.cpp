#include "../header/MySocket.h"

MySocket::MySocket(SocketType socketType, std::string ipAddress, unsigned int portNumber,
  ConnectionType connectionType, unsigned int bufferSize) {
  this->version_num1 = 2;
  this->version_num2 = 2;
  this->start_DLLS();

  SetType(socketType);
  SetIPAddr(ipAddress);
  SetPort(port);
  this->connectionType = connectionType;

  if (0 < bufferSize && bufferSize <= std::numeric_limits<int>::max()) {
    Buffer = new char[bufferSize];
    this->MaxSize = bufferSize;
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
    if (this->GetType() == SERVER) {
      this->WelcomeSocket = this->initialize_udp_socket();
      this->bind_socket();
      bTCPConnect = true;
      std::cout << "Server: Online and waiting for connection\n";
    }
    else {
      this->ConnectionSocket = this->initialize_udp_socket();
      this->bTCPConnect = true;
      std::cout << "Client: UDP connection initialized\n";
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
    if (this->GetType() == CLIENT) {
      this->SvrAddr.sin_family = AF_INET;
      this->SvrAddr.sin_port = htons(this->port);
      this->SvrAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str());

      //we transmit dataSize + 1 length so the \0 will be included
      sendto(this->ConnectionSocket, data, dataSize + 1, 0,
        (struct sockaddr *)&SvrAddr, sizeof(SvrAddr));
    }
    else if (this->GetType() == SERVER) {
      struct sockaddr_in CltAddr; //A struct that holds client IP and port info
      CltAddr.sin_family = AF_INET;
      CltAddr.sin_port = htons(this->port);
      CltAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str());

      sendto(this->WelcomeSocket, data, dataSize + 1, 0,
        (struct sockaddr *)&CltAddr, sizeof(CltAddr));

      //Close the conection and free server socket resource
      closesocket(this->WelcomeSocket);
      WSACleanup();
    }
  }
}

//Receives data, stores data into char* data and returns number of bytes stored
int MySocket::GetData(char * recvBuffer)
{
  if (this->connectionType == TCP) {
    memset(recvBuffer, 0, this->MaxSize);
    recv(this->ConnectionSocket, recvBuffer, this->MaxSize, 0);
  }
  else if (this->connectionType == UDP) {
    memset(recvBuffer, 0, this->MaxSize);
    if (this->GetType() == CLIENT) {
      this->SvrAddr.sin_family = AF_INET;
      this->SvrAddr.sin_port = htons(this->port);
      this->SvrAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str());
      int addr_len = sizeof(SvrAddr);

      recvfrom(this->ConnectionSocket, recvBuffer, this->MaxSize, 0, (struct sockaddr *) &SvrAddr, &addr_len);

      //Close the conection and free client socket resource
      closesocket(this->ConnectionSocket);
      WSACleanup();
    }
    else if (this->GetType() == SERVER) {
      struct sockaddr_in CltAddr; //A struct that holds client IP and port info
      CltAddr.sin_family = AF_INET;
      CltAddr.sin_port = htons(this->port);
      CltAddr.sin_addr.s_addr = inet_addr(this->GetIPAddr().c_str());
      int addr_len = sizeof(CltAddr);
      recvfrom(this->WelcomeSocket, recvBuffer, this->MaxSize, 0, (struct sockaddr *) &CltAddr, &addr_len);
    }
  }
  return this->MaxSize;
}

std::string MySocket::GetIPAddr()
{
  return this->IPAddr;
}

void MySocket::SetIPAddr(std::string ipAddress)
{
  if (!this->bTCPConnect) {
    this->IPAddr = ipAddress;
  }
  else {
    if (this->GetType() == CLIENT) {
      std::cerr << "Error: A connection is already active. Cannot set IP Address. Please disconnect and try again.\n";
    }
    else {
      std::cerr << "Error: A connection is already active. Cannot set IP Address. Please shutdown server and try again.\n";
    }
  }
}

void MySocket::SetPort(int portNumber)
{
  if (!this->bTCPConnect) {
    this->port = portNumber;
  }
  else {
    if (this->GetType() == CLIENT) {
      std::cerr << "Error: A connection is already active. Cannot set port number. Please disconnect and try again.\n";
    }
    else {
      std::cerr << "Error: A connection is already active. Cannot set port number. Please shutdown the server and try again.\n";
    }
  }
}

int MySocket::GetPort()
{
  return this->port;
}

SocketType MySocket::GetType()
{
  return this->mySocket;
}

void MySocket::SetType(SocketType socketType)
{
  this->mySocket = socketType;
}

ConnectionType MySocket::GetConnectionType()
{
  return this->connectionType;
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
    std::cout << "Could not initialize TCP socket" << std::endl;
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
    std::cout << "Could not initialize UDP socket" << std::endl;
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
  bTCPConnect = true;		//Server socket is active
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
