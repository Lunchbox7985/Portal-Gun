#include <QueueArray.h>

#include <Wire.h> // Include the I2C library (required)

// for the sound board
#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"

#include <Adafruit_NeoPixel.h>

// for led triggers
#define HIGH 0x1
#define LOW  0x0

// pins for 3 red leds and lights in buttons
int Leds = A0;
int BbtnLed = A1;
int ObtnLed = A2;

// neopixel pins / setup
#define NEO_PIXELS 2
Adafruit_NeoPixel NeoPixels = Adafruit_NeoPixel(31, NEO_PIXELS, NEO_GRB + NEO_KHZ800);

// Index numbers for the LEDs in the neopixels. since i used 2 jewels for the center, one ring for the end, and a single for the indicator, 
//i set them as below. if you use a different number or type of pixels, change these numbers accordingly. the indicator light is first in the chain
//so it is number 0
int centerStart = 1;
int centerEnd = 14;
int ringStart = 15;
int ringEnd = 30;

// inputs for switches and buttons
const int SONGBTN = 5;
const int POWERSW = 6;
const int BLUEBTN = 7;
const int ORANGEBTN = 8;
const int CANCELBTN = 9;

// soundboard pins and setup
#define SFX_RST 10
#define SFX_RX 11
#define SFX_TX 12
const int ACT = 13;    // this allows us to know if the audio is playing

SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard( &ss, NULL, SFX_RST);

// Possible states
bool Firing = false;    // firing animation is going
bool Portal = false;  // is there an open portal
bool Power = false; // power on or off
bool Song = false; //playing the song

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
  pinMode(Leds, OUTPUT);
  pinMode(BbtnLed, OUTPUT);
  pinMode(ObtnLed, OUTPUT);

  // configure neo pixels
  NeoPixels.begin();
  NeoPixels.setBrightness(255);
  NeoPixels.show(); // Initialize all pixels to 'off'

  // set the modes for the switches/buttons
  pinMode(SONGBTN, INPUT);
  digitalWrite(SONGBTN, HIGH);
  pinMode(POWERSW, INPUT);
  digitalWrite(POWERSW, HIGH);
  pinMode(BLUEBTN, INPUT);
  digitalWrite(BLUEBTN, HIGH);
  pinMode(ORANGEBTN, INPUT);
  digitalWrite(ORANGEBTN, HIGH);
  pinMode(CANCELBTN, INPUT);
  digitalWrite(CANCELBTN, HIGH);
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
  int SongBtn = digitalRead(SONGBTN);

  // if you press song button play song
  if (SongBtn == 0) {
    if (Song == false) {
      playAudio(alivesong, playing);
      Song = true;
    }
  } else {
    Song = false;
  }

  int PowerSw = digitalRead(POWERSW);
  int BlueBtn = digitalRead(BLUEBTN);
  int OrangeBtn = digitalRead(ORANGEBTN);
  int CancelBtn = digitalRead(CANCELBTN);

  // while the startup switch is set on
  if (PowerSw == 0) {
    // play the hum sound when on and idle
    if (playing == 1 && Power == true) {
      playAudio(hum, playing);
    }
  }

  // if we are not powered up we should play the power up sound and begin the loop
  if (PowerSw == 0 && Power == false) {
    Power = true;
    analogWrite(Leds, 175); //turn 3 red leds on
    analogWrite(BbtnLed, 200); // turn blue button led on
    analogWrite(ObtnLed, 200); // turn orange button led on
    playAudio(powerup, playing);
  }

  // if we are powered up and turn the power switch off then power down
  if (PowerSw == 1 && Power == true) {
    Power = false;
    playAudio(powerdown, playing);
    analogWrite(Leds, 0); //turn 3 red leds off
    analogWrite(BbtnLed, 0); // turn blue button led off
    analogWrite(ObtnLed, 0); // turn orange button led off
    setLightsState(2); //set blue or orange light to off
  }

  // Fire Blue
  if (BlueBtn == 0 && Power == true && Firing == false) {
    Firing = true;
    Portal = true;
    playAudio(bluefire, playing);
    setLightsState(1); //set light to blue

  } else {
    if (CancelBtn == 1 && BlueBtn == 1 && OrangeBtn == 1 && Firing == true)
      Firing = false;
  }

  // Fire Orange
  if (OrangeBtn == 0 && Power == true && Firing == false) {
    Firing = true;
    Portal = true;
    playAudio(orangefire, playing);
    setLightsState(0); //set lights to orange

  } else {
    if (CancelBtn == 1 && BlueBtn == 1 && OrangeBtn == 1 && Firing == true)
      Firing = false;
  }


  // Cancel Portal
  if (CancelBtn == 0 && Power == true && Firing == false && Portal == true) {
    Firing = true;
    Portal = false;
    playAudio(cancelportal, playing);
    setLightsState(2); //set light to off

  } else {
    if (CancelBtn == 1 && BlueBtn == 1 && OrangeBtn == 1 && Firing == true)
      Firing = false;
  }
}


