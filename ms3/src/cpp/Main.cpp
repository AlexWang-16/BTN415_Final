#ifndef MS3_MAIN
#define MS3_MAIN
#define _CRT_SECURE_NO_WARNINGS
#include "../header/library.h"
#include "../header/MySocket.h"

using namespace std;
int main() {
  /*
  Commands:
  • Create a Telemetry socket (DONE)
  • Perform 3 way handshake (DONE)
  • Receive telemetry packets from the robot. (DONE)

  1) Verify CRC (DONE)
     A telemetry packet is 16 bytes in size

  2) Verify Header data that STATUS bit is set to true (DONE - ?)
  3) Extract and display decimal version of sensor data in the body packet
  • Arm Reading
  • Sonar reading
  
  4) Extract and display Drive flag as 0 or 1
  5) Extract and display Arm and Claw status in English

  i.e. "Arm is Up, Claw is Closed"

  Side Note: If validation fails at any point, sotware should log error to std::cout
              and drop the remaining processing of the packet. (We need to simulate a 
              NACK to test this).
  
  
  */
  const int telemetryDataReadSize = 100;
  char dataBuffer[telemetryDataReadSize];
  MySocket telemetryClient(SocketType::CLIENT, "127.0.0.1", 5000, ConnectionType::TCP, telemetryDataReadSize);

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
    // TODO - Put all other validation below into 1 if statement HERE
  }
  else {
    cout << "CRC Check status: FAIL\n";
  }

  //DEBUG ONLY STATUS bit validation
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
  if (telemetryPacket.getLength() > 7 && telemetryPacket.getCmd() == STATUS) {
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
    };

    TelemetryData pkt;
    
    memcpy(&pkt, telemetryPacket.getBodyData, 16);


    cout << "Sonar Sensor Data: " 
      << pkt.sonarSensorData << '\n';

    cout << "Arm Position Data: "
      <<  pkt.armPositionData << "\n\n";
    
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
  return 0;
}
#endif
