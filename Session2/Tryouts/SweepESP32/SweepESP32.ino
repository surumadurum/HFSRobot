#define COUNT_LOW 700
#define COUNT_HIGH 2000
#define TIMER_WIDTH 16
#include "esp32-hal-ledc.h"

void setup() {
   ledcSetup(1, 60, 14); // channel 1, 50 Hz, 16-bit width
   ledcAttachPin(17, 1);   // GPIO 22 assigned to channel 1
//   ledcWrite(1,6000);
//   while(1);
   Serial.begin(115200);
   pinMode(34,ANALOG);
}
void loop() {


   for (int i=COUNT_HIGH ; i > COUNT_LOW ; i--)
   {
      ledcWrite(1, i);       // sweep servo 1
      Serial.println(i);
      delay(5);
   }

      for (int i=COUNT_LOW ; i < COUNT_HIGH ; i+=10)
   {
      ledcWrite(1, i);       // sweep servo 1
      Serial.print(i + "   ");
//      Serial.println(analogRead(34));
      delay(200);
     
   }

ledcWrite(1,1500);
delay(4000);
ledcWrite(1,1700);
delay(4000);
ledcWrite(1,1000);
delay(4000);   
}

