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
const int SONG_BUTTON = 13;
const int STARTUP_SWITCH = 5;
const int BLUE_BUTTON = 6;
const int ORANGE_BUTTON = 7;
const int CANCEL_BUTTON = 8;

// soundboard pins and setup
#define SFX_RST 9
#define SFX_RX 10
#define SFX_TX 11
const int ACT = 12;    // this allows us to know if the audio is playing

SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard( &ss, NULL, SFX_RST);

// Possible Pack states
bool bluelight = false;      // on and blue
bool orangelight = false;    // on and orange
bool nolight = false;  // on but no portal
bool poweredDown = true;    // powered down

// physical switch states
bool power = false;
bool blue = false;
bool orange = false;
bool cancel = false;
bool song = false;

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
  pinMode(SONG_BUTTON, INPUT);
  digitalWrite(SONG_BUTTON, HIGH);
  pinMode(STARTUP_SWITCH, INPUT);
  digitalWrite(STARTUP_SWITCH, HIGH);
  pinMode(BLUE_BUTTON, INPUT);
  digitalWrite(BLUE_BUTTON, HIGH);
  pinMode(ORANGE_BUTTON, INPUT);
  digitalWrite(ORANGE_BUTTON, HIGH);
  pinMode(CANCEL_BUTTON, INPUT);
  digitalWrite(CANCEL_BUTTON, HIGH);
  
 
  // set everything off initially
  shutdown_leds();
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
  int song_button = digitalRead(SONG_BUTTON);

  // if you press song button play song
  if (song_button == 0) {
    if (song == false) {
      playAudio(alivesong, playing);
      song = true;
    }
  } else {
    song = false;
  }

  int startup_switch = digitalRead(STARTUP_SWITCH);
  int blue_button = digitalRead(BLUE_BUTTON);
  int orange_button = digitalRead(ORANGE_BUTTON);
  int cancel_button = digitalRead(CANCEL_BUTTON);

  // while the startup switch is set on
  if (startup_switch == 1) {
    // in general we always try to play the idle sound if started
    if (playing == 1 && startup == true) {
      playAudio(hum, playing);
    }

    // choose the right powercell animation sequence for booted/on
    if ( powerBooted == true ) {
      // standard idle power sequence for the pack
      poweredDown = false;
      shuttingDown = false;
      venting = false;
      setWandLightState(3, 0, 0); //set sloblow red
      setVentLightState(ventStart, ventEnd, 2);
      powerSequenceOne(currentMillis, pwr_interval, cyc_interval, cyc_fade_interval);
    } else {
      // boot up the pack. powerSequenceBoot will set powerBooted when complete
      powerSequenceBoot(currentMillis);
      setWandLightState(3, 7, currentMillis);   //set sloblow red blinking
    }

    // if we are not started up we should play the startup sound and begin the boot sequence
    if (startup == false) {
      startup = true;
      playAudio(startupTrack, playing);

      // get the current safety switch state
      if (safety_switch == 1 && safety == false) {
        safety = true;
      }
    }

    if ( startup == true && safety_switch == 1 ) {
      if ( venting == false && powerBooted == true ) {
        setWandLightState(1, 2, 0);    //  set back light orange
        setWandLightState(2, 1, 0);    //  set body led white
      } else {
        setWandLightState(1, 4, 0);    //  set back light off
        setWandLightState(2, 4, 0);    //  set body led off
      }

      // if the safety switch is set off then we can fire when the button is pressed
      if ( fire_button == 0) {
        // if the button is just pressed we clear all led's to start the firing animations
        if ( isFiring == false ) {
          shutdown_leds();
          isFiring = true;
        }

        // show the firing bargraph sequence
        barGraphSequenceTwo(currentMillis);

        // strobe the nose pixels
        fireStrobe(currentMillis);

        // now powercell/cyclotron/wand lights
        // if this is the first time reset some variables and play the blast track
        if (fire == false) {
          shouldWarn = false;
          fire = true;
          firingStateMillis = millis();
          playAudio(blastTrack, playing);
        } else {
          // find out what our timing is
          unsigned long diff = (unsigned long)(millis() - firingStateMillis);

          if ( diff > firingWarnWaitTime) { // if we are in the fire warn interval
            pwr_interval = 10;      // speed up the powercell animation
            firing_interval = 20;   // speed up the bar graph animation
            cyc_interval = 50;      // really speed up cyclotron
            cyc_fade_interval = 5;  // speed up the fade of the cyclotron
            if (playing == 1 || shouldWarn == false ) {
              shouldWarn = true;
              playAudio(warnTrack, playing); // play the firing track with the warning
            }
            setWandLightState(0, 8, currentMillis);    // set top light red flashing fast
          } else if ( diff > firingWarmWaitTime) { // if we are in the dialog playing interval
            pwr_interval = 30;      // speed up the powercell animation
            firing_interval = 30;   // speed up the bar graph animation
            cyc_interval = 200;     // speed up cyclotron
            cyc_fade_interval = 10; // speed up the fade of the cyclotron
            if (playing == 1) {
              playAudio(blastTrack, playing); // play the normal blast track
            }
            setWandLightState(0, 6, currentMillis);    // set top light orange flashing
          }
        }
      } else { // if we were firing and are no longer reset the leds
        if ( isFiring == true ) {
          shutdown_leds();
          isFiring = false;
        }

        // and do the standard bargraph sequence
        barGraphSequenceOne(currentMillis);

        if (fire == true) { // if we were firing let's reset the animations and play the correct final firing track
          clearFireStrobe();
          setWandLightState(0, 4, currentMillis);    // set top light off

          pwr_interval = 60;
          firing_interval = 40;
          cyc_interval = 1000;
          cyc_fade_interval = 15;
          fire = false;

          // see if we've been firing long enough to get the dialog or vent sounds
          unsigned long diff = (unsigned long)(millis() - firingStateMillis);

          if ( diff > firingWarnWaitTime) { // if we are past the warning let's vent the pack
            playAudio(ventTrack, playing);
            venting = true;
            clearPowerStrip(); // play the boot animation on the powercell
          } else if ( diff > firingWarmWaitTime) { // if in the dialog time play the dialog in sequence
            if ( useDialogTracks == true ) {
              playDialogTrack(playing);
            } else {
              playAudio(endTrack, playing);
            }
          } else {
            playAudio(endTrack, playing);
          }
        }
      }

      // if the safety was just changed play the click track
      if (safety == false) {
        safety = true;
        playAudio(chargeTrack, playing);
      }
    } else {
      // if the safety is switched off play the click track
      if (safety == true) {
        setWandLightState(1, 4, 0);    // set back light off
        setWandLightState(2, 4, 0);    // set body off
        safety = false;
        playAudio(clickTrack, playing);
      }
    }
  } else { // if we are powering down
    if ( poweredDown == false ) {
      if ( shuttingDown == false ) {
        playAudio(shutdownTrack, playing); // play the pack shutdown track
        shuttingDown = true;
      }
      cyclotronRunningFadeOut = 255;
      powerSequenceShutdown(currentMillis);
    } else {
      if (startup == true) { // if started reset the variables
        clearPowerStrip(); // clear all led's
        shutdown_leds();
        startup = false;
        safety = false;
        fire = false;
      }
    }
  }
  delay(1);
}

