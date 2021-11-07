#include "ucode.h"
#include "mmu.h"

#define CODE(s, g)  { s, [](Thread &t){ g; }, 0 }
#define POP         t.pop()
#define PUSH(v)     t.push(v)
#if ESP32
#define analogWrite(c,v,mx) ledcWrite((c),(8191/mx)*min((int)(v),mx))
#endif // ESP32
static Method _esp32[] = {
#if ARDUINO && ESP32
    ///
    /// @definegroup ESP32 supporting functions
    /// @{
    CODE("pinMode",      DU p = POP; pinMode(p, POP)),
    CODE("digitalRead",  PUSH(digitalRead(POP))),
    CODE("digitalWrite", DU p = POP; digitalWrite(p, POP)),
    CODE("analogRead",   PUSH(analogRead(POP))),
    CODE("analogWrite",  DU p  = POP; analogWrite(p, POP, 255)),
    CODE("ledcAttachPin",DU p  = POP; ledcAttachPin(p, POP)),
    CODE("ledcSetup",    DU ch = POP; DU freq=POP; ledcSetup(ch, freq, POP)),
    CODE("ledcWriteTone",DU ch = POP; ledcWriteTone(ch, POP)),
    /// @}
#endif // ARDUINO
};
///
/// ESP32 built-in word in ROM, use extern by main program
///
Ucode uESP32(sizeof(_esp32)/sizeof(Method), _esp32);


