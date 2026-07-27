#include "night_light.h"

namespace adk {

    static constexpr uint16_t maximumPermille = 1000;

    NightLightInput::NightLightInput (uint16_t         lightPermille,
                                      LightSampleState sampleState) noexcept
        : lightPermille (lightPermille)
        , sampleState   (sampleState)
    {
    }

    NightLight::NightLight (const NightLightConfig& config) noexcept
        : config_         (config)
        , mode_           (NightLightMode::Off)
        , sampleState_    (LightSampleState::Valid)
        , status_         (StatusCode::NotInitialized)
        , lightPermille_  (0)
        , outputDuty_     (0)
        , initialized_    (false)
    {
    }

    Status NightLight::initialize () noexcept
    {
        if (!configValid ())
        {
            status_      = StatusCode::InvalidArgument;
            initialized_ = false;
            return status_;
        }

        mode_           = NightLightMode::Off;
        sampleState_    = LightSampleState::Valid;
        status_         = StatusCode::Ok;
        lightPermille_  = 0;
        outputDuty_     = 0;
        initialized_    = true;

        return status_;
    }

    Status NightLight::update (const NightLightInput& input) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (input.sampleState != LightSampleState::Valid)
        {
            enterFault (input);
            return status_;
        }

        if (input.lightPermille > maximumPermille)
        {
            mode_          = NightLightMode::Fault;
            sampleState_   = LightSampleState::AboveRange;
            status_        = StatusCode::InvalidArgument;
            lightPermille_ = input.lightPermille;
            outputDuty_    = 0;
            return status_;
        }

        sampleState_   = LightSampleState::Valid;
        status_        = StatusCode::Ok;
        lightPermille_ = input.lightPermille;

        if (mode_ == NightLightMode::Fault)
        {
            mode_ = NightLightMode::Off;
        }

        if (mode_ == NightLightMode::Off &&
            lightPermille_ < config_.turnOnBelowPermille)
        {
            mode_ = NightLightMode::On;
        } else if (mode_ == NightLightMode::On &&
                   lightPermille_ > config_.turnOffAbovePermille)
        {
            mode_ = NightLightMode::Off;
        }

        outputDuty_ = mode_ == NightLightMode::On
                    ? chooseDuty (lightPermille_)
                    : 0;

        return status_;
    }

    NightLightSnapshot NightLight::snapshot () const noexcept
    {
        NightLightDiagnostic diagnostic = NightLightDiagnostic::Ready;

        if (mode_ == NightLightMode::On)
        {
            diagnostic = NightLightDiagnostic::Active;
        } else if (mode_ == NightLightMode::Fault)
        {
            diagnostic = NightLightDiagnostic::SensorFault;
        }

        const NightLightSnapshot result = {
            mode_,
            sampleState_,
            diagnostic,
            status_,
            lightPermille_,
            outputDuty_,
            mode_ == NightLightMode::On};

        return result;
    }

    bool NightLight::configValid () const noexcept
    {
        return config_.turnOnBelowPermille < config_.turnOffAbovePermille &&
               config_.turnOffAbovePermille <= maximumPermille &&
               config_.minimumDuty > 0 &&
               config_.minimumDuty <= config_.maximumDuty;
    }

    uint8_t NightLight::chooseDuty (uint16_t lightPermille) const noexcept
    {
        const uint16_t boundedLight = lightPermille < config_.turnOffAbovePermille
                                    ? lightPermille
                                    : config_.turnOffAbovePermille;
        const uint16_t darkness = config_.turnOffAbovePermille - boundedLight;
        const uint16_t dutyRange = config_.maximumDuty - config_.minimumDuty;
        const uint32_t scaled = static_cast<uint32_t> (dutyRange) * darkness /
                                config_.turnOffAbovePermille;

        return static_cast<uint8_t> (config_.minimumDuty + scaled);
    }

    void NightLight::enterFault (const NightLightInput& input) noexcept
    {
        mode_           = NightLightMode::Fault;
        sampleState_    = input.sampleState;
        status_         = StatusCode::HardwareFailure;
        lightPermille_  = input.lightPermille;
        outputDuty_     = 0;
    }
}
