
/*---------------------------------------------------------------------------------------------

  Open Sound Control (OSC) library for the ESP8266/ESP32

  Example for receiving open sound control (OSC) bundles on the ESP8266/ESP32
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.

  This example code is in the public domain.

--------------------------------------------------------------------------------------------- */

#define EXT_MUXER

#define PIN_DEMUX_A_OE 32
#define PIN_DEMUX_A_S0 33
#define PIN_DEMUX_A_S1 25
#define PIN_DEMUX_A_S2 26
#define PIN_DEMUX_A_S3 27
#define PIN_DEMUX_A_SIG 34

#define N_SERVOS 10

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <driver/adc.h>

#include "esp32-hal-ledc.h"

char ssid[] = "ROBOT";          // your network SSID (name)
char pass[] = "Handstand";                    // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

const IPAddress localIp(192,168,88,10 ); 
const IPAddress localGateway(192,168,88,2); 
const IPAddress localSubnet(255,255,255,0); 


const IPAddress outIp(192,168,88,2);        // remote IP (not needed for receive)
const unsigned int outPort = 8000;          // remote port (not needed for receive)
const unsigned int localPort = 9000;        // local port to listen for UDP packets (here's where we send the packets)

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



OSCErrorCode error;



void setup() {

    #ifdef EXT_MUXER

      pinMode(PIN_DEMUX_A_OE,OUTPUT);
      pinMode(PIN_DEMUX_A_S0,OUTPUT);
      pinMode(PIN_DEMUX_A_S1,OUTPUT);
      pinMode(PIN_DEMUX_A_S2,OUTPUT);
      pinMode(PIN_DEMUX_A_S3,OUTPUT);
      pinMode(PIN_DEMUX_A_SIG,ANALOG);


    
      digitalWrite(PIN_DEMUX_A_OE, HIGH);  //active low -> activate demux by default
    #endif
    
     analogSetWidth(10);
    // adc1_config_width(ADC_WIDTH_12Bit); //not necessary, is default

//    analogSetAttenuation(ADC_ATTEN_11db); //not necessary, is default

  for(int i=0;i<10;i++)
  {
    
//    pinMode(pinServoMap[i][0],ANALOG); 
    pinMode(pinServoMap[i][1],OUTPUT); 
    ledcSetup(i, 60, 12); // channel X, 50 Hz, 12-bit depth
    ledcAttachPin(pinServoMap[i][1], i);   // GPIO servoPin on channel X
  }
  

  Serial.begin(115200);

   uint64_t chipid=ESP.getEfuseMac();;   //get chip MAC
   
  Serial.printf("%04X%08X",(uint16_t)(chipid>>32),(uint32_t)chipid); //Make it readible
  
//  if(strChip == "201A07A4AE30") 
//    println("Chip no:12");


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(localIp,localGateway,localSubnet); //assign static IP
  
  //Do we have to disable DHCP client manually?
  
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

}

void answerRequestAnalogValuesServo(OSCMessage &msg )
{

  OSCMessage outMsg("/ANSWER_ANALOG_VALUES_SERVO");
  //read all analog values and send them over 
 
    Serial.print("read: ");

#ifndef EXT_MUXER  
  for(int i=0;i<N_SERVOS;i++)
  {
    int data = analogRead(pinServoMap[i][0]);
    outMsg.add(data);

    Serial.printf("%d\t",data);
   }
#endif

#ifdef EXT_MUXER

      int smoothed_values[N_SERVOS];

      getFeedback(smoothed_values);   //read demultiplexer and smooth raw data

      for(int i=0;i<N_SERVOS;i++)
      {
        outMsg.add(smoothed_values[i]);
        Serial.printf("%d\t",smoothed_values[i]);
       }
#endif


     Serial.println();
    
    Udp.beginPacket(outIp, outPort);
    outMsg.send(Udp); // send the bytes
    Udp.endPacket(); // mark the end of the OSC Packet  

//  Serial.printf("Msg sent");
}

void setServos(OSCMessage &msg)
{
  Serial.print("set: ");
  for(int i=0;i<10;i++)
  {
    Serial.printf("%d\t",msg.getInt(i));
    ledcWrite(i, msg.getInt(i));
  }
  Serial.println();
}

void makeMoveable(OSCMessage &msg)
{
  for(int i=0;i<10;i++)
  {
    if(msg.isInt(0))
      ledcWrite(i,msg.getInt(0));
    else
      ledcWrite(i, 16000);
  }
}

void loop() {
  
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {

      Serial.println("Msg recvd");
      
      msg.dispatch("/REQUEST_ANALOG_VALUES_SERVO",answerRequestAnalogValuesServo);
      msg.dispatch("/REQUEST_SET_SERVOS",setServos);
      msg.dispatch("/REQUEST_MAKE_MOVEABLE",makeMoveable);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
//  answerRequestAnalogValuesServo();
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


      digitalWrite(PIN_DEMUX_A_OE, LOW);  //active low -> activate demux by default

//      delay(1);
 
          smoothed_values[servo] = analogRead(PIN_DEMUX_A_SIG);//digitalSmooth(analogRead(PIN_DEMUX_A_SIG), sensSmoothArray[servo]); //get raw data from servo potentiometer and put it into smoothing function
//          smoothed_values[servo] = digitalSmooth(analogRead(PIN_DEMUX_A_SIG), sensSmoothArray[servo]); //get raw data from servo potentiometer and put it into smoothing function

//      delay(1);

      digitalWrite(PIN_DEMUX_A_OE, HIGH);  //active low -> activate demux by default


    }

  }
}

