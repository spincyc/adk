#pragma once

#include "seven_segment_glyph.h"
#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {
    // clang-format off

    enum struct DigitSelectPolarity : uint8_t
    {
        ActiveHigh,
        ActiveLow
    };

    enum struct MultiplexedDigitFault : uint8_t
    {
        None,
        RefreshLost,
        LifecycleExhausted
    };

    enum struct MultiplexedDigitStage : uint8_t
    {
        BlankSelects,
        LoadSegments,
        SelectDigit
    };

    enum struct MultiplexedDigitDiagnosticGlyph : uint8_t
    {
        None,
        SegmentA,
        SegmentB,
        SegmentC,
        SegmentD,
        SegmentE,
        SegmentF,
        SegmentG,
        DecimalPoint,
        DigitIdentification,
        AllOn,
        AllOff
    };

    struct MultiplexedDigitConfig
    {
        MultiplexedDigitConfig (uint32_t ownerToken, uint16_t configurationRevision,
                                SevenSegmentPolarity segmentPolarity,
                                DigitSelectPolarity digitSelectPolarity,
                                Duration servicePeriod = Duration     (1),
                                Duration maximumServiceGap = Duration (4)) noexcept;

        uint32_t              ownerToken;
        uint16_t              configurationRevision;
        SevenSegmentPolarity  segmentPolarity;
        DigitSelectPolarity   digitSelectPolarity;
        Duration              servicePeriod;
        Duration              maximumServiceGap;
    };

    struct MultiplexedDigitFrame
    {
        SevenSegmentGlyph                 glyphs[4];
        MultiplexedDigitDiagnosticGlyph   diagnosticGlyphs[4];
        uint8_t                           decimalMask;
        uint32_t                          sourceSnapshotSequence;
        uint32_t                          generation;
        bool                              overflow;
    };

    struct MultiplexedDigitTransaction
    {
        uint32_t                ownerToken;
        uint16_t                configurationRevision;
        uint32_t                lifecycleGeneration;
        uint32_t                frameGeneration;
        uint32_t                sourceSnapshotSequence;
        TimePoint               emittedAt;
        uint8_t                 digitIndex;
        MultiplexedDigitStage   stages[3];
        uint8_t                 segmentLevels[3];
        uint8_t                 digitSelectLevels[3];
        MultiplexedDigitFault   fault;
        bool                    emitted;
    };

    struct MultiplexedDigitSnapshot
    {
        MultiplexedDigitFrame activeFrame;
        MultiplexedDigitFrame pendingFrame;
        uint32_t              lifecycleGeneration;
        uint8_t               nextDigitIndex;
        MultiplexedDigitFault fault;
        Status                status;
        bool                  pending;
        bool                  initialized;
    };

    struct MultiplexedDigitPolicy;
    struct MultiplexedDigitPolicyTestAccess;

    struct MultiplexedDigitPreview
    {
        MultiplexedDigitPreview () noexcept;

      private:
        const MultiplexedDigitPolicy* owner;
        uint32_t                      lifecycleGeneration;
        uint32_t                      baseFrameGeneration;
        MultiplexedDigitFrame         frame;

        friend struct MultiplexedDigitPolicy;
    };

    // Pure presentation policy. It owns no pins, timer, endpoint, or display.
    struct MultiplexedDigitPolicy
    {
        explicit MultiplexedDigitPolicy (
            const MultiplexedDigitConfig& config) noexcept;

        MultiplexedDigitPolicy (const MultiplexedDigitPolicy&)            = delete;
        MultiplexedDigitPolicy& operator= (const MultiplexedDigitPolicy&) = delete;
        MultiplexedDigitPolicy (MultiplexedDigitPolicy&&)                 = delete;
        MultiplexedDigitPolicy& operator= (MultiplexedDigitPolicy&&)      = delete;

        Status initialize (TimePoint now) noexcept;
        void   reset      (TimePoint now) noexcept;
        void   shutdown   () noexcept;

        Status preview (uint32_t value, bool showLeadingZeros, uint8_t decimalMask,
                        uint32_t sourceSnapshotSequence,
                        MultiplexedDigitPreview& candidate) const noexcept;
        Status previewDiagnostic (
            MultiplexedDigitDiagnosticGlyph glyph, uint8_t digitMask,
            uint32_t sourceSnapshotSequence,
            MultiplexedDigitPreview& candidate) const noexcept;
        bool   canCommit (const MultiplexedDigitPreview& candidate) const noexcept;
        Status commit    (const MultiplexedDigitPreview& candidate) noexcept;

        Result<MultiplexedDigitTransaction> refresh (TimePoint now) noexcept;

        bool                       initialized () const noexcept;
        MultiplexedDigitSnapshot   snapshot    () const noexcept;

      private:
        MultiplexedDigitConfig   config_;
        MultiplexedDigitFrame    activeFrame_;
        MultiplexedDigitFrame    pendingFrame_;
        TimePoint                lastServiceAt_;
        uint32_t                 lifecycleGeneration_;
        uint8_t                  nextDigitIndex_;
        uint8_t                  lastSegmentLevels_;
        MultiplexedDigitFault    fault_;
        Status                   status_;
        bool                     initialized_;
        bool                     pending_;
        bool                     firstServiceArmed_;

        friend struct MultiplexedDigitPolicyTestAccess;
    };
    // clang-format on
} // namespace adk
