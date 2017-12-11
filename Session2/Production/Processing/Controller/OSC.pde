
//Sends moveable Request to all boards
void SendMoveableRequest()
{
  for (NetAddress netAddress : netAddressBoard)
  {
    SendMoveableRequest(netAddress);
  }
}

void SendMoveableRequest(NetAddress remoteLocation)
{
  //TODO
  OscMessage oscMessage = new OscMessage("/REQUEST_MAKE_MOVEABLE");
  oscP5.send(oscMessage, remoteLocation);
  //SendIntOverSerial(CMD_REQUEST_MAKE_MOVEABLE);
}

//Splits the full sample and sends them to the boards
void SendFullPlaybackSample(int[] full_sample)
{
  //TODO
  //for(NetAddress netAddress : netAddressBoard)
  {
    SendPlaybackSample(full_sample, netAddressBoard[0]);
  }

  for (int i=0; i<SERVOS_PER_BOARD *3; i++)
  {
    //analogValue[i] = full_sample[i];
  }
}

void SendPlaybackSample(int[] partial_sample, NetAddress remoteLocation)
{

  //OscBundle myBundle = new OscBundle();
  OscMessage myMessage = new OscMessage("/REQUEST_SET_SERVOS");
  for (int i=0; i<SERVOS_PER_BOARD; i++)
  {
    myMessage.add(partial_sample[i]);
    //myBundle.add(myMessage);
  }
  oscP5.send(myMessage, remoteLocation);
}

//Requests one servo feedback sample (containing all servos)
void  SendRecordRequest(NetAddress remoteLocation) {
  OscMessage myMessage = new OscMessage("/REQUEST_ANALOG_VALUES_SERVO");
  oscP5.send(myMessage, netAddressBoard[0]); //TODO
}

// incoming osc message are forwarded to the oscEvent method. 
void oscEvent(OscMessage theOscMessage) {

  //TODO check against correct data types
  
  println("recvd: " + theOscMessage.addrPattern());

  if (theOscMessage.addrPattern().equals("/ANSWER_ANALOG_VALUES_SERVO")) { 
    int partial_sample[] = new int[SERVOS_PER_BOARD];

    for (int i=0; i<SERVOS_PER_BOARD; i++)
    {
      if (flRecord)
      {       
        //check which servo type it is
        String strCurServoType = "";
        try {
          strCurServoType= servo_types.get((int)ctrlP5.get("dropdown" + (i)).getValue());
        }
        catch(Exception e)
        {
          println(e.toString());
        }
        int val = theOscMessage.get(i).intValue();
        partial_sample[i] = map_feedback_to_pwm(val, strCurServoType);
        analogValue[i] = val;//partial_sample[i];
      }
    }

    lstSamples.add(partial_sample.clone());    //TODO: Copy partial sample to correct part of full_sample and add that
    btnRec.setCaptionLabel("Record (" +lstSamples.size()+ ")");
  }

  if (theOscMessage.addrPattern().equals("/ADD_TO_QUEUE")) {
  }

  if (theOscMessage.addrPattern().equals("/PLAY_NOW")) {
    String sceneName = theOscMessage.get(0).stringValue();
    try
    {
      LoadScene(sceneName);
      Playback();
    }
    catch(Exception e)
    {
      println("Error loading scene: " + e.toString()); 
    }
  }

  if (theOscMessage.addrPattern().equals("/PLAY_NOW_UNINTERRUPTIBLE")) {
  }
  
}