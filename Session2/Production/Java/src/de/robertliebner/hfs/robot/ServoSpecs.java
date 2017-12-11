package de.robertliebner.hfs.robot;

public class ServoSpecs {

    int feedback_min, feedback_max,pwm_min,pwm_max;

    public ServoSpecs( int feedback_min, int feedback_max, int pwm_min, int pwm_max) {

        this.feedback_min = feedback_min;
        this.feedback_max = feedback_max;
        this.pwm_min = pwm_min;
        this.pwm_max = pwm_max;
    }

    public int getFeedback_min() {
        return feedback_min;
    }

    public int getFeedback_max() {
        return feedback_max;
    }

    public int getPwm_min() {
        return pwm_min;
    }

    public int getPwm_max() {
        return pwm_max;
    }
}
