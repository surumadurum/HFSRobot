package de.robertliebner.hfs.robot;

import oscP5.*;

public class ComMiddleware {

    //TODO initialize OSC listener here
    void init()
    {

    }

    void oscEvent(OscMessage theOscMessage) {
        if (theOscMessage.addrPattern().equals("/ADD_TO_QUEUE")) {
        }

        if (theOscMessage.addrPattern().equals("/PLAY_NOW")) {
            //TODO check against SceneControl.state
            String sceneName = theOscMessage.get(0).stringValue();
            try {
                SceneControl.loadScene(sceneName);
                SceneControl.startPlayback();
            } catch (Exception e) {
                System.out.println("Error loading scene: " + e.toString());
            }
        }

        if (theOscMessage.addrPattern().equals("/PLAY_NOW_UNINTERRUPTIBLE")) {
        }
    }
}
