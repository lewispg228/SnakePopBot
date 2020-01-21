/*
  Snake In A Can Controller
  Pete Lewis
  Soup Can Tech LLC
  In collaboration with Mario the Maker Magician

   EEPROM plan
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

    Mario's original example tracks will be stored in tracks 1,2, and 3.
    They will also be "backed up" as variable arrays in this sketch.

    USER INTERFACE //////////////////////////////////////////////////////////

    To play a track:
    Turn on power.
    User sees current track number blink on LED.
    Press PLAY to play track.

    To select a different track:
    Turn on power.
    User sees current track number blink on LED.
    Prest TRACK SELECT to change tracks.
      -When you press this button, it will increment the current track
      -It will also "loop back around" to track 1, when you reach the end and press it again.
      -Each time you press it, you will see the current track number blink on the LED

    To Record a new track:
    Note, you can record over the current track, or you can increment to an empty track.
    Select desired track you wish to record on (or over).
    Press REC to begin recording.
    Press "YES", "NO", and "SNAKE" as desired.
      -It is recommended that you write out your skit, and then just read the lines as you record
    Press REC to stop recording.
    Now your new track is avaialable for playback.

    FACTORY RESET
    If you do not want to keep your current tracks,
    and would like to restore the original Mario example tracks,
    Hold Down track select, turn unit OFF, then ON (while continuing to hold down TRACK SELECT).
    Listen for 3 beeps.
    Release button.
    Now your controller has been set back to it's orginal tracks and your other tracks have been cleared.


*/

//#define POP 40
//#include <Servo.h>
//Servo myservo;  // create servo object to control a servo
#include <EEPROM.h>

// Event types
#define YES 1
#define NO 2
#define SNAKE 3
#define END 99 // had to choose something, so went with this, wanted to leave room for other future event types

// example tracks
// they are a sequence of events
// they are formatted like so:
// delay, type, delay, type, and so on... (max 50 events)
// delay values are in 2.1 format, (so 10 = 1.0 second, and 105 = 10.5 seconds)
// this gives you 1/10th of a second precision
// and the max delay is 25.5 seconds (EEPROM stores byte, so values of 0-255).
// all tracks must have a last event delay of 0, followed by an event type END

byte example_track_1[] = {
  10, YES, // first event, delay 1.0 second, sound "yes" 
  15, NO, // second event, delay 1.5 seconds, sound "no"
  10, YES, 
  20, NO, 
  20, SNAKE, // delay 2.0 seconds, pop the snake
  0, END};
  
byte example_track_2[] = {
  20, NO, 
  20, NO, 
  20, NO, 
  20, NO, 
  30, SNAKE, 
  0, END};
  
byte example_track_3[] = {
  30, YES, 
  30, YES, 
  30, YES, 
  30, YES, 
  10, YES, 
  40, SNAKE, 
  0, END};

void setup() {

  Serial.begin(57600);
  pinMode(4, OUTPUT); // buzzer low side (simon says kit)
  digitalWrite(4, LOW);   // note, other size fo buzzer is 7, and we will call that in tone, later.

  // check for fresh IC. If so, then do a factory reset
  if (EEPROM.read(1023) == 255) factory_reset();

  //print_EEPROM();

  set_track(1);

  play_track();

  while (1);
}

void loop()
{

}

boolean play_track()
{
  byte track = EEPROM.read(0);

  Serial.print("Playing track ");
  Serial.println(track);

  // First, find track length
  // scan through all memory locations for the track,
  // look for END event type, then store it in local
  // variable, use it later for playback for loop

  int start_mem_location;
  if (track == 1) start_mem_location = 1;
  if (track == 2) start_mem_location = 100;
  if (track == 3) start_mem_location = 200;
  if (track == 4) start_mem_location = 300;
  if (track == 5) start_mem_location = 400;
  if (track == 6) start_mem_location = 500;

  // Play back the track
  byte event_type = 0; // yes, no, snake, end
  int event_delay = 0; // milliseconds 0-2550

  for (byte pos = start_mem_location ; pos <= (start_mem_location + 99) ; pos += 2) // because each event has a timestamp and type, we increment 2
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
  for (int note = 150 ; note < 4000 ; note += 150)
  {
    tone(7, note, 40);
    delay(30);
  }
  noTone(7);
}

void no()
{
  for (int note = 4000 ; note > 150 ; note -= 150)
  {
    tone(7, note, 30);
    delay(20);
  }
  noTone(7);
}

void snake()
{
  // turn on servo power
  // move servo to open lid position
  // detach() servo control pin
  // turn off servo power
}

void factory_reset()
{
  Serial.println("Factory Reset!!");

  // store default track number in EEPROM location 0
  EEPROM.write(0, 1);

  // store example tracks in track EEPROM locations

  // example track 1
  for (int i = 0 ; i < sizeof(example_track_1) ; i++)
  {
    EEPROM.write( 1 + i, example_track_1[i]); // starts at address 1
    if (example_track_1[i] == END) break;
  }

  // example track 2
  for (int i = 0 ; i < sizeof(example_track_2) ; i++)
  {
    EEPROM.write( 100 + i, example_track_2[i]); // starts at address 100
    if (example_track_2[i] == END) break;
  }

  // example track 3
  for (int i = 0 ; i < sizeof(example_track_3) ; i++)
  {
    EEPROM.write( 200 + i, example_track_3[i]); // starts at address 200
    if (example_track_3[i] == END) break;
  }
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
  EEPROM.write(0, track); // store it in special EEPROM location for reading at play_track
  // and to save through a power cycle.
}
