// MVFS Jake Ross 2019


// Irrigation Sensor
// Designed for a Adafruit Feather development board
// report water, temp,hum


#define WaterAIPin          0
#define WaterDOPin          0
#define MeasureLEDPin       0


void report_temp(){
  Serial.println('Temp');
}
void report_hum(){
  Serial.println('Hum');
}

void report_water(){
  digitalWrite(MeasureLEDPin, HIGH);
  digitalWrite(WaterDOPin, HIGH);
  delay(200);
  int v = analogRead(WaterAIPin);
  digitalWrite(WaterDOPin, LOW);
  digitalWrite(MeasureLEDPin, LOW);
  Serial.println('Water: '+v);
}

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(WaterDOPin, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  report_temp();
  report_hum();
  report_water();
}
