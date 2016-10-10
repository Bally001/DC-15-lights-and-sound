
/* Project: DC-15A/S Lights and Sound
 * by Craig Ball
 * 
 * LED bar control code taken from bratan @ http://www.instructables.com/id/Controlling-simple-LED-Bar-Graph-with-Arduino/
 * 
 * Started 28 Jul 16
 * 
 */

#include <Button.h>
#include <LedControl.h>
LedControl lc=LedControl(5,6,7,1);

#include <TMRpcm.h>
#include <SD.h>  
#include <SPI.h>
#define SD_ChipSelectPin 10 

TMRpcm tmrpcm;   // create an object for use in this sketch

#define MAX_LED 10 // Number of LEDs in bar graph array

#include <Event.h>
#include <Timer.h>

Timer t;

// set pin numbers:
Button trigger(8);     // trigger on pin 8
Button mag(2);    // magazine loop on pin 2

const byte muzzleLed =  3;      // muzzle LED on pin 3
const byte rumble = 4;    // rumble motor on pin 4

// variables will change:
float ammoCount = 0;     // variable for ammunition count
bool lowLed = true;

unsigned long lastFlash = 0;
unsigned long lastLowPwr = 0;
const byte flashInterval = 200;
const int lowPwrWng = 10000;

void setup() {

  lc.shutdown(0,false);
  lc.setIntensity(0,12);
  lc.clearDisplay(0);

  pinMode(muzzleLed, OUTPUT);   // initialize the LED pin as an output
  trigger.begin();    // initialize the trigger button
  mag.begin();     // initialise the magazine loop
  pinMode(rumble, OUTPUT);    // set rumble motor as output
  Serial.begin(9600);     // serial for debugging and "stuff"

  SD.begin(SD_ChipSelectPin);
  tmrpcm.speakerPin = 9;
  tmrpcm.setVolume(4);

  //if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
  //  Serial.println("SD fail");  
  //  return;   // don't do anything more if not
  }

  //powerup();
  
  //}

void loop() {
  delay(50);

  // check if the trigger state is not still the same, it is pressed, rifle is not out of ammo 
  // and the magazine is fitted:
  if (trigger.pressed() && mag.read() == Button::PRESSED) {
    if (ammoCount > 0) {
      weaponFire();   // weapon firing function

      Serial.println("fire");   // serial write for debugging
      Serial.println(mag.read());
    }
    else {
      Serial.println("empty");
      emptyAmmo();  // out of ammo function
    }
  }

  if (ammoCount <= 2) {
    ammoLow();
  }

  if (mag.released()) {
    magOff();
  }

  if (mag.pressed()) {
    powerup();
  }

  else {
    }

  t.update();
  
}

void powerup() {
  delay(30);
  lc.clearDisplay(0);

  // startup display 
  for(int i=0; i<11; i++) {
    LEDBarDisplay(i);
    delay(120);
    }

  ammoCount = 10;   // start with full ammo

  tmrpcm.play("powerup.wav");  // online audio

  Serial.println("powerup complete");
}

void magOff() {
  lc.clearDisplay(0);

  Serial.println("mag off");
  delay(1000);
}

void weaponFire() {
  ammoCount = ammoCount - 1;   //reduce ammo available by 1

  LEDBarDisplay(ammoCount);   // reduce the number of LEDs turned on

  tmrpcm.play("fire.wav");
  
 // t.pulseImmediate(muzzleLed, 100, HIGH);
//  t.pulseImmediate(rumble, 300, HIGH);
  
  }

void emptyAmmo() {
  // sound to indicate out of ammo

  Serial.println("out_of_ammo");
  }

void ammo_reload() {
  ammoCount=10;
  Serial.println("reload");
  }

void ammoLow()  {
 if (millis() - lastFlash >= flashInterval)  {
   if (ammoCount >1) {
    if (lowLed) {
     lowLed = false;
     lc.setLed(0,0,0,true);
     lc.setLed(0,0,1,true); 
    }
    else  {
     lowLed = true;
     lc.setLed (0,0,0,false);
     lc.setLed(0,0,1,false);
    }
   }
   if (ammoCount <= 1) {
    if (lowLed) {
    lowLed = false;
    lc.setLed(0,0,0,true); 
    }
    else  {
     lowLed = true;
     lc.setLed (0,0,0,false);
     }
    }
   
   lastFlash = millis();
  }

 if (millis() - lastLowPwr >= lowPwrWng)  {
  tmrpcm.play("powerlow.wav");
  lastLowPwr = millis();
 }
}

// --- Light up "mark" number of sequential LEDs in Bar Graph
void LEDBarDisplay(int mark) {
  if (mark < 0 || mark > MAX_LED) return; // Sanity check, cannot be more than max or less than 0 LEDs
  byte LEDBit=B11111111;
  if (mark <=8) { // First row (first 8 LEDs)
    LEDBit=LEDBit << (8-mark); // Shift bits to the left
    lc.setRow(0,0,LEDBit);  //Light up LEDs
    lc.setRow(0,1,B00000000); // Blank out second row
  }
  else { // Second row (LED 9-10)
   LEDBit=LEDBit << (16-mark); // Shift bits to the left. 16 is 8*row
   lc.setRow(0,0,B11111111);  //Light first 8 LEDs
   lc.setRow(0,1,LEDBit); 
  }
    
}

