#pragma once

#include "digital.h"
#include "keypad.h"
#include "resource.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct MatrixKeypadPinMode : uint8_t
    {
        HighImpedance,
        InputPullUp,
        Output
    };

    struct MatrixKeypadIo
    {
        virtual ~MatrixKeypadIo () noexcept;

        virtual Status setMode (PinId pin, MatrixKeypadPinMode mode) noexcept = 0;
        virtual Status write   (PinId pin, bool high) noexcept                 = 0;
        virtual Status read    (PinId pin, bool& high) noexcept                = 0;
    };

    struct MatrixKeypadPins
    {
        PinId row0;
        PinId row1;
        PinId row2;
        PinId row3;
        PinId column0;
        PinId column1;
        PinId column2;
    };

    struct MatrixKeypad
    {
        MatrixKeypad  (ResourceRegistry&       resources,
                       const MatrixKeypadPins& pins,
                       const KeypadConfig&     config = KeypadConfig (),
                       MatrixKeypadIo*         io = nullptr) noexcept;
        ~MatrixKeypad () noexcept;

        MatrixKeypad& operator= (const MatrixKeypad&) = delete;
        MatrixKeypad  (const MatrixKeypad&)            = delete;
        MatrixKeypad& operator= (MatrixKeypad&&)       = delete;
        MatrixKeypad  (MatrixKeypad&&)                 = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (TimePoint now) noexcept;

        MatrixKeypadPins pins        () const noexcept;
        KeypadSnapshot   snapshot    () const noexcept;
        bool             initialized () const noexcept;

      private:
        bool     pinsValid     () const noexcept;
        void     makeRowsSafe  () noexcept;
        void     makePinsSafe  () noexcept;
        void     releaseClaims () noexcept;
        Status   scan          (uint16_t& pressedMask) noexcept;

        ResourceRegistry* resources_;
        MatrixKeypadPins  pins_;
        ResourceClaim     claims_[7];
        Keypad            keypad_;
        MatrixKeypadIo*   io_;
        bool              initialized_;
    };
}