/*************** Wand Light Helpers *********************/
unsigned long prevFlashMillis = 0; // last time we changed a powercell light in the idle sequence
bool flashState = false;
const unsigned long wandFastFlashInterval = 100; // interval at which we flash the top led on the wand
const unsigned long wandMediumFlashInterval = 500; // interval at which we flash the top led on the wand

void setWandLightState(int lednum, int state, unsigned long currentMillis) {
  switch ( state ) {
    case 0: // set led red
      wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
      break;
    case 1: // set led white
      wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
      break;
    case 2: // set led orange
      wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));
      break;
    case 3: // set led blue
      wandLights.setPixelColor(lednum, wandLights.Color(0, 0, 255));
      break;
    case 4: // set led off
      wandLights.setPixelColor(lednum, 0);
      break;
    case 5: // fast white flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandFastFlashInterval) {
        prevFlashMillis = currentMillis;
        if ( flashState == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 255, 255));
          flashState = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState = false;
        }
      }
      break;
    case 6: // slower orange flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandMediumFlashInterval) {
        prevFlashMillis = currentMillis;
        if ( flashState == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 127, 0));
          flashState = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState = false;
        }
      }
      break;
    case 7: // medium red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandMediumFlashInterval) {
        prevFlashMillis = currentMillis;
        if ( flashState == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState = false;
        }
      }
      break;
    case 8: // fast red flashing
      if ((unsigned long)(currentMillis - prevFlashMillis) >= wandFastFlashInterval) {
        prevFlashMillis = currentMillis;
        if ( flashState == false ) {
          wandLights.setPixelColor(lednum, wandLights.Color(255, 0, 0));
          flashState = true;
        } else {
          wandLights.setPixelColor(lednum, 0);
          flashState = false;
        }
      }
      break;
  }

  wandLights.show();
}

