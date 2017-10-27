/*
Snake In A Can.
Pete Lewis
SparkFun Electronics
In collaboration with Mario the Maker Magician

// watch light sensor on A0 for a delta downward (indicating someone waved their hand over the light sensor)
// choose a random time delay between 5-8 seconds, beeping the beeper on D6/D3 each second

*/
#define POP 40
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int reading = 0; // initiate light reading variable at zero, to avoid a false start.
int previous_reading = 0; // used to store previsou reading and help notice a delta downward.

void setup() {
  
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(6, OUTPUT); // buzzer side "1"
  digitalWrite(6, LOW);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW); // buzzer side "2" - the control side.  
}

void loop() {

  while(1)
  {
    reading = analogRead(A0);
    int delta = previous_reading - reading;
    Serial.print("\tprevious_reading: ");
    Serial.print(previous_reading);
    Serial.print("\treading: ");
    Serial.print(reading);
    Serial.print("\tdelta: ");
    Serial.println(delta);    
    if(delta > 10)
    {
    break;
    }
    previous_reading = reading; // store in this variable to use on next round of loop.
    

  }

// delay a random time from 5 to 8 seconds
  for(int i = 0 ; i < random(5,8) ; i++)
  {
  digitalWrite(13, HIGH);
  delay(500);
  int pitch = 300;
  int duration = 200;
  if(pitch > 100) 
  {
    pitch -= (10*i);
    duration += 100;
  }
  buzz(pitch, duration);
  digitalWrite(13,LOW);
  delay(500);
  }

  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  myservo.write(POP);
  delay(2000);
  myservo.detach();  // detach, to allow user to move servo freely and RE-arm the snake!!

  // reset variables for a new trigger next time around.
  reading = 0;
  previous_reading = 0;
}


void buzz(int pitch, int duration)
{
  for(int i = 0 ; i < duration ; i++)
  {
    digitalWrite(3,HIGH);
    delayMicroseconds(pitch);
    digitalWrite(3,LOW);
    delayMicroseconds(pitch);
  }
}

