package de.robertliebner.hfs.robot;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

import org.multiply.processing.TimedEventGenerator;
import processing.core.PApplet;
import sun.jvm.hotspot.runtime.Threads;
import sun.nio.ch.sctp.SctpNet;
import sun.reflect.annotation.ExceptionProxy;

import static de.robertliebner.hfs.robot.State.IDLE;
import static de.robertliebner.hfs.robot.State.PLAYBACK;
import static de.robertliebner.hfs.robot.State.RECORD;

enum State {RECORD,PLAYBACK,IDLE};

class SceneControl extends PApplet {

    private static State state = IDLE;

    private static HashMap<String, ArrayList<int[]>> lstScenes = new HashMap<String, ArrayList<int[]>>();  //holds all scenes

    //init list of int-array to hold currently active sample-set (both for recording and playing back
    static ArrayList<int[]> lstSamples = new ArrayList<int[]>();

    int currentAnswer;                 //index of recvd value in currently recorded sample
    static int currentSample = 0;              //index of currently played back sample

    static Thread heartbeatThread;

    static OnOneSamplerReceivedListener oneSamplerReceivedListener =null ;

    public static void init() {

        heartbeatThread = new Thread(new Runnable() {
            @Override
            public void run() {
                while(true) {


                    if (state == PLAYBACK) {
                        // println("heartbeat ->invoke SendPlaybackSample()" + currentSample);

                        if (lstSamples.size() == currentSample)
                        {
                            currentSample = 0; // loop if at end

                        }

                        if(currentSample == 0 ) {
                            new Thread(new Runnable() {
                                @Override
                                public void run() {
                                    lerpToFirstSample();
                                }
                            }).start();        //playback in-between steps
                            Thread.currentThread().suspend();
                        }

                        Controller.reportPlayedBackSamples(currentSample, lstSamples.size());
                        Controller.reportChannelValues(lstSamples.get(currentSample));

                        ComRobot.sendFullPlaybackSample((lstSamples.get(currentSample)));
                        currentSample++;

                    } else if (state == RECORD) {
                        println("record request");
                        //TODO Do this with all boards
                        ComRobot.sendRecordRequest();
                    }

                    try {
                        Thread.sleep(30);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }

            }
        });

        heartbeatThread.start();

    }

    //will check for current position and move towards the first sample in lstSamples
    static void lerpToFirstSample()
    {
        //pause heartbeat -> stop playback or recording

//        heartbeatThread.suspend();


        //request just one sample and catch the result

        requestOneSample(new OnOneSamplerReceivedListener() {
            @Override
            public void onSampleReceived(int[] sample) {

                List lerpSamples = new ArrayList<int[]>();


                //create lerped in-between samples
                for(int i=0;i<30;i++)   //we want to generate 30 samples, -> fixed time of 30ms*30 samples = 900ms
                {
                    int[] oneLerpedSample = new int[sample.length];
                    for(int j=0;j<sample.length;j++)
                    {
                        oneLerpedSample[j] = (int)PApplet.lerp(((float)sample[j]),((float)lstSamples.get(0)[j]),PApplet.map((float)i,0,29,0,1));
                    }

                    lerpSamples.add(oneLerpedSample);
                }

                //now play them back


                        for(int i=0;i<lerpSamples.size();i++)
                        {

                            ComRobot.sendFullPlaybackSample(((int[])lerpSamples.get(i)));

                            try {
                                Thread.sleep(100);
                            }
                            catch (InterruptedException e )
                            {
                                e.printStackTrace();
                            }
                        }

                        //resume
                        heartbeatThread.resume();

            }
        });

    }

    public static void requestOneSample(OnOneSamplerReceivedListener onOneSamplerReceivedListener)
    {

        SceneControl.oneSamplerReceivedListener = onOneSamplerReceivedListener;
        ComRobot.sendRecordRequest();

    }


    public static void onSampleReceived(int[] sample){

        if(oneSamplerReceivedListener != null) //is someone waiting for just one sample?
        {
            oneSamplerReceivedListener.onSampleReceived(sample);
            oneSamplerReceivedListener = null;  //clear the listener (and the action to wait for one at the same time
        }

        else
        {
            lstSamples.add(sample);    //TODO: Copy partial sample to correct part of full_sample and add that
            Controller.reportRecordedSamples(lstSamples.size());
        }
    }

    public static void startPlayback() //just startPlayback
    {
        currentSample = 0;  //reset cursor startPlayback position
        state = PLAYBACK;
    }

    public static void stopPlayback() {
        state = IDLE;
    }

    public static void loadScene(String sceneName) {
        System.out.println("Loading scene: " + sceneName);

        lstSamples = (ArrayList<int[]>) lstScenes.get(sceneName).clone();
    }


    public static void deleteScene(String sceneName) {

        lstScenes.remove(sceneName);
    }

    public static void saveScene(String sceneName) {
        if (lstSamples.size() == 0) {
            System.out.println("No samples recorded");
            return;
        }

        if(sceneName == "")
            return;

        lstScenes.put(sceneName, (ArrayList<int[]>) lstSamples.clone());

        Controller.FillSceneList();

        for (String key : lstScenes.keySet())
            System.out.println(key);

    }

    public static HashMap<String, ArrayList<int[]>> getLstScenes() {
        return lstScenes;
    }

    public static void setLstScenes(HashMap<String, ArrayList<int[]>> lstScenes) {
        SceneControl.lstScenes = lstScenes;
    }

    public static State getState() {
        return state;
    }


    public static void startRecording() {

        lstSamples.clear();
        currentSample = 0;

        state = RECORD;
    }

    public static void stopRecording() {

        state = IDLE;

        DialogBox dlgBox = new DialogBox(Controller.ctrlP5) {
            @Override
            void onFinished(int result, String resultAnswer) {
                saveScene(resultAnswer);
                lstSamples.clear(); // reset the prev. recorded samples

            }
        };

        dlgBox.createMessageBox();
        dlgBox.toggleBox(1);

    }

    public interface OnOneSamplerReceivedListener
    {
        void onSampleReceived(int[] sample);
    }
}
