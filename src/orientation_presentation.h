#pragma once

#include "inertial_observation.h"
#include "signed_axis_mapping.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    struct BoardFrame
    {
        SignedAxis right;
        SignedAxis forward;
        SignedAxis up;
    };

    enum struct OrientationQuality : uint8_t
    {
        Invalid,
        Unsteady,
        Level,
        Tilted,
        BeyondPresentationRange
    };

    enum struct BalanceDirection : uint8_t
    {
        None,
        Forward,
        Backward,
        Left,
        Right
    };

    struct OrientationConfig
    {
        BoardFrame boardFrame;
        int32_t    minimumGravityMicroG;
        int32_t    maximumGravityMicroG;
        int32_t    maximumStationaryRateMilliDegreesPerSecond;
        int32_t    levelThresholdMilliDegrees;
        int32_t    maximumPresentationAngleMilliDegrees;
    };

    struct OrientationEstimate
    {
        int32_t            pitchMilliDegrees;
        int32_t            rollMilliDegrees;
        OrientationQuality quality;
        Status             status;
    };

    Status validateOrientationConfig (const OrientationConfig& config) noexcept;

    struct OrientationPolicy;

    struct PreparedOrientationEstimate
    {
        PreparedOrientationEstimate () noexcept;

        const OrientationEstimate& result () const noexcept;

      private:
        friend struct OrientationPolicy;

        OrientationEstimate       result_;
        const OrientationPolicy*  owner_;
        uint32_t                  generation_;
    };

    // Pure gravity-only policy; it owns no sensor, transport, or clock.
    struct OrientationPolicy
    {
        explicit OrientationPolicy (const OrientationConfig& config) noexcept;

        OrientationPolicy (const OrientationPolicy&)            = delete;
        OrientationPolicy& operator= (const OrientationPolicy&) = delete;
        OrientationPolicy (OrientationPolicy&&)                 = delete;
        OrientationPolicy& operator= (OrientationPolicy&&)      = delete;

        Status              initialize  () noexcept;
        void                reset       () noexcept;
        Status              preview     (const InertialObservation& input,
                                         PreparedOrientationEstimate& prepared)
            const
            noexcept;
        // Derivation status and transaction admission are independent.
        bool                canCommit   (
                           const PreparedOrientationEstimate& prepared) const
            noexcept;
        Status              commit      (
                           const PreparedOrientationEstimate& prepared) noexcept;
        Status              update      (const InertialObservation& input) noexcept;
        OrientationEstimate snapshot    () const noexcept;
        bool                initialized () const noexcept;

      private:
        OrientationConfig   config_;
        OrientationEstimate estimate_;
        uint32_t             generation_;
        bool                initialized_;
    };

    struct BalanceLightIntent
    {
        uint16_t redPermille;
        uint16_t greenPermille;
        uint16_t bluePermille;
        bool     fault;
    };

    struct BalanceToneIntent
    {
        bool     enabled;
        uint16_t frequencyHertz;
        uint16_t durationMilliseconds;
    };

    struct BalancePresentationConfig
    {
        BalanceLightIntent level;
        BalanceLightIntent forward;
        BalanceLightIntent backward;
        BalanceLightIntent left;
        BalanceLightIntent right;
        BalanceLightIntent unsteadyPhaseA;
        BalanceLightIntent unsteadyPhaseB;
        BalanceLightIntent beyondRange;
        BalanceLightIntent invalid;
        int32_t            fullScaleAngleMilliDegrees;
        uint16_t           minimumTiltIntensityPermille;
        uint16_t           maximumTiltIntensityPermille;
        uint16_t           directionChangeFrequencyHertz;
        uint16_t           directionChangeDurationMilliseconds;
    };

    struct BalancePresentation
    {
        BalanceDirection   direction;
        BalanceLightIntent light;
        BalanceToneIntent  tone;
        Status             status;
    };

    Status validateBalancePresentationConfig (
        const BalancePresentationConfig& config) noexcept;

    struct BalancePresentationPolicy;

    struct PreparedBalancePresentation
    {
        PreparedBalancePresentation () noexcept;

        const BalancePresentation& result () const noexcept;

      private:
        friend struct BalancePresentationPolicy;

        BalancePresentation               result_;
        const BalancePresentationPolicy*  owner_;
        uint32_t                          generation_;
    };

    // Pure intent policy; a caller owns all physical presentation endpoints.
    struct BalancePresentationPolicy
    {
        explicit BalancePresentationPolicy (
            const BalancePresentationConfig& config) noexcept;

        BalancePresentationPolicy (const BalancePresentationPolicy&) = delete;
        BalancePresentationPolicy&
        operator= (const BalancePresentationPolicy&)                       = delete;
        BalancePresentationPolicy (BalancePresentationPolicy&&)            = delete;
        BalancePresentationPolicy& operator= (BalancePresentationPolicy&&) = delete;

        Status              initialize  () noexcept;
        void                reset       () noexcept;
        Status              preview     (const OrientationEstimate& estimate,
                                         uint16_t sensitivityPermille,
                                         bool diagnosticPhase,
                                         PreparedBalancePresentation& prepared)
            const
            noexcept;
        // Admitted safe-state classifications remain committable.
        bool                canCommit   (
                           const PreparedBalancePresentation& prepared) const
            noexcept;
        Status              commit      (
                           const PreparedBalancePresentation& prepared) noexcept;
        Status              update      (const OrientationEstimate& estimate,
                                         uint16_t sensitivityPermille,
                                         bool diagnosticPhase) noexcept;
        BalancePresentation snapshot    () const noexcept;
        bool                initialized () const noexcept;

      private:
        BalancePresentationConfig config_;
        BalancePresentation       presentation_;
        BalanceDirection          previousDirection_;
        uint32_t                  generation_;
        bool                      initialized_;
    };
} // namespace adk
