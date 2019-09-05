// MVFS Jake Ross 2019


// Irrigation Sensor
// Designed for a Adafruit Feather development board
#include <RH_RF95.h>
#include <RHMesh.H>
#include <Adafruit_SleepyDog.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// General Configuration
#define useHandShake       0
const int updatePeriod=    1;   // in seconds
const int MeasurePeriod=   10;   // in ms 
const int threshold=       800;
const int NODE_ID=         1;
const int BASE_STATION_ID= 0;


#define VBATPIN            A9
#define DEBUG              1
#define LOWBAT_LED         5


// Radio
#define RFM95_CS           8
#define RFM95_RST          4
#define RFM95_INT          7
#define RF95_FREQ          915.0
#define TX_POWER           20
#define TX_LED             13

// Radio singleton
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHMesh manager(rf95, NODE_ID);


// Sensor pins
#define WaterAIPin         A11
#define WaterDOPin         11
#define Status_LED         10
#define ONE_WIRE_BUS       18  // Data wire is plugged into port 2 on the Arduino


OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature tempsensors(&oneWire); // Pass our oneWire reference to Dallas Temperature. 


// Helper setup
int packetnum = 0;  // packet counter, we increment per xmission

   

int get_temp(int idx, bool trigger){
  int t=0;
  if (trigger){
     tempsensors.requestTemperatures();
  }
  t = tempsensors.getTempCByIndex(idx);
  return t;
}
void report_status_word(){

  int tg = get_temp(0, true);
  int tt = get_temp(1, false);
  int tm = get_temp(2, false);
  
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

  char packet[32]="";

  // message format
  sprintf(packet, "%02x%02x%04x%04x%04x%04x%01x%04x", NODE_ID,       // 02 node id      
                                                      packetnum++,   // 02 packet counter
                                                      tg,            // 04 temp ground
                                                      tm,            // 04 temp mid
                                                      tt,            // 04 temp top
                                                      v,             // 04 Ain
                                                      w,             // 01 State
                                                      rb);           // 04 VBatt 
  
  if (DEBUG){
    Serial.print("Count:      " ); Serial.println(packetnum);
    Serial.print("TempGND:    " ); Serial.println(tg);
    Serial.print("TempMID:    " ); Serial.println(tm);
    Serial.print("TempTOP:    " ); Serial.println(tt);
    Serial.print("Ain:        " ); Serial.println(v);
    Serial.print("State:      " ); Serial.println(w);
    Serial.print("VBatRaw:    " ); Serial.println(rb);
    Serial.print("VBat:       " ); Serial.println(b);
    Serial.print("LOW VBat:   " ); Serial.println(b<3.7);
    Serial.print("Packet:     " ); Serial.println(packet);
  }

//  if (digitalRead(TX_ENABLE) {
    vtransmitt(packet);  
//  }
  

  // reset packet counter
  if (packetnum > 255){
    packetnum = 0;
  }
}

void vtransmitt(char *packet){
  digitalWrite(TX_LED, HIGH);
//  transmitt(packet);
  meshtransmitt(packet);
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

void meshtransmitt(char* packet){
  digitalWrite(TX_LED, HIGH);
  manager.sendtoWait((uint8_t *)packet, sizeof(packet), BASE_STATION_ID);
  digitalWrite(TX_LED, LOW);
}

void transmitt(char *packet){  
  //Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)packet, sizeof(packet));
 
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

  manager.init();
  
}
   

void loop() {
  report_status_word();
  rf95.sleep();
  if (DEBUG){
    delay(updatePeriod*1000);
  } else{
    Watchdog.sleep(updatePeriod*1000);
    
  }
  
}
