#include <QueueArray.h>

#include <Wire.h> // Include the I2C library (required)

// for the sound board
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

#include <Adafruit_NeoPixel.h>

// for led triggers
#define HIGH 0x1
#define LOW  0x0

// neopixel pins / setup
#define NEO_CENTER 2 // center rod
Adafruit_NeoPixel CenterLights = Adafruit_NeoPixel(7, NEO_CENTER, NEO_GRB + NEO_KHZ800);

// neopixel pins / setup
#define NEO_END 3 // end of barrel
Adafruit_NeoPixel EndLights = Adafruit_NeoPixel(7, NEO_END, NEO_GRB + NEO_KHZ800);

// neopixel pins / setup
#define NEO_INDICATOR 4 // top indicator
Adafruit_NeoPixel IndicatorLight = Adafruit_NeoPixel(1, NEO_INDICATOR, NEO_GRB + NEO_KHZ800);

// inputs for switches and buttons
const int SONGBTN_BUTTON = 13;
const int POWERSW_SWITCH = 5;
const int BLUEBTN_BUTTON = 6;
const int ORANGEBTN_BUTTON = 7;
const int CANCELBTN_BUTTON = 8;

// soundboard pins and setup
#define SFX_RST 9
#define SFX_RX 10
#define SFX_TX 11
const int ACT = 12;    // this allows us to know if the audio is playing

SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard( &ss, NULL, SFX_RST);

// Possible Pack states
bool StateBlue = false;      // on and blue
bool StateOrange = false;    // on and orange
bool StateNone = false;  // on but no portal
bool StateOff = true;    // powered down

// physical switch states
bool PowerSw = false;
bool BlueBtn = false;
bool OrangeBtn = false;
bool CancelBtn = false;
bool SongBtn = false;

// audio track names on soundboard
char powerup[] =              "T00     WAV";
char hum[] =                  "T01     WAV";
char bluefire[] =             "T02     WAV";
char orangefire[] =           "T03     WAV";
char cancelportal[] =         "T04     WAV";
char powerdown[] =            "T05     WAV";
char alivesong[] =            "T06     OGG";


// Arduino setup function
void setup() {
  // softwareserial at 9600 baud for the audio board
  ss.begin(9600);

  // set act modes for the fx board
  pinMode(ACT, INPUT);

  // configure center
  CenterLights.begin();
  CenterLights.setBrightness(100);
  CenterLights.show(); // Initialize all pixels to 'off'

  // configure end
  EndLights.begin();
  EndLights.setBrightness(75);
  EndLights.show(); // Initialize all pixels to 'off'

  // configure indicator
  IndicatorLight.begin();
  IndicatorLight.setBrightness(75);
  IndicatorLight.show();

  // set the modes for the switches/buttons
  pinMode(SONGBTN_BUTTON, INPUT);
  digitalWrite(SONGBTN_BUTTON, HIGH);
  pinMode(POWERSW_SWITCH, INPUT);
  digitalWrite(POWERSW_SWITCH, HIGH);
  pinMode(BLUEBTN_BUTTON, INPUT);
  digitalWrite(BLUEBTN_BUTTON, HIGH);
  pinMode(ORANGEBTN_BUTTON, INPUT);
  digitalWrite(ORANGEBTN_BUTTON, HIGH);
  pinMode(CANCELBTN_BUTTON, INPUT);
  digitalWrite(CANCELBTN_BUTTON, HIGH);
}

/* ************* Audio Board Helper Functions ************* */
// helper function to play a track by name on the audio board
void playAudio( char* trackname, int playing ) {
  // stop track if one is going
  if (playing == 0) {
    sfx.stop();
  }

  // now go play
  if (sfx.playTrack(trackname)) {
    sfx.unpause();
  }
}


/* ************* Main Loop ************* */

void loop() {
  // find out of the audio board is playing audio
  int playing = digitalRead(ACT);

  // get the current switch states
  int SongBtn_button = digitalRead(SONGBTN_BUTTON);

  // if you press song button play song
  if (SongBtn_button == 1) {
    if (SongBtn == false) {
      playAudio(alivesong, playing);
      SongBtn = true;
    }
  } else {
    SongBtn = false;
  }

  int PowerSw_switch = digitalRead(POWERSW_SWITCH);
  int BlueBtn_button = digitalRead(BLUEBTN_BUTTON);
  int OrangeBtn_button = digitalRead(ORANGEBTN_BUTTON);
  int CancelBtn_button = digitalRead(CANCELBTN_BUTTON);

  // while the startup switch is set on
  if (PowerSw_switch == 1) {
    // in general we always try to play the idle sound if started
    if (playing == 1 && PowerSw == true) {
      playAudio(hum, playing);
    }
  }

      // if we are not started up we should play the startup sound and begin the boot sequence
    if (PowerSw == false) {
      PowerSw = true;
      playAudio(powerup, playing);
 }
}
