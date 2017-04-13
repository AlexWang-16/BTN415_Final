#ifndef MS3_MAIN
#define MS3_MAIN
#define NOMINMAX
#define RESPONSE_DATA_SIZE 12
#define _CRT_SECURE_NO_WARNINGS
#define DATA_BYTE_SIZE 100
#include <algorithm>
#include <iomanip>
#include "../header/library.h"
#include "../header/MySocket.h"

using namespace std;

bool ExeComplete = false, dataSent = true;
int pktCount = 0;
string commandIP, telemetryIP;
int commandPort, telemetryPort = 0;

int main() {
      /*cout << "Command socket connection information\n";
      cout << "-------------------------------------\n";
      cout << "IP Address: ";
      getline(cin, commandIP);
      cout << "Port: ";
      cin >> commandPort;

      cin.clear();
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');*/

      cout << "\nTelemetry socket connection information\n";
      cout << "-------------------------------------\n";
      cout << "IP Address: ";
      getline(cin, telemetryIP);
      cout << "Port: ";
      cin >> telemetryPort;
      
      cin.clear();
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      //thread command(commandThread, commandIP, commandPort);
      thread telemetry(telemetryThread, telemetryIP, telemetryPort);

      //command.join();
      telemetry.join();

  return 0;
}

void commandThread(string ip, int port) {
  
  PktDef sendPkt;
  std::string cmdType;

  MotorBody driveData;
  std::string direction;
  int duration = 0, counter = 0;
  
  char* pktData = nullptr;
  
  char buff[DATA_BYTE_SIZE];
  int rxSize = 0;

  //Establish connection
  MySocket CommandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, DATA_BYTE_SIZE);
  CommandSocket.connectTCP();

  while (!ExeComplete) {
    duration = 0;   //duration will change according to user input each time

    std::cout << std::endl;
    std::cout << "Please enter the command: ";
    getline(std::cin, cmdType);
    std::transform(cmdType.begin(), cmdType.end(), cmdType.begin(), ::tolower);
    
    if (cmdType != "sleep") {
      
      std::cout << "Please enter the direction: ";
      getline(std::cin, direction);
      std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);
      
      if (cmdType == "drive"){
        std::cout << "Please enter the duration: ";
        std::cin >> duration;

      }
      if (duration > 0) {
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
    
    // Set duration
    driveData.duration = duration;

    // Write body data to sendPkt
    sendPkt.setBodyData(reinterpret_cast<char*>(&driveData), 2);

    //Calc CRC
    sendPkt.calcCRC();

    //Generate packet
    pktData = sendPkt.genPacket();

    //Send DefPkt through socket
    CommandSocket.sendData(pktData, sendPkt.getLength()-1);
    
    dataSent = true;
    
    memset(buff, 0, DATA_BYTE_SIZE);
    CommandSocket.getData(buff);
    
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
    counter++;
    pktCount++;
  } 
}

void telemetryThread(std::string ipAddress, int port) {
  
  PktDef telemetryPacket;
  char dataBuffer[DATA_BYTE_SIZE];
  char* readPtr;
  MySocket telemetryClient(SocketType::CLIENT, ipAddress, port, ConnectionType::TCP,
                            DATA_BYTE_SIZE);

  telemetryClient.connectTCP();
  
  while (telemetryPacket.getCmd() != SLEEP) {
    int dataSize = telemetryClient.getData(dataBuffer);

    readPtr = dataBuffer;
    if (dataSent) {
      //Display raw data packet
      cout << "Raw data buffer contents: ";

      for (int i = 0; i < RESPONSE_DATA_SIZE; i++) {
        cout << hex << (unsigned int)dataBuffer[i];
        
        if (i == RESPONSE_DATA_SIZE - 1) {
          cout << endl;
        }
        else {
          cout << ", ";
        }
      }

      //Deserialize telemetry packet
      telemetryPacket = dataBuffer;
      telemetryPacket.calcCRC();

      //CRC check
      if (telemetryPacket.checkCRC(dataBuffer, RESPONSE_DATA_SIZE - 1)) {
        cout << "CRC Check status: OK\n";

        //DEBUG ONLY STATUS bit validation. Remove after GM.
        if (telemetryPacket.getCmd() == STATUS) {
          cout << "Status bit is TRUE\n";
        }
        else {
          cout << "Status bit is FALSE\n";
        }

        if (telemetryPacket.getLength() > 7
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

          memcpy(&pkt, telemetryPacket.getBodyData(), 5);

          cout << "Sonar Sensor Data: "
            << pkt.sonarSensorData << '\n';

          cout << "Arm Position Data: "
            << pkt.armPositionData << "\n\n";

          cout << "Robot status data\n\n";
          cout << "Drive: " << static_cast<int>(pkt.drive) << '\n';

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
      //dataSent = false;
    }
  }
}
#endif
