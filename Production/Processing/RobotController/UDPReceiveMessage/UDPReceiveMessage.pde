import processing.serial.*; //<>// //<>// //<>// //<>//

/*
*2017/07 Robert Liebner
 *Frontend for Servo-Capture and Replay
 *
 */


static final int N_SERVOS = 30;


//commands to communicate with arduino. can be from 768 to 1023 (x>>8 (highbyte)==3 is start-byte)
static final int CMD_REQUEST_ANALOG_VALUES_SERVO = 1000;
static final int CMD_REQUEST_ANALOG_VALUES_POTI = 1001;
static final int CMD_REQUEST_SET_SERVOS = 1002;
static final int CMD_ANSWER_ANALOG_VALUES_SERVO = 1003;
static final int CMD_ANSWER_ANALOG_VALUES_POTI = 1004;
static final int CMD_REQUEST_MAKE_MOVEABLE = 1005;

//static final int SERIAL_TIMEOUT = 2000;

import oscP5.*;
import netP5.*;
import controlP5.*;

import java.util.*;
import java.io.*;

import org.gwoptics.graphics.graph2D.Graph2D;
import org.gwoptics.graphics.graph2D.traces.ILine2DEquation;
import org.gwoptics.graphics.graph2D.traces.RollingLine2DTrace;

import org.multiply.processing.TimedEventGenerator;

private TimedEventGenerator HeartBeatTimedEventGenerator;


OutputStream fos = null;  //used for data serialization

NetAddress myRemoteLocation =  new NetAddress("127.0.0.1", 9000);

Button btnReqSendData;
Button btnReqStopSendData;
Button btnRec;
Button btnPlayback;

boolean flRecord;
boolean flPlayback;
boolean flRecordOneSample;

Serial myPort;
String val;

boolean flParseAnswerAnalogValuesServo;

int currentAnswer;                 //index of recvd value in currently recorded sample
int currentSample =0;              //index of currently played back sample
int sample[] = new int[N_SERVOS];  //holds currently recvd sample
ArrayList<int[]> lstSamples;       //holds active sample-set  (one movement sequence)

//class eq implements ILine2DEquation{
//  public double computePoint(double x,int pos) {
//    return analogValue0;
//  }    
//}

RollingLine2DTrace[] line2DTrace = new RollingLine2DTrace[N_SERVOS];  //holds instances of trace lines that will be used in graphs
Graph2D[] graph = new Graph2D[N_SERVOS];  //holds instances of graphs 

int[] analogValue = new int[N_SERVOS];    //holds update values for rolling graphs


OscP5 oscP5;
ControlP5 ctrlP5;

List<String> servo_types = Arrays.asList( "TowerPro", "Torcster", "Savoex", "HeadBlue", "HeadBlack");  //available servo-types, used to fill dropdown menus

