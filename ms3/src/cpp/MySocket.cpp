#include "../header/MySocket.h"

MySocket::MySocket(SocketType socketType, std::string ipAddress, unsigned int portNumber,
  ConnectionType connectionType, unsigned int bufferSize) {
  this->version_num1 = 2;
  this->version_num2 = 2;
  this->start_DLLS();

  setType(socketType);
  setIPAddr(ipAddress);
  setPort(portNumber);
  this->connectionType = connectionType;

  if (0 < bufferSize && bufferSize <= std::numeric_limits<unsigned int>::max()) {
    Buffer = new char[bufferSize];
    this->maxSize = bufferSize;
  }
  else {
    Buffer = new char[DEFAULT_SIZE];
    this->maxSize = DEFAULT_SIZE;
  }

  if (this->getConnectionType() == TCP) {
    if (this->getType() == SERVER) {
      this->welcomeSocket = this->initializeTcpSocket();
      this->bindSocket();
      this->listenSocket();
      this->acceptConnection();
    }
    else {
      this->connectionSocket = this->initializeTcpSocket();
    }
  }
  else if (this->getConnectionType() == UDP) {
    if (this->getType() == SERVER) {
      this->welcomeSocket = this->initializeUdpSocket();
      this->bindSocket();
      bTCPConnect = true;
      std::cout << "Server: Online and waiting for connection\n";
    }
    else {
      this->connectionSocket = this->initializeUdpSocket();
      this->bTCPConnect = true;
      std::cout << "Client: UDP connection initialized\n";
    }
  }
}