/* ************* Lights States************* */
void setLightsState(int state)
{
  switch ( state )
  {

    case 0: // orange fire
      for (int d = 25, b = 25; (d > 0) && (b < 50); d--, b++) //starts at 50 percent, center light dims then brightens, end light brightens then dims
      {
        for (int j = centerStart; j <= centerEnd; j++)
        {
          NeoPixels.setPixelColor(j, NeoPixels.Color(d * 5, d, 0)); //i get orange to dim properly by setting green to a number and setting red to 5 times what green is
        }
        {
          for (int k = ringStart; k <= ringEnd; k++)
          {
            NeoPixels.setPixelColor(k, NeoPixels.Color(b * 5, b, 0));
          }
          NeoPixels.show();
          delay(0);
        }
      }
      for (int d = 0, b = 50; (d < 25) && (b > 25); d++, b--) {
        for (int j = centerStart; j <= centerEnd; j++) {
          NeoPixels.setPixelColor(j, NeoPixels.Color(d * 5, d, 0));
        }

        {
          for (int k = ringStart; k <= ringEnd; k++) {
            NeoPixels.setPixelColor(k, NeoPixels.Color(b * 5, b, 0));
          }

          NeoPixels.setPixelColor(0, NeoPixels.Color(255, 50, 0));
          NeoPixels.show();
          delay(20);
        }
      }
      break;

    case 1: // blue fire

      for (int d = 128, b = 128; (d > 0) && (b < 255); d--, b++)// same as orange above, except you just have to set blue between half and full
      {
        for (int j = centerStart; j <= centerEnd; j++)
        {
          NeoPixels.setPixelColor(j, NeoPixels.Color(0, 0, d));
        }
        {
          for (int k = ringStart; k <= ringEnd; k++)
          {
            NeoPixels.setPixelColor(k, NeoPixels.Color(0, 0, b));
          }
          NeoPixels.setPixelColor(0, 0, 0, 255);
          NeoPixels.show();
          delay(0);
        }
      }
      for (int d = 0, b = 255; (d < 128) && (b > 128); d++, b--) {
        for (int j = centerStart; j <= centerEnd; j++) {
          NeoPixels.setPixelColor(j, NeoPixels.Color(0, 0, d));
        }

        {
          for (int k = ringStart; k <= ringEnd; k++) {
            NeoPixels.setPixelColor(k, NeoPixels.Color(0, 0, b));
          }

          NeoPixels.setPixelColor(0, NeoPixels.Color(0, 0, 255));
          NeoPixels.show();
          delay(4);
        }
      }
      break;


    case 2: // set led off
      for (int j = 0; j < NeoPixels.numPixels(); j++) {
        NeoPixels.setPixelColor(j, 0);
      }
      break;
  }
  NeoPixels.show();
}
