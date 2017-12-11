package de.robertliebner.hfs.robot;


import controlP5.*;
import de.robertliebner.hfs.robot.Controller;
import processing.core.*;
import sun.plugin.net.proxy.PluginAutoProxyHandler;

abstract class DialogBox {


    ControlGroup messageBox;
    int messageBoxResult = -1;
    String messageBoxString = "";

    private ControlP5 ctrlP5;

    DialogBox(ControlP5 _ctrlP5)
    {
        ctrlP5 = _ctrlP5;
    }

    void toggleBox(int theValue) {
        if (messageBox.isVisible()) {
            messageBox.hide();
        } else {
            messageBox.show();
        }
    }

    @Override
    protected void finalize() throws Throwable {

        ctrlP5.remove(messageBox);
        ctrlP5.remove("Name Scene");
        ctrlP5.remove("inputbox");

        super.finalize();
    }

    void createMessageBox() {
        
        
        // create a group to store the messageBox elements
        messageBox = ctrlP5.addGroup("messageBox", 700, 600, 300);
        messageBox.setBackgroundHeight(120);
        messageBox.setBackgroundColor(ctrlP5.papplet.color(0, 100));
        messageBox.hideBar();

        // add a TextLabel to the messageBox.
        Textlabel l = ctrlP5.addTextlabel("Name Scene", "Please enter Name for scene", 20, 20);
        l.moveTo(messageBox);

        // add a textfield-controller with named-id inputbox
        // this controller will be linked to function inputbox() below.
        Textfield f = ctrlP5.addTextfield("inputbox", 20, 36, 260, 20);
        f.getCaptionLabel().setVisible(false);
        f.moveTo(messageBox);
        f.setColorForeground(ctrlP5.papplet.color(20));
        f.setColorBackground(ctrlP5.papplet.color(20));
        f.setColorActive(ctrlP5.papplet.color(100));
        // add the OK button to the messageBox.
        // the name of the button corresponds to function buttonOK
        // below and will be triggered when pressing the button.
        Button b1 = ctrlP5.addButton("buttonOK", 0, 65, 80, 80, 24);
        b1.moveTo(messageBox);
        b1.setColorBackground(ctrlP5.papplet.color(40));
        b1.setColorActive(ctrlP5.papplet.color(20));
        // by default setValue would trigger function buttonOK,
        // therefore we disable the broadcasting before setting
        // the value and enable broadcasting again afterwards.
        // same applies to the cancel button below.
        b1.setBroadcast(false);
        b1.setValue(1);
        b1.setBroadcast(true);
        b1.setCaptionLabel("OK");

        b1.onRelease(new CallbackListener() {
            @Override
            public void controlEvent(CallbackEvent callbackEvent) {
                messageBoxString = ((Textfield) ctrlP5.getController("inputbox")).getText();
                messageBox.hide();

                onFinished(((int) callbackEvent.getController().getValue()),messageBoxString);
            }
        });
        // centering of a label needs to be done manually
        // with marginTop and marginLeft
        //b1.captionLabel().style().marginTop = -2;
        //b1.captionLabel().style().marginLeft = 26;

        // add the Cancel button to the messageBox.
        // the name of the button corresponds to function buttonCancel
        // below and will be triggered when pressing the button.
        Button b2 = ctrlP5.addButton("buttonCancel");
        b2.setPosition( 155, 80);
        b2.setSize(80, 24);
        b2.moveTo(messageBox);
        b2.setBroadcast(false);
        b2.setValue(0);
        b2.setBroadcast(true);
        b2.setCaptionLabel("Cancel");
        b2.setColorBackground(ctrlP5.papplet.color(40));
        b2.setColorActive(ctrlP5.papplet.color(20));

        b2.onRelease(new CallbackListener() {
            @Override
            public void controlEvent(CallbackEvent callbackEvent) {
                messageBox.hide();
                onFinished(((int) callbackEvent.getController().getValue()),"");
            }
        });
        //b2.captionLabel().toUpperCase(false);
        // centering of a label needs to be done manually
        // with marginTop and marginLeft
        //b2.captionLabel().style().marginTop = -2;
        //b2.captionLabel().style().marginLeft = 16;
        messageBox.hide();
    }

    //has to be overridden
    abstract void onFinished(int result, String resultAnswer);

    // inputbox is called whenever RETURN has been pressed
    // in textfield-controller inputbox
    void inputbox(String theString) {
    //        println("got something from the inputbox : " + theString);
        messageBoxString = theString;
        messageBox.hide();

        //FIXME is result of 1 correct for OK?
        onFinished(1,messageBoxString);

    }
}
