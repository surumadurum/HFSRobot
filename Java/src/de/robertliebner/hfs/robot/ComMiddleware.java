package de.robertliebner.hfs.robot;

import javafx.scene.Scene;
import oscP5.*;


//singleton
public class ComMiddleware {

    static ComMiddleware me = new ComMiddleware();

    static OscP5 oscP5;

    //TODO initialize OSC listener here
    static void init()
    {
        oscP5 = new OscP5(me, Controller.UDP_RECEIVE_PORT_MIDDLEWARE);
    }

    static public void oscEvent(OscMessage theOscMessage) {

        if (theOscMessage.addrPattern().equals("/ADD_TO_QUEUE")) {

            String sceneName = theOscMessage.get(0).stringValue().toLowerCase();
            int priority = theOscMessage.get(1).intValue();
            try {
                SceneControl.enqueueScene(sceneName,priority);
                System.out.println(String.format("Adding \"%s\" with priority %d to queue",sceneName,priority));

                //                SceneControl.startPlayback();
            } catch (Exception e) {
                System.out.println("Error enqueuing scene: " + e.toString());
            }
        }

        if (theOscMessage.addrPattern().equals("/PLAY_NOW")) {
            //TODO check against SceneControl.state
            String sceneName = theOscMessage.get(0).stringValue();
            try {

                System.out.println(String.format("Playing instantly \"%s\" ",sceneName));
                SceneControl.stopPlayback();    //reset the current playback position
                SceneControl.enqueueScene(sceneName,100);

                for(String str: SceneControl.getSceneQueue())
                    System.out.println(str);

                //SceneControl.startPlayback();

            } catch (Exception e) {
                System.out.println("Error enqueuing scene: " + e.toString());
            }
        }

        if (theOscMessage.addrPattern().equals("/PLAY_NOW_UNINTERRUPTIBLE")) {
        }
    }
}
