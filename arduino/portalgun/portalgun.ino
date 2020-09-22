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
int BtnLeds = A1;

// neopixel pins / setup
#define NEO_INDICATOR 2 // top indicator
Adafruit_NeoPixel IndLight = Adafruit_NeoPixel(1, NEO_INDICATOR, NEO_GRB + NEO_KHZ800);

#define NEO_CENTER 3 // center rod
Adafruit_NeoPixel CenterLights = Adafruit_NeoPixel(7, NEO_CENTER, NEO_GRB + NEO_KHZ800);

#define NEO_END 4 // end of barrel
Adafruit_NeoPixel EndLights = Adafruit_NeoPixel(7, NEO_END, NEO_GRB + NEO_KHZ800);

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
  pinMode(BtnLeds, OUTPUT);

  // configure indicator light
  IndLight.begin();
  IndLight.setBrightness(255);
  IndLight.show(); // Initialize all pixels to 'off'

  // configure center lights
  CenterLights.begin();
  CenterLights.setBrightness(255);
  CenterLights.show(); // Initialize all pixels to 'off'

  // configure end lights
  EndLights.begin();
  EndLights.setBrightness(255);
  EndLights.show(); // Initialize all pixels to 'off'

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
    analogWrite(BtnLeds, 200); // turn button leds on
    playAudio(powerup, playing);
  }

  // if we are powered up and turn the power switch off then power down
  if (PowerSw == 1 && Power == true) {
    Power = false;
    playAudio(powerdown, playing);
    analogWrite(Leds, 0); //turn 3 red leds off
    analogWrite(BtnLeds, 0); // turn leds in buttons off
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
        for (int j = 0; j < CenterLights.numPixels(); j++)
        {
          CenterLights.setPixelColor(j, CenterLights.Color(d * 5, d, 0)); //i get orange to dim properly by setting green to a number and setting red to 5 times what green is
        }
        {
          for (int k = 0; k < EndLights.numPixels(); k++)
          {
            EndLights.setPixelColor(k, EndLights.Color(b * 5, b, 0));
          }
          CenterLights.show();
          EndLights.show();
          delay(0);
        }
      }
      for (int d = 0, b = 50; (d < 25) && (b > 25); d++, b--) {
        for (int j = 0; j < CenterLights.numPixels(); j++) {
          CenterLights.setPixelColor(j, CenterLights.Color(d * 5, d, 0));
        }

        {
          for (int k = 0; k < EndLights.numPixels(); k++) {
            EndLights.setPixelColor(k, EndLights.Color(b * 5, b, 0));
          }

          IndLight.setPixelColor(0, IndLight.Color(255, 50, 0));
          IndLight.show();
          EndLights.show();
          CenterLights.show();
          delay(20);
        }
      }
      break;

    case 1: // blue fire

      for (int d = 128, b = 128; (d > 0) && (b < 255); d--, b++)// same as orange above, except you just have to set blue between half and full
      {
        for (int j = 0; j < CenterLights.numPixels(); j++)
        {
          CenterLights.setPixelColor(j, CenterLights.Color(0, 0, d));
        }
        {
          for (int k = 0; k < EndLights.numPixels(); k++)
          {
            EndLights.setPixelColor(k, EndLights.Color(0, 0, b));
          }
          IndLight.setPixelColor(1, 255, 50, 0);
          CenterLights.show();
          EndLights.show();
          delay(0);
        }
      }
      for (int d = 0, b = 255; (d < 128) && (b > 128); d++, b--) {
        for (int j = 0; j < CenterLights.numPixels(); j++) {
          CenterLights.setPixelColor(j, CenterLights.Color(0, 0, d));
        }

        {
          for (int k = 0; k < EndLights.numPixels(); k++) {
            EndLights.setPixelColor(k, EndLights.Color(0, 0, b));
          }

          IndLight.setPixelColor(0, IndLight.Color(0, 0, 255));
          IndLight.show();
          EndLights.show();
          CenterLights.show();
          delay(4);
        }
      }
      break;


    case 2: // set led off
      for (int j = 0; j < CenterLights.numPixels(); j++) {
        CenterLights.setPixelColor(j, 0);
        EndLights.setPixelColor(j, 0);
        IndLight.setPixelColor(0, 0);
      }
      break;
  }
  IndLight.show();
  CenterLights.show();
  EndLights.show();
}