void setup() {

  //print available serial ports for debugging 
  for (int i=0; i<Serial.list().length; i++)
  {
    println(Serial.list()[i].toString());
  }

  try //just in case we also want to work on this sketch without arduino, we catch an exception here
  {
    String portName = Serial.list()[6];
    myPort = new Serial(this, portName, 614400);  //bottle-neck seems not to be serial baud rate, but i2c sending time on arduino when setting pwm multiplexer
    //  myPort.buffer(2); //only trigger when 2 elements have been received
  }
  catch (Exception e)
  {
    println("Could not open Serial: " + e);
  }

  //Setup event for sample processing (playback and record)
  HeartBeatTimedEventGenerator = new TimedEventGenerator(
    this, "onHeartBeat", true);
  HeartBeatTimedEventGenerator.setIntervalMs(30);  //30ms seems to be the fastest stable working sample rate.


  //init list of int-array to hold recorded sample
  lstSamples = new ArrayList<int[]>();


  //setup UDP
  //set this to the receiving port
  oscP5 = new OscP5(this, 9001);

  //setup GUI

  ctrlP5 = new ControlP5(this);

  ctrlP5.addButton("MakeMoveableButton")
    .setPosition(700, 20)
    .setSize(400, 19)
    .setCaptionLabel("Make Moveable"); 

  btnRec = ctrlP5.addButton("RecordButton")
    .setPosition(700, 60)
    .setSize(200, 19)
    .setCaptionLabel("Record");  
    
  ctrlP5.addButton("RecordOnlyOneButton")
    .setPosition(901, 60)
    .setSize(200, 19)
    .setCaptionLabel("Record one sample");  

  btnPlayback = ctrlP5.addButton("PlaybackButton")
    .setPosition(700, 80)
    .setSize(200, 19)
    .setCaptionLabel("Playback");

  ctrlP5.addButton("SaveButton")
    .setPosition(700, 100)
    .setSize(200, 19)
    .setCaptionLabel("Save config");

  ctrlP5.addButton("LoadButton")
    .setPosition(700, 120)
    .setSize(200, 19)
    .setCaptionLabel("Load config");

  ctrlP5.addScrollableList("scenes")
    .setPosition(700, 160)
    .setSize(400, 300)
    .setBarHeight(19)
    .setItemHeight(19)
    .setType(ScrollableList.LIST); // currently supported DROPDOWN and LIST
 
  ctrlP5.addButton("LoadSceneButton")
    .setPosition(700, 470)
    .setSize(199, 19)
    .setCaptionLabel("Load scene");

  ctrlP5.addButton("DeleteSceneButton")
    .setPosition(900, 470)
    .setSize(199, 19)
    .setCaptionLabel("Delete scene");


  for (int i=29; i>-1; i--)
  {
    ctrlP5.addButton("twitch" + i)
      .setPosition(600, i*20)
      .setSize(99, 19)
      .setCaptionLabel("twitch servo " +i);
  }

  //we need to start with the down-most dropdown box, otherwise in expanded state it will be overlapped by the next one 
  for (int i=29; i>-1; i--)
  {
    /* add a ScrollableList, by default it behaves like a DropdownList */
    ctrlP5.addScrollableList("dropdown" +i)
      .setPosition(500, i*20)
      .setSize(99, 100)
      .setBarHeight(19)
      .setItemHeight(19)
      .addItems(servo_types)
      .setType(ScrollableList.DROPDOWN) // currently supported DROPDOWN and LIST
      .setOpen(false);
    ;
  }


  createMessageBox();

  size(1200, 1080);
  frameRate(50);

  //get us some graphs

  //line2DTrace[0] = new RollingLine2DTrace(new eq() ,100,0.1f);
  for (int i=0; i<30; i++)
  { 
    final int index = i;   //use final in order to be able to use it in anonymous class definition

    line2DTrace[i] = new RollingLine2DTrace(
      new ILine2DEquation() {
      public double computePoint(double x, int pos) {
        return analogValue[index];
      }
    } 
    , 100, 0.1f);

    line2DTrace[i].setTraceColour(255, 0, 0);

    graph[i] = new Graph2D(this, 400, 30, false);
    graph[i].setYAxisMax(800);    //we show the mapped pwm signals, so we don't need the full 1024 range
    graph[i].addTrace(line2DTrace[i]);


    graph[i].position.y = i*30;
    graph[i].position.x = 0;
    graph[i].setYAxisTickSpacing(100);
    graph[i].setXAxisMax(5f);
  }
}

