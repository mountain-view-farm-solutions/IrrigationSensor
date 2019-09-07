// MVFS Jake Ross 2019


// Irrigation Receiver
// Designed for a Adafruit Feather development board
// Companion device to the Irrigation Sensor
// receives message from sensor every x seconds


#include <RH_RF95.h>
#include <RHMesh.h>

// Radio
#define RFM95_CS           8
#define RFM95_RST          4
#define RFM95_INT          7
#define RF95_FREQ          915.0
#define TX_POWER           20
#define NODE_ID            0

RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHMesh manager(rf95, NODE_ID);

// general configuration
#define RxLED              20  // Red
#define LOW_BATT_LED       23  // Red
#define STATE_LED          22  // Green
#define STATUS_LED         21  // Yellow
#define BUZZER_PIN         19

#define DEBUG              0
#define TIMEOUT            3000


void setup() {
  Serial1.begin(19200);
  if (DEBUG){
    Serial.begin(9600);
  }
  
  pinMode(LOW_BATT_LED, OUTPUT);
  pinMode(STATE_LED, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);
  
  // Radio setup
  pinMode(RxLED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // Manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
  while (!rf95.init()) {
    //Serial.println("LoRa radio init failed");
    //Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  //Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    //Serial.println("setFrequency failed");
    while (1);
  }
  //Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  //rf95.setModemConfig(rf95.Bw125Cr48Sf4096);
  
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(TX_POWER, false);
  
  manager.init();
  
}

uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];

void loop() {  
  meshreceive();
}

void meshreceive(){
  uint8_t len = sizeof(buf);
  uint8_t from;
  digitalWrite(STATUS_LED, HIGH);
  if (manager.recvfromAckTimeout(buf, &len, TIMEOUT, &from))
  {  
    digitalWrite(STATUS_LED,  LOW);
    serial_relay(from);
  } else {
    digitalWrite(STATUS_LED,  LOW);
    delay(1000);
  }
}


void serial_relay(uint8_t from){
  digitalWrite(RxLED, HIGH);
  char rssi[5];
  sprintf(rssi, "%02x%02x", abs(rf95.lastRssi()), from);
  
  Serial1.print((char*)buf);
  Serial1.println(rssi);


  char target[4];
  strncpy(target, buf+21, 4);
  int vbatt = (int)strtol(target, NULL, 16); 

  char sb[1];
  strncpy(sb, buf+20, 1);
  int state = (int)strtol(sb, NULL, 16);
  if (DEBUG){
    Serial.println((char*)buf);
    Serial.print("vbatt:");
    Serial.println(vbatt);
    Serial.print("state:");
    Serial.println(state);
  }
  
  digitalWrite(LOW_BATT_LED, LOW);
  digitalWrite(STATE_LED, state);
  
  if (vbatt<4.0){
    digitalWrite(STATE_LED, LOW);
    digitalWrite(LOW_BATT_LED, HIGH);
    if (vbatt<3.8){
       // play buzzer
       tone(BUZZER_PIN, 640, 400);
       if (vbatt<3.7){
         digitalWrite(STATE_LED, HIGH);
       }
    }
  }
  
//  if (state){
//     //play buzzer
//     //tone(BUZZER_PIN, 640, 1000);
//  }
//  else {
//    //noTone(BUZZER_PIN);
//    delay(100);
//  }
  delay(300);
  digitalWrite(RxLED, LOW);
  
}
// EOF
//void loop()
//{
//  uint8_t len = sizeof(buf);
//  uint8_t from;
//  if (manager.recvfromAck(buf, &len, &from))
//  {
//    Serial.print("got request from : 0x");
//    Serial.print(from, HEX);
//    Serial.print(": ");
//    Serial.println((char*)buf);
//    // Send a reply back to the originator client
////    if (manager.sendtoWait(data, sizeof(data), from) != RH_ROUTER_ERROR_NONE)
////      Serial.println("sendtoWait failed");
//  }
//}
//void receive(){
//  if (rf95.available())
//  {
//    // Should be a message for us now
//    uint8_t len = sizeof(buf);
// 
//    if (rf95.recv(buf, &len))
//    {
//      
////      RH_RF95::printBuffer("Received: ", buf, len);
////      Serial.print("Got: ");
////      Serial.println((char*)buf);
////      setData((char*)buf);
////      Serial.print("RSSI: ");
////      Serial.println(rf95.lastRssi(), DEC);
//        serial_relay(0);
//        
//      // Send a reply
////      uint8_t data[] = "And hello back to you";
////      rf95.send(data, sizeof(data));
////      rf95.waitPacketSent();
////      Serial.println("Sent a reply");
//      
//    }
//    else
//    {
//      Serial.println("Receive failed");
//    }
//  }
//}
