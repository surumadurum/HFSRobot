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
#define SERVOMIN  180 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  520 // this is the 'maximum' pulse length count (out of 4096)
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  analogReference(EXTERNAL);
  
  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

}

void loop() {
  Serial.print("Set min - feedback value:");
  pwm.setPWM(3, 0,  map(0,0,1023,SERVOMIN,SERVOMAX));
  delay(800);
  Serial.println(analogRead(3));
  delay(2000);
  Serial.print("Set max - feedback value:");
  pwm.setPWM(3, 0,  map(1023,0,1023,SERVOMIN,SERVOMAX));
  delay(800);
  Serial.println(analogRead(3));
  delay(2000);  
}