void controlEvent(ControlEvent theEvent) {
  /* Checks, if twitch button was pressed (used to identify servos on puppet)
   * if so, current servo positions are requested
   * and respective servo is then moved a little back and forth
   * as at this point not all servo types might have been set correctly, there might be some jittery movements here
   */

  for (int i=0; i<30; i++)
  {
    if (theEvent.isFrom("twitch" + i))
    {
      println("recvd event from twitch" + i);
      //get current position of servo

      SendRecordRequest();

      int values[] = nRecvIntOverSerial(31); //ugly to read cmd-int as well, but there were problems using a RecvIntOverSerial() first.
      if (values[0]==-1)  //if there wasn't the expected amount of data on the buffer
        println("Error: could not retrieve values from arduino");
      else
      {
        values = Arrays.copyOfRange(values, 1, values.length);  //strip off the cmd-integer

        println(values[i]);
        //map values
        for (int j=0; j<N_SERVOS; j++)
        {
          //check which servo type it is
          String strCurServoType = "";
          try {
            strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + (j)).getValue());
          }
          catch(Exception e)
          {
            println(e.toString());
          }
          //map values
          values[j] = map_feedback_to_pwm(values[j], strCurServoType);
        }
        //now twitch requested servo a little back and forward
        for (int j=0; j<10; j++)
        {

          values[i] += 20;
          SendPlaybackSample(values);
          println(values[i]); 
          int millis_last = millis();
          while (millis()-millis_last<200)draw();
          values[i] -= 20;
          SendPlaybackSample(values);
          println(values[i]);          
          millis_last = millis();
          while (millis()-millis_last<200)draw();
        }
      }
    }
  }

  //println(theEvent.getName());
  //println(servo_types.get((int)ctrlP5.get("dropdown1").getValue()));
}


void draw() {
  background(255); 

  //draw our graphs
  for (Graph2D g : graph)
    g.draw();


  ctrlP5.draw();

  if (flRecord)
    btnRec.setColorLabel(color(255, 0, 0));
  else
    btnRec.setColorLabel(color(255, 255, 255));

  if (flPlayback)
    btnPlayback.setColorLabel(color(255, 0, 0));
  else
    btnPlayback.setColorLabel(color(255, 255, 255));
}



int  map_feedback_to_pwm(int intRecvd, String strCurServoType) {

  int rec_feedback_min, rec_feedback_max, servo_pwm_min, servo_pwm_max;

  //TODO: refactor!
  switch(strCurServoType)
  {
  case "TowerPro":
    rec_feedback_min = 95;
    rec_feedback_max = 755;
    servo_pwm_min = 160;
    servo_pwm_max = 520;
    break;

  case "Torcster":
    rec_feedback_min = 101;
    rec_feedback_max = 532;
    servo_pwm_min = 120;
    servo_pwm_max = 540;
    break;

  case "Savoex":
    rec_feedback_min = 124;
    rec_feedback_max = 899;
    servo_pwm_min = 160;
    servo_pwm_max = 530;      
    break;

  default:
    throw new RuntimeException("Servo type not found: " + strCurServoType);
  }
  //println(strCurServoType);

  return (int)constrain(map(intRecvd, rec_feedback_min, rec_feedback_max, servo_pwm_min, servo_pwm_max),servo_pwm_min,servo_pwm_max);
}

//Will get called periodically when recording or playing back
public void onHeartBeat()
{

  if (flPlayback)
  {
    println("heartbeat ->involke SendPlaybackSample()" + currentSample);

    if (lstSamples.size() == currentSample) currentSample =0; // loop if at end
    btnPlayback.setCaptionLabel("Playback " + "(" + currentSample + "/" + lstSamples.size());
    SendPlaybackSample(lstSamples.get(currentSample));
    currentSample++;
  } 
  
  
  else if (flRecord)
  {
    //SendIntOverSerial(0x00);  //Send sth to clear up buffer on Arduino side TODO: This is urgly!
    SendRecordRequest();
    //delay(5);
    println("heartbeat ->involke SendRecordRequest()");
    int[] answer = nRecvIntOverSerial(31);
    if (answer[0] != CMD_ANSWER_ANALOG_VALUES_SERVO)  //TODO: Ugly that we have the command in the array
      println("unexpected answer: " + answer[0]);
    //throw new RuntimeException("Retrieved unexpected value from Arduino");
    else
    {


      for (int i=1; i<answer.length; i++)
      {
        //check which servo type it is
        String strCurServoType = "";
        try {
          strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + (i-1)).getValue());
        }
        catch(Exception e)
        {
          println(e.toString());
        }

        answer[i] = map_feedback_to_pwm(answer[i], strCurServoType);
        analogValue[i-1] = answer[i];  
      }
      println();

      lstSamples.add(Arrays.copyOfRange(answer, 1, answer.length).clone());
      btnRec.setCaptionLabel("Record (" +lstSamples.size()+ ")");
      
      //if(flRecordOneSample)
      //{
      //  flRecordOneSample = false;
      //  RecordButton();  //if we only want to record one sample, trigger record button to stop recording
      //}
    }
  }
}

