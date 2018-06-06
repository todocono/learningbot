/*Learning Bot Testing Peripheries Code

   Libraries: Servo, WS2812

   When Uploaded, Learningbot will have 5 states:

   Initial State -  LEDs are pink and none of the peripheries are not being used.
   Learningbot stays still

   Barrier Detector - Learningbot travels until an obsicale or wall is detected via the
   Ultrasonic Ranger. When this happens, Learning will back up and then turn either
   left or Right

   Line Follower - Learningbot will follow a black line. For best results, use black
   vinyl tape.

   Light Follower - When provided enough light, Learningbot will turn in the direction
   of that light soure and follow it.

   Test Analog Inputs - Correlates an LED to the Periphery sensor near it. each LEDs
   brightness is correlated to the analog input from that sensor.

   To change state, simple push the on-board push button.
   In each of these states, the buzzer will turn on when turning or moving backwards.
   The potentiometer affects the buzzer's pitch.

   Neopixel Color Indication:
   Pink - Initial state
   Green - Forward
   Blue - Turning either left or right
   Red- Backward
   Yellow - Stalling

   Citations:
   HCSR04 example code and ws2812 library example code
  light_ws2812 example

  Created: 07.03.2014 12:49:30
   Author: Matthias Riegler
*/
#include <Servo.h>
Servo leftServo, rightServo;
#define RightServo 11
#define LeftServo 10

#include <WS2812.h>
#define numofLEDs 4
WS2812 LED(numofLEDs); // 4 LEDs
cRGB value;
/*
   Arduino pins for peripheries
*/
#define Sensor1DigitalPin 13
#define Sensor2DigitalPin 12
#define Sensor3DigitalPin 5
#define Sensor4DigitalPin 4

#define Sensor1AnaloglPin 0
#define Sensor2AnaloglPin 1
#define Sensor3AnaloglPin 2
#define Sensor4AnaloglPin 3

#define Potentiometer 4//Verified 5-17-18 Analog Pin A4
#define Buzzer 3//Verified

#define PushButton 8
// digital pins for ultrasonic sensor
#define TriggerPin 7
#define EchoPin 6

// values for servos
#define LeftServoForward 105
#define RightServoForward 75
#define LeftServoBackward 75
#define RightServoBackward 105
//#define LeftServoForward 120
//#define RightServoForward 60
//#define LeftServoBackward 60
//#define RightServoBackward 120

long duration, distance;

int distanceArray[2];//for comparing distance samples to eradicate sporadic values

int counter = 0;// keeps track of pushbutton and helps turn learning bot "on" and "off"
long prevMillis = 0;//long for "AlternateBackward" timer
bool AlternateLeftRight;
bool AlternateBackward;
bool onOff = false;//boolean to turn learning bot "on" and "off"
bool online = false;//boolean to keep learningbot online within working perameters
bool leftSensor, rightSensor, leftSensor2, rightSensor2;
void setup() {
  LED.setOutput(9);//neopixels connected to pin 9

  pinMode(Sensor1DigitalPin, INPUT);
  pinMode(Sensor2DigitalPin, INPUT);
  pinMode(Sensor3DigitalPin, INPUT);
  pinMode(Sensor4DigitalPin, INPUT);
  pinMode(TriggerPin, OUTPUT); // Sets the TriggerPin as an Output
  pinMode(EchoPin, INPUT); // Sets the EchoPin as an Input

  Serial.begin(9600);
}

