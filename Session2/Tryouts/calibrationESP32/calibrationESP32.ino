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

#include <driver/adc.h>



void setup() {

  //analogReference(EXTERNAL);

   ledcSetup(1, 60, 14); // channel 1, 60 Hz, 16-bit width
   ledcAttachPin(2, 1);   // GPIO 22 assigned to channel 1

    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_11db);
    int analog = adc1_get_voltage(ADC1_CHANNEL_3);

  Serial.begin(115200);
  Serial.println("Calibration on 12 bit PWM");
  //while(1);
}

void loop() {


//  Serial.println("Now moving towards high end. Stopping when feedback does not change anymore.");
//
//  search_end_pos(10);
//
//
//  Serial.println("Now moving towards low end. Stopping when feedback does not change anymore.");
//
//  search_end_pos(-10);
//

  check_calibration(680,217,2270,3495);


  while (1);

  //      pwm.setPWM(i, 0, map(intIn,0,1023,SERVOMIN,SERVOMAX));
  //    Serial.print(String(i) + ": " + intIn + "\t");
}



void search_end_pos(int step)
{
  int currentPWM = 1000; //was 300

  Serial.println("Moving Servo to somewhat middle position PWM:300/4096");
 
  delay(1000);

  char strOut[100];

  int currentFeedback;
  for (int i = 0; i < 10; i++)
  {
    currentFeedback = (currentFeedback +  adc1_get_voltage(ADC1_CHANNEL_3)) / 2;
    delay(1);
  }

  int lastFeedback = 0;

  while (1)
  {
//    pwm.setPWM(0, 0, currentPWM);

     ledcWrite(1, currentPWM);       // sweep servo 1

    delay(200);

    currentPWM += step;

    currentFeedback = getFeedback(0);

    sprintf(strOut, "currentPWM: %d\tcurrentFeedback: %d\tlastFeedback: %d\tOffset: %d", currentPWM, currentFeedback, lastFeedback, currentFeedback - lastFeedback);
    Serial.println( strOut);

//    if (currentFeedback - lastFeedback == 0 )
//      break;

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
    reading[j] = adc1_get_voltage(ADC1_CHANNEL_3); //get raw data from servo potentiometer
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
    ledcWrite(1, calcPWM); 

    delay(30);
    int actualFeedback = getFeedback(0);
    
    sprintf(strOut, "Playback: %d\tcalculated PWM: %d\tactualFeedback: %d\tOffset: %d", i, calcPWM, actualFeedback, i - actualFeedback);
    Serial.println( strOut);

  }
}



