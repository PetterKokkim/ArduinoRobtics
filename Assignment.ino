#include <Servo.h>
Servo leftServo;
Servo rightServo;

#define StopR 82
#define StopL 91

#define GREEN 7
#define YELLOW 12
#define RED 13

#define PBW 4
#define PBB 2

#define LDRRight A0
#define LDRCentre A1
#define LDRLeft A2

#define IRR 2
#define IRT 3

bool first = 1;

int darkMax;

int IRValue = HIGH;

bool found;

String instructionValues[] {
  "2211", //Turn left instruction [0]
  "1212", //Turn right instruction [1]
  "1121", //U-turn instruction [2]
  "1221", //Stop instruction [3]
  "2112", //BOOGIE! instruction [4]
};



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Initialize serial communications with the PC

  pinMode(GREEN, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);

  pinMode(PBW, INPUT);
  pinMode(PBB, INPUT);

  pinMode(LDRRight, INPUT);
  pinMode(LDRCentre, INPUT);
  pinMode(LDRLeft, INPUT);

  pinMode(IRT, OUTPUT); // TX pin
  pinMode(IRR, INPUT); // RX pin

  leftServo.attach(6);
  rightServo.attach(5);

  Stop();
  while (digitalRead(PBW) == HIGH) {}; // Start when button released
  while (digitalRead(PBW) == LOW) {};
  darkMax = calibrateDark();
  delay(1000);

}

void loop() {
  // put your main code here, to run repeatedly:

  String barcodeSeq = "";
  while (barcodeSeq == "") {
    setSpeed(10, 10);
    detect();
    if (found != 0) {
      Serial.println("found obstacle");
      avoidObstacle();
      found = 0;
    }

    bool foundBarCode = checkforBarcode();
    if (foundBarCode || first) {
      Serial.println("found barcode");
      barcodeSeq = barcodePattern();
      foundBarCode = 0;
      first = 0;
    }
  }
  Serial.println("doing instruction");
  Serial.println(barcodeSeq);
  instruction(barcodeSeq);

}


//Functions from here

void setLEDs(int green_s, int yellow_s, int red_s) { // Sets up the LEDs
  digitalWrite(GREEN, green_s);
  digitalWrite(YELLOW, yellow_s);
  digitalWrite(RED, red_s);
}

void Start() {
  while (digitalRead(PBW) == HIGH) {}; // Start when button released
  while (digitalRead(PBW) == LOW) {};
  setLEDs(0, 0, 1);
  delay(500);
  setLEDs(0, 1, 0);
  delay(500);
  setLEDs(1, 0, 0);
  delay(500);
  setLEDs(0, 0, 0);
}

void setSpeed(int Right, int Left) { 
  rightServo.write(StopR + (-Right));
  leftServo.write(StopL + (Left));
}

void Stop() { //function to make the robot to Stop
  setSpeed(0, 0);
}

void moveStraight(int Distance) { //function to make the robot go a distance (Forward and backwards)
  if (Distance < 0) {
    setSpeed(-20, -20);
    delay((137) * -(Distance));
  }
  else if (Distance > 0) {
    setSpeed(20, 20);
    delay((137)*Distance);
  }
  else {
    Serial.println("No distance to go");
  }
}

void Turn(int Degree) { // function to make the robot turn clockwise(+) and counterclockwise(-)
  if (Degree < 0) {
    Serial.println("Turning left");
    setSpeed(20, -20);
    delay((21) * -(Degree));
  }
  else if (Degree > 0) {
    Serial.println("Turning right");
    setSpeed(-20, 20);
    delay((21)*Degree);
  }
  else {
    Serial.println("No turn necessary");
  }
}

void twerk() { //Makes the robot dance (Boogie!)
  int startmillis = millis();
  const int duration = 20000;
  Turn(-7);
  while (millis() - startmillis < duration) {
    setLEDs(1, 1, 1);
    Turn(14);
    delay(25);
    setLEDs(0, 0, 0);
    Turn(-14);
    delay(25);
  }
  Turn(7);
}

