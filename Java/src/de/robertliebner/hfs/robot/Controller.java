/*
*2017/07 Robert Liebner
 *Frontend for Servo-Capture and Replay
 * 2017/11
 */
package de.robertliebner.hfs.robot;



import java.util.*;

import javafx.scene.Scene;
import netP5.*;
import controlP5.*;

import java.io.*;

import org.apache.commons.collections4.map.ListOrderedMap;


import processing.core.PApplet;
import processing.event.MouseEvent;

import processing.sound.*;


public class Controller extends processing.core.PApplet {



    //NetAddress netAddressBoard[] =  {new NetAddress("192.168.88.10", 9000),new NetAddress("192.168.88.11", 9000),new NetAddress("192.168.88.12", 9000)};
    public static final NetAddress netAddressBoard[] = {new NetAddress("192.168.88.10", 9000), new NetAddress("192.168.88.11", 9000), new NetAddress("192.168.88.12", 9000)};

    static PApplet soundApplet = new PApplet(); //just for sound
    static SoundFile soundFile= new SoundFile(soundApplet, "/Users/HP-Printer4/Nextcloud/robot/Session2/Production/Java/out/production/Java/de/robertliebner/hfs/robot/data/Error.mp3");


    static ListOrderedMap<String,ServoSpecs> servos = new ListOrderedMap<String,ServoSpecs>(
    );



//    public  static final List<String> servo_types = Arrays.asList("TowerPro", "Master", "Savoex", "HeadBlue", "HeadBlack");  //available servo-types, used to fill dropdown menus

    public static final int SERVOS_PER_BOARD = 10;
    public static final int UDP_RECEIVE_PORT_ROBOT = 8000;
    public static final int UDP_RECEIVE_PORT_MIDDLEWARE = 7000;

    OutputStream fos = null;  //used for data serialization

    static Button btnReqSendData;
    static Button btnReqStopSendData;
    static Button btnRec;
    static Button btnPlayback;

//    RollingLine2DTrace[] line2DTrace = new RollingLine2DTrace[SERVOS_PER_BOARD * 3];  //holds instances of trace lines that will be used in graphs
//    Graph2D[] graph = new Graph2D[SERVOS_PER_BOARD * 3];  //holds instances of graphs

    static int[]  analogValue = new int[SERVOS_PER_BOARD * 3];    //holds update values for rolling graphs

    public static ControlP5 ctrlP5;

    static Chart[] chart = new Chart[SERVOS_PER_BOARD * netAddressBoard.length];

    ComRobot comRobot = new ComRobot();

    ComMiddleware comMiddleware = new ComMiddleware();

    public static void main(String[] args) {
        main("de.robertliebner.hfs.robot.Controller");
    }

    public void settings() {
        size(1200, 1080);
//        frameRate(60);
    }

