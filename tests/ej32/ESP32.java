package ej32;

public class ESP32 {
    // constants
    public static final int LED_BUILTIN  = 2;
    public static final int INPUT        = 0;
    public static final int OUTPUT       = 1;
    public static final int INPUT_PULLUP = 2;
    public static final int LOW          = 0;
    public static final int HIGH         = 1;
    // methods
    public static void pinMode(int pin, int mode)    {}
    public static int  digitalRead(int pin)          { return 0; }
    public static void digitalWrite(int pin, int v)  {}
    public static int  analogRead(int pin)           { return 0; }
    public static void analogWrite(int pin, int v)   {}
    public static void ledcAttachPin(int pin, int v) {}
    public static void ledcSetup(int ch, int freq, int v) {}
    public static void ledcWriteTone(int ch, int v)  {}
}
