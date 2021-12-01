/*
  Snake In A Can Controller
  Written by: Pete Lewis, with contributions from Jeff Haas
  Jumping Snakes Engineering LLC
  A collaboration with Mario the Maker Magician
  https://www.mariothemagician.com/
  
  Original start date Jan 21st, 2020
  
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Jan 12th, 2021
  This firmware also has contributions from Jeff Haas.
  Big thanks for the laughing, crying and happy birthday song code!

  Also thanks to the Adafruit animal sounds example code found here:
  https://learn.adafruit.com/adafruit-trinket-modded-stuffed-animal/animal-sounds
  
  And JSHENGs happy birthday example code found here:
  https://create.arduino.cc/projecthub/jsheng/happy-birthday-lights-and-sounds-1745cd

  This firmware is intended to be used with the Snake in a Can Controller.
  That product, videos and more info can be found here:
  https://www.mariothemagician.com/snake
  Note, it could also be wired up with any Arduino board.
  It allows the user to blink LEDs, buzz sounds, and turn a servo.
  The servo holds the latch on a lid, that can let a spring snake jump out.

  The controller is often placed inside a brown paper lunch bag, and a robot
  face is drawn on the front of the bag. Now you've got a robot that can blink,
  talk (buzz) and pop a snake!

  With these elements in the design, a performer (most likely a magician)
  can perform a skit using this controller as a robot to interact with.
  By timing the events of various buzz sounds, LED blinks, and snake pops,
  the robot can now be part of the show.

  The controller comes pre-programmed with one skit that Mario the Maker has
  written. Visit https://mariothemagician.com/snake to watch a video, learn the
  magic show and perform it yourself!

  You can also write your own magic shows using the on-board recording buttons.
  See the USER INTERFACE comments below.

  /////////////// USER INTERFACE //////////////////////////////////////////////////////////

  To play a track:
  Turn on power.
  User sees current track number blink on LED.
  Press PLAY to play track.

  To select a different track:
  Turn on power.
  User sees current track number blink on LED.
  Prest TRACK SELECT (hold down for 3 seconds) to change tracks.
    -When you press this button, it will increment the current track
    -It will also "loop back around" to track 1, when you reach the end and press it again.
    -Each time you press it, you will see the current track number blink on the LED

  To Record a new track:
  Note, you can record over the current track, or you can increment to an empty track.
  Select desired track you wish to record on (or over).
  Press REC (hold for 3 seconds) to begin recording.
  Press "YES", "NO", and "SNAKE" as desired.
    -It is recommended that you write out your skit, and then just read the lines as you record
  Press REC again to stop recording.
  Now your new track is avaialable for playback.

  *  Note, new sounds have been added into firmware version 1.1
  *  Crying, laughing and happy birthday.
  *  To access these additional sounds, you must hold down the "TRACK SELECT" button before
  *  pressing the other buttons. The "TRACK SELECT" button acts like a "shift" key.
  *  TRACK SELECT + YES = LAUGH
  *  TRACK SELECT + NO = CRY
  *  TRACK SELECT + SNAKE = HAPPY BIRTHDAY

  FACTORY RESET
  If you do not want to keep your current tracks,
  and would like to restore the original Mario example track,
  (1) Turn unit OFF
  (2) Press and Hold REC button
  (3) Turn the unit ON (while continuing to hold down REC)
  (4) Watch for 3 red blinks
  (5) Release button
  Now your controller has been set back to it's orginal track and your other tracks have been cleared.

  AUTOPLAY
  When autoplay is engaged, your controller will begin the selected track on power up (without requiring you to press the PLAY button).
  This is useful for performances, because you can more easily access the power slide switch (on the battery pack).
  To turn on autoplay:
  (1) Turn unit OFF
  (2) Press and Hold PLAY button
  (3) Turn the unit ON (while continuing to hold down PLAY)
  (4) Watch for 3 green blinks
  (5) Release button
  Now autoplay has been engaged. Each time you power on your controller, it will begin the selected track.
  To turn off autoplay (aka go back into "normal mode"):
  Repeat the process above.
  Note, each time you do the process above it will effectivly "toggle" autoplay on or off.
  Also, a factory_reset() will also set autoplay to OFF.
  
  ///////////////  EEPROM NOTES //////////////////////////////////////////////////////////

  We will use EEPROM to store "tracks"
  A track is a recorded sequence of events.
  Each event is a button press, which will have a timestamp and a type
  timestamps will be a value from 0-255
  a timestamp will represent seconds in 2.1 format
  for example 010 is 1.0 seconds, 215 is 21.5 seconds
  This means that the longest timestamp possible is 25.5 seconds

  Events will be either "yes", "no", "snake", or "end"
  These will be represented by the values 1,2, or 3.
  More event types might come later.

  A track consists of a sequence of timestamps and event types.

  NOTE!! EEPROM location 0 is special and will contain the selected track.

  EEPROM location 1023 is special and will tell us if this is a fresh IC.
  Fresh ICs come with all EEPROM set to 255, so that means we need to do a factory reset.
  After we do a factory reset, then we will set location 1023 to a 1, indicating that this is no longer
  a fresh chip.
  To cause a factory reset, we will simply write location 1023 to 255, and require a power cycle reset.

  EEPROM locations 1-101 will be track 1.
  For example a track might look like this:
  010, 001, 010, 002, 025, 003
  This track would pause 1.0 seconds,
  Then play a "yes"
  Pause for 1.0 seconds,
  Them play a "no"
  Pause for 2.5 seconds
  Then pop open the snake.

  NOTE, each track can have 50 events.
  NOTE, we will allow for 5 tracks, so that will requres 500 memory locations.
  The ATMEGA328 has 1000 EEPROM locations, so we should be fine.

  Mario's original example track will be stored in track 1.
  It is also "backed up" as a variable array in this sketch, and can be restored via factory_reset().


  /////////////// HARDWARE SETUP //////////////////////////////////////////////////////////

  Micro:      ATEMGA328
  Ext osc:    16 MHz
  Brownout:   2.7V
  HIGH FUSE:  0xDE
  LOW FUSE:   0xFF
  EXT FUSE:   0xFD
  LOCK:       0xCF
  Bootloader: Optiboot
  Board select: Arduino/Genuino Uno

*/

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

