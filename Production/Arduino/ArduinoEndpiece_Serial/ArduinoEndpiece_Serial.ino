

//#include <OSCBundle.h>
//#include <OSCBoards.h>

//#ifdef BOARD_HAS_USB_SERIAL
//#include <SLIPEncodedUSBSerial.h>
////SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
//#else
//#include <SLIPEncodedSerial.h>
//// SLIPEncodedSerial SLIPSerial(Serial);
//#endif

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x42);
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);

#define N_SERVOS  30

#define SensorPin1      0
#define SensorPin2      0
#define filterSamples   3                        // filterSamples should  be an odd number, no smaller than 3
 //TODO Check how this is interactiing with the smoothing
int sensSmoothArray[N_SERVOS][filterSamples];   // arrays for holding raw sensor values


//can be from 768 to 2023 (high-byte == 3 (e.g. 1003>>8))
const int CMD_REQUEST_ANALOG_VALUES_SERVO = 1000;
const int CMD_REQUEST_ANALOG_VALUES_POTI = 1001;
const int CMD_REQUEST_SET_SERVOS = 1002;
const int CMD_ANSWER_ANALOG_VALUES_SERVO = 1003;
const int CMD_ANSWER_ANALOG_VALUES_POTI = 1004;
const int CMD_REQUEST_MAKE_MOVEABLE = 1005;

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


void OnRequestPlayback(/*OSCMessage &msg, int Offset*/) {

  /*for(int i=0;i<5;i++)
    {
    pwm.setPWM(i, 0, map(msg.getInt(i),0,1023,SERVOMIN,SERVOMAX));
    }*/
  //    OSCMessage out("/analog/servos/sample");
  //
  //
  //      out.add((int32_t)analogRead(0));  //2do implement demultiplexer
  //
  //
  //    SLIPSerial.beginPacket();
  //      out.send(SLIPSerial); // send the bytes to the SLIP stream
  //    SLIPSerial.endPacket(); // mark the end of the OSC Packet
  //    out.empty(); // free space occupied by message


}

void OnRequestAnalogValuesServo(/*OSCMessage &msg*/) {

  String strOut;

  /*   for(int i=0;i<30;i++)
     {
       strOut +=  10 ;//analogRead(0);  //2do implement demultiplexer
     }*/



  //    OSCMessage out("/analog/servos/sample");

  //    for(int i=0;i<30;i++)
  //    {


  //      out.add((int32_t)analogRead(0));  //2do implement demultiplexer
  //    }

  //    SLIPSerial.beginPacket();
  //      out.send(SLIPSerial); // send the bytes to the SLIP stream
  //    SLIPSerial.endPacket(); // mark the end of the OSC Packet
  //    out.empty(); // free space occupied by message



}

void setup() {

  //we only have outputs... 0 and 1 are Serial... 13 is not used yet
  for (int i = 2; i < 13; i++)
  {
    pinMode(i, OUTPUT);
  }

  digitalWrite(PIN_DEMUX_A_OE, LOW);  //active low -> activate demux by default
  digitalWrite(PIN_DEMUX_B_OE, LOW);  //active low -> activate demux by default



  analogReference(EXTERNAL);

  Serial.begin(614400);   // set this as high as you can reliably run on your platform
  // Serial.setTimeout(10);

  /*#if ARDUINO >= 100
      while(!Serial)
        ;   // Leonardo bug
    #endif
  */
  pwm.begin();
  pwm2.begin();

  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  pwm2.setPWMFreq(60);  // Analog servos run at ~60 Hz updates


  /* This is actually the bottle-neck.
   *  Found in https://playground.arduino.cc/Main/WireLibraryDetailedReference:
   *  However, notice that the Wire library has blocking I/O. 
   *  That means, it goes into a busy loop and waits for the I2C communications to complete. 
   *  Your application can do nothing while the TWI hardware is talking over the I2C bus. 
   *  Note that this may not be what you want: If you have a time-critical sketch, having your 16MHz processor stuck in a busy loop waiting on a 100kHz communication stream is not efficient. 
   *  You may be better off writing your own I2C code. There is an example in ./doc/examples/twitest/twitest.c in the avr-libc source code (see http://www.nongnu.org/avr-libc/)
   *  
   *  In Wire.cpp
   *  uint8_t TwoWire::endTransmission(uint8_t sendStop)
      {
        // transmit buffer (blocking)
        uint8_t ret = twi_writeTo(txAddress, txBuffer, txBufferLength, 1, sendStop);
   
      In twi.c

        // wait for write operation to complete
        while(wait && (TWI_MTX == twi_state)){
          continue;}
   
   */
   
   
   
  //Wire.setClock(400000);

}

