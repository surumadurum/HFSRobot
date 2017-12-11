/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/
#include <driver/adc.h>

const int pinServoMap[10][2] = {
  {36,23},
  {39,19},
  {34,18},
  {35,5},
  {32,17},
  {33,16},
  {25,4},
  {26,2},
  {27,15},
  {14,13}
};

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);

    adc1_config_width(ADC_WIDTH_12Bit); //not necessary, is default

//    analogSetAttenuation(ADC_ATTEN_11db); //not necessary, is default
  
}

// the loop routine runs over and over again forever:
void loop() {


  for(int i=0;i<10;i++)
  {
    Serial.printf("%d\t",analogRead(pinServoMap[i][0])); 
  }

  Serial.println();
  
  delay(1);        // delay in between reads for stability
}