#include <EEPROM.h>

#define EEPROM_LOCATION_TRACK 0 // to store which track we have selected, 0-5
#define EEPROM_LOCATION_AUTOPLAY 1023 // to store if we're in Autoplay (1) or not (0)
#define AUTOPLAY_ON 1
#define AUTOPLAY_OFF 0

#include "pitches.h" // used for the happy birthday function

// Event types
#define YES 1
#define NO 2
#define SNAKE 3
#define LAUGH 4
#define CRY 5
#define HAPPY 6
#define END 99 // had to choose something, so went with this, wanted to leave room for other future event types

// Control Buttons
#define RECORD_BUTTON A0
#define PLAY_BUTTON A1
#define TRACK_SELECT_BUTTON A2
#define YES_BUTTON A5
#define NO_BUTTON A4
#define SNAKE_BUTTON A3

// User input commands
#define RECORD_CMD 1
#define PLAY_CMD 2
#define TRACK_CMD 3
#define YES_CMD 4
#define NO_CMD 5
#define SNAKE_CMD 6
#define LAUGH_CMD 7
#define CRY_CMD 8
#define HAPPY_CMD 9

byte userInput = 0; // global variable to store current user input command

// button counters, used to accept "long presses" on a button.
// Record and track select are special, they require the user to
// press the button longer (2 seconds?) for it to cause a valid press.
// play is instant effective and will begin the current track instantly,
// because users often want a skit to start playing instantly.
// This is most important to avoid accidental record or track select presses.
//
// Note, snake button was also added in FW ver 1.1 to allow the user to
// cause a snake pop to set the servo horn. If you hold down servo for a
// long press, then it calls the snake function moving the servo to the
// open position.
int recordButtonCounter = 0;
int incrementTrackButtonCounter = 0;
int snakeButtonCounter = 0;