int LDRAverage() { //filter for the LDR to use average readings instead
  int average = 0;
  for (int i = 0; i < 5; i++) {
    average = average + analogRead(LDRCentre);
  }
  average = average / 5;
}

int calibrateDark() { //Calibrates the first dark stripe
  int darkValues;
  int darkMax = 1000;
  for (int i = 0; i < 10; i++) {
    if (analogRead(LDRCentre) < darkMax)
      darkMax = analogRead(LDRCentre) + 10;
    delay(10);
  }
  moveStraight(-5);
  Stop();
  return darkMax;
}

String barcodePattern() {
  String barcodeSeq;
  for (int i = 0; i < 4; i++) {
    char barType = barcodeType();
    barcodeSeq += barType;
  }
  return barcodeSeq;
}

char barcodeType() { // reads and seperate the differents barcode type
  int barLength = 0;
  char barType;
  int average;
  while ((average = LDRAverage()) > darkMax) {
  }
  int initMillis = millis();
  while ((average = LDRAverage()) < darkMax + 100) {
    Serial.print("LDRCentre value: ");
    Serial.println(average = LDRAverage());
  }
  barLength = millis() - initMillis;
  Serial.print("barLength: ");
  Serial.println(barLength);
  if (barLength >= 100 && barLength <= 800) {
    setLEDs(0, 0, 0);
    barType = '1';
    setLEDs(1, 0, 0);
    Serial.println("found barcode type 1");
  }
  else if (barLength >= 801) {
    setLEDs(0, 0, 0);
    barType = '2';
    setLEDs(0, 1, 0);
    Serial.println("found barcode type 2");
  }
  if (barType != '0')
    return barType;
  setLEDs(0, 0, 0);
}


void instruction(String s) { // Diffrent instructions for the barcodes
  if (s == instructionValues[0] /*Turn Left*/) {
    setLEDs(1, 0, 0); //Turn green light on
    Turn(-90); //Turn left
    setLEDs(0, 0, 0); //Turn off lights
  }
  else if (s == instructionValues[1] /*Turn right*/) {
    setLEDs(0, 1, 0); //Turn on yellow light
    Turn(90); // Turn right
    setLEDs(0, 0, 0); //Turn off lights
  }
  else if (s == instructionValues[2] /*Uturn*/) {
    setLEDs(1, 1, 0); //Turns green and yellow lights on
    Turn(180); //Turns 180 degrees around
    moveStraight(15);
    setLEDs(0, 0, 0); //Turn off lights
  }
  else if (s == instructionValues[3] /*Stop*/) {
    int i = 0;
    setLEDs(0, 0, 1); // Turn red light on
    Stop(); //Stops the motor
    while ( i = 0 ) {}
  }
  else if (s == instructionValues[4] /*BOOGIE!*/) {
    moveStraight(10);
    twerk();
  }
  else {
    Stop();
    for (int i = 0; i < 5; i++) {
      setLEDs(0, 0, 1);
      delay(200);
      setLEDs(0, 0, 0);
    }
  }
}

bool checkforBarcode() { //Looks for a barcode to read
  long startTime = millis();
  bool foundBlack = 0;
  int average;
  while ((millis() - startTime < 100) && !foundBlack) {
    if ((average = LDRAverage()) < darkMax) {
      foundBlack = 1;
    }
    return foundBlack;
  }
}

int detect() { // Turns on the ir so it can send and recive data
  tone (IRT, 38000);
  delay(200);
  IRValue = digitalRead(IRR);
  obstacle();
  noTone(IRT);
}

bool obstacle() {
  if (IRValue == LOW) {
    Serial.println("Found an obstacle");
    found = 1;
  }
  else if (IRValue == HIGH) {
    //Serial.println("No obstacle detected");
    found = 0;
  }
  else {
    Serial.println("Wierd");
  }
}

void avoidObstacle() { // Avoid an obstacle sequence
  setLEDs(1, 1, 1);
  delay(200);
  Turn(90);
  moveStraight(25);
  Turn(-90);
  Stop();
  delay(200);
  setLEDs(0, 0, 0);
}