/*************** Powercell/Cyclotron Animations *********************/

// LED tracking variables
const int powerSeqTotal = powercellLedCount;                              // total number of led's for powercell 0 based
int powerSeqNum = powercellIndexOffset;                                   // current running powercell sequence led
int powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;       // shutdown sequence counts down

// animation level trackers for the boot and shutdown
int currentBootLevel = powercellIndexOffset;                              // current powercell boot level sequence led
int currentLightLevel = powercellLedCount - powercellIndexOffset;         // current powercell boot light sequence led

void setCyclotronLightState(int startLed, int endLed, int state ) {
  switch ( state ) {
    case 0: // set all leds to red
      for (int i = startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, powerStick.Color(255, 0, 0));
      }
      break;
    case 1: // set all leds to orange
      for (int i = startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, powerStick.Color(255, 106, 0));
      }
      break;
    case 2: // set all leds off
      for (int i = startLed; i <= endLed; i++) {
        powerStick.setPixelColor(i, 0);
      }
      break;
    case 3: // fade all leds from red
      for (int i = startLed; i <= endLed; i++) {
        if ( cyclotronRunningFadeOut >= 0 ) {
          powerStick.setPixelColor(i, 255 * cyclotronRunningFadeOut / 255, 0, 0);
          cyclotronRunningFadeOut--;
        } else {
          powerStick.setPixelColor(i, 0);
        }
      }
      break;
    case 4: // fade all leds to red
      for (int i = startLed; i <= endLed; i++) {
        if ( cyclotronRunningFadeIn < 255 ) {
          powerStick.setPixelColor(i, 255 * cyclotronRunningFadeIn / 255, 0, 0);
          cyclotronRunningFadeIn++;
        } else {
          powerStick.setPixelColor(i, powerStick.Color(255, 0, 0));
        }
      }
      break;
  }
}

// shuts off and resets the powercell/cyclotron leds
void clearPowerStrip() {
  // reset vars
  powerBooted = false;
  poweredDown = true;
  powerSeqNum = powercellIndexOffset;
  powerShutdownSeqNum = powercellLedCount - powercellIndexOffset;
  currentLightLevel = powercellLedCount;
  currentBootLevel = powercellIndexOffset;
  cyclotronRunningFadeIn = 0;

  // shutoff the leds
  for ( int i = 0; i <= c4End; i++) {
    powerStick.setPixelColor(i, 0);
  }
  powerStick.show();

  for ( int j = 0; j <= 3; j++ ) {
    wandLights.setPixelColor(j, 0);
  }
  wandLights.show();

  if ( venting == true ) {
    setVentLightState(ventStart, ventEnd, 0);
  }
}