#define SERVO_PWM_PIN 9
#define SERVO_PWR_CONTROL_PIN 7
#define BUZZER_PIN 8

#define RED_LED_pin 13
#define GREEN_LED_pin 10

// example tracks
// they are a sequence of events
// they are formatted like so:
// delay, type, delay, type, and so on... (max 50 events)
// delay values are in 2.1 format, (so 10 = 1.0 second, and 105 = 10.5 seconds)
// this gives you 1/10th of a second precision
// and the max delay is 25.5 seconds (EEPROM stores byte, so values of 0-255).
// all tracks must have a last event delay of 0, followed by an event type END

byte example_track_1[] = {
  56, YES, // first event, delay 5.3 seconds, sound "yes"
  51, YES, // second event, delay 5.1 seconds, sound "yes"
  2, YES,
  72, NO,
  67, NO,
  43, NO,
  55, YES,
  1, YES,
  15, SNAKE, // delay 1.5 seconds, pop the snake
  0, END
};

/* The following example tracks are only here to show how to write
  byte example_track_2[] = {
  20, YES,
  20, NO,
  20, YES,
  20, NO,
  30, SNAKE,
  0, END
  };

  byte example_track_3[] = {
  30, YES,
  30, YES,
  30, YES,
  30, YES,
  10, YES,
  40, SNAKE,
  0, END
  };
*/

void setup() {

  Serial.begin(9600);
  Serial.println("OpenSnake Firmware Version 1.1");

  pinMode(4, OUTPUT); // buzzer low side (simon says kit)
  digitalWrite(4, LOW);   // note, other size fo buzzer is 7, and we will call that in tone, later.

  pinMode(RED_LED_pin, OUTPUT); // "NO LED"
  digitalWrite(RED_LED_pin, LOW);
  pinMode(GREEN_LED_pin, OUTPUT); // "YES LED"
  digitalWrite(GREEN_LED_pin, LOW);

  // buttons
  pinMode(RECORD_BUTTON, INPUT_PULLUP);
  pinMode(PLAY_BUTTON, INPUT_PULLUP);
  pinMode(TRACK_SELECT_BUTTON, INPUT_PULLUP);
  pinMode(YES_BUTTON, INPUT_PULLUP);
  pinMode(NO_BUTTON, INPUT_PULLUP);
  pinMode(SNAKE_BUTTON, INPUT_PULLUP);

  pinMode(SERVO_PWR_CONTROL_PIN, OUTPUT); //
  digitalWrite(SERVO_PWR_CONTROL_PIN, LOW);

  // check for fresh IC. If so, then do a factory reset
  if (EEPROM.read(EEPROM_LOCATION_AUTOPLAY) == 255) // fresh ICs have all of their EEPROM values at 255, so we only need to check one location
  {
    factory_reset();
    // to indicate that we've done a "1st time" factory reset.
    // aka this is no longer a fresh IC.
  }

  // check for manual factory reset - user must hold down "REC" button through a power cycle,
  // then continue to hold down for 3 seconds
  int time_pressed = 0;
  while ( (check_buttons() == true) && (userInput == RECORD_CMD) ) // user must hold down record for 3 seconds
  {
    blink_led(RED_LED_pin);
    delay(1000);
    time_pressed++;
    if (time_pressed == 3)
    {
      factory_reset();
      break;
    }
  }

  // check for autoplay toggle on/off - user must hold down "PLAY" button through a power cycle,
  // then continue to hold down for 3 seconds
  time_pressed = 0;
  while ( (check_buttons() == true) && (userInput == PLAY_CMD) ) // user must hold down play for 3 seconds
  {
    blink_led(GREEN_LED_pin);
    delay(1000);
    time_pressed++;
    if (time_pressed == 3)
    {
      toggle_autoplay_onoff();
      break;
    }
  }

  if (EEPROM.read(EEPROM_LOCATION_AUTOPLAY) == AUTOPLAY_ON) 
  {
    Serial.println("Autoplay on.");
    play_track();
    while (1);
  }
  
  byte track = EEPROM.read(EEPROM_LOCATION_TRACK);
  Serial.println("Autoplay off.");
  Serial.print("Current track ");
  Serial.println(track);
  blink_track(track);

  //print_EEPROM();

  //set_track(1);

  //play_track();

  //while (1);
}

