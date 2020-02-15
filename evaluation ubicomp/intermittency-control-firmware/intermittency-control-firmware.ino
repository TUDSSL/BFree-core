#define ON_TIME 500
#define OFF_TIME 500
void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(OFF_TIME);
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(ON_TIME);
  digitalWrite(13, LOW);   // turn the LED on (HIGH is the voltage level)

}
