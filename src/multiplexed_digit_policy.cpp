#include "multiplexed_digit_policy.h"

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        bool validConfig (const MultiplexedDigitConfig& config) noexcept
        {
            const uint32_t period = config.servicePeriod.milliseconds     ();
            const uint32_t gap    = config.maximumServiceGap.milliseconds ();
            return config.ownerToken != 0 && config.configurationRevision != 0 &&
                   validSevenSegmentPolarity (config.segmentPolarity) &&
                   (config.digitSelectPolarity == DigitSelectPolarity::ActiveHigh ||
                    config.digitSelectPolarity == DigitSelectPolarity::ActiveLow) &&
                   period != 0 && period < halfRange && gap >= period &&
                   gap < halfRange;
        }

        MultiplexedDigitFrame blankFrame () noexcept
        {
            return {{SevenSegmentGlyph::Blank, SevenSegmentGlyph::Blank,
                     SevenSegmentGlyph::Blank, SevenSegmentGlyph::Blank},
                    {MultiplexedDigitDiagnosticGlyph::None,
                     MultiplexedDigitDiagnosticGlyph::None,
                     MultiplexedDigitDiagnosticGlyph::None,
                     MultiplexedDigitDiagnosticGlyph::None},
                    0, 0, 0, false};
        }

        bool validDiagnosticGlyph (
            MultiplexedDigitDiagnosticGlyph glyph) noexcept
        {
            return glyph >= MultiplexedDigitDiagnosticGlyph::SegmentA &&
                   glyph <= MultiplexedDigitDiagnosticGlyph::AllOff;
        }

        uint8_t encodeDiagnosticGlyph (
            MultiplexedDigitDiagnosticGlyph glyph,
            SevenSegmentPolarity polarity) noexcept
        {
            uint8_t levels = 0;
            switch (glyph)
            {
            case MultiplexedDigitDiagnosticGlyph::SegmentA: levels = 0x01U; break;
            case MultiplexedDigitDiagnosticGlyph::SegmentB: levels = 0x02U; break;
            case MultiplexedDigitDiagnosticGlyph::SegmentC: levels = 0x04U; break;
            case MultiplexedDigitDiagnosticGlyph::SegmentD: levels = 0x08U; break;
            case MultiplexedDigitDiagnosticGlyph::SegmentE: levels = 0x10U; break;
            case MultiplexedDigitDiagnosticGlyph::SegmentF: levels = 0x20U; break;
            case MultiplexedDigitDiagnosticGlyph::SegmentG: levels = 0x40U; break;
            case MultiplexedDigitDiagnosticGlyph::DecimalPoint:
                levels = 0x80U;
                break;
            case MultiplexedDigitDiagnosticGlyph::DigitIdentification:
                levels = 0x7fU;
                break;
            case MultiplexedDigitDiagnosticGlyph::AllOn: levels = 0xffU; break;
            case MultiplexedDigitDiagnosticGlyph::AllOff: break;
            case MultiplexedDigitDiagnosticGlyph::None: break;
            }
            return polarity == SevenSegmentPolarity::CommonAnode
                       ? static_cast<uint8_t> (~levels)
                       : levels;
        }

        MultiplexedDigitTransaction noTransaction (
            const MultiplexedDigitConfig& config, uint32_t lifecycleGeneration,
            MultiplexedDigitFault fault, TimePoint now) noexcept
        {
            return {config.ownerToken,
                    config.configurationRevision,
                    lifecycleGeneration,
                    0,
                    0,
                    now,
                    0,
                    {MultiplexedDigitStage::BlankSelects,
                     MultiplexedDigitStage::LoadSegments,
                     MultiplexedDigitStage::SelectDigit},
                    {0, 0, 0},
                    {0, 0, 0},
                    fault,
                    false};
        }

        uint8_t inactiveSelects (DigitSelectPolarity polarity) noexcept
        {
            return polarity == DigitSelectPolarity::ActiveHigh ? 0x00U : 0x0fU;
        }

        uint8_t selectedDigit (DigitSelectPolarity polarity, uint8_t digit) noexcept
        {
            const uint8_t bit = static_cast<uint8_t> (1U << digit);
            return polarity == DigitSelectPolarity::ActiveHigh
                       ? bit
                       : static_cast<uint8_t> (0x0fU & static_cast<uint8_t> (~bit));
        }

        uint8_t encodedSegments (const MultiplexedDigitConfig& config,
                                 const MultiplexedDigitFrame& frame,
                                 uint8_t digit) noexcept
        {
            if (frame.diagnosticGlyphs[digit] !=
                MultiplexedDigitDiagnosticGlyph::None)
            {
                return encodeDiagnosticGlyph (frame.diagnosticGlyphs[digit],
                                              config.segmentPolarity);
            }
            const uint8_t decimalBit = static_cast<uint8_t> (1U << (3U - digit));
            return encodeSevenSegmentGlyph (
                frame.glyphs[digit], config.segmentPolarity,
                (frame.decimalMask & decimalBit) != 0);
        }
    } // namespace

    MultiplexedDigitConfig::MultiplexedDigitConfig (
        uint32_t ownerTokenValue, uint16_t configurationRevisionValue,
        SevenSegmentPolarity segmentPolarityValue,
        DigitSelectPolarity digitSelectPolarityValue, Duration servicePeriodValue,
        Duration maximumServiceGapValue) noexcept
        : ownerToken            (ownerTokenValue),
          configurationRevision (configurationRevisionValue),
          segmentPolarity       (segmentPolarityValue),
          digitSelectPolarity   (digitSelectPolarityValue),
          servicePeriod         (servicePeriodValue),
          maximumServiceGap     (maximumServiceGapValue)
    {
    }

    MultiplexedDigitPreview::MultiplexedDigitPreview () noexcept
        : owner               (nullptr),
          lifecycleGeneration (0),
          baseFrameGeneration (0),
          frame               (blankFrame ())
    {
    }

    MultiplexedDigitPolicy::MultiplexedDigitPolicy (
        const MultiplexedDigitConfig& config) noexcept
        : config_              (config),
          activeFrame_         (blankFrame ()),
          pendingFrame_        (blankFrame ()),
          lastServiceAt_       (),
          lifecycleGeneration_ (0),
          nextDigitIndex_      (0),
          lastSegmentLevels_   (0),
          fault_               (MultiplexedDigitFault::None),
          status_              (StatusCode::NotInitialized),
          initialized_         (false),
          pending_             (false),
          firstServiceArmed_   (false)
    {
    }

    Status MultiplexedDigitPolicy::initialize (TimePoint now) noexcept
    {
        if (fault_ == MultiplexedDigitFault::LifecycleExhausted)
        {
            return StatusCode::CapacityExceeded;
        }
        if (initialized_)
        {
            return status_;
        }
        if (!validConfig (config_))
        {
            status_ = StatusCode::InvalidConfiguration;
            return status_;
        }

        initialized_ = true;
        reset (now);
        return status_;
    }

    void MultiplexedDigitPolicy::reset (TimePoint now) noexcept
    {
        if (fault_ == MultiplexedDigitFault::LifecycleExhausted)
        {
            return;
        }
        if (initialized_)
        {
            ++lifecycleGeneration_;
            if (lifecycleGeneration_ == 0)
            {
                activeFrame_       = blankFrame ();
                pendingFrame_      = blankFrame ();
                nextDigitIndex_    = 0;
                lastSegmentLevels_ = encodeSevenSegmentGlyph (
                    SevenSegmentGlyph::Blank, config_.segmentPolarity);
                fault_             = MultiplexedDigitFault::LifecycleExhausted;
                status_            = StatusCode::CapacityExceeded;
                pending_           = false;
                firstServiceArmed_ = false;
                return;
            }
        }
        activeFrame_       = blankFrame ();
        pendingFrame_      = blankFrame ();
        lastServiceAt_     = now;
        nextDigitIndex_    = 0;
        lastSegmentLevels_ = initialized_
                                 ? encodeSevenSegmentGlyph (
                                       SevenSegmentGlyph::Blank,
                                       config_.segmentPolarity)
                                 : 0;
        fault_             = MultiplexedDigitFault::None;
        status_            = initialized_ ? StatusCode::Ok : StatusCode::NotInitialized;
        pending_           = false;
        firstServiceArmed_ = initialized_;
    }

    void MultiplexedDigitPolicy::shutdown () noexcept
    {
        if (fault_ == MultiplexedDigitFault::LifecycleExhausted)
        {
            activeFrame_       = blankFrame ();
            pendingFrame_      = blankFrame ();
            nextDigitIndex_    = 0;
            lastSegmentLevels_ = encodeSevenSegmentGlyph (
                SevenSegmentGlyph::Blank, config_.segmentPolarity);
            status_            = StatusCode::CapacityExceeded;
            initialized_       = false;
            pending_           = false;
            firstServiceArmed_ = false;
            return;
        }
        activeFrame_       = blankFrame ();
        pendingFrame_      = blankFrame ();
        nextDigitIndex_    = 0;
        lastSegmentLevels_ = 0;
        fault_             = MultiplexedDigitFault::None;
        status_            = StatusCode::NotInitialized;
        initialized_       = false;
        pending_           = false;
        firstServiceArmed_ = false;
    }

    Status MultiplexedDigitPolicy::preview (
        uint32_t value, bool showLeadingZeros, uint8_t decimalMask,
        uint32_t sourceSnapshotSequence,
        MultiplexedDigitPreview& candidate) const noexcept
    {
        candidate.owner = nullptr;
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (fault_ != MultiplexedDigitFault::None || sourceSnapshotSequence == 0 ||
            (decimalMask & 0xf0U) != 0)
        {
            return StatusCode::InvalidArgument;
        }

        MultiplexedDigitFrame frame = blankFrame ();
        frame.sourceSnapshotSequence = sourceSnapshotSequence;
        frame.generation             = activeFrame_.generation + 1U;
        if (frame.generation == 0)
        {
            return StatusCode::CapacityExceeded;
        }

        if (value > 9999U)
        {
            frame.overflow   = true;
            frame.decimalMask = 0;
            for (uint8_t index = 0; index < 4; ++index)
            {
                frame.glyphs[index] = SevenSegmentGlyph::Dash;
            }
        }
        else
        {
            frame.decimalMask = decimalMask;
            uint32_t remaining = value;
            for (uint8_t offset = 0; offset < 4; ++offset)
            {
                const uint8_t index = static_cast<uint8_t> (3U - offset);
                frame.glyphs[index] =
                    static_cast<SevenSegmentGlyph> (remaining % 10U);
                remaining /= 10U;
            }
            if (!showLeadingZeros)
            {
                for (uint8_t index = 0; index < 3; ++index)
                {
                    if (frame.glyphs[index] != SevenSegmentGlyph::Zero)
                    {
                        break;
                    }
                    frame.glyphs[index] = SevenSegmentGlyph::Blank;
                }
            }
        }

        candidate.owner               = this;
        candidate.lifecycleGeneration = lifecycleGeneration_;
        candidate.baseFrameGeneration = activeFrame_.generation;
        candidate.frame               = frame;
        return StatusCode::Ok;
    }

    Status MultiplexedDigitPolicy::previewDiagnostic (
        MultiplexedDigitDiagnosticGlyph glyph, uint8_t digitMask,
        uint32_t sourceSnapshotSequence,
        MultiplexedDigitPreview& candidate) const noexcept
    {
        candidate.owner = nullptr;
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (fault_ != MultiplexedDigitFault::None ||
            !validDiagnosticGlyph (glyph) || digitMask == 0 ||
            (digitMask & 0xf0U) != 0 ||
            sourceSnapshotSequence == 0)
        {
            return StatusCode::InvalidArgument;
        }

        MultiplexedDigitFrame frame = blankFrame ();
        frame.sourceSnapshotSequence = sourceSnapshotSequence;
        frame.generation             = activeFrame_.generation + 1U;
        if (frame.generation == 0)
        {
            return StatusCode::CapacityExceeded;
        }
        if (glyph != MultiplexedDigitDiagnosticGlyph::AllOff)
        {
            for (uint8_t digit = 0; digit < 4; ++digit)
            {
                if ((digitMask & static_cast<uint8_t> (1U << digit)) != 0)
                {
                    frame.diagnosticGlyphs[digit] = glyph;
                }
            }
        }

        candidate.owner               = this;
        candidate.lifecycleGeneration = lifecycleGeneration_;
        candidate.baseFrameGeneration = activeFrame_.generation;
        candidate.frame               = frame;
        return StatusCode::Ok;
    }

    bool MultiplexedDigitPolicy::canCommit (
        const MultiplexedDigitPreview& candidate) const noexcept
    {
        return initialized_ && fault_ == MultiplexedDigitFault::None &&
               !pending_ && candidate.owner == this &&
               candidate.lifecycleGeneration == lifecycleGeneration_ &&
               candidate.baseFrameGeneration == activeFrame_.generation &&
               candidate.frame.generation == activeFrame_.generation + 1U &&
               candidate.frame.generation != 0;
    }

    Status MultiplexedDigitPolicy::commit (
        const MultiplexedDigitPreview& candidate) noexcept
    {
        if (!canCommit (candidate))
        {
            return pending_ ? StatusCode::ResourceBusy : StatusCode::InvalidArgument;
        }
        pendingFrame_ = candidate.frame;
        pending_      = true;
        return StatusCode::Ok;
    }

    Result<MultiplexedDigitTransaction>
    MultiplexedDigitPolicy::refresh (TimePoint now) noexcept
    {
        MultiplexedDigitTransaction transaction =
            noTransaction (config_, lifecycleGeneration_, fault_, now);
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, transaction};
        }
        if (fault_ != MultiplexedDigitFault::None)
        {
            return {status_, transaction};
        }

        const uint32_t elapsed = now.elapsedSince (lastServiceAt_).milliseconds ();
        if (elapsed >= halfRange)
        {
            return {StatusCode::InvalidArgument, transaction};
        }
        if (elapsed > config_.maximumServiceGap.milliseconds ())
        {
            fault_  = MultiplexedDigitFault::RefreshLost;
            status_ = StatusCode::Timeout;
            transaction.fault = fault_;
            const uint8_t blank = encodeSevenSegmentGlyph (
                SevenSegmentGlyph::Blank, config_.segmentPolarity);
            transaction.segmentLevels[0] = lastSegmentLevels_;
            transaction.segmentLevels[1] = blank;
            transaction.segmentLevels[2] = blank;
            transaction.digitSelectLevels[0] =
                inactiveSelects (config_.digitSelectPolarity);
            transaction.digitSelectLevels[1] =
                transaction.digitSelectLevels[0];
            transaction.digitSelectLevels[2] =
                transaction.digitSelectLevels[0];
            transaction.emitted = true;
            activeFrame_ = blankFrame ();
            lastSegmentLevels_ = blank;
            return {status_, transaction};
        }
        if (!(firstServiceArmed_ && elapsed == 0) &&
            elapsed < config_.servicePeriod.milliseconds ())
        {
            return {StatusCode::Ok, transaction};
        }

        if (nextDigitIndex_ == 0 && pending_)
        {
            activeFrame_  = pendingFrame_;
            pendingFrame_ = blankFrame ();
            pending_      = false;
        }

        const uint8_t segment = encodedSegments (config_, activeFrame_,
                                                 nextDigitIndex_);
        const uint8_t inactive = inactiveSelects (config_.digitSelectPolarity);
        transaction.frameGeneration       = activeFrame_.generation;
        transaction.sourceSnapshotSequence =
            activeFrame_.sourceSnapshotSequence;
        transaction.digitIndex            = nextDigitIndex_;
        transaction.segmentLevels[0]      = lastSegmentLevels_;
        transaction.segmentLevels[1]      = segment;
        transaction.segmentLevels[2]      = segment;
        transaction.digitSelectLevels[0]  = inactive;
        transaction.digitSelectLevels[1]  = inactive;
        transaction.digitSelectLevels[2]  =
            selectedDigit (config_.digitSelectPolarity, nextDigitIndex_);
        transaction.emitted               = true;

        nextDigitIndex_ = static_cast<uint8_t> ((nextDigitIndex_ + 1U) % 4U);
        lastServiceAt_  = now;
        lastSegmentLevels_ = segment;
        firstServiceArmed_ = false;
        return {StatusCode::Ok, transaction};
    }

    bool MultiplexedDigitPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    MultiplexedDigitSnapshot MultiplexedDigitPolicy::snapshot () const noexcept
    {
        return {activeFrame_, pendingFrame_, lifecycleGeneration_, nextDigitIndex_,
                fault_, status_, pending_, initialized_};
    }
    // clang-format on
} // namespace adk