void loop()
{
  if (check_buttons() == true)
  {
    switch (userInput) {
      case RECORD_CMD:
        if (recordButtonCounter > 200) record_track(); // require the user to hold it down for 2 seconds
        break;
      case PLAY_CMD:
        play_track();
        break;
      case TRACK_CMD:
        if (incrementTrackButtonCounter > 200) increment_track(); // require the user to hold it down for 2 seconds
        break;
      case SNAKE_CMD:
        if (snakeButtonCounter > 200) snake(); // requires the user to hold it down for 2 seconds
        break;
      default:
        Serial.print("Invalid userInput: ");
        Serial.println(userInput);
        break;
    }
  }
  delay(10); // button debounce
}

boolean play_track()
{
  byte track = EEPROM.read(EEPROM_LOCATION_TRACK);

  Serial.print("Playing track ");
  Serial.println(track);

  // First, find track length
  // scan through all memory locations for the track,
  // look for END event type, then store it in local
  // variable, use it later for playback for loop

  int start_mem_location = get_start_mem_location(track);

  Serial.print("start_mem_location: ");
  Serial.println(start_mem_location);

  // Play back the track
  byte event_type = 0; // yes, no, snake, end
  int event_delay = 0; // milliseconds 0-2550

  for (int pos = start_mem_location ; pos <= (start_mem_location + 99) ; pos += 2) // because each event has a timestamp and type, we increment 2
  {
    event_delay = int(EEPROM.read(pos));
    event_type = EEPROM.read(pos + 1); // the following memory location is type

    Serial.print("event_delay: ");
    Serial.println(event_delay);

    event_delay *= 100; // convert to milliseconds
    delay(event_delay);

    //Serial.print("event_type: ");
    //Serial.println(event_type);

    switch (event_type) {
      case YES:
        Serial.println("YES");
        yes();
        break;
      case NO:
        Serial.println("NO");
        no();
        break;
      case SNAKE:
        Serial.println("SNAKE");
        snake();
        break;
      case LAUGH:
        Serial.println("LAUGH");
        laugh();
        break;
      case CRY:
        Serial.println("CRY");
        cry();
        break;
      case HAPPY:
        Serial.println("HAPPY BIRTHDAY");
        happy_birthday();
        break;
      case END:
        Serial.println("END");
        break;
      default:
        Serial.println("Invalid event type!");
        break;
    }

    if (event_type == END) break;
  }

}


void yes()
{
  digitalWrite(GREEN_LED_pin, HIGH);
  digitalWrite(RED_LED_pin, LOW);
  for (int note = 150 ; note < 4000 ; note += 150)
  {
    tone(BUZZER_PIN, note, 40);
    delay(30);
  }
  noTone(BUZZER_PIN);
  digitalWrite(GREEN_LED_pin, LOW);
}

void no()
{
  digitalWrite(RED_LED_pin, HIGH);
  digitalWrite(GREEN_LED_pin, LOW);
  for (int note = 4000 ; note > 150 ; note -= 150)
  {
    tone(BUZZER_PIN, note, 30);
    delay(20);
  }
  noTone(BUZZER_PIN);
  digitalWrite(RED_LED_pin, LOW);
}

