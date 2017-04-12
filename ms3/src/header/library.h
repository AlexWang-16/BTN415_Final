#pragma once
#ifndef MS1_LIBRARY_H
#define MS1_LIBRARY_H

#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <string>

enum CmdType {DRIVE, STATUS, SLEEP, ARM, CLAW, ACK};
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
	unsigned char direction;
	unsigned char duration;
};

struct Header {
	unsigned int pktCount;
	unsigned char drive : 1;	//This is the first bit (2^0)
	unsigned char status : 1;	//Used to see if robot has recieved cmd
	unsigned char sleep : 1;
	unsigned char arm : 1;
	unsigned char claw : 1;
	unsigned char ack : 1;
	unsigned char padding : 2;
	unsigned char length;		//Num of bytes in packet
};

struct CmdPacket {
	Header header;
	char* data;
	char CRC;
};

class PktDef {
	CmdPacket cmdPacket;
	char* rawBuffer;
public:
  //Constructors
	PktDef();
	PktDef(char* rawDataBuffer);

  //Setters
	void setCmd(CmdType); 
	void setBodyData(char* rawDataBuffer, int bufferByteSize);
	void setPktCount(int); 

  //Getters
  int getPktCount();
  CmdType getCmd(); 
	bool getAck();
	int getLength();  //Returns size of pkt in bytes
	char* getBodyData();
	

  //Validation and generation
	bool checkCRC(char* ptr, int bufferSize);
	void calcCRC();		
	char* genPacket(); 

  //overloaded operators
  void operator= (char*);

  //Special functions
  void copy(char* data);
  void clearCmd();
}; 

#endif