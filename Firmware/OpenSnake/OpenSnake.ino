/*
Snake In A Can.
Pete Lewis
SparkFun Electronics
In collaboration with Mario the Maker Magician

*/
#define POP 40
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

void setup() {
  
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop() {

  while(1)
  {
    Serial.println(analogRead(A0));
    if(analogRead(A0) < 50)
    {
    break;
    }

  }

// delay a random time from 6 to 9 seconds
  for(int i = 0 ; i < random(5,8) ; i++)
  {
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13,LOW);
  delay(900);
  }

  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(POP);
  delay(2000);
  myservo.detach();  // detach, to allow user to move servo freely and RE-arm the snake!!

}

