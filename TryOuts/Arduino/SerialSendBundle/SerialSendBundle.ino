
/*
    Make an OSC bundle and send it over SLIP serial

    OSCBundles allow OSCMessages to be grouped together to\  preserve the order and completeness of related messages.
    They also allow for timetags to be carried to represent the presentation time of the messages.
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


void setup() {
  //begin SLIPSerial just like Serial
    SLIPSerial.begin(9600);   // set this as high as you can reliably run on your platform  
#if ARDUINO >= 100
    while(!Serial)
      ;   // Leonardo bug
#endif

}

void loop(){
    //declare the bundle


  OSCMessage msg("/analog/servos/sample");

  
    for(int i=0;i<30;i++)
      msg.add((int32_t)analogRead(0));

  SLIPSerial.beginPacket();  
    msg.send(SLIPSerial); // send the bytes to the SLIP stream
  SLIPSerial.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message


    delay(5);
}
