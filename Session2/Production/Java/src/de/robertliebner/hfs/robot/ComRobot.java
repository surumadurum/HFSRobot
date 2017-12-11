package de.robertliebner.hfs.robot;

import netP5.NetAddress;
import oscP5.*;
import processing.core.PApplet;

import java.util.Arrays;

import static de.robertliebner.hfs.robot.Controller.SERVOS_PER_BOARD;
import static de.robertliebner.hfs.robot.Controller.netAddressBoard;

class ComRobot
{

    Controller pApplet;
    static OscP5 oscP5;

    int partialSamples[][] = new int[3][Controller.SERVOS_PER_BOARD ];  //holds currently recvd sample
    int sampleSequenceNumber = 0;  //will hold a sequence number in order to merge matching sample-pieces of different ESP boards

    boolean flagSamplePartReceived[] = new boolean[Controller.netAddressBoard.length];

    public  void init( Controller _pApplet)
    {
        pApplet = _pApplet;

        //setup UDP
        //set this to the receiving port
        oscP5 = new OscP5(this, Controller.UDP_RECEIVE_PORT);
    }

    //Sends moveable Request to all boards
    static void sendMoveableRequest() {
        for (NetAddress netAddress : netAddressBoard) {
            sendMoveableRequest(netAddress);
        }
    }

    static void sendMoveableRequest(NetAddress remoteLocation) {
        OscMessage oscMessage = new OscMessage("/REQUEST_MAKE_MOVEABLE");
        oscMessage.add(4095);

        oscP5.send(oscMessage, remoteLocation);
        //SendIntOverSerial(CMD_REQUEST_MAKE_MOVEABLE);
    }

    //Splits the full sample and sends them to the boards
    static void sendFullPlaybackSample(int[] full_sample) {
        //TODO
        for(int i=0;i<netAddressBoard.length;i++)
        {
            int[] partial_sample = Arrays.copyOfRange(full_sample,i*SERVOS_PER_BOARD,i*SERVOS_PER_BOARD+SERVOS_PER_BOARD);
            sendPartialPlaybackSample(partial_sample, netAddressBoard[i]);
        }

        for (int i = 0; i < Controller.SERVOS_PER_BOARD * 3; i++) {
            Controller.reportChannelValue(i,full_sample[i]);
            //analogValue[i] = full_sample[i];
        }
    }

    static void sendPartialPlaybackSample(int[] partial_sample, NetAddress remoteLocation) {

        OscMessage myMessage = new OscMessage("/REQUEST_SET_SERVOS");
        for (int i = 0; i < Controller.SERVOS_PER_BOARD; i++) {
            myMessage.add(partial_sample[i]);
        }
        oscP5.send(myMessage, remoteLocation);
    }

    //Requests one servo feedback sample (containing all servos)
    static void sendRecordRequest() {
        OscMessage myMessage = new OscMessage("/REQUEST_ANALOG_VALUES_SERVO");
        for(NetAddress netAddress : netAddressBoard)
            oscP5.send(myMessage, netAddress);
    }

    //TODO forward events to dedicated methods
    // incoming osc message are forwarded to the oscEvent method.
    void oscEvent(OscMessage theOscMessage) {

        //TODO check against correct data types

        int boardNumber = resolveBoardNumber(theOscMessage.netAddress());

        System.out.println("recvd: " + theOscMessage.addrPattern());

        if (theOscMessage.addrPattern().equals("/ANSWER_ANALOG_VALUES_SERVO")) {
            int currentSample[] = new int[Controller.SERVOS_PER_BOARD];

            for (int i = 0; i < Controller.SERVOS_PER_BOARD; i++) {
//                if (state == RECORD)
                    {
                    //check which servo type it is
                    String strCurServoType = "";
                    try {
                        strCurServoType = Controller.getServoType(boardNumber*SERVOS_PER_BOARD + i);
                    } catch (Exception e) {
                        System.out.println(e.toString());
                    }
                    int val = theOscMessage.get(i).intValue();

                    currentSample[i] = map_feedback_to_pwm(val, strCurServoType);

                    Controller.reportChannelValue(boardNumber*SERVOS_PER_BOARD + i,currentSample[i]/*val*/);
                }
            }

            //flag the current netAddress
//            assert(flagSamplePartReceived[boardNumber] == false);
            flagSamplePartReceived[boardNumber] = true;


            boolean[] allTrue = Arrays.copyOf(new boolean[]{false},netAddressBoard.length);
            Arrays.fill(allTrue,true);

            //copy sample to correct range in full_sample.
            partialSamples[boardNumber] = currentSample;

            if(Arrays.equals(flagSamplePartReceived,allTrue)) //did we receive a sample of all boards?
            {
                SceneControl.onSampleReceived(concat(concat(partialSamples[0],partialSamples[1]),partialSamples[2]));

                //empty
                flagSamplePartReceived = new boolean[netAddressBoard.length];

            }

        }
    }

