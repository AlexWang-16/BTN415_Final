#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "../header/library.h"

using namespace std;

int main()
{
	
	MotorBody driveData;
	driveData.direction = FORWARD;
	driveData.duration = 5;
	
	//Create and configure PktDef test
	PktDef test;
	test.cmdPacket.header.pktCount = 2;

	test.cmdPacket.header.drive = 0;
	test.cmdPacket.header.status = 0;
	test.cmdPacket.header.sleep = 0;
	test.cmdPacket.header.arm = 0;
	test.cmdPacket.header.claw = 0;
	test.cmdPacket.header.ack = 0;

	/*
	Test setCmd() and getCmd()

	test.setCmd(ARM);
	std::cout << "Command retrieved: " << test.getCmd() << std::endl;
	*/

	test.cmdPacket.header.length = 9;
	test.cmdPacket.data = reinterpret_cast<char*> (&driveData);
	//test.cmdPacket.data = nullptr;
	test.cmdPacket.CRC = 9;

	//Create buffer to hold data
	char* buffer = serialize(test, 2);

	//PktDef testRecv(buffer);

	//MotorBody DriveCmd;
	//DriveCmd.direction = FORWARD;
	//DriveCmd.duration = 20;

	//PktDef TestPkt;
	//char *ptr;

	////Testing the PktDef creation interface
	//TestPkt.setCmd(DRIVE);
	//TestPkt.setBodyData((char *)&DriveCmd, 2);
	//TestPkt.setPktCount(1);
	//TestPkt.calcCRC();
	//ptr = TestPkt.genPacket();

	//cout << showbase
	//	<< internal
	//	<< setfill('0');

	//for (int x = 0; x < (int)TestPkt.getLength(); x++)
	//	cout << hex << setw(4) << (unsigned int)*(ptr++) << ", ";

	//cout << endl;
	//TestPkt.setCmd(ACK);
	//TestPkt.calcCRC();
	//ptr = TestPkt.genPacket();

	//for (int x = 0; x < (int)TestPkt.getLength(); x++)
	//	cout << hex << setw(4) << (unsigned int)*(ptr++) << ", ";

	//cout << endl << noshowbase << dec;
	////Testing Rx Buffer interface
	////You should create RAW data packets (like below) to test your overloaded constructor
	//char buffer[9] = { 0x02, 0x00, 0x00, 0x00, 0x02, 0x09, 0x11, 0x24, 0x08};
	//PktDef RxPkt(buffer);
	//cout << "CommandID:  " << RxPkt.getCmd() << endl;
	//cout << "PktCount:   " << RxPkt.getPktCount() << endl;
	//cout << "Pkt Length: " << RxPkt.getLength() << endl;
	//cout << "Body Data: " << endl;

	//ptr = RxPkt.getBodyData();
	//cout << showbase << hex;
	//cout << "Byte 1 " << (int)*ptr++ << endl;
	//cout << "Byte 2 " << (int)*ptr << endl;

	//return 1;
}
