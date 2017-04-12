#ifndef MS3_MAIN
#define MS3_MAIN
#define _CRT_SECURE_NO_WARNINGS
#define DATA_BYTE_SIZE 100
#include "../header/library.h"
#include "../header/MySocket.h"

using namespace std;

int pktCount = 0;
string ip;
int port = 0;
bool ExeComplete = false;

int main() {

  cout << "IP Address: ";
  getline(cin, ip);
  cout << "Port: ";
  cin >> port;
  
  // ms3 instructions say that global bool ExeComplete = false, and two threads should be spawned from the main process. 
  // once detached, the main process should loop until ExeComplete == true

  thread command(commandThread, ip, port);
  thread telemetry(telemetryThread, ip, port);
  
  
	//while (ExeComplete == false) {
		command.join();
		telemetry.join();
	//}
  

  return 0;
}

void commandThread(std::string ipAddress, int port) {

	/*
	Commands:
	• Create a MySocket object configured as a SocketType::CLIENT and ConnectionType::TCP (DONE)
	• Perform 3 way handshake (DONE)
	• Query the user to get all required information to form a packet (Somewhat done, will Anthony's querying code come in here?)
	• Generate a Packet of type PktDe based on user input and increment the PktCount number (DONE)
	• Transmit the packet to the robot via the MySocket connection (DONE)
	• Wait for an acknowledgement packet from the robot (Confused about this - will the telemetryThread receive the robots transmissions, or does the commandThread parse through it as well?)
	• Continue this process (what process - querying the user to create overwrite the previous packet and then send it? Or just ask for the command type and overwrite that?) unti
	the user requests to send a SLEEP command. Upon receiving an Ack packet from the robot, acknowledging the SLEEP command, the logic should:
		- disconnect the MySocket  
		- set the ExeComplete flag to TRUE
		- end the thread
	
	*/
	
    MySocket CommandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, DATA_BYTE_SIZE);

    CommandSocket.ConnectTCP();

	while(ExeComplete == false) {
		
	MotorBody driveData;
    std::string direction;
    int duration;
	
	std::string command;
	CmdType cmdType;
	int dirType;
	 PktDef sendPkt;
	std::cout << "Please enter one of the following commands: DRIVE, STATUS, SLEEP, ARM, CLAW";
	std::cin >> command;
	
	if (command == "DRIVE")
	{
		cmdType == DRIVE;
		std::cout << "Drive command - Please Enter the direction: FORWARD, BACKWARD, RIGHT, LEFT";
	}
	else if (command == "ARM")
	{
		cmdType == ARM;
		std::cout << "Arm command - Please Enter the direction: UP, DOWN";
	}
	else if (command == "CLAW")
	{
		cmdType == CLAW;
		std::cout << "Claw command - Please Enter the direction: OPEN, CLOSE";
	}
	else if (command == "SLEEP")
	{
		std::cout << "Sleeping";
		cmdType == SLEEP;
	}

	sendPkt.setCmd(cmdType);

    std::cin >> direction;
	
	if (direction == "FORWARD")
	{
		driveData.direction = 1;
	}
	else if (direction == "BACKWARD")
	{
		driveData.direction = 2;
	}
	else if (direction == "RIGHT")
	{
		driveData.direction = 3;
	}
	else if (direction == "LEFT")
	{
		driveData.direction = 4;
	}
	else if (direction == "UP")
	{
		driveData.direction = 5;
	}
	else if (direction == "DOWN")
	{
		driveData.direction = 6;
	}
	else if (direction == "OPEN")
	{
		driveData.direction = 7;
	}
	else if (direction == "CLOSE")
	{
		driveData.direction = 8;
	}

	if (cmdType == ARM || cmdType == CLAW || cmdType == SLEEP)
	{
		std::cout << "Claw/Arm/Sleep command detected, no duration required" << std::endl;
		driveData.duration = 0;
	}
	else
	{
		std::cout << "Please enter the duration: ";
		std::cin >> duration;
		driveData.duration = duration;
	}
   
    sendPkt.setBodyData((char*)&driveData, sizeof(driveData));
    sendPkt.calcCRC();
	
	//generate packet
	char *ptr;
	ptr = sendPkt.genPacket();
	
	//send packet over the socket
    CommandSocket.SendData(ptr, strlen(ptr));
	pktCount++; 	//increment PktCount

	//confused about this part - receive an ack packet from the robot and parse through it?
    char buff[DATA_BYTE_SIZE];
    int RxSize = CommandSocket.GetData(buff);
	
	PktDef recPacket(buff);
	recPacket.calcCRC();
	
	if (sendPkt.getCmd() == SLEEP && (recPacket.checkCRC(buff, RxSize))) {
		if (recPacket.getCmd() == ACK) {	//if sendPkt sent SLEEP and recPacket acknowledged SLEEP
			std::cout << "Robot has acknowledged the SLEEP command. Thank you." << std::endl;
			
			sendPkt.setCmd(SLEEP);
			ptr = sendPkt.genPacket();
			CommandSocket.SendData(ptr, strlen(ptr));
			pktCount++;
			
			CommandSocket.DisconnectTCP();
			ExeComplete = true;
			break;	//break while(true) loop
			
		} else if (recPacket.getCmd() !=ACK) {
			std::cout << "Negative acknowledgement. Please try again!" << std::endl;
		}
	} else if (sendPkt.getCmd() != SLEEP && recPacket.checkCRC(buff, RxSize)) {
		if (recPacket.getCmd() != ACK) {
			std::cout << "Negative acknowledgement. Please try again!" << std::endl;
		}
	}

  } 
}

