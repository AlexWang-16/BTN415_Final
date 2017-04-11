#ifndef MS3_MAIN
#define MS3_MAIN
#define _CRT_SECURE_NO_WARNINGS
#include "../header/library.h"
#include "../header/MySocket.h"

using namespace std;
bool ExeComplete = false;
#include <iostream>
#include  <thread>

int main() {

	thread command, telemetry;
	command = thread(commandThread());
	telemetry = thread(telemetryThread());


}

void commandThread() {

	while (ExeComplete == false) {

		std::string ip = "127.0.0.1";
		int port = 5000;

		MySocket CommandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, 100);

		CommandSocket.ConnectTCP();

		PktDef sendPkt;

		MotorBody driveData;
		std::string direction;
		int duration;

		int pktCount;

		CmdType cmdType;

		std::cout << "Please enter the direction: ";
		std::cin >> direction;
		std::cout << "Please enter the duration: ";
		std::cin >> duration;
		sendPkt.setBodyData(reinterpret_cast<char*>(&direction), duration);

		std::cout << "Please enter the PktCount: ";
		std::cin >> pktCount;
		sendPkt.setPktCount(pktCount);

		std::cout << "Please enter the command: ";
		std::cin >> cmdType;
		sendPkt.setCmd(cmdType);

		sendPkt.calcCRC()();

		char* buffer = sendPkt.genPacket();

		char buff[100];
		int RxSize = CommandSocket.GetData(buff);
		if (buff != "0") {
			if (cmdType != "SLEEP") {

			}
			else if (cmdType == "SLEEP") {
				if (buff.)
					CommandSocket.DisconnectTCP();
				ExeComplete = true;

			}
		}




		/*
		std::string Pkt = "I love BTN415";

		//ClientSocket.ConnectTCP();

		ClientSocket.SendData(Pkt.c_str(), strlen(Pkt.c_str()));

		char buff[100];
		int RxSize = ClientSocket.GetData(buff);

		std::cout << "Msg = " << buff << ", Bytes = " << RxSize << std::endl;

		//ClientSocket.DisconnectTCP();
		*/

	}
}

void telemetryThread()
{

};

#endif