    public void setup() {

        servos.put("TowerPro",
                new ServoSpecs(60,859,180,566));

        servos.put("Master",
                new ServoSpecs(104,755,123,621));

        servos.put("HeadBlue",
                new ServoSpecs(302,668,261,630));


        servos.put("HeadBlack",
                new ServoSpecs(36,504,276,486));


//        soundFile.loop();


        comRobot.init(this);    //Initialize communication with robot

        ComMiddleware.init();


        SceneControl.init();

        //setup GUI

        ctrlP5 = new ControlP5(this);

        ctrlP5.addButton("makeMoveableButton")
                .setPosition(700, 20)
                .setSize(400, 19)
                .setCaptionLabel("Make Moveable");

        btnRec = ctrlP5.addButton("recordButton")
                .setPosition(700, 60)
                .setSize(200, 19)
                .setCaptionLabel("Record");

        ctrlP5.addButton("recordOnlyOneButton")
                .setPosition(901, 60)
                .setSize(200, 19)
                .setCaptionLabel("Record one sample");

        btnPlayback = ctrlP5.addButton("playbackButton")
                .setPosition(700, 80)
                .setSize(200, 19)
                .setCaptionLabel("Playback");

        ctrlP5.addButton("saveButton")
                .setPosition(700, 100)
                .setSize(200, 19)
                .setCaptionLabel("Save config");

        ctrlP5.addButton("loadButton")
                .setPosition(700, 120)
                .setSize(200, 19)
                .setCaptionLabel("Load config");

        ctrlP5.addScrollableList("scenes")
                .setPosition(700, 160)
                .setSize(400, 300)
                .setBarHeight(19)
                .setItemHeight(19)
                .setType(ScrollableList.LIST);

        ctrlP5.addScrollableList("queue")
                .setPosition(700, 500)
                .setSize(400, 300)
                .setBarHeight(19)
                .setItemHeight(19)
                .setType(ScrollableList.LIST);

        ctrlP5.addButton("loadSceneButton")
                .setPosition(700, 470)
                .setSize(199, 19)
                .setCaptionLabel("Load scene");

        ctrlP5.addButton("deleteSceneButton")
                .setPosition(900, 470)
                .setSize(199, 19)
                .setCaptionLabel("Delete scene");

//        ctrlP5.addButton("zeroPositionButton")
//                .setPosition(700, 800)
//                .setSize(199, 19)
//                .setCaptionLabel("Set all servos to middle position");


//        for (int i = 29; i > -1; i--) {
//            ctrlP5.addButton("twitch" + i)
//                    .setPosition(600, i * 32)
//                    .setSize(99, 30)
//                    .setCaptionLabel("twitch servo " + i);
//        }

        //we need to start with the down-most dropdown box, otherwise in expanded state it will be overlapped by the next one
        for (int i = 29; i > -1; i--) {
            /* add a ScrollableList, by default it behaves like a DropdownList */
            ctrlP5.addScrollableList("dropdown" + i)
                    .setPosition(500, i * 32)
                    .setSize(99, 100)
                    .setBarHeight(30)
                    .setItemHeight(30)
                    .addItems(servos.asList())
                    .setType(ScrollableList.DROPDOWN) // currently supported DROPDOWN and LIST
                    .setOpen(false);
            ;
        }

        //get us some graphs

        for (int i = 0; i < 30; i++) {

            chart[i] = ctrlP5.addChart("channel: " + i)
                    .setPosition(20, i*32)
                    .setSize(400, 30)
                    .setRange(0, 800)
                    .setView(Chart.LINE) // use Chart.LINE, Chart.PIE, Chart.AREA, Chart.BAR_CENTERED
                    .setStrokeWeight(1.5f)
                    .setColorCaptionLabel(color(40))
            ;

            chart[i].addDataSet("pwm");
            chart[i].setData("pwm", new float[100]);


            loadButton();
        }
    }