void snake()
{
  digitalWrite(SERVO_PWR_CONTROL_PIN, HIGH);   // turn on servo power

  myservo.attach(SERVO_PWM_PIN);  // attaches the servo on pin # to the servo object

  myservo.write(180); // move servo to open lid position

  // make siren sounds and blink red/green leds!!
  for (int i = 0 ; i <= 3 ; i++)
  {
    digitalWrite(RED_LED_pin, HIGH);
    digitalWrite(GREEN_LED_pin, LOW);
    for (int note = 150 ; note < 4000 ; note += 150)
    {
      tone(BUZZER_PIN, note, 30);
      delay(10);
    }
    digitalWrite(RED_LED_pin, LOW);
    digitalWrite(GREEN_LED_pin, HIGH);
    for (int note = 4000 ; note > 150 ; note -= 150)
    {
      tone(BUZZER_PIN, note, 30);
      delay(10);
    }
  }

  noTone(BUZZER_PIN);

  digitalWrite(RED_LED_pin, LOW);
  digitalWrite(GREEN_LED_pin, LOW);

  myservo.detach();  // detach() servo control pin

  digitalWrite(SERVO_PWR_CONTROL_PIN, LOW); // turn off servo power
}

void factory_reset()
{
  Serial.println("Factory Reset!!");

  // indicate a flashing of red/green (X3) to show user it's happening
  blink_led(RED_LED_pin);
  blink_led(GREEN_LED_pin);
  blink_led(RED_LED_pin);
  blink_led(GREEN_LED_pin);
  blink_led(RED_LED_pin);
  blink_led(GREEN_LED_pin);

  // store default track number in EEPROM location 0
  EEPROM.write(EEPROM_LOCATION_TRACK, 1);

  EEPROM.write(EEPROM_LOCATION_AUTOPLAY, AUTOPLAY_OFF); // turn autoplay off

  // store example tracks in track EEPROM locations

  // example track 1
  for (int i = 0 ; i < sizeof(example_track_1) ; i++)
  {
    EEPROM.write( 1 + i, example_track_1[i]); // starts at address 1
    if (example_track_1[i] == END) break;
  }

  //  // example track 2
  //  for (int i = 0 ; i < sizeof(example_track_2) ; i++)
  //  {
  //    EEPROM.write( 100 + i, example_track_2[i]); // starts at address 100
  //    if (example_track_2[i] == END) break;
  //  }
  //
  //  // example track 3
  //  for (int i = 0 ; i < sizeof(example_track_3) ; i++)
  //  {
  //    EEPROM.write( 200 + i, example_track_3[i]); // starts at address 200
  //    if (example_track_3[i] == END) break;
  //  }
  while (check_buttons() == true); // wait for release (aka debouce)
}

void print_EEPROM()
{
  // read a byte from the current address of the EEPROM
  int address = 0;
  int eeprom_length = EEPROM.length();
  for (int address = 0 ; address < eeprom_length ; address++)
  {
    Serial.print(address);
    Serial.print("\t");
    Serial.print(EEPROM.read(address), DEC);
    Serial.println();
  }
}

void set_track(byte track)
{
  EEPROM.write(EEPROM_LOCATION_TRACK, track); // store it in special EEPROM location for reading at play_track
  // and to save through a power cycle.
}

void increment_track()
{
  Serial.println("Incrementing track...");
  byte track = EEPROM.read(EEPROM_LOCATION_TRACK);
  track++;
  if (track == 6) track = 1; // loop back to track 1 (we only have EEPROM room for 5 total tracks)
  Serial.print("track: ");
  Serial.println(track);
  blink_track(track);
  set_track(track);
}

void blink_track(byte times)
{
  delay(1000);
  for (byte i = 0 ; i < times ; i++)
  {
    digitalWrite(RED_LED_pin, HIGH);
    delay(100);
    digitalWrite(RED_LED_pin, LOW);
    delay(500);
  }
}

void blink_led(byte pin)
{
  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
}

