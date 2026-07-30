#pragma once

#include "qualified_18b20_probe_set_policy.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {

    enum struct ThermalGradientHealth : uint8_t
    {
        Qualifying,
        Stable,
        Gradient,
        Disagreement,
        Fault
    };

    enum struct ThermalGradientQuality : uint8_t
    {
        Unqualified,
        Flat,
        Rising,
        Falling,
        Indeterminate,
        Fault
    };

    enum struct ThermalMapperPageKind : uint8_t
    {
        Overall,
        Probe,
        AdjacentGradient
    };

    struct ThermalMapperControl
    {
        uint32_t  ownerToken;
        uint8_t   sourceId;
        uint16_t  configurationRevision;
        uint32_t  sequence;
        TimePoint observedAt;
        bool      nextEdge;
        bool      recordEdge;
        Status    status;
    };

    struct ThermalGradientPair
    {
        uint8_t                leftSlot;
        uint8_t                rightSlot;
        int32_t                lowerRawSixteenths;
        int32_t                upperRawSixteenths;
        ThermalGradientQuality quality;
        uint8_t                faultMask;
    };

    struct ThermalMapperProbeIntent
    {
        OneWireRomCode      rom;
        int16_t             lowerRawSixteenths;
        int16_t             upperRawSixteenths;
        Ds18b20Resolution   resolution;
        Ds18b20ProbeQuality quality;
        Duration            age;
        Status              status;
    };

    struct ThermalGradientIntent
    {
        // Cells outside probeCount and gradientCount remain canonical zero.
        ThermalMapperProbeIntent probes[4];
        ThermalGradientPair      gradients[3];
        OneWireRomCode           minimumRom;
        OneWireRomCode           maximumRom;
        int16_t                  minimumLowerRawSixteenths;
        int16_t                  maximumUpperRawSixteenths;
        uint8_t                  minimumTieMask;
        uint8_t                  maximumTieMask;
        ThermalGradientHealth    health;
        ThermalMapperPageKind    pageKind;
        uint8_t                  pageIndex;
        // Only the selector named by pageKind may be nonzero.
        uint8_t selectedSlot;
        uint8_t selectedGradient;
        uint8_t probeCount;
        uint8_t gradientCount;
        uint8_t overallFaultMask;
        uint8_t ledSelectionMask;
        bool    lcdShowsIdentity;
        bool    lcdShowsAgeOrFault;
        bool    outputsInactive;
    };

    struct ThermalMapperConfig
    {
        uint32_t       ownerToken;
        uint16_t       configurationRevision;
        uint8_t        expectedSetSourceId;
        uint16_t       expectedSetConfigurationRevision;
        OneWireRomCode sourceRoms[4];
        OneWireRomCode spatialOrder[4];
        uint8_t        spatialCount;
        uint32_t       expectedControlOwnerToken;
        uint8_t        expectedControlSourceId;
        uint16_t       expectedControlConfigurationRevision;
        Duration       maximumControlAge;
        uint16_t       meaningfulGradientRawSixteenths;
    };

    struct ThermalMapperEnvelope
    {
        TimePoint                now;
        QualifiedDs18b20Snapshot probes;
        ThermalMapperControl     control;
    };

    struct ThermalMapperRecordProbe
    {
        OneWireRomCode      rom;
        int16_t             lowerRawSixteenths;
        int16_t             upperRawSixteenths;
        Ds18b20Resolution   resolution;
        Ds18b20ProbeQuality quality;
        Duration            age;
        uint32_t            conversionGeneration;
        uint32_t            readTransactionGeneration;
        Status              status;
    };

    struct ThermalMapperRecordImage
    {
        // Cells outside probeCount and gradientCount remain canonical zero.
        ThermalMapperRecordProbe probes[4];
        ThermalGradientPair      gradients[3];
        uint32_t                 ownerToken;
        uint32_t                 lifecycleGeneration;
        uint16_t                 configurationRevision;
        uint32_t                 recordSequence;
        uint32_t                 recordEdgeOwnerToken;
        uint8_t                  recordEdgeSourceId;
        uint16_t                 recordEdgeConfigurationRevision;
        uint32_t                 recordEdgeSequence;
        TimePoint                recordEdgeObservedAt;
        uint8_t                  setSourceId;
        uint16_t                 setConfigurationRevision;
        uint32_t                 setCycleSequence;
        TimePoint                setObservedAt;
        TimePoint                mappedAt;
        OneWireRomCode           sourceRoms[4];
        uint32_t                 witnessDigest;
        uint8_t                  formatVersion;
        uint8_t                  probeCount;
        uint8_t                  gradientCount;
        ThermalGradientHealth    health;
        uint8_t                  faultMask;
    };

    struct ThermalMapperResult
    {
        ThermalGradientIntent    intent;
        ThermalMapperRecordImage record;
        bool                     hasRecord;
        Status                   status;
    };

#if defined(ADK_TESTING)
    struct ThermalGradientMapperTestAccess;
#endif

    // Structurally validates copied evidence; it cannot authenticate a producer.
    struct ThermalGradientMapper
    {
        explicit ThermalGradientMapper (const ThermalMapperConfig& config) noexcept;

        ThermalGradientMapper (const ThermalGradientMapper&)            = delete;
        ThermalGradientMapper& operator= (const ThermalGradientMapper&) = delete;
        ThermalGradientMapper (ThermalGradientMapper&&)                 = delete;
        ThermalGradientMapper& operator= (ThermalGradientMapper&&)      = delete;

        Status initialize (TimePoint now) noexcept;
        Status reset      (TimePoint now) noexcept;
        Status update     (const ThermalMapperEnvelope& envelope,
                       ThermalMapperResult&         result) noexcept;
        Status shutdown () noexcept;

        Status snapshot    (ThermalGradientIntent& intent) const noexcept;
        bool   initialized () const noexcept;

      private:
#if defined(ADK_TESTING)
        friend struct ThermalGradientMapperTestAccess;
#endif

        ThermalMapperConfig      config_;
        ThermalGradientIntent    intent_;
        QualifiedDs18b20Snapshot lastProbes_;
        ThermalMapperControl     lastControl_;
        TimePoint                lastUpdateAt_;
        uint32_t                 lifecycleGeneration_;
        uint32_t                 nextRecordSequence_;
        bool                     initialized_;
        bool                     generationExhausted_;
        bool                     recordSequenceExhausted_;
        bool                     hasProbes_;
        bool                     hasControl_;
    };
} // namespace adk