    public void controlEvent(ControlEvent theEvent) {
  /* Checks, if twitch button was pressed (used to identify servos on puppet)
   * if so, current servo positions are requested
   * and respective servo is then moved a little back and forth
   * as at this point not all servo types might have been set correctly, there might be some jittery movements here
   */

        for (int i = 0; i < 30; i++) {
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


    public void draw() {
        background(255);

        //draw our graphs
//        for (Graph2D g : graph)
//            g.draw();

        ctrlP5.draw();

        fill(255,0,0);

        textMode(CENTER);

        for (int i = 0; i < SERVOS_PER_BOARD * netAddressBoard.length; i++) {
            text( "" + i + ": " + analogValue[i], 425, i * 32 + 15);

        }

        switch(SceneControl.getState())
        {
            case RECORD:
                btnRec.setColorLabel(color(255, 0, 0));
                btnPlayback.setColorLabel(color(255, 255, 255));
                break;

            case PLAYBACK_QUEUE:    //TODO get a separate button for this
            case PLAYBACK:
                btnRec.setColorLabel(color(255, 255, 255));
                btnPlayback.setColorLabel(color(255, 0, 0));
                break;

            default:
                btnRec.setColorLabel(color(255, 255, 255));
                btnPlayback.setColorLabel(color(255, 255, 255));

        }

    }

    public void makeMoveableButton() {
        println("Moveable request");

        ComRobot.sendMoveableRequest();
    }

    public void saveButton() {

        ctrlP5.saveProperties();

        try {
            fos = new FileOutputStream("test.ser");
            ObjectOutputStream o = new ObjectOutputStream(fos);
            //  o.writeObject( ctrlP5 );
            o.writeObject(SceneControl.getLstScenes());
        } catch (IOException e) {
            System.err.println(e);
            e.printStackTrace();
        } finally {
            try {
                fos.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void loadButton() {
        InputStream fis = null;

       //

        try {
            fis = new FileInputStream("test.ser");
            ObjectInputStream o = new ObjectInputStream(fis);
            //   ctrlP5 = (ControlP5) o.readObject();
            SceneControl.setLstScenes((HashMap<String, ArrayList<int[]>>) o.readObject());
            fillSceneList();
            //System.out.println( string );
            //System.out.println( date );
        } catch (IOException e) {
            System.err.println(e);
        } catch (ClassNotFoundException e) {
            System.err.println(e);
        } finally {
            try {
                fis.close();
            } catch (Exception e) {
            }
        }

        ctrlP5.loadProperties();

        //TODO user feedback
    }

    public static void fillSceneList() {
        ((ScrollableList) ctrlP5.getController("scenes"))
                .clear()
                .addItems(SceneControl.getLstScenes().keySet().toArray(new String[SceneControl.getLstScenes().keySet().size()]));
    }

    public static void fillQueueList() {
        ((ScrollableList) ctrlP5.getController("queue"))
                .clear()
                .addItems(SceneControl.getSceneQueue());
    }

    public void mouseClicked(MouseEvent m) {
        if (mouseEvent.getClickCount() == 2 && ctrlP5.get("scenes").isMouseOver()) {
            LoadSceneButton();
            SceneControl.startPlayback();
        }
    }

    void LoadSceneButton() {
        ScrollableList mememe = ctrlP5.get(ScrollableList.class, "scenes");
        String selectedSceneName = (String) (mememe.getItem((int) mememe.getValue()).get("text"));

        SceneControl.loadScene(selectedSceneName);

    }

    public void zeroPositionButton()
    {
        ComRobot.sendMiddlePosition();;
    }

    public void deleteSceneButton() {
        ScrollableList mememe = ctrlP5.get(ScrollableList.class, "scenes");
        String selectedSceneName = (String) (mememe.getItem((int) mememe.getValue()).get("text"));

        SceneControl.deleteScene(selectedSceneName);

        fillSceneList();
    }

    public void recordButton() {

        switch(SceneControl.getState())
        {
            //recording in process, stop and save it
            case RECORD:
                SceneControl.stopRecording();
                break;

            case IDLE:
                SceneControl.startRecording();
                break;
        }

    }

    public  static void reportRecordedSamples( int sampleNumberTotal)
    {
            btnRec.setCaptionLabel("Record (" + sampleNumberTotal + ")");
    }

    public  static void reportPlayedBackSamples(int currentSample, int size)
    {
            btnPlayback.setCaptionLabel("Playback " + "(" + currentSample + "/" + size);
    }

    public  static void reportChannelValue(int channel,int value)
    {
        analogValue[channel] = value;

        chart[channel].push("pwm",value);

        if(value>= servos.get(getServoType(channel)).getPwm_max() || value <= servos.get(getServoType(channel)).pwm_min)

        {


            soundFile.play();
            chart[channel].setColorForeground(soundApplet.color(255,0,0));
            chart[channel].setColorBackground(soundApplet.color(255,0,0));
            chart[channel].setColorActive(soundApplet.color(255,0,0));


        }
        else
        {
            chart[channel].setColorForeground(soundApplet.color(0,0,255));
            chart[channel].setColorBackground(soundApplet.color(0,0,255));
            chart[channel].setColorActive(soundApplet.color(0,0,255));
        }

///           analogValue[channel] = value;
    }

    public static void reportChannelValues(int[] sample)
    {
        for(int i=0;i<sample.length;i++)
        {
            reportChannelValue(i,sample[i]);
        }
    }

    public void playbackButton()   //toggle startPlayback
    {
        switch(SceneControl.getState()) {

            case RECORD:
                SceneControl.stopRecording(); //turn off recording if it is turned on
                SceneControl.startPlayback();
                break;

            case IDLE:
                SceneControl.startPlayback();
                break;

            case PLAYBACK:
                SceneControl.stopPlayback();
                break;
        }

    }

    public static String getServoType(int i)
    {
        return  servos.get((int)ctrlP5.get("dropdown" + (i)).getValue());
    }

}





//public void RecordOnlyOneButton()
//{


//      lstPlaybackSamplesCurrentSet.clear();

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


//       lstPlaybackSamplesCurrentSet.add(outter_values.clone());

//       toggleBox(1);
//}