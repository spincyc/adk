#include "serial_port.h"

// Weak, so that naming a port here links it into a sketch only if the
// sketch uses it itself: each port has buffers taking 157 bytes of RAM.
extern HardwareSerial Serial1 __attribute__ ((weak));
extern HardwareSerial Serial2 __attribute__ ((weak));
extern HardwareSerial Serial3 __attribute__ ((weak));

namespace adk {

    bool claimSerial (HardwareSerial& port)
    {
        Pin tx;

        if (&port == &Serial1)
        {
            tx = 18;
        }
        else if (&port == &Serial2)
        {
            tx = 16;
        }
        else if (&port == &Serial3)
        {
            tx = 14;
        }
        else
        {
            return refuse (Fault::NotSerial, 1);
        }

        return claimOutput (tx, true) && claimInput (static_cast<Pin> (tx + 1));
    }
}
