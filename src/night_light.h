#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    enum struct NightLightMode : uint8_t
    {
        Off,
        On,
        Fault
    };

    enum struct LightSampleState : uint8_t
    {
        Valid,
        BelowRange,
        AboveRange,
        Stale
    };

    enum struct NightLightDiagnostic : uint8_t
    {
        Ready,
        Active,
        SensorFault
    };

    struct NightLightInput
    {
        NightLightInput (uint16_t         lightPermille = 0,
                         LightSampleState sampleState   = LightSampleState::Valid) noexcept;

        uint16_t         lightPermille;
        LightSampleState sampleState;
    };

    struct NightLightConfig
    {
        uint16_t turnOnBelowPermille  = 350;
        uint16_t turnOffAbovePermille = 450;
        uint8_t  minimumDuty          = 24;
        uint8_t  maximumDuty          = 192;
    };

    struct NightLightSnapshot
    {
        NightLightMode       mode;
        LightSampleState     sampleState;
        NightLightDiagnostic diagnostic;
        Status               status;
        uint16_t             lightPermille;
        uint8_t              outputDuty;
        bool                 lampOn;
    };

    struct NightLight
    {
        explicit NightLight (const NightLightConfig& config) noexcept;

        Status initialize () noexcept;
        Status update     (const NightLightInput& input) noexcept;

        NightLightSnapshot snapshot () const noexcept;

      private:
        bool    configValid  () const noexcept;
        uint8_t chooseDuty   (uint16_t lightPermille) const noexcept;
        void    enterFault   (const NightLightInput& input) noexcept;

        NightLightConfig config_;
        NightLightMode   mode_;
        LightSampleState sampleState_;
        Status           status_;
        uint16_t         lightPermille_;
        uint8_t          outputDuty_;
        bool             initialized_;
    };
}
