#include <Servo.h>
  
  Servo serv1;

void setup() {
  // put your setup code here, to run once:

  pinMode(3,OUTPUT);


  serv1.attach(3);

  //serv1.write(0);
  //hile(1);

  Serial.begin(115200);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  for(int x=0;x<=180;x+=10)
  {    serv1.write(x);
    delay(800);
    
  }


}
