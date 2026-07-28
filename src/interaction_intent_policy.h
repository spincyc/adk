#pragma once

#include "contact_dynamics.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {
    // clang-format off

    enum struct InteractionSourceKind : uint8_t
    {
        SyntheticFixture,
        CopiedContact,
        CopiedJoystick
    };

    struct InteractionSource
    {
        InteractionSourceKind kind;
        uint8_t               sourceId;
        uint16_t              configurationRevision;
    };

    struct DirectionalEvidence
    {
        InteractionSource source;
        TimePoint         observedAt;
        uint32_t          sequence;
        int16_t           xPermille;
        int16_t           yPermille;
        bool              saturated;
        Status            status;
    };

    enum struct InteractionDirection : uint8_t
    {
        Neutral,
        North,
        NorthEast,
        East,
        SouthEast,
        South,
        SouthWest,
        West,
        NorthWest
    };

    enum struct InteractionQuality : uint8_t
    {
        Invalid,
        Current,
        Stale,
        SourceFault,
        TimingFault,
        StuckActive
    };

    struct InteractionIntentConfig
    {
        InteractionIntentConfig (const ContactDynamicsConfig& contact,
                                 Duration                     maximumContactAge,
                                 Duration                     maximumDirectionalAge,
                                 uint16_t                     engageMagnitudePermille,
                                 uint16_t releaseMagnitudePermille) noexcept;

        ContactDynamicsConfig contact;
        Duration              maximumContactAge;
        Duration              maximumDirectionalAge;
        uint16_t              engageMagnitudePermille;
        uint16_t              releaseMagnitudePermille;
    };

    struct InteractionIntent
    {
        InteractionSource    contactSource;
        InteractionSource    directionalSource;
        TimePoint            observedAt;
        uint32_t             contactSequence;
        uint32_t             directionalSequence;
        InteractionDirection direction;
        uint16_t             magnitudePermille;
        bool                 touchActive;
        bool                 touchEvent;
        bool                 touchReleaseEvent;
        bool                 directionEvent;
        InteractionQuality   quality;
        Duration             contactAge;
        Duration             directionalAge;
        bool                 directionalSaturated;
        ContactQuality       contactQuality;
        Status               contactStatus;
        Status               directionalStatus;
        Status               status;
    };

    struct InteractionIntentPolicy;

    struct InteractionIntentPreview
    {
        InteractionIntentPreview () noexcept;

      private:
        const InteractionIntentPolicy* owner;
        uint32_t                       generation;
        TimePoint                      now;
        InteractionSource              contactSource;
        uint32_t                       contactSequence;
        ContactSample                  contact;
        DirectionalEvidence            directional;
        InteractionDirection           direction;
        uint16_t                       magnitudePermille;
        bool                           directionEvent;
        bool                           contactRepeat;
        bool                           directionalRepeat;
        bool                           contactDomainChanged;
        bool                           directionalDomainChanged;
        bool                           contactRecovery;
        bool                           recoveryBaseline;

        friend struct InteractionIntentPolicy;
    };

    // Pure copied-evidence policy; it owns no endpoint or shutdown action.
    struct InteractionIntentPolicy
    {
        explicit InteractionIntentPolicy (
            const InteractionIntentConfig& config) noexcept;

        InteractionIntentPolicy (const InteractionIntentPolicy&)            = delete;
        InteractionIntentPolicy& operator= (const InteractionIntentPolicy&) = delete;
        InteractionIntentPolicy (InteractionIntentPolicy&&)                 = delete;
        InteractionIntentPolicy& operator= (InteractionIntentPolicy&&)      = delete;

        Status initialize () noexcept;
        void   reset      () noexcept;
        Status preview    (TimePoint now, const InteractionSource& contactSource,
                           uint32_t contactSequence, const ContactSample& contact,
                           const DirectionalEvidence& directional,
                           InteractionIntentPreview&  candidate) const noexcept;
        bool   canCommit  (const InteractionIntentPreview& candidate) const noexcept;
        Status commit     (const InteractionIntentPreview& candidate) noexcept;

        bool              initialized () const noexcept;
        InteractionIntent snapshot    () const noexcept;

      private:
        ContactDynamics         contact_;
        InteractionIntentConfig config_;
        InteractionIntent       intent_;
        InteractionSource       lastContactSource_;
        InteractionSource       lastDirectionalSource_;
        ContactSample           lastContact_;
        DirectionalEvidence     lastDirectional_;
        uint32_t                lastContactSequence_;
        uint32_t                generation_;
        bool                    initialized_;
        bool                    hasContact_;
        bool                    hasDirectional_;
    };
    // clang-format on
} // namespace adk