void MakeMoveableButton()
{
  SendMoveableRequest();
}



public void SaveButton() {

  ctrlP5.saveProperties();

  try
  {
    fos = new FileOutputStream("test.ser");
    ObjectOutputStream o = new ObjectOutputStream( fos );
    //  o.writeObject( ctrlP5 );
    o.writeObject( lstScenes );
  }
  catch ( IOException e ) { 
    System.err.println( e );
  }
  finally { 
    try { 
      fos.close();
    } 
    catch ( Exception e ) { 
      e.printStackTrace();
    }
  }
}

public void LoadButton() {
  InputStream fis = null;

  ctrlP5.loadProperties();

  try
  {
    fis = new FileInputStream( "test.ser");
    ObjectInputStream o = new ObjectInputStream( fis );
    //   ctrlP5 = (ControlP5) o.readObject();
    lstScenes  = (HashMap<String,ArrayList<int[]>>) o.readObject();
    FillSceneList();
    //System.out.println( string );
    //System.out.println( date );
  }
  catch ( IOException e ) { 
    System.err.println( e );
  }
  catch ( ClassNotFoundException e ) { 
    System.err.println( e );
  }
  finally { 
    try { 
      fis.close();
    } 
    catch ( Exception e ) {
    }
  }
}

public void RecordOnlyOneButton()
{

  
      lstSamples.clear();
     
     int complete_samples = 0;
   
     int outter_values[] = new int[N_SERVOS];
   
     for(int i =0;i<20;i++)
     {
   
      myPort.clear();      
      SendRecordRequest();
      delay(10);      

      int values[] = nRecvIntOverSerial(31); //ugly to read cmd-int as well, but there were problems using a RecvIntOverSerial() first.

     
      if (values[0]==-1)  //if there wasn't the expected amount of data on the buffer
        println("Error: could not retrieve values from arduino");
      else
      {
        values = Arrays.copyOfRange(values, 1, values.length);  //strip off the cmd-integer


        //map values
        for (int j=0; j<N_SERVOS; j++)
        {
          //check which servo type it is
          String strCurServoType = "";
          try {
            strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + (j)).getValue());
          }
          catch(Exception e)
          {
            println(e.toString());
          }
          //map values
          values[j] = map_feedback_to_pwm(values[j], strCurServoType);
          outter_values[j] += values[j];
        }
         
         complete_samples++;
       
      }
     }
       
       
       for(int i=0;i<N_SERVOS;i++)
         outter_values[i] /= complete_samples;
       
       
       lstSamples.add(outter_values.clone());
        
       toggleBox(1);
 
      
}


public void RecordButton() {

  flRecord = !flRecord;
  
  if (flRecord) {      //beginning of record

    lstSamples.clear(); // reset the prev. recorded samples
  }
  else                 //end of record
  {
     toggleBox(1);
  }
  

}

public void PlaybackButton()   //toggle playback
{
  if (flRecord)flRecord= false; //turn off recording if it is turned on

  flPlayback = !flPlayback;


}

void Playback() //just playback
{
    flPlayback = true;
    currentSample =0;  //reset cursor playback position
  
}

void SendMoveableRequest()
{
    SendIntOverSerial(CMD_REQUEST_MAKE_MOVEABLE);
}




