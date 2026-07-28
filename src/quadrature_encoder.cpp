#include "quadrature_encoder.h"

#include "board.h"

#include <limits.h>

namespace adk {

    QuadratureEncoderConfig::QuadratureEncoderConfig (PinId phaseA,
                                                      PinId phaseB,
                                                      Pull  pull,
                                                      bool  reversed) noexcept
        : phaseA   (phaseA)
        , phaseB   (phaseB)
        , pull     (pull)
        , reversed (reversed)
    {
    }

    QuadratureEncoder::QuadratureEncoder (
        ResourceRegistry&              resources,
        const QuadratureEncoderConfig& config) noexcept
        : config_   (config)
        , phaseA_   (resources, config.phaseA, config.pull)
        , phaseB_   (resources, config.phaseB, config.pull)
        , snapshot_ {0,
                     0,
                     Rotation::None,
                     0,
                     0,
                     false,
                     StatusCode::NotInitialized}
    {
    }

    QuadratureEncoder::~QuadratureEncoder () noexcept
    {
        shutdown ();
    }

    Status QuadratureEncoder::initialize () noexcept
    {
        if (initialized ())
        {
            return StatusCode::Ok;
        }

        if (config_.phaseA == config_.phaseB)
        {
            snapshot_.status = StatusCode::InvalidArgument;
            return snapshot_.status;
        }

        if (!Mega2560Board::validPin (config_.phaseA) ||
            !Mega2560Board::validPin (config_.phaseB))
        {
            snapshot_.status = StatusCode::InvalidPin;
            return snapshot_.status;
        }

        if (!Mega2560Board::supports (config_.phaseA,
                                      PinCapability::DigitalInput) ||
            !Mega2560Board::supports (config_.phaseB,
                                      PinCapability::DigitalInput))
        {
            snapshot_.status = StatusCode::Unsupported;
            return snapshot_.status;
        }

        if (!validPull ())
        {
            snapshot_.status = StatusCode::InvalidConfiguration;
            return snapshot_.status;
        }

        Status status = phaseA_.initialize ();

        if (!status.ok ())
        {
            snapshot_.status = status;
            return status;
        }

        status = phaseB_.initialize ();

        if (!status.ok ())
        {
            phaseA_.shutdown ();
            snapshot_.status = status;
            return status;
        }

        snapshot_.phaseMask = currentPhase ();
        snapshot_.delta     = 0;
        snapshot_.rotation  = Rotation::None;
        snapshot_.status    = StatusCode::Ok;
        return StatusCode::Ok;
    }

    void QuadratureEncoder::shutdown () noexcept
    {
        phaseB_.shutdown ();
        phaseA_.shutdown ();

        snapshot_.delta    = 0;
        snapshot_.rotation = Rotation::None;
        snapshot_.status   = StatusCode::NotInitialized;
    }

    Status QuadratureEncoder::update () noexcept
    {
        if (!initialized ())
        {
            snapshot_.delta    = 0;
            snapshot_.rotation = Rotation::None;
            snapshot_.status   = StatusCode::NotInitialized;
            return snapshot_.status;
        }

        phaseA_.update ();
        phaseB_.update ();

        const uint8_t previous = snapshot_.phaseMask;
        const uint8_t current = currentPhase ();
        int8_t        delta   = transition   (previous, current);

        snapshot_.phaseMask = current;
        snapshot_.delta     = 0;
        snapshot_.rotation  = Rotation::None;
        snapshot_.status    = StatusCode::Ok;

        if (delta == 0)
        {
            if (current != previous)
            {
                if (snapshot_.invalidTransitions != UINT16_MAX)
                {
                    ++snapshot_.invalidTransitions;
                }
            }

            return StatusCode::Ok;
        }

        if (config_.reversed)
        {
            delta = static_cast<int8_t> (-delta);
        }

        snapshot_.delta = delta;
        snapshot_.rotation =
            delta > 0 ? Rotation::Clockwise : Rotation::CounterClockwise;
        applyDelta (delta);
        return StatusCode::Ok;
    }

    void QuadratureEncoder::resetPosition (int32_t position) noexcept
    {
        snapshot_.position          = position;
        snapshot_.positionSaturated = false;
    }

    bool QuadratureEncoder::initialized () const noexcept
    {
        return phaseA_.initialized () && phaseB_.initialized ();
    }

    QuadratureEncoderSnapshot QuadratureEncoder::snapshot () const noexcept
    {
        return snapshot_;
    }

    const DigitalInput& QuadratureEncoder::phaseAInput () const noexcept
    {
        return phaseA_;
    }

    const DigitalInput& QuadratureEncoder::phaseBInput () const noexcept
    {
        return phaseB_;
    }

    bool QuadratureEncoder::validPull () const noexcept
    {
        return config_.pull == Pull::None || config_.pull == Pull::Up;
    }

    uint8_t QuadratureEncoder::currentPhase () const noexcept
    {
        const uint8_t phaseA = phaseA_.read () == Level::High ? 2U : 0U;
        const uint8_t phaseB = phaseB_.read () == Level::High ? 1U : 0U;

        return static_cast<uint8_t> (phaseA | phaseB);
    }

    int8_t QuadratureEncoder::transition (uint8_t previous,
                                          uint8_t current) const noexcept
    {
        static const int8_t transitions[16] = {
             0,  1, -1,  0,
            -1,  0,  0,  1,
             1,  0,  0, -1,
             0, -1,  1,  0
        };

        return transitions[(previous << 2U) | current];
    }

    void QuadratureEncoder::applyDelta (int8_t delta) noexcept
    {
        if (delta > 0)
        {
            if (snapshot_.position == INT32_MAX)
            {
                snapshot_.positionSaturated = true;
                return;
            }

            ++snapshot_.position;
            return;
        }

        if (snapshot_.position == INT32_MIN)
        {
            snapshot_.positionSaturated = true;
            return;
        }

        --snapshot_.position;
    }
}
