/*
    Serial Call Response
    Send responses to calls for information from a remote host
*/

#include <OSCBundle.h>
#include <OSCBoards.h>

#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
#else
#include <SLIPEncodedSerial.h>
 SLIPEncodedSerial SLIPSerial(Serial);
#endif
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


void OnRequestPlayback(OSCMessage &msg, int Offset){

  /*for(int i=0;i<5;i++)
  {
    pwm.setPWM(i, 0, map(msg.getInt(i),0,1023,SERVOMIN,SERVOMAX));
  }*/
    OSCMessage out("/analog/servos/sample");


      out.add((int32_t)analogRead(0));  //2do implement demultiplexer

      
    SLIPSerial.beginPacket();  
      out.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    out.empty(); // free space occupied by message
}

void OnRequestRecord(OSCMessage &msg){

   
   
    //the message wants an OSC address as first argument
    OSCMessage out("/analog/servos/sample");
    
    for(int i=0;i<30;i++)
    {


      out.add((int32_t)analogRead(0));  //2do implement demultiplexer
    }
      
    SLIPSerial.beginPacket();  
      out.send(SLIPSerial); // send the bytes to the SLIP stream
    SLIPSerial.endPacket(); // mark the end of the OSC Packet
    out.empty(); // free space occupied by message
}

/**
 * MAIN METHODS
 * 
 * setup and loop, bundle receiving/sending, initial routing
 */
void setup() {
    SLIPSerial.begin(19200);   // set this as high as you can reliably run on your platform
#if ARDUINO >= 100
    while(!Serial)
      ;   // Leonardo bug
#endif

  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

}

//reads and routes the incoming messages
void loop(){ 
    OSCBundle bundleIN;

   int size;
 
    while(!SLIPSerial.endofPacket())
       if ((size =SLIPSerial.available()) > 0)
        {
           while(size--)
              bundleIN.fill(SLIPSerial.read());
        }
    if(!bundleIN.hasError())
     {
      bundleIN.route("/pwm", OnRequestPlayback);      
      bundleIN.dispatch("/request/servos/sample",OnRequestRecord);
}
}

