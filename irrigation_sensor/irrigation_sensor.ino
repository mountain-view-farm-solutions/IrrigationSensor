// MVFS Jake Ross 2019


// Irrigation Sensor
// Designed for a Adafruit Feather development board
#include <RH_RF95.h>
#include "Adafruit_SleepyDog.h"

// Radio
#define RFM95_CS           8
#define RFM95_RST          4
#define RFM95_INT          7
#define RF95_FREQ          915.0
#define TX_POWER           13
#define TX_LED             13
// Radio singleton
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Sensor pins
#define WaterAIPin         A1
#define WaterDOPin         11
#define Status_LED         10

// General Configuration
#define useHandShake       0
const int updatePeriod=    1;   // in seconds
const int MeasurePeriod=   10;   // in ms 
const int threshold=       100;
#define VBATPIN            A9
#define NODE_ID            0
#define DEBUG              0
#define LOWBAT_LED         5


// Helper setup
int packetnum = 0;  // packet counter, we increment per xmission

   

void report_status_word(){
  // status word format
  //  @@IITTTHHVVWBB\n
  // @@ Starts frame
  // ID word
  // Temperature word
  // humdity word
  // raw analog in word
  // state byte
  // battery voltage word
  // new line ends frame
  int t = 1;
  int h = 10;
  int v = get_water_raw();
  int w = v>threshold;
  
  digitalWrite(Status_LED, w);

  // get battery voltage
  int rb = analogRead(VBATPIN);
  float b = rb* 2;    // we divided by 2, so multiply back
  b *= 3.3;  // Multiply by 3.3V, our reference voltage
  b /= 1024; // convert to voltage

  digitalWrite(LOWBAT_LED, b<3.7);

  char packet[20];
  sprintf(packet, "@@%02x%02x%02x%02x%04x%02x%04x\n",NODE_ID, packetnum++, t,h,v,w,rb);
  
  if (DEBUG){
    Serial.print("Count:      " ); Serial.println(packetnum);
    Serial.print("Temp:       " ); Serial.println(t);
    Serial.print("Hum:        " ); Serial.println(h);
    Serial.print("Ain:        " ); Serial.println(v);
    Serial.print("State:      " ); Serial.println(w);
    Serial.print("VBatRaw:    " ); Serial.println(rb);
    Serial.print("VBat:       " ); Serial.println(b);
    Serial.print("LOW VBat:   " ); Serial.println(b<3.7);
    Serial.print("Packet:     " ); Serial.println(packet);
  }
  
  vtransmitt(packet);

  // reset packet counter
  if (packetnum > 255){
    packetnum = 0;
  }
}

void vtransmitt(char *packet){
  digitalWrite(TX_LED, HIGH);
  transmitt(packet);
  digitalWrite(TX_LED, LOW);
  
}

int get_water_raw(){
//  digitalWrite(Measure_LED, HIGH);
  digitalWrite(WaterDOPin, HIGH);
  delay(MeasurePeriod);
  int v = analogRead(WaterAIPin);
  digitalWrite(WaterDOPin, LOW);
//  digitalWrite(Measure_LED, LOW);
  return v;
}

void transmitt(char *packet){
//  itoa(packetnum++, packet+13, 10);
//  Serial.print("Sending "); Serial.print(packet);
//  itoa(packetnum++, packet+13, 10);
//  packet[19] = 0;
  
  //Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)packet, 20);
 
  //Serial.println("Waiting for packet to complete..."); 
  delay(10);
  rf95.waitPacketSent();

//  if (useHandShake){
//    //Now wait for a reply
//    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
//    uint8_t len = sizeof(buf);
//    Serial.println("Waiting for reply...");
//    if (rf95.waitAvailableTimeout(1000))
//    { 
//      // Should be a reply message for us now   
//      if (rf95.recv(buf, &len))
//     {
//        Serial.print("Got reply: ");
//        Serial.println((char*)buf);
//        Serial.print("RSSI: ");
//        Serial.println(rf95.lastRssi(), DEC);    
//      }
//      else
//      {
//        Serial.println("Receive failed");
//      }
//    }
//    else
//    {
//      Serial.println("No reply, is there a listener around?");
//    }
//  }
}

void setup() {
  // comms setup
  Serial.begin(9600);

  
  // sensor setup
  pinMode(WaterDOPin, OUTPUT);
  pinMode(LOWBAT_LED, OUTPUT);
  pinMode(Status_LED, OUTPUT);
  
  // radio setup
  pinMode(TX_LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // radio reset
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
  // put your main code here, to run repeatedly:

//  report_temp();
//  report_hum();
//  report_water();
  report_status_word();
  rf95.sleep();
  if (DEBUG){
    delay(updatePeriod*1000);
  } else{
    Watchdog.sleep(updatePeriod*1000);
    
  }
  
}
