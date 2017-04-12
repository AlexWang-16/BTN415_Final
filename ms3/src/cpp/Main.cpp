#ifndef MS3_MAIN
#define MS3_MAIN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define DATA_BYTE_SIZE 100
#include "../header/library.h"
#include "../header/MySocket.h"

using namespace std;

bool ExeComplete = false, dataSent = false;
int pktCount = 0;
string commandIP, telemetryIP;
int commandPort, telemetryPort = 0;

int main() {
      cout << "Command socket connection information\n";
      cout << "-------------------------------------\n";
      cout << "IP Address: ";
      getline(cin, commandIP);
      cout << "Port: ";
      cin >> commandPort;

      cin.clear();
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      /*cout << "\nTelemetry socket connection information\n";
      cout << "-------------------------------------\n";
      cout << "IP Address: ";
      getline(cin, telemetryIP);
      cout << "Port: ";
      cin >> telemetryPort;
      
      cin.clear();
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');*/

      thread command(commandThread, commandIP, commandPort);
      //thread telemetry(telemetryThread, telemetryIP, telemetryPort);

      command.join();
      //telemetry.join();

  return 0;
}

void commandThread(string ip, int port) {
  
  
  PktDef sendPkt;
  std::string cmdType;

  MotorBody driveData;
  std::string direction;
  int duration = 0;
  
  char* pktData = nullptr;
  
  char buff[DATA_BYTE_SIZE];
  int rxSize = 0;

  //Establish connection
  MySocket CommandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, DATA_BYTE_SIZE);
  CommandSocket.connectTCP();

  while (!ExeComplete) {
    duration = 0;   //duration will change according to user input each time

    std::cout << "Please enter the command: ";
    getline(std::cin, cmdType);
    
    if (cmdType != "sleep") {
      std::cout << "Please enter the direction: ";
      std::cin >> direction;
      
      if (cmdType == "drive"){
        std::cout << "Please enter the duration: ";
        std::cin >> duration;
      }
    }
    
    //Set Packet Count
    sendPkt.setPktCount(pktCount + 1);

    // Setting command
    if (cmdType == "drive") {
      sendPkt.setCmd(DRIVE);
    }
    else if (cmdType == "status") {
      sendPkt.setCmd(STATUS);
    }
    else if (cmdType == "sleep") {
      sendPkt.setCmd(SLEEP);
      ExeComplete = true;   //Use this to defer disconnecting until after sending the pkt
    }
    else if (cmdType == "arm") {
      sendPkt.setCmd(ARM);
    }
    else if (cmdType == "claw") {
      sendPkt.setCmd(CLAW);
    }

    // Setting direction
    if (direction == "forward") {
      driveData.direction = FORWARD;
    }
    else if (direction == "backward") {
      driveData.direction = BACKWARD;
    }
    else if (direction == "right") {
      driveData.direction = RIGHT;
    }
    else if (direction == "left") {
      driveData.direction = LEFT;
    }
    else if (direction == "up") {
      driveData.direction = UP;
    }
    else if (direction == "down") {
      driveData.direction = DOWN;
    }
    else if (direction == "open") {
      driveData.direction = OPEN;
    }
    else if (direction == "close") {
      driveData.direction = CLOSE;
    }
    
    // Write body data to sendPkt
    sendPkt.setBodyData(reinterpret_cast<char*>(&driveData), 2);

    //Calc CRC
    sendPkt.calcCRC();

    //Generate packet
    pktData = sendPkt.genPacket();

    /*PktDef test(pktData);
    std::cout << "PktCount: " << test.getPktCount() << '\n';
    std::cout << "Command: " << test.getCmd() << '\n';
    std::cout << "Length: " << test.getLength() << '\n';
    std::cout << "Bo  dy data: " << *test.getBodyData() << '\n';*/
    
    //this_thread::sleep_for(std::chrono::milliseconds(1000000));
    //Send DefPkt through socket
    CommandSocket.sendData(pktData, sendPkt.getLength());
    
    dataSent = 1;

    PktDef responsePkt(buff);
    
    //Check for NACK response, repeat until ACK
    while (!responsePkt.getAck()) {
      this_thread::sleep_for(std::chrono::milliseconds(500));
      memset(buff, 0, DATA_BYTE_SIZE);
      rxSize = CommandSocket.getData(buff);
      responsePkt = buff;
      std::cerr << "Error: CRC check failed. NACK response received.\n";
    }

    if (ExeComplete) {
      CommandSocket.disconnectTCP();
    }

  } 
}

void telemetryThread(std::string ipAddress, int port) {
  
  char dataBuffer[DATA_BYTE_SIZE];
  MySocket telemetryClient(SocketType::CLIENT, ipAddress, port, ConnectionType::TCP,
                            DATA_BYTE_SIZE);

  telemetryClient.connectTCP();
  int dataSize = telemetryClient.getData(dataBuffer);
  
  if (!dataSent) {
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
    dataSent = 0;
  }
  
}
#endif
