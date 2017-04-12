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

int main() {

  cout << "IP Address: ";
  getline(cin, ip);
  cout << "Port: ";
  cin >> port;

  //thread command(commandThread).detach();
  thread telemetry(telemetryThread, ip, port);

  //command.join();
  telemetry.join();

  return 0;
}

void commandThread() {
  bool ExeComplete = false;

  while (ExeComplete == false) {

    std::string ip = "127.0.0.1";
    int port = 5000;

    MySocket CommandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, 100);

    CommandSocket.ConnectTCP();

    MotorBody driveData;
    std::string direction;
    int duration;

    std::cout << "Please enter the direction: ";
    std::cin >> direction;
    std::cout << "Please enter the duration: ";
    std::cin >> duration;

    PktDef sendPkt;
    int pktCount;
    std::string cmdType;

    std::cout << "Please enter the PktCount: ";
    std::cin >> pktCount;
    sendPkt.setPktCount(pktCount);

    std::cout << "Please enter the command: ";
    getline(std::cin, cmdType);
    //sendPkt.setCmd(cmdType);

    //sendPkt.setBodyData((char*)&driveData);
    sendPkt.calcCRC();

    //CommandSocket.SendData(sendPkt.c_str(), strlen(sendPkt.c_str()));

    char buff[100];
    int RxSize = CommandSocket.GetData(buff);
    if (buff != "0") {
      if (cmdType != "SLEEP") {

      }
      else if (cmdType == "SLEEP") {
        if (0)
          CommandSocket.DisconnectTCP();
        ExeComplete = true;
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