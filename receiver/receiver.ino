// MVFS Jake Ross 2019


// Irrigation Receiver
// Designed for a Adafruit Feather development board
// Companion device to the Irrigation Sensor
// receives message from sensor every x seconds
// parses message for debugging here
// relays message via I2C (Wire.h) to server board

#include <RH_RF95.h>
#include <Wire.h>

// Radio
#define RFM95_CS           8
#define RFM95_RST          4
#define RFM95_INT          7
#define RF95_FREQ          915.0
#define TX_POWER           5

RH_RF95 rf95(RFM95_CS, RFM95_INT);


// general configuration
#define DataReadyPin       0
#define RxLED              13

// globals
int lastReadingTime;
char data[18];


void setup() {
  Serial.begin(9600);
  delay(1000);
  
  // relay comms
  Wire.begin(8);
  Wire.onRequest(requestEvent);

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
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
 
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(TX_POWER, false);
  
}

void loop() {  
  receive();

}

void receive(){
  if (rf95.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
 
    if (rf95.recv(buf, &len))
    {
      digitalWrite(RxLED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      Serial.print("Got: ");
      Serial.println((char*)buf);
      setData((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
 
      // Send a reply
//      uint8_t data[] = "And hello back to you";
//      rf95.send(data, sizeof(data));
//      rf95.waitPacketSent();
//      Serial.println("Sent a reply");
      digitalWrite(RxLED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}

void setData(char *buf){

  // copy buffer into data
  strncpy(data, buf, 16);
  char buf2[2]="";
  char buf4[6]="";

  // node
  strncpy(buf2, buf+2, 2);
  int v = (int)strtol(buf2, NULL, 16);  
  Serial.print("node buff: "); Serial.println(buf2);
  Serial.print("node: "); Serial.println(v);

  // count
  strncpy(buf2, buf+4, 2);
  v = (int)strtol(buf2, NULL, 16);  
  Serial.print("count buff: "); Serial.println(buf2);
  Serial.print("count: "); Serial.println(v);

  // temp
  strncpy(buf2, buf+6, 2);
  v = (int)strtol(buf2, NULL, 16);  
  Serial.print("temp buff: "); Serial.println(buf2);
  Serial.print("temp: "); Serial.println(v);

  // hum
  strncpy(buf2, buf+8, 2);
  v = (int)strtol(buf2, NULL, 16);  
  Serial.print("hum buff: "); Serial.println(buf2);
  Serial.print("hum: "); Serial.println(v);

  // Ain
  strncpy(buf4, buf+10, 4);
  v = (int)strtol(buf4, NULL, 16);  
  Serial.print("Ain buff: "); Serial.println(buf4);
  Serial.print("Ain: "); Serial.println(v);

  //State
  strncpy(buf2, buf+14, 2);
  v = (int)strtol(buf2, NULL, 16);  
  Serial.print("State buff: "); Serial.println(buf2);
  Serial.print("State: "); Serial.println(v);
  
  // Vbatt
  strncpy(buf4, buf+16, 4);
  v = (int)strtol(buf4, NULL, 16);  
  Serial.print("battery buff: "); Serial.println(buf4);
  Serial.print("battery voltage raw: "); Serial.println(v);
  float vbat = v*3.3*2/1024;
  Serial.print("battery voltage: "); Serial.println(vbat);
}
  
void requestEvent(){
  Serial.println("Request event");
  Wire.write(data);
}

// EOF
//void getData() {
//  Serial.println("Getting reading");
//  //Read the temperature data
//  int tempData = readRegister(0x21, 2);
//
//  // convert the temperature to celsius and display it:
//  temperature = (float)tempData / 20.0;
//
//  //Read the pressure data highest 3 bits:
//  byte  pressureDataHigh = readRegister(0x1F, 1);
//  pressureDataHigh &= 0b00000111; //you only needs bits 2 to 0
//
//  //Read the pressure data lower 16 bits:
//  unsigned int pressureDataLow = readRegister(0x20, 2);
//  //combine the two parts into one 19-bit number:
//  pressure = ((pressureDataHigh << 16) | pressureDataLow) / 4;
//
//  Serial.print("Temperature: ");
//  Serial.print(temperature);
//  Serial.println(" degrees C");
//  Serial.print("Pressure: " + String(pressure));
//  Serial.println(" Pa");
//}