void loop() {
  pushButton();
}
void pushButton() {
  if (digitalRead(PushButton)) {
    counter++;
    delay(250);
  }
  if (counter <= 4) {
    switch (counter) {
      case 0:
        onOff = false;
        NeopixelsHigh(100, 0, 100, 0, numofLEDs); //pink
        rightServo.detach();
        leftServo.detach();
        noTone(Buzzer);
        break;
      case 1:
        onOff = true;
        barrierDetector();//function for barrier detection
        //Serial.println("Barrier Detector w/ Ultrasonic Sensor: barrierDetector");
        Serial.print("On: ");
        Serial.print(onOff);
        Serial.print("  ||  Online:  ");
        Serial.print(online);
        Serial.print("  ||  Distance:  ");
        Serial.print(distance);
        Serial.print("  ||  Counter:  ");
        Serial.println(counter);
        break;
      case 2:
        onOff = true;
        leftSensor = !digitalRead(Sensor3DigitalPin);
        rightSensor = !digitalRead(Sensor2DigitalPin);
        lineFollower();// function for running learningbot
        //for troubleshooting lineFollower() function
        //Serial.println("Line Follower w/ Internal IR Sensors : lineFollower1");
        Serial.print("Left sensor: ");
        Serial.print(leftSensor);
        Serial.print(" || Right sensor: ");
        Serial.println(rightSensor);
        break;
      case 3:
        onOff = true;
        leftSensor = digitalRead(Sensor4DigitalPin);
        rightSensor = digitalRead(Sensor1DigitalPin);
        lightFollower();// function for running learningbot
        //for troubleshooting lineFollower() function
        //Serial.println("Line Follower w/ Internal IR Sensors : lineFollower2");
        Serial.print("Left sensor: ");
        Serial.print(leftSensor2);
        Serial.print(" || Right sensor: ");
        Serial.println(rightSensor2);
        break;
      case 4:
        onOff = false;
        testAnalogInput();
        rightServo.detach();
        leftServo.detach();
        noTone(Buzzer);
        break;
    }
  } else {
    counter = 0;
  }
}
void testAnalogInput() {
  /*
     This function tests the analog input of the IR and LDR sensors
     The analog input will affect the brightness of a corresponding
     neopixel near it.
  */
  int analog1 = (analogRead(Sensor1AnaloglPin) / 4) - 155 ;
  int analog2 = (analogRead(Sensor2AnaloglPin) / 4) - 155;
  int analog3 = (analogRead(Sensor3AnaloglPin) / 4) - 155;
  int analog4 = (analogRead(Sensor4AnaloglPin) / 4) - 155;
  NeopixelsHigh(analog1, analog1, analog1, 3, numofLEDs);
  NeopixelsHigh(analog2, analog2, analog2, 2, 3);
  NeopixelsHigh(analog3, analog3, analog3, 1, 2);
  NeopixelsHigh(analog4, analog4, analog4, 0, 1);
  Serial.print("Analog 1:  ");
  Serial.print(analog1);
  Serial.print("  ||  Analog 2:  ");
  Serial.print(analog2);
  Serial.print("  ||  Analog 3:  ");
  Serial.print(analog3);
  Serial.print("  ||  Analog 4:  ");
  Serial.println(analog4);
  delay(10);
}
void lightFollower() {
  /* Learningbot will follow a black line using two IR sensors
  */
  if (onOff) {
    rightServo.attach(RightServo);
    leftServo.attach(LeftServo);
    online = true;
  }
  else {
    rightServo.detach();
    leftServo.detach();
    NeopixelsOff();
    online = false;
    noTone(Buzzer);
  }
  if (online) {
    if (!leftSensor && !rightSensor) {
      forward();
      noTone(Buzzer);
    }
    else {
      rightServo.attach(RightServo);
      leftServo.attach(LeftServo);
      //alternate();
      if (leftSensor && !rightSensor) {
        left();
        buzzer();
      }
      else if (!leftSensor && rightSensor)  {
        right();
        buzzer();
      }
      else if (leftSensor && rightSensor) {
        stall();
        noTone(Buzzer);
      }
    }
  }
  delay(1);
}
void lineFollower() {
  /* Learningbot will follow a black line using two IR sensors
  */
  if (onOff) {
    rightServo.attach(RightServo);
    leftServo.attach(LeftServo);
    online = true;
  }
  else {
    rightServo.detach();
    leftServo.detach();
    NeopixelsOff();
    online = false;
    noTone(Buzzer);
  }
  if (online) {
    if (!leftSensor && !rightSensor) {
      stall();
      noTone(Buzzer);
    }
    else {
      rightServo.attach(RightServo);
      leftServo.attach(LeftServo);
      //alternate();
      if (leftSensor && !rightSensor) {
        left();
        buzzer();
      }
      else if (!leftSensor && rightSensor)  {
        right();
        buzzer();
      }
      else if (leftSensor && rightSensor) {
        forward();
        noTone(Buzzer);
      }
    }
  }
  delay(1);
}

