#include <iostream>
#include <HardwareSerial.h>
#include <Wire.h>

#define BAUD_RATE 115200
#define TX2 17
#define RX2 16
#define SERIAL_MODE SERIAL_8N1

uint32_t data = 0; // data -> full after 4 bytes have been added
uint8_t byteNum = 0; // tracks the number of bytes that have been added to the data variable
uint8_t dataCount = 0; // flag to see if the full data packet is ready to be decoded -> 4 32bit words
bool packetFull = false;
uint32_t dataPacket[4];
uint16_t redDistance;
uint16_t blueDistance;
uint16_t yellowDistance;
uint16_t walls; 

void decode(uint32_t frame[4]){
  for(int i = 0; i < 4; i++ ){
    uint32_t identifier = (frame[i] >> 24) & 0xFF; // extracts the most significant 8 bits for the identifier.
    uint32_t distance = frame[i] | 0b11111111111; // bottom 11 bits is used for distance
    
    switch(identifier){
      case 0x79: // red
        redDistance = distance;
        break;
      case 0x62: // blue
        blueDistance = distance;
        break;
      case 0x72: // yellow
        yellowDistance = distance;
        break;
      case 0x0: // lines
        walls = (distance | 0b111);
        break;
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE, SERIAL_MODE, RX2, TX2);
}

void loop() {
  packetFull = false;
  while(Serial2.available() && !packetFull){
    uint8_t byte = Serial2.read();
    data = data | (uint32_t)byte << (8 * byteNum); 
    byteNum++;
    if(byteNum == 4){
      byteNum = 0;
      data = 0;
      dataCount++;
    }
    if(dataCount == 4){
      packetFull = true;
      decode(dataPacket);
    }
  }
}