void telemetryThread(std::string ipAddress, int port) {
  /*
  Commands:
  • Create a Telemetry socket (DONE)
  • Perform 3 way handshake (DONE)
  • Receive telemetry packets from the robot. (DONE)
  1) Verify CRC (DONE)
  A telemetry packet is 16 bytes in size
  2) Verify Header data that STATUS bit is set to true (DONE)
  3) Extract and display decimal version of sensor data in the body packet
  • Arm Reading (DONE)
  • Sonar reading (DONE)
  4) Extract and display Drive flag as 0 or 1 (DONE)
  5) Extract and display Arm and Claw status in English (DONE)
  i.e. "Arm is Up, Claw is Closed"
  Side Note: If validation fails at any point, sotware should log error to std::cout
  and drop the remaining processing of the packet. (We need to simulate a
  NACK to test this).
  */
  char dataBuffer[DATA_BYTE_SIZE];
  MySocket telemetryClient(SocketType::CLIENT, ip, port, ConnectionType::TCP,
    DATA_BYTE_SIZE);

  telemetryClient.ConnectTCP();
  int dataSize = telemetryClient.GetData(dataBuffer);

  //Display raw data packet
  cout << "Raw data buffer contents: " << dataBuffer << '\n';

  //Deserialize telemetry packet
  PktDef telemetryPacket(dataBuffer);
  telemetryPacket.calcCRC();

  //CRC check
  if (telemetryPacket.checkCRC(dataBuffer, dataSize)) {
    cout << "CRC Check status: OK\n";

    //DEBUG ONLY STATUS bit validation. Remove after GM.
    if (telemetryPacket.getCmd() == STATUS) {
      cout << "Status bit is TRUE\n";
    }
    else {
      cout << "Status bit is FALSE\n";
    }

    //Extract and display decimal version of sensor data in body reading
    /*A telemetry response is when
    • Status set to 1, all other bits 0
    • PktCount sent + 1 == PktCount received
    • MotorBody has data
    */

    //int pktCount = 0;   // Temp variable. Need to map this to main
    if (telemetryPacket.getPktCount() == (pktCount + 1)
      && telemetryPacket.getLength() > 7
      && telemetryPacket.getCmd() == STATUS) {
      //MotorBody data exists. Create a struct and memcpy into the struct

      struct TelemetryData {
        short sonarSensorData = 0;
        short armPositionData = 0;
        unsigned char drive : 1;
        unsigned char armUp : 1;
        unsigned char armDown : 1;
        unsigned char clawOpen : 1;
        unsigned char clawClosed : 1;
        unsigned char padding : 3;

        TelemetryData() {
          padding = 0;
        }
      } pkt;

      memcpy(&pkt, telemetryPacket.getBodyData(), sizeof(TelemetryData));

      cout << "Sonar Sensor Data: "
        << pkt.sonarSensorData << '\n';

      cout << "Arm Position Data: "
        << pkt.armPositionData << "\n\n";

      cout << "Robot status data\n\n";
      cout << "Drive: " << pkt.drive << '\n';

      if (pkt.armUp) {
        cout << "Arm is up, ";
      }
      else {
        cout << "Arm is down, ";
      }

      if (pkt.clawOpen) {
        cout << "Claw is Open.\n";
      }
      else {
        cout << "Claw is Closed.\n";
      }
    }
  }
  else {
    cout << "CRC Check status: FAIL\n";
  }


}
#endif