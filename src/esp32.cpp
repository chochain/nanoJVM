#include "ucode.h"

#define CODE(s, g)  { s, [](Thread &t){ g; }, ACL_BUILTIN }
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
    CODE("pinMode",      DU m = POP; pinMode(POP, m)),
    CODE("digitalRead",  PUSH(digitalRead(POP))),
    CODE("digitalWrite", DU v = POP; digitalWrite(POP, v)),
    CODE("analogRead",   PUSH(analogRead(POP))),
    CODE("analogWrite",  DU v = POP; analogWrite(POP, v, 255)),
    CODE("ledcAttachPin",DU v = POP; ledcAttachPin(POP, v)),
    CODE("ledcSetup",    DU v = POP; DU freq=POP; ledcSetup(POP, freq, v)),
    CODE("ledcWriteTone",DU v = POP; ledcWriteTone(POP, v)),
#else
    /// for debugging
    CODE("pinMode",      DU m = POP; DU p = POP),
    CODE("digitalRead",  DU p = POP; PUSH(t.IP)),
    CODE("digitalWrite", DU v = POP; DU p = POP),
    CODE("analogRead",   DU p = POP; PUSH(t.IP)),
    CODE("analogWrite",  DU v = POP; DU p = POP),
    CODE("ledcAttachPin",DU v = POP; DU p = POP),
    CODE("ledcSetup",    DU v = POP; DU freq=POP; DU p = POP),
    CODE("ledcWriteTone",DU v = POP; DU p = POP)
#endif // ARDUINO
    /// @}
};
///
/// ESP32 built-in word in ROM, use extern by main program
///
Ucode uESP32(VTSZ(_esp32), _esp32);


