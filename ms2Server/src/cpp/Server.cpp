#include <iostream>
#include "../header/MySocket.h"

int main()
{
	MySocket ServerSocket(SocketType::SERVER, "127.0.0.1", 5000, ConnectionType::TCP, 100);

	/*
	char buff[100];

	int RxSize = ServerSocket.GetData(buff);

	std::cout << "Msg = " << buff << ", Bytes = " << RxSize << std::endl;

	std::string Pkt = "I Love BTN415 too!";

	ServerSocket.SendData(Pkt.c_str(), strlen(Pkt.c_str()));

	*/

	std::cout << "Socket type: " << ServerSocket.GetType() << std::endl;
	char rx_buffer[128] = {};
	strcpy(rx_buffer, ServerSocket.receive_message());
	std::cout << rx_buffer << std::endl;
	ServerSocket.send_message("Message_Received");

	ServerSocket.DisconnectTCP();

	return 1;
}