// boot animation on the powercell/cyclotron
bool reverseBootCyclotron = false;
void powerSequenceBoot(unsigned long currentMillis) {
  bool doUpdate = false;

  // START CYCLOTRON
  if ( useCyclotronFadeInEffect == false ) {
    if ((unsigned long)(currentMillis - prevCycBootMillis) >= cyc_boot_interval) {
      prevCycBootMillis = currentMillis;

      if ( reverseBootCyclotron == false ) {
        setCyclotronLightState(c1Start, c1End, 1);
        setCyclotronLightState(c2Start, c2End, 2);
        setCyclotronLightState(c3Start, c3End, 1);
        setCyclotronLightState(c4Start, c4End, 2);

        doUpdate = true;
        reverseBootCyclotron = true;
      } else {
        setCyclotronLightState(c1Start, c1End, 2);
        setCyclotronLightState(c2Start, c2End, 1);
        setCyclotronLightState(c3Start, c3End, 2);
        setCyclotronLightState(c4Start, c4End, 1);

        doUpdate = true;
        reverseBootCyclotron = false;
      }
    }
  } else {
    if ((unsigned long)(currentMillis - prevCycBootMillis) >= cyc_boot_alt_interval) {
      prevCycBootMillis = currentMillis;
      setCyclotronLightState(c1Start, c4End, 4);
      doUpdate = true;
    }
  }
  // END CYCLOTRON

  if ((unsigned long)(currentMillis - prevPwrBootMillis) >= pwr_boot_interval) {
    // save the last time you blinked the LED
    prevPwrBootMillis = currentMillis;

    // START POWERCELL
    if ( currentBootLevel != powerSeqTotal ) {
      if ( currentBootLevel == currentLightLevel) {
        if (currentLightLevel + 1 <= powerSeqTotal) {
          powerStick.setPixelColor(currentLightLevel + 1, 0);
        }
        powerStick.setPixelColor(currentBootLevel, powerStick.Color(0, 0, 255));
        currentLightLevel = powerSeqTotal;
        currentBootLevel++;
      } else {
        if (currentLightLevel + 1 <= powerSeqTotal) {
          powerStick.setPixelColor(currentLightLevel + 1, 0);
        }
        powerStick.setPixelColor(currentLightLevel, powerStick.Color(0, 0, 255));
        currentLightLevel--;
      }
      doUpdate = true;
    } else {
      powerBooted = true;
      currentBootLevel = powercellIndexOffset;
      currentLightLevel = powercellLedCount - powercellIndexOffset;
    }
    // END POWERCELL
  }

  // if we have changed an led
  if ( doUpdate == true ) {
    powerStick.show(); // commit all of the changes
  }
}

// idle/firing animation for the powercell/cyclotron
int cycOrder = 0;     // which cyclotron led will be lit next
int cycFading = -1;   // which cyclotron led is fading out for game style
void powerSequenceOne(unsigned long currentMillis, unsigned long anispeed, unsigned long cycspeed, unsigned long cycfadespeed) {
  bool doUpdate = false;  // keep track of if we changed something so we only update on changes

  // START CYCLOTRON
  if ( useGameCyclotronEffect == true ) { // if we are doing the video game style cyclotron
    if ((unsigned long)(currentMillis - prevCycMillis) >= cycspeed) {
      prevCycMillis = currentMillis;

      switch ( cycOrder ) {
        case 0:
          setCyclotronLightState(c1Start, c1End, 0);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 2);
          cycFading = 0;
          cyclotronRunningFadeOut = 255;
          cycOrder = 1;
          break;
        case 1:
          setCyclotronLightState(c2Start, c2End, 0);
          setCyclotronLightState(c3Start, c3End, 2);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 1;
          cyclotronRunningFadeOut = 255;
          cycOrder = 2;
          break;
        case 2:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c3Start, c3End, 0);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 2;
          cyclotronRunningFadeOut = 255;
          cycOrder = 3;
          break;
        case 3:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c4Start, c4End, 0);
          cycFading = 3;
          cyclotronRunningFadeOut = 255;
          cycOrder = 0;
          break;
      }

      doUpdate = true;
    }

    // now figure out the fading light
    if ( (unsigned long)( currentMillis - prevFadeCycMillis) >= cycfadespeed ) {
      prevFadeCycMillis = currentMillis;
      if ( cycFading != -1 ) {
        switch ( cycFading ) {
          case 0:
            setCyclotronLightState(c4Start, c4End, 3);
            break;
          case 1:
            setCyclotronLightState(c1Start, c1End, 3);
            break;
          case 2:
            setCyclotronLightState(c2Start, c2End, 3);
            break;
          case 3:
            setCyclotronLightState(c3Start, c3End, 3);
            break;
        }
        doUpdate = true;
      }
    }
  } else { // otherwise this is the standard version
    if ((unsigned long)(currentMillis - prevCycMillis) >= cycspeed) {
      prevCycMillis = currentMillis;

      switch ( cycOrder ) {
        case 0:
          setCyclotronLightState(c4Start, c4End, 2);
          setCyclotronLightState(c1Start, c1End, 0);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 2);
          cycFading = 0;
          cyclotronRunningFadeOut = 255;
          cycOrder = 1;
          break;
        case 1:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 0);
          setCyclotronLightState(c3Start, c3End, 2);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 1;
          cyclotronRunningFadeOut = 255;
          cycOrder = 2;
          break;
        case 2:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 0);
          setCyclotronLightState(c4Start, c4End, 2);
          cycFading = 2;
          cyclotronRunningFadeOut = 255;
          cycOrder = 3;
          break;
        case 3:
          setCyclotronLightState(c1Start, c1End, 2);
          setCyclotronLightState(c2Start, c2End, 2);
          setCyclotronLightState(c3Start, c3End, 2);
          setCyclotronLightState(c4Start, c4End, 0);
          cycFading = 3;
          cyclotronRunningFadeOut = 255;
          cycOrder = 0;
          break;
      }

      doUpdate = true;
    }
  }
  // END CYCLOTRON

  // START POWERCELL
  if ((unsigned long)(currentMillis - prevPwrMillis) >= anispeed) {
    // save the last time you blinked the LED
    prevPwrMillis = currentMillis;

    for ( int i = powercellIndexOffset; i <= powerSeqTotal; i++) {
      if ( i <= powerSeqNum ) {
        powerStick.setPixelColor(i, powerStick.Color(0, 0, 150));
      } else {
        powerStick.setPixelColor(i, 0);
      }
    }

    if ( powerSeqNum <= powerSeqTotal) {
      powerSeqNum++;
    } else {
      powerSeqNum = powercellIndexOffset;
    }

    doUpdate = true;
  }
  // END POWERCELL

  // if we changed anything update
  if ( doUpdate == true ) {
    powerStick.show();
  }
}


