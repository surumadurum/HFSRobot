/* //<>//
*2017/07 Robert Liebner
 *Frontend for Servo-Capture and Replay
 * 2017/11
 */

//NetAddress netAddressBoard[] =  {new NetAddress("192.168.88.10", 9000),new NetAddress("192.168.88.11", 9000),new NetAddress("192.168.88.12", 9000)};
NetAddress netAddressBoard[] =  {new NetAddress("255.255.255.255", 9000), new NetAddress("192.168.88.11", 9000), new NetAddress("192.168.88.12", 9000)};

static final int N_SERVOS = 30;
static final int SERVOS_PER_BOARD = 10;
static final int UDP_RECEIVE_PORT = 8000;

//commands to communicate with arduino. can be from 768 to 1023 (x>>8 (highbyte)==3 is start-byte)
//static final int CMD_REQUEST_ANALOG_VALUES_SERVO = 1000;
//static final int CMD_REQUEST_ANALOG_VALUES_POTI = 1001;
//static final int CMD_REQUEST_SET_SERVOS = 1002;
//static final int CMD_ANSWER_ANALOG_VALUES_SERVO = 1003;
//static final int CMD_ANSWER_ANALOG_VALUES_POTI = 1004;
//static final int CMD_REQUEST_MAKE_MOVEABLE = 1005;

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

Button btnReqSendData;
Button btnReqStopSendData;
Button btnRec;
Button btnPlayback;

boolean flRecord;
boolean flPlayback;
boolean flRecordOneSample;

int sampleSequenceNumber = 0;  //will hold a sequence number in order to merge matching sample-pieces of different ESP boards

String val;

boolean flParseAnswerAnalogValuesServo;

int currentAnswer;                 //index of recvd value in currently recorded sample
int currentSample =0;              //index of currently played back sample
int full_sample[] = new int[SERVOS_PER_BOARD * 3];  //holds currently recvd sample
ArrayList<int[]> lstSamples;       //holds active sample-set  (one movement sequence)


RollingLine2DTrace[] line2DTrace = new RollingLine2DTrace[SERVOS_PER_BOARD * 3];  //holds instances of trace lines that will be used in graphs
Graph2D[] graph = new Graph2D[SERVOS_PER_BOARD * 3];  //holds instances of graphs 

int[] analogValue = new int[SERVOS_PER_BOARD * 3];    //holds update values for rolling graphs

OscP5 oscP5;
ControlP5 ctrlP5;

List<String> servo_types = Arrays.asList( "TowerPro", "Torcster", "Savoex", "HeadBlue", "HeadBlack");  //available servo-types, used to fill dropdown menus

void setup() {


  //Setup event for sample processing (playback and record)
  HeartBeatTimedEventGenerator = new TimedEventGenerator(
    this, "onHeartBeat", true);
  HeartBeatTimedEventGenerator.setIntervalMs(30);  //30ms seems to be the fastest stable working sample rate.


  //init list of int-array to hold recorded sample
  lstSamples = new ArrayList<int[]>();

  //setup UDP
  //set this to the receiving port
  oscP5 = new OscP5(this, UDP_RECEIVE_PORT);

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

  size(1200, 1080,P2D);
  frameRate(20);

  //get us some graphs

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
    graph[i].setYAxisMax(4096);    //we show the raw data, not the mapped pwm signals, so we  need the full 12bit range
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
    //if (theEvent.isFrom("twitch" + i))
    //{
    //  println("recvd event from twitch" + i);
    //  //get current position of servo

    //  SendRecordRequest();

    //  int values[] = nRecvIntOverSerial(31); //ugly to read cmd-int as well, but there were problems using a RecvIntOverSerial() first.
    //  if (values[0]==-1)  //if there wasn't the expected amount of data on the buffer
    //    println("Error: could not retrieve values from arduino");
    //  else
    //  {
    //    values = Arrays.copyOfRange(values, 1, values.length);  //strip off the cmd-integer

    //    println(values[i]);
    //    //map values
    //    for (int j=0; j<N_SERVOS; j++)
    //    {
    //      //check which servo type it is
    //      String strCurServoType = "";
    //      try {
    //        strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + (j)).getValue());
    //      }
    //      catch(Exception e)
    //      {
    //        println(e.toString());
    //      }
    //      //map values
    //      values[j] = map_feedback_to_pwm(values[j], strCurServoType);
    //    }
    //    //now twitch requested servo a little back and forward
    //    for (int j=0; j<10; j++)
    //    {

    //      values[i] += 20;
    //      SendPlaybackSample(values);
    //      println(values[i]); 
    //      int millis_last = millis();
    //      while (millis()-millis_last<200)draw();
    //      values[i] -= 20;
    //      SendPlaybackSample(values);
    //      println(values[i]);          
    //      millis_last = millis();
    //      while (millis()-millis_last<200)draw();
    //    }
    //  }
    //}
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
  case "TowerPro": //new calibration nov 13th 2017
    rec_feedback_min = 217; //95;
    rec_feedback_max = 3495; //755;
    servo_pwm_min = 680; //160;
    servo_pwm_max = 2270; //520;
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
  return (int)constrain(map(intRecvd, rec_feedback_min, rec_feedback_max, servo_pwm_min, servo_pwm_max), servo_pwm_min, servo_pwm_max);
}

//Will get called periodically when recording or playing back
public void onHeartBeat()
{
  if (flPlayback)
  {
   // println("heartbeat ->invoke SendPlaybackSample()" + currentSample);

    if (lstSamples.size() == currentSample) currentSample =0; // loop if at end
    btnPlayback.setCaptionLabel("Playback " + "(" + currentSample + "/" + lstSamples.size());

    SendFullPlaybackSample(lstSamples.get(currentSample));
    currentSample++;
  } else if (flRecord)
  {
    println("record request");
    //TODO
    SendRecordRequest(netAddressBoard[0]);
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
    lstScenes  = (HashMap<String, ArrayList<int[]>>) o.readObject();
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

//public void RecordOnlyOneButton()
//{


//      lstSamples.clear();

//     int complete_samples = 0;

//     int outter_values[] = new int[N_SERVOS];

//     for(int i =0;i<20;i++)
//     {

//      myPort.clear();      
//      SendRecordRequest();
//      delay(10);      

//      int values[] = nRecvIntOverSerial(31); //ugly to read cmd-int as well, but there were problems using a RecvIntOverSerial() first.


//      if (values[0]==-1)  //if there wasn't the expected amount of data on the buffer
//        println("Error: could not retrieve values from arduino");
//      else
//      {
//        values = Arrays.copyOfRange(values, 1, values.length);  //strip off the cmd-integer


//        //map values
//        for (int j=0; j<N_SERVOS; j++)
//        {
//          //check which servo type it is
//          String strCurServoType = "";
//          try {
//            strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + (j)).getValue());
//          }
//          catch(Exception e)
//          {
//            println(e.toString());
//          }
//          //map values
//          values[j] = map_feedback_to_pwm(values[j], strCurServoType);
//          outter_values[j] += values[j];
//        }

//         complete_samples++;

//      }
//     }


//       for(int i=0;i<N_SERVOS;i++)
//         outter_values[i] /= complete_samples;


//       lstSamples.add(outter_values.clone());

//       toggleBox(1);   
//}

public void RecordButton() {

  flRecord = !flRecord;

  if (flRecord) {      //beginning of record

    lstSamples.clear(); // reset the prev. recorded samples
  } else                 //end of record
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
  currentSample =0;  //reset cursor playback position
  flPlayback = true;
}