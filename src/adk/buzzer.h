#pragma once

#include "digital.h"

namespace adk {

    // An active buzzer: it makes its own tone whenever its pin is high.
    struct Buzzer : Object
    {
        Buzzer (Pin pin, Polarity polarity = ActiveHigh);

        void on   ();
        void off  ();
        void beep (Millis duration);
        bool isOn () const;

      protected:
        void setup  () override;
        void update (Millis now) override;
        void stop   () override;

      private:
        void sound (bool on);

        Millis   beepStart_;
        Millis   beepLength_;
        Pin      pin_;
        Polarity polarity_;
        bool     on_;
        bool     starting_;
    };
}
