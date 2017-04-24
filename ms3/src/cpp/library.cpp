#ifndef MS1_LIBRARY_
#define MS1_LIBRARY
#define _CRT_SECURE_NO_WARNINGS
#include "../header/library.h"

PktDef::PktDef() {
  cmdPacket.header.pktCount = (char)0;
  clearCmd();
	cmdPacket.header.length = HEADERSIZE + sizeof(cmdPacket.CRC);
	cmdPacket.data = nullptr;
	cmdPacket.CRC = (char)0;
}

PktDef::PktDef(char* rawDataBuffer) {
	//Constructor that takes Raw data Buffer
	//Reconstruct PktDef via data from rawDataBuffer
  copy(rawDataBuffer);
}

void PktDef::operator= (char* data) {
  //Copies raw buffer data into the class
  copy(data);
}

void PktDef::clearCmd() {
  cmdPacket.header.drive = (char)0;
  cmdPacket.header.status = (char)0;
  cmdPacket.header.sleep = (char)0;
  cmdPacket.header.arm = (char)0;
  cmdPacket.header.claw = (char)0;
  cmdPacket.header.ack = (char)0;
  cmdPacket.header.padding = (char)0;
}

void PktDef::copy(char* data) {
  char* ptr = data;

  memcpy(&cmdPacket.header, ptr, HEADERSIZE);

  ptr += HEADERSIZE;		//2 extra bytes comes from windows pad bytes

  if (cmdPacket.header.length < 9) {		//Checking for Motorbody data
                                        // No motorbody, initialize to nullptr
    cmdPacket.data = nullptr;
  }
  else {
    //Motorbody exists
    char motorbodySize = cmdPacket.header.length
      - HEADERSIZE
      - sizeof(cmdPacket.CRC);

    setBodyData(ptr, motorbodySize);
    ptr += motorbodySize;	//Advance ptr past MotorBody
  }
  cmdPacket.CRC = *ptr;
}
void PktDef::setCmd(CmdType type) {
  clearCmd();
	switch (type) {
	case DRIVE:
		cmdPacket.header.drive = 1;
		break;
  case STATUS:
    cmdPacket.header.status = 1;
    break;
	case SLEEP:
		cmdPacket.header.sleep = 1;
		break;
	case ARM:
		cmdPacket.header.arm = 1;
		break;
	case CLAW:
		cmdPacket.header.claw = 1;
		break;
	}
}

void PktDef::setBodyData(char* rawDataBuffer, int bufferByteSize) {
	cmdPacket.data = new char[bufferByteSize];
	memcpy(cmdPacket.data, rawDataBuffer, bufferByteSize);

	cmdPacket.header.length = HEADERSIZE + sizeof(cmdPacket.CRC) + bufferByteSize;
}

void PktDef::setPktCount(int countNumber) {
	cmdPacket.header.pktCount = countNumber;
}

CmdType PktDef::getCmd() {
	if (cmdPacket.header.drive) {
		return DRIVE;
	}
  else if (cmdPacket.header.status) {
    return STATUS;
  }
	else if (cmdPacket.header.sleep) {
		return SLEEP;
	}
	else if (cmdPacket.header.arm) {
		return ARM;
	}
	else if (cmdPacket.header.claw) {
		return CLAW;
	}
}

bool PktDef::getAck() {
	return cmdPacket.header.ack;
}

int PktDef::getLength() {
	return cmdPacket.header.length;
}

char* PktDef::getBodyData() {
	return cmdPacket.data;
}

int PktDef::getPktCount() {
	return cmdPacket.header.pktCount;
}

bool PktDef::checkCRC(char* ptr, int bufferSize) {
	int crc = 0;
	for (int i = 0; i < bufferSize; i++) {
		for (int j = 0; j < 8; j++) {		//iterate through 8 bits
			if ((*ptr >> j) & 1) {
				crc++;
			}
		}
		ptr++;
	}
	return crc == cmdPacket.CRC;
}

void PktDef::calcCRC() {
	//Calculates the CRC on the fly and saves it to the current object
	cmdPacket.CRC = 0;
	char* ptr = reinterpret_cast<char*>(&cmdPacket.header);

	for (int i = 0; i < HEADERSIZE; i++) {
		for (int j = 0; j < 8; j++) { //Loop through 8 bits
			if ((*ptr >> j) & 1) {
				cmdPacket.CRC++;
			}
		}
		ptr++;
	}

	if (cmdPacket.data != nullptr) {
		ptr = cmdPacket.data;
		int bodySize = cmdPacket.header.length - HEADERSIZE - sizeof(cmdPacket.CRC);

		for (int i = 0; i < bodySize; i++) {
			for (int j = 0; j < 8; j++) { //Loop through 8 bits
				if ((*ptr >> j) & 1) {
					cmdPacket.CRC++;
				}
			}
			ptr++;
		}
	}
}

char* PktDef::genPacket(){
	//Creates a RawBuffer in the heap and serialize data
	//Return address to RawBuffer

	int bufferHeader = 0;	//Buffer header location tracker
	int bodySize = this->getLength() - HEADERSIZE - sizeof(cmdPacket.CRC);
	char* rawBuffer = new char[cmdPacket.header.length];

	char* ptr = reinterpret_cast<char*> (&cmdPacket.header);
	memcpy(rawBuffer, ptr, HEADERSIZE);

	bufferHeader = HEADERSIZE;

	if (cmdPacket.data != nullptr) {		//Motorbody is not empty
		ptr = cmdPacket.data;
		memcpy(rawBuffer + bufferHeader, ptr, bodySize);
		bufferHeader += bodySize;
	}

	ptr = reinterpret_cast<char*>(&cmdPacket.CRC);
	memcpy(rawBuffer + bufferHeader, ptr, sizeof(cmdPacket.CRC));

	return std::ref(rawBuffer);
}
#endif
