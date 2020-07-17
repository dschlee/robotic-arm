#include <Arduino.h>
#include <Servo.h>   // servo motor lib
#include <NewPing.h> // ultrasonic sensor lib

// ultrasonic sensor
const int trigPin = 2;
const int echoPin = 3;

// servo pins
const int basePin = 6;
const int elbowPin = 9;
const int wristPin = 10;
const int gripperPin = 11;

Servo baseServo;
Servo elbowServo;
Servo wristServo;
Servo gripperServo;
NewPing sonar = NewPing(trigPin, echoPin);

int basePos;
int elbowPos;
int wristPos;
int gripperPos;

int objectDistance;
int basePosForObject;
int elbowPosForObject;
int wristPosForObject;
int gripperPosForObject;

bool objectDetected;
bool sweepTo140;

int baseDefaultPos = 90;
int elbowDefaultPos = 155;
int wristDefaultPos = 90;
int gripperDefaultPos = 20;

int currentPos;

/* 
updates the current position of the servo in the direction of the target position
*/
int updatePos(int currentPos, int targetPos)
{
  if (currentPos < targetPos)
  {
    currentPos++;
  }
  else if (currentPos > targetPos)
  {
    currentPos--;
  }

  return currentPos;
}

/*
moves servo to the desired position with a constant speed
*/
void moveServo(Servo servo, int targetPos)
{
  currentPos = servo.read();

  while (currentPos != targetPos)
  {
    currentPos = updatePos(currentPos, targetPos);
    servo.write(currentPos);
    delay(10);
  }
}

/*
moves the servos to their default position
*/
void moveToDefaultPos() 
{
  moveServo(baseServo, baseDefaultPos);
  moveServo(elbowServo, elbowDefaultPos);
  moveServo(wristServo, wristDefaultPos);
  moveServo(gripperServo, gripperDefaultPos);
}

/* 
displays the distance to the serial monitor and updates the flag objectDetected
*/
void scan()
{
  digitalWrite(trigPin, LOW);

  // objectDistance = sonar.ping_median(5, 10) / 1000;
  objectDistance = sonar.ping_cm();
  Serial.print("Ping: ");
  Serial.print(objectDistance);
  // Serial.print(sonar.ping_median()/ 1000);
  Serial.println(" cm");

  // updates the flag objectDetected
  if (objectDistance < 12 && objectDistance > 8 && baseServo.read() <= 120 && baseServo.read() >= 70) 
  {
    objectDetected = true;
    Serial.println("Object detected");
    delay(1000);
  }
  else
  {
    objectDetected = false;
  }
}

/*
sweeps the range of motion fom 50° to 160°
*/
void sweep()
{
  // updates the sweepTo140 flag
  if (baseServo.read() <= 50)
  {
    sweepTo140 = true;
  }
  else if (baseServo.read() >= 140)
  {
    sweepTo140 = false;
  }
  // Serial.print(baseServo.read());

  // determines to which direction to sweep
  if (sweepTo140)
  {
    moveServo(baseServo, baseServo.read() + 1);
  }
  else
  {
    moveServo(baseServo, baseServo.read() - 1);
  }

  // delay(5);
}

/*
grabs the object, drops the object to the set position and finally returns to the default pos
*/
void handleObject()
{
  // adjusts pos of baseServo
  if (!sweepTo140 && baseServo.read() <= 120)
  {
    moveServo(baseServo, baseServo.read() - 40);
  }
  else if (sweepTo140 && baseServo.read() >= 70)
  {
    moveServo(baseServo, baseServo.read()- 20);
  }
  
  // grab Object
  moveServo(wristServo, 150);
  moveServo(elbowServo, 50);
  moveServo(gripperServo, 100);

  // transport the object the bowl
  moveServo(elbowServo, 100);
  moveServo(wristServo, 100);
  moveServo(baseServo, 0);
  moveServo(gripperServo, 20);

  // moves default pos for the sweep
  moveToDefaultPos();
}

void setup()
{
  // ultrasonic setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // servo setup
  baseServo.attach(basePin);
  elbowServo.attach(elbowPin);
  wristServo.attach(wristPin);
  gripperServo.attach(gripperPin);
  moveToDefaultPos();

  // serial monitor setup
  Serial.begin(9600);
}

void loop()
{
  scan();

  if (!objectDetected)
  {
    sweep();
  }
  else 
  {
    handleObject();
  }
}