/*************** Nose Jewel Firing Animations *********************/
unsigned long prevFireMillis = 0;
const unsigned long fire_interval = 50;     // interval at which to cycle lights (milliseconds).
int fireSeqNum = 0;
int fireSeqTotal = 5;

void clearFireStrobe() {
  for ( int i = 0; i < 7; i++) {
    noseJewel.setPixelColor(i, 0);
  }
  noseJewel.show();
  fireSeqNum = 0;
}

void fireStrobe(unsigned long currentMillis) {
  if ((unsigned long)(currentMillis - prevFireMillis) >= fire_interval) {
    prevFireMillis = currentMillis;

    switch ( fireSeqNum ) {
      case 0:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(2, 0);
        noseJewel.setPixelColor(3, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(4, 0);
        noseJewel.setPixelColor(5, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(6, 0);
        break;
      case 1:
        noseJewel.setPixelColor(0, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 2:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(1, 0);
        noseJewel.setPixelColor(2, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(3, 0);
        noseJewel.setPixelColor(4, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(5, 0);
        noseJewel.setPixelColor(6, noseJewel.Color(255, 0, 0));
        break;
      case 3:
        noseJewel.setPixelColor(0, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 4:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(1, 0);
        noseJewel.setPixelColor(2, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(3, 0);
        noseJewel.setPixelColor(4, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(5, 0);
        noseJewel.setPixelColor(6, noseJewel.Color(255, 255, 255));
        break;
      case 5:
        noseJewel.setPixelColor(0, noseJewel.Color(255, 0, 255));
        noseJewel.setPixelColor(1, noseJewel.Color(0, 255, 0));
        noseJewel.setPixelColor(2, noseJewel.Color(255, 0, 0));
        noseJewel.setPixelColor(3, noseJewel.Color(0, 0, 255));
        noseJewel.setPixelColor(4, noseJewel.Color(255, 0, 255));
        noseJewel.setPixelColor(5, noseJewel.Color(255, 255, 255));
        noseJewel.setPixelColor(6, noseJewel.Color(0, 0, 255));
        break;
    }

    noseJewel.show();

    fireSeqNum++;
    if ( fireSeqNum > fireSeqTotal ) {
      fireSeqNum = 0;
    }
  }
}


/************************* Shutdown and helper functions ****************************/
void shutdown_leds() {
  // reset the sequence
  seq_1_current = 1;
  fireSequenceNum = 1;

  // shut all led's off
  for (int i = 1; i <= 15; i++) {
    switch_graph_led(i, LOW);
  }
}
