import ej32.Forth;
import ej32.ESP32;

class ESP32Test
{
    public static void blink() {
        int led = ESP32.LED_BUILTIN;
        ESP32.pinMode(led, ESP32.OUTPUT);
        for (int i=0; i<10; i++) {
            ESP32.digitalWrite(led, ESP32.HIGH);
            Forth.delay(100);
            ESP32.digitalWrite(led, ESP32.LOW);
            Forth.delay(100);
        }
    }
    public static void main()
    {
        Forth.interpreter();
    }                                   // b1
}
