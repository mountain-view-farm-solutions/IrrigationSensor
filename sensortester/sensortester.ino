
// Sensor pins
#define WaterAIPin         A1
#define WaterDOPin         4
#define Measure_LED        53
#define Status_LED         10
#define MeasurementDelay   10

int get_water_raw(){
  digitalWrite(Measure_LED, HIGH);
  digitalWrite(WaterDOPin, HIGH);
  delay(MeasurementDelay);
  int v = analogRead(WaterAIPin);
  digitalWrite(WaterDOPin, LOW);

  delay(500);
  digitalWrite(Measure_LED, LOW);
  return v;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(WaterDOPin, OUTPUT);
  pinMode(Status_LED, OUTPUT);
  pinMode(Measure_LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int v = get_water_raw();
  digitalWrite(Status_LED, v>512);
  
  Serial.print("Ain: "); Serial.println(v);
  delay(1000);
}
