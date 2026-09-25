#pragma once

#include "object.h"

namespace adk {

    // An infrared receiver module (KY-022, or a bare VS1838B) and the kit's
    // remote control, which sends NEC codes. Wire S to an interrupt pin (2,
    // 3, 18, 19, 20 or 21), + to 5 V and - to GND.
    //
    // The module's output is low while infrared arrives. An interrupt times
    // every edge and decodes the code as it comes, so the sketch never waits
    // for one. Each receiver has its own interrupt, so several can be used.
    struct IrReceiver : Object
    {
        IrReceiver (Pin pin);

        // A code arrived in this update: a button press, or a repeat, which
        // the remote sends about nine times a second while it is held.
        bool wasReceived () const;
        bool isRepeat    () const;

        // The latest button's command byte, such as remote::power, and the
        // address of the remote that sent it. A repeat keeps both.
        uint8_t  command () const;
        uint16_t address () const;

      protected:
        void setup  () override;
        void update (Millis now) override;

      private:
        template <uint8_t Interrupt>
        static void onEdge ();

        void edge   ();
        void space  (uint32_t width);
        void finish (uint32_t now);

        // The decoder, run by the interrupt.
        uint32_t lastEdge_;
        uint32_t heldAt_;
        uint32_t bits_;
        uint8_t  phase_;
        uint8_t  count_;
        bool     held_;

        // The latest code, passed from the interrupt to the next update.
        volatile uint16_t heardAddress_;
        volatile uint8_t  heardCommand_;
        volatile uint8_t  heard_;

        // What the sketch sees until the next update.
        uint16_t address_;
        uint8_t  command_;
        Pin      pin_;
        bool     received_;
        bool     repeat_;
    };

    // The command bytes of the kit's 21-button remote. Other remotes send
    // other codes: print command () to find theirs.
    namespace remote {

        inline constexpr uint8_t power      = 0x45;
        inline constexpr uint8_t volumeUp   = 0x46;
        inline constexpr uint8_t stop       = 0x47;   // FUNC/STOP
        inline constexpr uint8_t back       = 0x44;
        inline constexpr uint8_t play       = 0x40;   // play/pause
        inline constexpr uint8_t forward    = 0x43;
        inline constexpr uint8_t down       = 0x07;
        inline constexpr uint8_t volumeDown = 0x15;
        inline constexpr uint8_t up         = 0x09;
        inline constexpr uint8_t eq         = 0x19;
        inline constexpr uint8_t repeat     = 0x0D;   // ST/REPT
        inline constexpr uint8_t digit0     = 0x16;
        inline constexpr uint8_t digit1     = 0x0C;
        inline constexpr uint8_t digit2     = 0x18;
        inline constexpr uint8_t digit3     = 0x5E;
        inline constexpr uint8_t digit4     = 0x08;
        inline constexpr uint8_t digit5     = 0x1C;
        inline constexpr uint8_t digit6     = 0x5A;
        inline constexpr uint8_t digit7     = 0x42;
        inline constexpr uint8_t digit8     = 0x52;
        inline constexpr uint8_t digit9     = 0x4A;

        // The number a digit button stands for, 0 to 9, or -1 for any other
        // button.
        constexpr int digitOf (uint8_t command)
        {
            constexpr uint8_t digits [] = {digit0, digit1, digit2, digit3, digit4,
                                           digit5, digit6, digit7, digit8, digit9};

            for (int digit = 0; digit < 10; ++digit)
            {
                if (digits[digit] == command)
                {
                    return digit;
                }
            }

            return -1;
        }
    }
}