void barrierDetector() {
  /* Learningbot will go straight untli the ultrasonic sensor detctes a wall
     Potentiometer controls brightness of neopixels
     Pushbutton turns everything "on" or "off"
     Buzzer beeps when wall is detected
  */
  int averageDistance;
  ultrasonicSensor();
  for (int i = 0; i < 2; i ++) {
    distanceArray [i] = distance;
  }
  averageDistance = (distanceArray[0] - distanceArray [1]) / 2;
  if (onOff) {
    if (distance > 4300 || distance < 5) {
      online = false;
    }
    else {
      rightServo.attach(RightServo);
      leftServo.attach(LeftServo);
      online = true;
    }
  }
  else {
    rightServo.detach();
    leftServo.detach();
    NeopixelsOff();
    online = false;
    noTone(Buzzer);
  }
  if (online) {
    if (abs(averageDistance) < 4500 && distance > 15) {
      forward();
      noTone(Buzzer);
    }
    else {
      buzzer();
      left();
      //alternateLeftRightandBackward();
    }
  }
  delay(5);
}
void alternateLeftRightandBackward() {
  bool alternate;
  if (millis() - prevMillis >= 2000) {
    prevMillis = millis();
    AlternateLeftRight = !AlternateLeftRight;
    //Serial.println("backward");
  }
  if (AlternateLeftRight) {
    left();
    Serial.println("left");
  }
  else {
    right();
    Serial.println("right");
  }
}
/*
   DO NOT TOUCH THESE FUNCTIONS
*/
/*
   for neopixels
*/
void NeopixelsOff() {
  value.b = 0; value.g = 0; value.r = 0; // RGB Value -> Blue
  for (int i = 0; i < numofLEDs; i++) {
    LED.set_crgb_at(i, value); // Set value at LED found at index i
    LED.sync(); // Sends the value to the LED
  }
}
void NeopixelsHigh(int r, int g, int b, int lowerLimit, int upperLimit) {
  value.b = b; value.g = g; value.r = r;
  for (int i = lowerLimit; i < upperLimit; i++) {
    LED.set_crgb_at(i, value); // Set value at LED found at index i
    LED.sync(); // Sends the value to the LED
  }
}
void buzzer() {
  int potValue = analogRead(Potentiometer) * 2;
  tone(Buzzer, potValue);
  //Serial.println(potValue);
}
/*
   functions for servo directions, don't touch!
*/
void forward() {
  rightServo.write(RightServoForward);
  leftServo.write(LeftServoForward);
  NeopixelsHigh(0, 255, 0, 0, numofLEDs);//green
}
void backward() {
  rightServo.write(RightServoBackward);
  leftServo.write(LeftServoBackward);
  NeopixelsHigh(255, 0, 0, 0, numofLEDs);//red
}
void left() {
  rightServo.write(RightServoBackward);
  leftServo.write(LeftServoForward);
  NeopixelsHigh(0, 0, 255, 0, 2);//blue
}
void right() {
  rightServo.write(RightServoForward);
  leftServo.write(LeftServoBackward);
  NeopixelsHigh(0, 0, 255, 2, numofLEDs);//blue
}
void stall() {
  rightServo.detach();
  leftServo.detach();
  NeopixelsHigh(255, 255, 0, 0, numofLEDs);//yellow
}
/*
   functions for ultrasonic sensor, don't touch!
*/
void ultrasonicSensor() {
  // Clears the TriggerPin
  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(2);
  // Sets the TriggerPin on HIGH state for 10 micro seconds
  digitalWrite(TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  // Reads the EchoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(EchoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor

  //  Serial.print("Distance: ");
  //  Serial.println(distance);
}
long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}

