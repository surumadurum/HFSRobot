#define PIN_DEMUX_A_OE 2
#define PIN_DEMUX_A_S0 3
#define PIN_DEMUX_A_S1 4
#define PIN_DEMUX_A_S2 5
#define PIN_DEMUX_A_S3 6
#define PIN_DEMUX_A_SIG 0


#define PIN_DEMUX_B_OE 8
#define PIN_DEMUX_B_S0 9
#define PIN_DEMUX_B_S1 10
#define PIN_DEMUX_B_S2 11
#define PIN_DEMUX_B_S3 12
#define PIN_DEMUX_B_SIG 1



#define SensorPin1      0
#define SensorPin2      0
#define filterSamples   13              // filterSamples should  be an odd number, no smaller than 3

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x42);


int sensSmoothArray[30][filterSamples];   // array for holding raw sensor values for sensor1 

int rawData1, smoothData1;  // variables for sensor1 data
int rawData2, smoothData2;  // variables for sensor2 data

void setup(){
  Serial.begin(115200);

   

    pwm.begin();
  pwm2.begin();

  Wire.setClock(1000000);

  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  pwm2.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
}
void loop(){       // test the digitalSmooth function
  long int millis_last = millis();


      //make servos moveable
      for (int i = 0; i < 30; i++)
      {

        if(i<16)
          pwm.setPin(i, 0, 0);
        else
          pwm2.setPin((i%16), 0, 0);
      }
      
        //iterate over all servos
    for (int servo = 0; servo < 30; servo++)
    {

      //address correct pin on demultiplexer
      digitalWrite(PIN_DEMUX_A_S0, bitRead(servo, 0));
      digitalWrite(PIN_DEMUX_A_S1, bitRead(servo, 1));
      digitalWrite(PIN_DEMUX_A_S2, bitRead(servo, 2));
      digitalWrite(PIN_DEMUX_A_S3, bitRead(servo, 3));

      digitalWrite(PIN_DEMUX_B_S0, bitRead((servo % 16), 0)); //modulo 16 to strip off everything above 15 to second multiplexer
      digitalWrite(PIN_DEMUX_B_S1, bitRead((servo % 16), 1));
      digitalWrite(PIN_DEMUX_B_S2, bitRead((servo % 16), 2));
      digitalWrite(PIN_DEMUX_B_S3, bitRead((servo % 16), 3));

      if (servo < 16)
           analogRead(0);//digitalSmooth(analogRead(PIN_DEMUX_A_SIG), sensSmoothArray[servo]); //get raw data from servo potentiometer and put it into smoothing function
      else
          analogRead(1);//digitalSmooth(analogRead(PIN_DEMUX_B_SIG), sensSmoothArray[servo]);


    }
    
   //rawData1 = analogRead(SensorPin1);                        // read sensor 1
    //smoothData1 = digitalSmooth(rawData1, sensSmoothArray[i]);  // every sensor you use with digitalSmooth needs its own array
  //}
    long int millis_next = millis();
    Serial.print(rawData1);
    Serial.print("   ");
    Serial.print(smoothData1);
    Serial.print("   ");
    Serial.println(millis_next-millis_last);


}

int digitalSmooth(int rawIn, int *sensSmoothArray){     // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
  int j, k, temp, top, bottom;
  long total;
  static int i;
 // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j=0; j<filterSamples; j++){     // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting              
  while(done != 1){        // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++){
      if (sorted[j] > sorted[j + 1]){     // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j+1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

/*
  for (j = 0; j < (filterSamples); j++){    // print the array to debug
    Serial.print(sorted[j]); 
    Serial.print("   "); 
  }
  Serial.println();
*/

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1); 
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j< top; j++){
    total += sorted[j];  // total remaining indices
    k++; 
    // Serial.print(sorted[j]); 
    // Serial.print("   "); 
  }

//  Serial.println();
//  Serial.print("average = ");
//  Serial.println(total/k);
  return total / k;    // divide by number of samples
}

