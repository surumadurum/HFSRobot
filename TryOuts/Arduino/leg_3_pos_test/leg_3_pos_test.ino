/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  Servo test - this will drive 16 servos, one after the other

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These displays use I2C to communicate, 2 pins are required to  
  interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  520 // this is the 'maximum' pulse length count (out of 4096)

unsigned long millis_new =0;
int toggle = 1;

// our servo # counter
uint8_t servonum = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("16 channel Servo test!");

  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  yield();

  
}

void loop() {

      char strOut[100];
    //sprintf(strOut,"A: %d\tB: %d\tC:%d", analogRead(0),analogRead(1),analogRead(2));
    //Serial.println( strOut);

      pwm.setPWM(0, 0, map(0,0,1023,SERVOMIN,SERVOMAX));
      pwm.setPWM(1, 0, map(344,0,1023,SERVOMIN,SERVOMAX));
      pwm.setPWM(2, 0, map(0,0,1023,SERVOMIN,SERVOMAX));
      delay(200);
      pwm.setPin(0,0,0);
      pwm.setPin(1,0,0);
      pwm.setPin(2,0,0);      
      delay(1500);
/*
      pwm.setPWM(0, 0, map(993,0,1023,SERVOMIN,SERVOMAX));
      pwm.setPWM(1, 0, map(289,0,1023,SERVOMIN,SERVOMAX));
      pwm.setPWM(2, 0, map(470,0,1023,SERVOMIN,SERVOMAX));
      delay(200);
      pwm.setPin(0,0,0);
      pwm.setPin(1,0,0);
      pwm.setPin(2,0,0);      
      delay(1500);
*/
      pwm.setPWM(0, 0, map(203,0,1023,SERVOMIN,SERVOMAX));
      pwm.setPWM(1, 0, map(1006,0,1023,SERVOMIN,SERVOMAX));
      pwm.setPWM(2, 0, map(207,0,1023,SERVOMIN,SERVOMAX));
      delay(200);
      pwm.setPin(0,0,0);
      pwm.setPin(1,0,0);
      pwm.setPin(2,0,0);      
      delay(1500);
  }

  

