
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

#define MAX_LED 10 // Number of LEDs in bar graph array

// set pin numbers:
Button trigger(8);     // trigger on pin 8
Button mag(2);    // magazine loop on pin 2

const int muzzleLed =  3;      // muzzle LED on pin 3
const int rumble = 13;    // rumble motor on pin 13

// variables will change:
int buttonState = 0;         // variable for reading the trigger status
int lastButtonState = 0;     // variable to check if trigger has been released before re-pressing
float ammoCount = 0;     // variable for ammunition count
int magState = 0;     // variable for checking for mag removal/reload
int lastMagState = 0;
int lowLed = true;

unsigned long lastFlash = 0;
unsigned long lastLowPwr = 0;
const int flashInterval = 200;
const int lowPwrWng = 10000;

void setup() {

  lc.shutdown(0,false);
  lc.setIntensity(0,12);
  lc.clearDisplay(0);

  // initialize the LED pin as an output:
  pinMode(muzzleLed, OUTPUT);
  // initialize the trigger button:
  trigger.begin();
  mag.begin();
  // set rumble motor as output
  pinMode(rumble, OUTPUT);
  Serial.begin(9600);

  //powerup();
  
  }

void loop() {
  // read the state of the trigger and magazine:
  // magState = digitalRead(mag);
  delay(50);

  // check if the trigger state is not still the same, it is pressed, rifle is not out of ammo 
  // and the magazine is fitted:
  if (trigger.pressed() && mag.read() == Button::PRESSED) {
    if (ammoCount > 0) {
      weaponFire();   // weapon firing function

      Serial.print("Ammo remaining: ");
      Serial.println(ammoCount);
  
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
    /* turn LED off:
    digitalWrite(muzzleLed, LOW);
    lastButtonState = buttonState;*/
  }
  
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

  // online audio

  Serial.println("powerup complete");
}

void magOff() {
  lc.clearDisplay(0);

  Serial.println("mag off");
  delay(1000);
}

void weaponFire() {
  ammoCount = ammoCount - .25;   //reduce ammo available by 1

  LEDBarDisplay(ammoCount);   // reduce the number of LEDs turned on
  
  digitalWrite(rumble, HIGH);
  digitalWrite(muzzleLed, HIGH);
  delay(100);
  digitalWrite(muzzleLed, LOW);
  delay(200);
  digitalWrite(rumble, LOW);
  lastButtonState = buttonState;
  Serial.println("fire_process");
}

void emptyAmmo() {
  // sound to indicate out of ammo

  Serial.println("out_of_ammo");
}

void ammo_reload() {
  ammoCount=10;
  lastMagState = magState;
  Serial.println("reload");
  Serial.println(magState);
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
    
  //Serial.print ("Mark is: "); Serial.print (mark); Serial.print (". In Binary: ");
  //  Serial.println (LEDBit,BIN);

}