int getBytesAsWord(byte* bytes) {

  return   (bytes[0] << 8) | (bytes[1] & 0x00ff);
}

int RecvIntOverSerial() {
  int h = -1; int l = -1;
  while (h == -1) h = Serial.read();
  while (l == -1) l =   Serial.read();

  byte bytes[2] = {(byte)h, (byte)l};
  //Serial.flush();
  return getBytesAsWord(bytes);

}

void SendIntOverSerial(int data)
{
  byte high = (byte)(data >> 8);
  byte low  = (byte) (data & 0xff);
  Serial.write(high);
  Serial.write(low);
}




//reads and routes the incoming messages
void loop() {
  /*    OSCBundle bundleIN;

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

    }*/




  if (Serial.available() > 1) 
  {
    if(Serial.peek() != (1000>>8))
      Serial.read();//do we have the high_byte of our CMDs (take as start-byte)
    else
    {
    int intRecvd = RecvIntOverSerial();

    if (intRecvd == CMD_REQUEST_ANALOG_VALUES_SERVO)
    {

      SendIntOverSerial(CMD_ANSWER_ANALOG_VALUES_SERVO);

      int smoothed_values[N_SERVOS];

      getFeedback(smoothed_values);   //read demultiplexer and smooth raw data

      for (int i = 0; i < N_SERVOS; i++)
      {
        SendIntOverSerial(smoothed_values[i]);
      }
    }
    
    else if (intRecvd == CMD_REQUEST_SET_SERVOS)
    {
      for (int i = 0; i < N_SERVOS; i++)
      {
        if(i<16)
          pwm.setPWM(i, 0, RecvIntOverSerial());
        else
          pwm2.setPWM((i%16), 0, RecvIntOverSerial());
        
      }
    }

    else if (intRecvd == CMD_REQUEST_MAKE_MOVEABLE)
    {
      //make servos moveable
      for (int i = 0; i < N_SERVOS; i++)
      {

        if(i<16)
          pwm.setPin(i, 0, 0);
        else
          pwm2.setPin((i%16), 0, 0);
      }
    }
    
    }
    
  }
}





void getFeedback(int *smoothed_values) {

//TODO ENABLE SMOOTHING
  //gather 20 samples
  
 // for (int j = 0; j < 5; j++) 
  { //TODO: On an Arduino Uno, communication will break if you use this. On a Teensy this should not be a problem.

    //iterate over all servos
    for (int servo = 0; servo < N_SERVOS; servo++)
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
          smoothed_values[servo] = analogRead(PIN_DEMUX_A_SIG);//digitalSmooth(analogRead(PIN_DEMUX_A_SIG), sensSmoothArray[servo]); //get raw data from servo potentiometer and put it into smoothing function
      else
         smoothed_values[servo] = analogRead(PIN_DEMUX_B_SIG);//digitalSmooth(analogRead(PIN_DEMUX_B_SIG), sensSmoothArray[servo]);
//      if (servo < 16)
//          smoothed_values[servo] = digitalSmooth(analogRead(PIN_DEMUX_A_SIG), sensSmoothArray[servo]); //get raw data from servo potentiometer and put it into smoothing function
//      else
//         smoothed_values[servo] = digitalSmooth(analogRead(PIN_DEMUX_B_SIG), sensSmoothArray[servo]);

    
    
    }

  }
}


int digitalSmooth(int rawIn, int *sensSmoothArray) {
  int j, k, temp, top, bottom;
  long total;
  static int i;
  // static int raw[filterSamples];
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[i] = rawIn;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j = 0; j < filterSamples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++) {
      if (sorted[j] > sorted[j + 1]) {    // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j + 1] =  sorted[j] ;
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
  for ( j = bottom; j < top; j++) {
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
