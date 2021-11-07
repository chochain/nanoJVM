#include "ucode.h"
#include "mmu.h"

#define CODE(s, g)  { s, [](Thread &t){ g; }, 0 }
#define POP         t.pop()
#define PUSH(v)     t.push(v)
#if ESP32
#define analogWrite(c,v,mx) ledcWrite((c),(8191/mx)*min((int)(v),mx))
#endif // ESP32
static Method _esp32[] = {
    ///
    /// @definegroup ESP32 supporting functions
    /// @{
#if ARDUINO && ESP32
    CODE("pinMode",      DU p = POP; pinMode(p, POP)),
    CODE("digitalRead",  PUSH(digitalRead(POP))),
    CODE("digitalWrite", DU p = POP; digitalWrite(p, POP)),
    CODE("analogRead",   PUSH(analogRead(POP))),
    CODE("analogWrite",  DU p  = POP; analogWrite(p, POP, 255)),
    CODE("ledcAttachPin",DU p  = POP; ledcAttachPin(p, POP)),
    CODE("ledcSetup",    DU ch = POP; DU freq=POP; ledcSetup(ch, freq, POP)),
    CODE("ledcWriteTone",DU ch = POP; ledcWriteTone(ch, POP)),
#else
    /// for debugging
    CODE("pinMode",      DU p = POP; DU m = POP),
    CODE("digitalRead",  DU p = POP; PUSH(t.IP)),
    CODE("digitalWrite", DU p = POP; DU v = POP),
    CODE("analogRead",   DU p = POP; PUSH(t.IP)),
    CODE("analogWrite",  DU p  = POP; DU v = POP),
    CODE("ledcAttachPin",DU p  = POP; DU v = POP),
    CODE("ledcSetup",    DU ch = POP; DU freq=POP; DU v = POP),
    CODE("ledcWriteTone",DU ch = POP; DU v = POP)
#endif // ARDUINO
    /// @}
};
///
/// ESP32 built-in word in ROM, use extern by main program
///
Ucode uESP32(sizeof(_esp32)/sizeof(Method), _esp32);