void MySocket::connectTCP()
{
  std::cout << "Trying to connect to the server" << std::endl;

  this->svrAddr.sin_family = AF_INET; //Address family type internet
  this->svrAddr.sin_port = htons(this->getPort()); //port (host to network conversion)
  this->svrAddr.sin_addr.s_addr = inet_addr(this->getIPAddr().c_str()); //IP address

  int tries = 0;
  while (!this->bTCPConnect) {
    if ((connect(this->connectionSocket, (struct sockaddr *)&svrAddr, sizeof(svrAddr))) == SOCKET_ERROR) {
      if ((tries % 5) == 0) {
        std::cout << "Connection is failing. Retried 5 times\n";
      }
      tries++;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    else {
      std::cout << "Connection Established" << std::endl;
      this->bTCPConnect = true;
    }
  }
}

void MySocket::disconnectTCP()
{
  if (this->getType() == SERVER) {
    closesocket(this->connectionSocket);
    closesocket(this->welcomeSocket);
    WSACleanup();
  }
  else {
    closesocket(this->connectionSocket);
    WSACleanup();
  }
}

void MySocket::sendData(const char* data, int dataSize)
{
  if (this->connectionType == TCP) {
    //we transmit dataSize + 1 length so the \0 will be included
    send(this->connectionSocket, data, dataSize + 1, 0);
  }
  else if (this->connectionType == UDP) {
    if (this->getType() == CLIENT) {
      this->svrAddr.sin_family = AF_INET;
      this->svrAddr.sin_port = htons(this->port);
      this->svrAddr.sin_addr.s_addr = inet_addr(this->getIPAddr().c_str());

      //we transmit dataSize + 1 length so the \0 will be included
      sendto(this->connectionSocket, data, dataSize + 1, 0,
        (struct sockaddr *)&svrAddr, sizeof(svrAddr));
    }
    else if (this->getType() == SERVER) {
      struct sockaddr_in CltAddr; //A struct that holds client IP and port info
      CltAddr.sin_family = AF_INET;
      CltAddr.sin_port = htons(this->port);
      CltAddr.sin_addr.s_addr = inet_addr(this->getIPAddr().c_str());

      sendto(this->welcomeSocket, data, dataSize + 1, 0,
        (struct sockaddr *)&CltAddr, sizeof(CltAddr));
    }
  }
}

//Receives data, stores data into char* data and returns number of bytes stored
int MySocket::getData(char * recvBuffer)
{
  if (this->connectionType == TCP) {
    memset(recvBuffer, 0, this->maxSize);
    recv(this->connectionSocket, recvBuffer, this->maxSize, 0);
  }
  else if (this->connectionType == UDP) {
    memset(recvBuffer, 0, this->maxSize);
    if (this->getType() == CLIENT) {
      this->svrAddr.sin_family = AF_INET;
      this->svrAddr.sin_port = htons(this->port);
      this->svrAddr.sin_addr.s_addr = inet_addr(this->getIPAddr().c_str());
      int addr_len = sizeof(svrAddr);

      recvfrom(this->connectionSocket, recvBuffer, this->maxSize, 0, (struct sockaddr *) &svrAddr, &addr_len);
    }
    else if (this->getType() == SERVER) {
      struct sockaddr_in CltAddr; //A struct that holds client IP and port info
      CltAddr.sin_family = AF_INET;
      CltAddr.sin_port = htons(this->port);
      CltAddr.sin_addr.s_addr = inet_addr(this->getIPAddr().c_str());
      int addr_len = sizeof(CltAddr);
      recvfrom(this->welcomeSocket, recvBuffer, this->maxSize, 0, (struct sockaddr *) &CltAddr, &addr_len);
    }
  }
  return this->maxSize;
}

std::string MySocket::getIPAddr()
{
  return this->IPAddr;
}

void MySocket::setIPAddr(std::string ipAddress)
{
  if (!this->bTCPConnect) {
    this->IPAddr = ipAddress;
  }
  else {
    if (this->getType() == CLIENT) {
      std::cerr << "Error: A connection is already active. Cannot set IP Address. Please disconnect and try again.\n";
    }
    else {
      std::cerr << "Error: A connection is already active. Cannot set IP Address. Please shutdown server and try again.\n";
    }
  }
}

void MySocket::setPort(int portNumber)
{
  if (!this->bTCPConnect) {
    this->port = portNumber;
  }
  else {
    if (this->getType() == CLIENT) {
      std::cerr << "Error: A connection is already active. Cannot set port number. Please disconnect and try again.\n";
    }
    else {
      std::cerr << "Error: A connection is already active. Cannot set port number. Please shutdown the server and try again.\n";
    }
  }
}

int MySocket::getPort()
{
  return this->port;
}

SocketType MySocket::getType()
{
  return this->mySocket;
}

void MySocket::setType(SocketType socketType)
{
  this->mySocket = socketType;
}

ConnectionType MySocket::getConnectionType()
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

SOCKET MySocket::initializeTcpSocket()
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

SOCKET MySocket::initializeUdpSocket()
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

void MySocket::bindSocket()
{
  struct sockaddr_in svrAddr;
  svrAddr.sin_family = AF_INET; //Address family type internet
  svrAddr.sin_port = htons(this->getPort()); //port (host to network conversion)
  svrAddr.sin_addr.s_addr = inet_addr(this->getIPAddr().c_str()); //IP address
  if ((bind(this->welcomeSocket, (struct sockaddr *)&svrAddr, sizeof(svrAddr)))
    == SOCKET_ERROR) {
    closesocket(this->welcomeSocket);
    WSACleanup();
    std::cout << "Could not bind to the socket" << std::endl;
    std::cin.get();
    exit(0);
  }
  bTCPConnect = true;		//Server socket is active
}

void MySocket::listenSocket()
{
  if (listen(this->welcomeSocket, 1) == SOCKET_ERROR) {
    closesocket(this->welcomeSocket);
    WSACleanup();
    std::cout << "Could not listen to the provided socket." << std::endl;
    std::cin.get();
    exit(0);
  }
  else {
    std::cout << "Waiting for client connection" << std::endl;
  }
}

void MySocket::acceptConnection() {
  if ((this->connectionSocket = accept(this->welcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
    closesocket(this->welcomeSocket);
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

  //Close UDP socket and free resource
  if (this->getConnectionType() == UDP) {
    if (this->getType() == SERVER) {
      closesocket(this->welcomeSocket);
    }
    else {
      closesocket(this->connectionSocket);
    }
    WSACleanup();  
  }
}
