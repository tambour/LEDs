const int pinButton = 2;
 
void setup() {
  pinMode(pinButton, INPUT);
  Serial.begin(9600);
}
 
void loop() {
  int stateButton = digitalRead(pinButton);
  Serial.println(stateButton);
  delay(20);
}