    private int[] concat(int[] a, int[] b) {
        int aLen = a.length;
        int bLen = b.length;
        int[] c= new int[aLen+bLen];
        System.arraycopy(a, 0, c, 0, aLen);
        System.arraycopy(b, 0, c, aLen, bLen);
        return c;
    }

    private int resolveBoardNumber(NetAddress netAddress) {
        for(int i=0; i<netAddressBoard.length;i++) {
            if (netAddress.address().equals(netAddressBoard[i].address())) {     //look for the respective address
                return i;
            }
        }

        return -1;
    }

    public static void sendMiddlePosition()
    {

        int[] zeroPosSample = new int[SERVOS_PER_BOARD * netAddressBoard.length];



        for(int i=0;i<SERVOS_PER_BOARD*netAddressBoard.length;i++)
        {
            int pwm_middle;

            switch (Controller.getServoType(i)) {
                case "TowerPro":
                    pwm_middle = 373;
                    break;

                case "Master":
                    pwm_middle = 372;
                    break;

                case "HeadBlack":
                    pwm_middle = 381;
                    break;

                case "HeadBlue":
                    pwm_middle = 445;
                    break;

                default:
                    pwm_middle = 375;
                    break;
            }

            zeroPosSample[i] = pwm_middle;

        }

        ComRobot.sendFullPlaybackSample(zeroPosSample);

    }

    public static int map_feedback_to_pwm(int intRecvd, String strCurServoType) {

        int rec_feedback_min, rec_feedback_max, servo_pwm_min, servo_pwm_max;

        //TODO: refactor!
        switch (strCurServoType) {
            case "TowerPro": //new calibration dec 7th 2017
                rec_feedback_min = 60; //95;
                rec_feedback_max = 859; //755;
                servo_pwm_min = 180; //160;
                servo_pwm_max = 566; //520;
                break;

            case "Master":
                rec_feedback_min = 104;
                rec_feedback_max = 755;
                servo_pwm_min = 123;
                servo_pwm_max = 621;
                break;

            case "HeadBlack":
                rec_feedback_min = 36;
                rec_feedback_max = 504;
                servo_pwm_min = 276;
                servo_pwm_max = 486;
                break;

            case "HeadBlue":
                rec_feedback_min = 302;
                rec_feedback_max = 668;
                servo_pwm_min = 261;
                servo_pwm_max = 630;
                break;
//        case "TowerPro": //new calibration nov 13th 2017
//                rec_feedback_min = 217; //95;
//                rec_feedback_max = 3495; //755;
//                servo_pwm_min = 680; //160;
//                servo_pwm_max = 2270; //520;
//                break;
//
//            case "Torcster":
//                rec_feedback_min = 101;
//                rec_feedback_max = 532;
//                servo_pwm_min = 120;
//                servo_pwm_max = 540;
//                break;
//
//            case "Savoex":
//                rec_feedback_min = 124;
//                rec_feedback_max = 899;
//                servo_pwm_min = 160;
//                servo_pwm_max = 530;
//                break;

            default:
                throw new RuntimeException("Servo type not found: " + strCurServoType);
        }
        return (int) PApplet.constrain(PApplet.map(intRecvd, rec_feedback_min, rec_feedback_max, servo_pwm_min, servo_pwm_max), servo_pwm_min, servo_pwm_max);
//        return (int) PApplet.map(intRecvd, rec_feedback_min, rec_feedback_max, servo_pwm_min, servo_pwm_max);
    }


}

