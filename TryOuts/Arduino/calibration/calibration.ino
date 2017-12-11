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


void setup() {

  analogReference(EXTERNAL);

  Serial.begin(115200);
  Serial.println("Calibration on 12 bit PWM");

  pwm.begin();

  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  yield();

  //millis_new = millis();

}

void loop() {


/*  Serial.println("Now moving towards high end. Stopping when feedback does not change anymore.");

  search_end_pos(5);


  Serial.println("Now moving towards low end. Stopping when feedback does not change anymore.");

  search_end_pos(-5);
*/

  check_calibration(160,118,530,907);


  while (1);

  //      pwm.setPWM(i, 0, map(intIn,0,1023,SERVOMIN,SERVOMAX));
  //    Serial.print(String(i) + ": " + intIn + "\t");
}



void search_end_pos(int step)
{
  int currentPWM = 300;

  Serial.println("Moving Servo to somewhat middle position PWM:300/4096");
  pwm.setPWM(0, 0, currentPWM);
  delay(1000);

  char strOut[100];

  int currentFeedback;
  for (int i = 0; i < 10; i++)
  {
    currentFeedback = (currentFeedback + analogRead(0)) / 2;
    delay(1);
  }

  int lastFeedback = 0;

  while (1)
  {
    pwm.setPWM(0, 0, currentPWM);
    delay(200);

    currentPWM += step;

    currentFeedback = getFeedback(0);

    sprintf(strOut, "currentPWM: %d\tcurrentFeedback: %d\tlastFeedback: %d\tOffset: %d", currentPWM, currentFeedback, lastFeedback, currentFeedback - lastFeedback);
    Serial.println( strOut);

    if (currentFeedback - lastFeedback == 0 )
      break;

    lastFeedback = currentFeedback;
  }

  Serial.println("Endposition found");

}


/*
  THIS FUNCTION READS THE INTERNAL SERVO POTENTIOMETER
*/
int getFeedback(int a) {
  int j;
  int mean;
  int result;
  int test;
  int reading[20];
  boolean done;

  for (j = 0; j < 20; j++) {
    reading[j] = analogRead(a); //get raw data from servo potentiometer
    delay(3);
  } // sort the readings low to high in array
  done = false; // clear sorting flag
  while (done != true) { // simple swap sort, sorts numbers from lowest to highest
    done = true;
    for (j = 0; j < 20; j++) {
      if (reading[j] > reading[j + 1]) { // sorting numbers here
        test = reading[j + 1];
        reading [j + 1] = reading[j] ;
        reading[j] = test;
        done = false;
      }
    }
  }
  mean = 0;
  for (int k = 6; k < 14; k++) { //discard the 6 highest and 6 lowest readings
    mean += reading[k];
  }
  result = mean / 8; //average useful readings
  return (result);
}    // END GET FEEDBACK


void check_calibration(int pwm_min,int feedback_min,int pwm_max,int feedback_max)
{
  for(int i=feedback_min;i<feedback_max;i++)
  {
    char strOut[100];

    int calcPWM = map(i,feedback_min,feedback_max,pwm_min,pwm_max);
    pwm.setPWM(0, 0, calcPWM);

    delay(30);
    int actualFeedback = getFeedback(0);
    
    sprintf(strOut, "Playback: %d\tcalculated PWM: %d\tactualFeedback: %d\tOffset: %d", i, calcPWM, actualFeedback, i - actualFeedback);
    Serial.println( strOut);

  }
}