boolean check_buttons()
{
  userInput = 0;
  if (digitalRead(RECORD_BUTTON) == true) recordButtonCounter = 0; // user released record button, reset counter
  if (digitalRead(TRACK_SELECT_BUTTON) == true) incrementTrackButtonCounter = 0; // user released track sel button, reset counter
  if (digitalRead(SNAKE_BUTTON) == true) snakeButtonCounter = 0; // user released snake button, reset counter

  if (digitalRead(PLAY_BUTTON) == false) userInput = PLAY_CMD;
  else if (digitalRead(RECORD_BUTTON) == false) // RECORD (note, requires long press)
  {
    recordButtonCounter++;
    userInput = RECORD_CMD;
  }
  else if (digitalRead(TRACK_SELECT_BUTTON) == false) // increment track (note, requires long press)
  {
    incrementTrackButtonCounter++; // needed to know if it is gonna be a long press
    if (digitalRead(YES_BUTTON) == false) userInput = LAUGH_CMD; // "shift yes" (aka track_select + yes) = laugh
    else if (digitalRead(NO_BUTTON) == false) userInput = CRY_CMD; // "shift no" (aka track_select + no) = cry
    else if (digitalRead(SNAKE_BUTTON) == false) userInput = HAPPY_CMD; // "shift snake" (aka track_select + snake) = happy birthday
    else userInput = TRACK_CMD; // if no other buttons are held down a the same time, then it's just a plain old "track select command".
  }
  else if (digitalRead(YES_BUTTON) == false) userInput = YES_CMD;
  else if (digitalRead(NO_BUTTON) == false) userInput = NO_CMD;
  else if (digitalRead(SNAKE_BUTTON) == false)
  {
    snakeButtonCounter++;
    userInput = SNAKE_CMD;
  }
  if (userInput) return true;
  else return false;
}

void record_track()
{
  // record user "playing in" their track.
  // user will press yes, no and snake to create a "magic show"
  // we need to keep track of time (using millis()) and record event times and event types.
  // as the user "plays in" their track, we will record each event into EEPROM.
  // user hits 'record' button a second time to end the recording.

  blink_led(RED_LED_pin); // indicate we are about to record

  while (check_buttons() == true); // wait for release (aka debouce)

  byte track = EEPROM.read(EEPROM_LOCATION_TRACK);

  Serial.print("Recording track ");
  Serial.println(track);

  long last_press_time = millis(); // grab the current start time
  boolean recording_status = true; // used to know when we are recording, and when we are done recording.
  byte recording_length = 0; // used to keep track of how many events we've recorded, and where we should record the next event in EEPROM

  while (recording_status == true) // keep recording in each button press until we want to stop recording
  {
    // note, we may want to consider flashing (or glowing) the "talk LED" during the recording to indicate that we're recording.
    // I also think a chirp on the buzzer every second might be a good indicator that we're recording.
    digitalWrite(RED_LED_pin, HIGH);
    delay(10);
    digitalWrite(RED_LED_pin, LOW);
    delay(100);

    long event_delay_long = millis() - last_press_time; // grab event delay ***STILL need to truncate this to a byte format for 0-255 (00.0-25.5 second format)
    event_delay_long /= 100; // starts as milliseconds, so devide by 100 gets you to 00.0 second format
    if (event_delay_long > 255) event_delay_long = 255; // max out at 255, cause this is all an EEPROM spot can hold
    byte event_delay = byte(event_delay_long);

    Serial.print("event_delay");
    Serial.println(event_delay);

    if (check_buttons() == true) // if they press something, let's record it...
    {
      // check for valid userInput, we want to ignore if the user presses anything other than yes, no snake or record.
      digitalWrite(RED_LED_pin, HIGH);
      if (userInput == YES_CMD)
      {
        Serial.println("YES");
        record_event(track, event_delay, YES, recording_length); // record event delay and and event type into EEPROM
        recording_length++;
        yes();
      }
      else if (userInput == NO_CMD)
      {
        Serial.println("NO");
        record_event(track, event_delay, NO, recording_length); // record event delay and and event type into EEPROM
        recording_length++;
        no();
      }
      else if (userInput == SNAKE_CMD)
      {
        Serial.println("SNAKE");
        record_event(track, event_delay, SNAKE, recording_length); // record event delay and and event type into EEPROM
        recording_length++;
        snake();
      }
      else if (userInput == LAUGH_CMD)
      {
        Serial.println("LAUGH");
        record_event(track, event_delay, LAUGH, recording_length); // record event delay and and event type into EEPROM
        recording_length++;
        laugh();
      }
      else if (userInput == CRY_CMD)
      {
        Serial.println("CRY");
        record_event(track, event_delay, CRY, recording_length); // record event delay and and event type into EEPROM
        recording_length++;
        cry();
      }
      else if (userInput == HAPPY_CMD)
      {
        Serial.println("HAPPY BIRTHDAY");
        record_event(track, event_delay, HAPPY, recording_length); // record event delay and and event type into EEPROM
        recording_length++;
        happy_birthday();
      }
      else if (userInput == RECORD_CMD)
      {
        Serial.println("END");
        record_event(track, event_delay, END, recording_length); // record an "end command" to EEPROM
        recording_status = false; // user hit "record button" a second time, so stop recording.
      }
      //while (check_buttons() == true); // wait for release (aka debouce)
      //delay(10); // debounce
      if ((userInput == TRACK_CMD) || (userInput == PLAY_CMD)) ; // don't do any recording and don't reset last_press_time
      else last_press_time = millis(); // reset last_press_time so that we can no the next delay time, from this press (that just happened) to the next
      digitalWrite(RED_LED_pin, LOW);
    }
  }
}

