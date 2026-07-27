#include "mega_pulse_capture_io.h"

#include <Arduino.h>

#if defined(__AVR__)
#include <avr/interrupt.h>
#include <avr/io.h>
#endif

namespace adk {

    namespace {
        MegaPulseCaptureIo* owners[6] = {};

        uint8_t interruptFor (PinId pin) noexcept
        {
#if defined(ARDUINO)
            const int interrupt = digitalPinToInterrupt (pin);
            return interrupt < 0 ? UINT8_MAX : static_cast<uint8_t> (interrupt);
#else
            switch (pin)
            {
                case 2: return 0;
                case 3: return 1;
                case 21: return 2;
                case 20: return 3;
                case 19: return 4;
                case 18: return 5;
                default: return UINT8_MAX;
            }
#endif
        }

#if defined(ARDUINO)
        using InterruptHandler = void (*) ();

        const InterruptHandler handlers[6] = {megaPulseInterrupt0, megaPulseInterrupt1,
                                              megaPulseInterrupt2, megaPulseInterrupt3,
                                              megaPulseInterrupt4, megaPulseInterrupt5};
#endif
    } // namespace

    MegaPulseCaptureIo::MegaPulseCaptureIo () noexcept
        : capture_ (nullptr), pin_ (0), interrupt_ (UINT8_MAX), interruptState_ (0),
          started_ (false)
    {
    }

    MegaPulseCaptureIo::~MegaPulseCaptureIo () noexcept
    {
        stop ();
    }

    Status MegaPulseCaptureIo::start (PinId pin, PulseCapture& capture) noexcept
    {
        if (started_)
        {
            return StatusCode::InvalidArgument;
        }

        const uint8_t interrupt = interruptFor (pin);
        if (interrupt == UINT8_MAX)
        {
            return StatusCode::Unsupported;
        }
        if (owners[interrupt])
        {
            return StatusCode::ResourceBusy;
        }

#if defined(ARDUINO)
        pinMode (pin, INPUT);
        capture_          = &capture;
        pin_              = pin;
        interrupt_        = interrupt;
        owners[interrupt] = this;
        attachInterrupt (interrupt, handlers[interrupt], CHANGE);
        started_ = true;
        return StatusCode::Ok;
#else
        (void)capture;
        return StatusCode::Unsupported;
#endif
    }

    void MegaPulseCaptureIo::stop () noexcept
    {
        if (!started_)
        {
            return;
        }

#if defined(ARDUINO)
        detachInterrupt (interrupt_);
        pinMode         (pin_, INPUT);
#endif
        owners[interrupt_] = nullptr;
        capture_           = nullptr;
        interrupt_         = UINT8_MAX;
        started_           = false;
    }

    bool MegaPulseCaptureIo::inputHigh () const noexcept
    {
#if defined(ARDUINO)
        return started_ && digitalRead (pin_) == HIGH;
#else
        return true;
#endif
    }

    MicrosecondTimePoint MegaPulseCaptureIo::now () const noexcept
    {
#if defined(ARDUINO)
        return MicrosecondTimePoint (micros ());
#else
        return MicrosecondTimePoint ();
#endif
    }

    void MegaPulseCaptureIo::lock () noexcept
    {
#if defined(__AVR__)
        interruptState_ = SREG;
        cli ();
#endif
    }

    void MegaPulseCaptureIo::unlock () noexcept
    {
#if defined(__AVR__)
        SREG = interruptState_;
#endif
    }

    void MegaPulseCaptureIo::handleInterrupt () noexcept
    {
#if defined(ARDUINO)
        if (capture_)
        {
            capture_->recordActiveLowEdge (MicrosecondTimePoint (micros ()),
                                           digitalRead (pin_) == HIGH);
        }
#endif
    }

    void megaPulseInterrupt0 () noexcept
    {
        if (owners[0])
        {
            owners[0]->handleInterrupt ();
        }
    }

    void megaPulseInterrupt1 () noexcept
    {
        if (owners[1])
        {
            owners[1]->handleInterrupt ();
        }
    }

    void megaPulseInterrupt2 () noexcept
    {
        if (owners[2])
        {
            owners[2]->handleInterrupt ();
        }
    }

    void megaPulseInterrupt3 () noexcept
    {
        if (owners[3])
        {
            owners[3]->handleInterrupt ();
        }
    }

    void megaPulseInterrupt4 () noexcept
    {
        if (owners[4])
        {
            owners[4]->handleInterrupt ();
        }
    }

    void megaPulseInterrupt5 () noexcept
    {
        if (owners[5])
        {
            owners[5]->handleInterrupt ();
        }
    }
} // namespace adk
