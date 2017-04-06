
#include <iostream>
#include "../header/MySocket.h"

int main()
{
	MySocket ClientSocket(SocketType::CLIENT, "127.0.0.1", 5000, ConnectionType::TCP, 100);
	/*
	std::string Pkt = "I love BTN415";

	ClientSocket.ConnectTCP();
	ClientSocket.SendData(Pkt.c_str(), strlen(Pkt.c_str()));

	char buff[100];
	int RxSize = ClientSocket.GetData(buff);

	std::cout << "Msg = " << buff << ", Bytes = " << RxSize << std::endl;
*/

	std::cout << "Socket type: " <<  ClientSocket.GetType() << std::endl;

	char tx_message[128] = {}, rx_buffer[128] = {};
	std::cout << "Message: ";
	std::cin.getline(tx_message, sizeof(tx_message));
	ClientSocket.send_message(tx_message);
	strcpy(rx_buffer, ClientSocket.receive_message());
	std::cout << rx_buffer << std::endl;

	ClientSocket.DisconnectTCP();

	return 1;
}
