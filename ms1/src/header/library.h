#pragma once
#ifndef MS1_LIBRARY_H
#define MS1_LIBRARY_H

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

enum CmdType {DRIVE, SLEEP, ARM, CLAW, ACK};
const int FORWARD = 1;
const int BACKWARD = 2;
const int RIGHT = 3;
const int LEFT = 4;
const int UP = 5;
const int DOWN = 6;
const int OPEN = 7;
const int CLOSE = 8;
const int HEADERSIZE = 6;

struct MotorBody {
	char direction;
	char duration;
};

struct Header {
	unsigned int pktCount;
	unsigned char drive : 1;	//This is the first bit (2^0)
	unsigned char status : 1;
	unsigned char sleep : 1;
	unsigned char arm : 1;
	unsigned char claw : 1;
	unsigned char ack : 1;
	unsigned char : 2; //Padding
	unsigned char length;		//Num of bytes in packet
};

struct CmdPacket {
	Header header;
	char* data;
	char CRC;
};

//TODO Change PktDef to "Class" instead of "Struct"
struct PktDef {
	CmdPacket cmdPacket;
	char* rawBuffer;
public:
	PktDef();	//DONE
	PktDef(char*); // DONE
	void setCmd(CmdType); // DONE
	void setBodyData(char*, int);  // DONE
	void setPktCount(int); //DONE
	CmdType getCmd(); // DONE
	bool getAck();
	int getLength();
	char* getBodyData();
	int getPktCount();
	bool checkCRC(char*, int);
	void calcCRC();
	char* genPacket();

	//Alex's temp functions for development only
	//Remove after GM 
}; 

class winsock {
protected:
	int version_num1, version_num2; // determines the WSADATA version numbers
	int port; // port number
	std::string ip; //ip string e.g. "127.0.0.1"
	char rx_buffer[128] = {}; //note that the rx_buffer has only 128 bytes
	WSADATA wsa_data;
public:
	void start_DLLS();
	SOCKET initialize_tcp_socket();
	winsock();
};

class winsock_client : public winsock {
protected:
	SOCKET client_socket;
public:
	char * receive_message(); //receives message from the client_socket
	void send_message(char *); //sends message to the client_socket
	void get_messages(); //continuously prints messages received
	void connect_to_tcp_server(); //tries to connect, exits if no server available
	void connect_to_tcp_server_loop(); //keeps trying to connect until successful
	winsock_client(int, std::string);
	~winsock_client();
};

// Alex's temp functions remove after GM
char* serialize(PktDef src); 
#endif