// MVFS Jake Ross 2019


// Irrigation Receiver
// Designed for a Adafruit Feather development board
// Companion device to the Irrigation Sensor
#define DataReadyPin       0
#define SendEnablePin      0

int lastReadingTime;

void getData() {
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
}


void setup() {
  // put your setup code here, to run once:
  pinMode(SendEnablePin, INPUT);
}

void loop() {  
  // check for a reading no more than once a second.
  if (millis() - lastReadingTime > 1000) {
    // if there's a reading ready, read it:
    // don't do anything until the data ready pin is high:
    if (digitalRead(DataReadyPin) == HIGH) {
      getData();
      // timestamp the last time you got a reading:
      lastReadingTime = millis();
    }
  }

  report();

}

void report(){
  if (digitalRead(SendEnablePin)==HIGH){
    Serial.println("Send Report");
  }
}