void record_event(byte track, byte event_delay, byte event_type, byte recording_length)
{
  int start_mem_location = get_start_mem_location(track); // get the first event location of track (1, 100, 200, etc.)
  int offset = recording_length * 2; // because we record both event delay and event type, each "event" acutally takes up 2 EEPROM spots
  int record_to_location = start_mem_location + offset;

  EEPROM.write(record_to_location, event_delay);
  EEPROM.write(record_to_location + 1, event_type);
}

int get_start_mem_location(byte track)
{
  if (track == 1) return 1;
  if (track == 2) return 100;
  if (track == 3) return 200;
  if (track == 4) return 300;
  if (track == 5) return 400;
  if (track == 6) return 500;
}

/*
 * Janurary, 2021
 * Cry, Laugh, PlayTone Functions were adapted from Adafruit animal sounds
 * by Magician/hacker Jeff Haas. Thanks Jeff!!
 * 
 * https://learn.adafruit.com/adafruit-trinket-modded-stuffed-animal/animal-sounds
 * 
 * How these sound effects work
 * Example from cry(), below.
 * 
 * (i=500; i<700; i+=2)
 * This line, taken apart, means:
 * 
 * i=500 // Initial number (500) is the starting tone.
 *       // Lower numbers = higher tones
 * 
 * i+=2 // Decrease tone each time through the loop.  Replace with 3 to get faster effect.
 *      // Keep consistent in the different sections of the function for better effect.
 * 
 * i<700 // How many times through the loop (700 - 500 = 200 times).
 *       // Adjust difference to shorten or lengthen effect.
 *       // Use < to lower the sound as it plays, > to raise it.
 *       // Refer to cry() and laugh() functions.
*/
void cry() {  // Like a dog whining
  pinMode(BUZZER_PIN, OUTPUT);
  int crydelay = 500;

  uint16_t i;
  digitalWrite(GREEN_LED_pin, LOW);
  digitalWrite(RED_LED_pin, HIGH);
  for (i = 500; i < 700; i += 3) // vary "ooo" down
    playTone(i, 8);
  digitalWrite(RED_LED_pin, LOW);
  delay(crydelay);

  digitalWrite(RED_LED_pin, HIGH);
  for (i = 600; i < 800; i += 3) // vary "ooo" down
    playTone(i, 8);
  digitalWrite(RED_LED_pin, LOW);
  delay(crydelay);

  digitalWrite(RED_LED_pin, HIGH);
  for (i = 700; i < 950; i += 3) // vary "ooo" down
    playTone(i, 8);
  digitalWrite(RED_LED_pin, LOW);

}