void SendPlaybackSample(int[] sample)
{
  /* for(int i=0;i<30;i++)
   {
   OscBundle myBundle = new OscBundle();
   OscMessage myMessage = new OscMessage("/pwm" + str(i));
   myMessage.add(sample[i]);
   myBundle.add(myMessage);
   oscP5.send(myBundle, myRemoteLocation);
   
   }*/
  SendIntOverSerial(CMD_REQUEST_SET_SERVOS);

  for (int i=0; i<N_SERVOS; i++)
  {
    SendIntOverSerial(sample[i]);
    analogValue[i] = sample[i];
  }
}


/* Reads n integers from serial buffer
 * This function is NON-BLOCKING, meaning that if less ints than requested are available
 * function will return -1
 * TODO: even if function fails, bytes will be read from buffer and are therefore "discarded".
 *       would be better to peek bytes from buffer without reading them right in front.
 *       serial.available() did not give the desired results
 */
int[] nRecvIntOverSerial(int count) {


  byte[] bytes = new byte[count*2];
  int[] ints = new int[count];

  int nRead = myPort.readBytes(bytes);
  //myPort.clear();
  println(nRead);

  for (int i=0; i<count; i++)
  {
    ints[i] = getBytesAsWord(new byte[]{bytes[i*2], bytes[(i*2)+1]});
  }

  if (nRead != count*2)  //we could not receive enough bytes
    return new int[]{-1};
  else
    return ints;
}

int RecvIntOverSerial() {

  byte[] bytes = {(byte)myPort.read(), (byte)myPort.read()};

  return getBytesAsWord(bytes);
}

//Splits an int into two bytes and sends them (hi-byte first)
void SendIntOverSerial(int data)
{
  byte high = (byte)(data>>8);
  byte low  = (byte) (data & 0xff);
  myPort.write(high);
  myPort.write(low);
}

//Concats an int from two bytes
int getBytesAsWord(byte[] bytes) {

  return  ((bytes[0] & 0xff) << 8) | (bytes[1] & 0xff);
}

//Requests one servo feedback sample (containing all servos)
void  SendRecordRequest() {

  /*  OscBundle myBundle = new OscBundle();
   OscMessage myMessage = new OscMessage("/request/servos/sample");
   myBundle.add(myMessage);
   oscP5.send(myBundle, myRemoteLocation);*/
  SendIntOverSerial(CMD_REQUEST_ANALOG_VALUES_SERVO);
}

//deprecated
void serialEventasdfasdf(Serial p) { 


  int intRecvd = RecvIntOverSerial();

  if (flParseAnswerAnalogValuesServo)  // we are recording a value sequence
  {
    if (currentAnswer >= N_SERVOS)
    {
      flParseAnswerAnalogValuesServo = false;
      println("");
      lstSamples.add(sample.clone());
      btnRec.setCaptionLabel("Record (" +lstSamples.size()+ ")");
      return;
    }

    print(currentAnswer + ": " + intRecvd + " ");

    //check which servo type it is
    String strCurServoType = "";
    try {
      strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + currentAnswer).getValue());
    }
    catch(Exception e)
    {
      println(e.toString());
    }

    sample[currentAnswer] = map_feedback_to_pwm(intRecvd, strCurServoType);
//    analogValue0 = sample[0]; 

    currentAnswer++;
  } else if ( intRecvd == CMD_ANSWER_ANALOG_VALUES_SERVO) //We receive a new sample, start recording
  {
    flParseAnswerAnalogValuesServo = true;
    currentAnswer = 0;
  }
} 



//deprecated
// incoming osc message are forwarded to the oscEvent method. 
void oscEvent(OscMessage theOscMessage) {

  println(theOscMessage.addrPattern());

  if (theOscMessage.addrPattern().equals("/analog/servos/sample")) { 
    int sample[] = new int[N_SERVOS];

    for (int i=0; i<N_SERVOS; i++)
    {
      if (flRecord)
      {
        sample[i] = theOscMessage.get(i).intValue();
//        analogValue0 = sample[0];
      }
    }

//    println(analogValue0);

    lstSamples.add(sample.clone());
  }
}