void laugh() {  // Make this different than the cry - "Ha ha ha ha"
  pinMode(BUZZER_PIN, OUTPUT);
  int laughdelay = 200;
  uint16_t i;

  digitalWrite(RED_LED_pin, LOW);
  digitalWrite(GREEN_LED_pin, HIGH);
  for (i = 650; i > 525; i -= 3) // vary up
    playTone(i, 8);
  digitalWrite(GREEN_LED_pin, LOW);
  delay(laughdelay);

  digitalWrite(GREEN_LED_pin, HIGH);
  for (i = 800; i > 660; i -= 3) //
    playTone(i, 8);
  digitalWrite(GREEN_LED_pin, LOW);
  delay(laughdelay);

  digitalWrite(GREEN_LED_pin, HIGH);
  for (i = 900; i > 745; i -= 3) //
    playTone(i, 8);
  digitalWrite(GREEN_LED_pin, LOW);
  delay(laughdelay);

  digitalWrite(GREEN_LED_pin, HIGH);
  for (i = 990; i > 850; i -= 3) //
    playTone(i, 8);
  digitalWrite(GREEN_LED_pin, LOW);
}

// play tone on a piezo speaker: tone shorter values produce higher frequencies
//  which is opposite beep() but avoids some math delay - similar to code by Erin Robotgrrl

void playTone(uint16_t tone1, uint16_t duration) {
  if (tone1 < 50 || tone1 > 15000) return; // these do not play on a piezo
  for (long i = 0; i < duration * 1000L; i += tone1 * 2) {
    digitalWrite(BUZZER_PIN , HIGH);
    delayMicroseconds(tone1);
    digitalWrite(BUZZER_PIN , LOW);
    delayMicroseconds(tone1);
  }
}

/*
 * Janurary, 2021
 * Happy birthday
 * Plays happy birthday on the buzzer with random blinking LEDs
 * 
 * The following happy_brithday() function was adapted from JSHENG version on the arduino website here:
 * https://create.arduino.cc/projecthub/jsheng/happy-birthday-lights-and-sounds-1745cd
 * 
 * Includes melody[] noteDurations[], then a for loop to play the song.
 * Also creates a randome pattern of on/off with the red/green leds, but changes them on the beat
 * 
 * Note, this uses a header file called pitches.h, included at the top of this sketch
*/
void happy_birthday()
{
  //notes in the melody
  int melody[] = {
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4,
    NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4,
    NOTE_AS4, NOTE_AS4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4
  };

  //note durations: 4 = quarter note, 8 = eight note, etc.
  int noteDurations[] = {
    8, 8, 4, 4, 4, 2, 8, 8, 4, 4, 4, 2, 8, 8, 4, 4, 4, 4, 4, 8, 8, 4, 4, 4, 2,
  };

  for (int thisNote = 0 ; thisNote < 25 ; thisNote++) {
    // Using a random number, we will choose to turn on green, red, or both
    int randomChoice = random(0, 3);
    if (randomChoice == 0) digitalWrite (GREEN_LED_pin, HIGH);
    else if (randomChoice == 1) digitalWrite (RED_LED_pin, HIGH);
    else if (randomChoice == 2)
    {
      digitalWrite (GREEN_LED_pin, HIGH);
      digitalWrite (RED_LED_pin, HIGH);
    }

    int noteDuration = 1130 / noteDurations[thisNote];
    tone (BUZZER_PIN, melody[thisNote], noteDuration);

    int pause = noteDuration * 1.275;
    delay (pause);

    noTone(BUZZER_PIN);
    digitalWrite(GREEN_LED_pin, LOW);
    digitalWrite(RED_LED_pin, LOW);
  }
}


void toggle_autoplay_onoff()
{
  if (EEPROM.read(EEPROM_LOCATION_AUTOPLAY) == AUTOPLAY_ON) 
  {
    EEPROM.write(EEPROM_LOCATION_AUTOPLAY, AUTOPLAY_OFF);
    Serial.println("Autoplay off.");
  }
  else 
  {
    EEPROM.write(EEPROM_LOCATION_AUTOPLAY, AUTOPLAY_ON);
    Serial.println("Autoplay on.");
  }
}
