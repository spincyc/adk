#include "dual_display_timing_desk.h"

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange       = 0x80000000UL;
        constexpr uint32_t stopwatchLimit  = 599900UL;
        constexpr uint32_t lapDuration     = 2000UL;

        bool identityEqual (const TimingDeskControlIdentity& left,
                            const TimingDeskControlIdentity& right) noexcept
        {
            return left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sessionEpoch == right.sessionEpoch;
        }

        bool validIdentity (const TimingDeskControlIdentity& value) noexcept
        {
            return value.sourceId != 0 && value.configurationRevision != 0 &&
                   value.sessionEpoch != 0;
        }

        bool validDuration (Duration value, uint32_t minimum,
                            uint32_t maximum) noexcept
        {
            const uint32_t milliseconds = value.milliseconds ();
            return milliseconds >= minimum && milliseconds <= maximum &&
                   milliseconds < halfRange;
        }

        bool validConfig (const DualDisplayTimingDeskConfig& config) noexcept
        {
            return config.ownerToken != 0 && config.configurationRevision != 0 &&
                   validIdentity  (config.startPauseSource) &&
                   validIdentity  (config.lapSource) &&
                   validIdentity  (config.resetSource) &&
                   !identityEqual (config.startPauseSource, config.lapSource) &&
                   !identityEqual (config.startPauseSource, config.resetSource) &&
                   !identityEqual (config.lapSource, config.resetSource) &&
                   validDuration  (config.presentationGrace, 1, 1000) &&
                   validDuration  (config.selfTestTimeout, 1, 1000) &&
                   validDuration  (config.controlFreshness, 1, 1000);
        }

        bool frameEqual (const MultiplexedDigitFrame& left,
                         const MultiplexedDigitFrame& right) noexcept
        {
            for (uint8_t index = 0; index < 4; ++index)
            {
                if (left.glyphs[index] != right.glyphs[index])
                {
                    return false;
                }
                if (left.diagnosticGlyphs[index] !=
                    right.diagnosticGlyphs[index])
                {
                    return false;
                }
            }
            return left.decimalMask == right.decimalMask &&
                   left.sourceSnapshotSequence ==
                       right.sourceSnapshotSequence &&
                   left.generation == right.generation &&
                   left.overflow == right.overflow;
        }

        bool frameEqual (const Max7219Frame& left,
                         const Max7219Frame& right) noexcept
        {
            for (uint8_t index = 0; index < 8; ++index)
            {
                if (left.rows[index] != right.rows[index])
                {
                    return false;
                }
            }
            return left.sourceSnapshotSequence ==
                       right.sourceSnapshotSequence &&
                   left.generation == right.generation;
        }

        void digestByte (uint32_t& digest, uint8_t value) noexcept
        {
            digest ^= value;
            digest *= 0x01000193UL;
        }

        void digestUint32 (uint32_t& digest, uint32_t value) noexcept
        {
            digestByte (digest, static_cast<uint8_t> (value));
            digestByte (digest, static_cast<uint8_t> (value >> 8U));
            digestByte (digest, static_cast<uint8_t> (value >> 16U));
            digestByte (digest, static_cast<uint8_t> (value >> 24U));
        }

        uint32_t digitDigest (const MultiplexedDigitFrame& frame,
                              uint8_t mode) noexcept
        {
            static const char domain[] = "ADK.DIGIT.FRAME.V1";
            uint32_t digest = 0x811c9dc5UL;
            for (uint8_t index = 0; index < sizeof (domain) - 1U; ++index)
            {
                digestByte (digest, static_cast<uint8_t> (domain[index]));
            }
            for (uint8_t index = 0; index < 4; ++index)
            {
                digestByte (digest, static_cast<uint8_t> (frame.glyphs[index]));
                digestByte (
                    digest,
                    static_cast<uint8_t> (frame.diagnosticGlyphs[index]));
            }
            digestByte   (digest, frame.decimalMask);
            digestByte   (digest, frame.overflow ? 1U : 0U);
            digestUint32 (digest, frame.sourceSnapshotSequence);
            digestUint32 (digest, frame.generation);
            digestByte   (digest, mode);
            return digest;
        }

        uint32_t matrixDigest (const Max7219Frame& frame, uint8_t mode) noexcept
        {
            static const char domain[] = "ADK.MATRIX.FRAME.V1";
            uint32_t digest = 0x811c9dc5UL;
            for (uint8_t index = 0; index < sizeof (domain) - 1U; ++index)
            {
                digestByte (digest, static_cast<uint8_t> (domain[index]));
            }
            for (uint8_t index = 0; index < 8; ++index)
            {
                digestByte (digest, frame.rows[index]);
            }
            digestUint32 (digest, frame.sourceSnapshotSequence);
            digestUint32 (digest, frame.generation);
            digestByte   (digest, mode);
            return digest;
        }

        MultiplexedDigitFrame blankDigitFrame () noexcept
        {
            return {{SevenSegmentGlyph::Blank, SevenSegmentGlyph::Blank,
                     SevenSegmentGlyph::Blank, SevenSegmentGlyph::Blank},
                    {MultiplexedDigitDiagnosticGlyph::None,
                     MultiplexedDigitDiagnosticGlyph::None,
                     MultiplexedDigitDiagnosticGlyph::None,
                     MultiplexedDigitDiagnosticGlyph::None},
                    0, 0, 0, false};
        }

        Max7219Frame blankMatrixFrame () noexcept
        {
            return {{0, 0, 0, 0, 0, 0, 0, 0}, 0, 0};
        }

        MultiplexedDigitTransaction noDigitTransaction () noexcept
        {
            return {0, 0, 0, 0, 0, TimePoint (), 0,
                    {MultiplexedDigitStage::BlankSelects,
                     MultiplexedDigitStage::LoadSegments,
                     MultiplexedDigitStage::SelectDigit},
                    {0, 0, 0}, {0, 0, 0}, MultiplexedDigitFault::None, false};
        }

        Max7219Command noMatrixCommand () noexcept
        {
            return {0, 0, 0, 0, 0, 0, 0,
                    Max7219Operation::Configure, false};
        }

        DualDisplayTimingDeskResult noResult (
            TimingDeskPresentationDisposition disposition) noexcept
        {
            return {StatusCode::Ok, disposition, noDigitTransaction (),
                    noMatrixCommand (), 0, 0, 0, false, false, false, false};
        }

        uint32_t elapsedAt (TimingDeskStopwatchState state, Duration materialized,
                            TimePoint runStartedAt, TimePoint now) noexcept
        {
            uint32_t elapsed = materialized.milliseconds ();
            if (state == TimingDeskStopwatchState::Running)
            {
                const uint32_t increment =
                    now.elapsedSince (runStartedAt).milliseconds ();
                if (increment >= halfRange || increment > stopwatchLimit ||
                    elapsed > stopwatchLimit - increment)
                {
                    return stopwatchLimit;
                }
                elapsed += increment;
            }
            return elapsed > stopwatchLimit ? stopwatchLimit : elapsed;
        }

        uint32_t timingValue (uint32_t elapsed) noexcept
        {
            const uint32_t minute  = elapsed / 60000UL;
            const uint32_t seconds = (elapsed / 1000UL) % 60UL;
            const uint32_t tenths  = (elapsed / 100UL) % 10UL;
            return static_cast<uint32_t> (
                minute * 1000UL + seconds * 10UL + tenths);
        }

        void setPixel (uint8_t rows[8], uint8_t row, uint8_t column) noexcept
        {
            rows[row] |= static_cast<uint8_t> (0x80U >> column);
        }

        void ordinaryMatrix (uint32_t elapsed, TimingDeskStopwatchState state,
                             bool lapVisible, uint8_t rows[8]) noexcept
        {
            for (uint8_t row = 0; row < 8; ++row)
            {
                rows[row] = 0;
            }

            const uint32_t minuteMilliseconds = elapsed % 60000UL;
            uint8_t litCount = minuteMilliseconds == 0
                                   ? 0
                                   : static_cast<uint8_t> (
                                         1UL + minuteMilliseconds * 28UL /
                                                   60000UL);
            if (litCount > 28)
            {
                litCount = 28;
            }
            for (uint8_t index = 0; index < litCount; ++index)
            {
                uint8_t row = 0;
                uint8_t column = 0;
                if (index < 8)
                {
                    column = index;
                }
                else if (index < 15)
                {
                    row    = static_cast<uint8_t> (index - 7U);
                    column = 7;
                }
                else if (index < 22)
                {
                    row    = 7;
                    column = static_cast<uint8_t> (21U - index);
                }
                else
                {
                    row = static_cast<uint8_t> (28U - index);
                }
                setPixel (rows, row, column);
            }

            static const uint8_t stopped[4] = {0x0fU, 0x09U, 0x09U, 0x0fU};
            static const uint8_t running[4] = {0x06U, 0x03U, 0x0fU, 0x02U};
            static const uint8_t paused[4]  = {0x0aU, 0x0aU, 0x0aU, 0x0aU};
            static const uint8_t lap[4]     = {0x08U, 0x08U, 0x08U, 0x0fU};
            static const uint8_t fault[4]   = {0x09U, 0x06U, 0x06U, 0x09U};
            const uint8_t* glyph = stopped;
            if (lapVisible)
            {
                glyph = lap;
            }
            else if (state == TimingDeskStopwatchState::Running)
            {
                glyph = running;
            }
            else if (state == TimingDeskStopwatchState::Paused)
            {
                glyph = paused;
            }
            else if (state == TimingDeskStopwatchState::Faulted)
            {
                glyph = fault;
            }
            for (uint8_t row = 0; row < 4; ++row)
            {
                rows[row + 2U] |= static_cast<uint8_t> (glyph[row] << 2U);
            }
        }

        bool evidenceValid (const TimingDeskControlEvidence& evidence,
                            const TimingDeskControlIdentity& expected,
                            TimePoint now, Duration freshness,
                            uint32_t lastSequence) noexcept
        {
            if (!identityEqual (evidence.source, expected) ||
                evidence.sequence == 0 || !evidence.status.ok () ||
                (evidence.pressEvent && !evidence.pressed))
            {
                return false;
            }
            const uint32_t age =
                now.elapsedSince (evidence.observedAt).milliseconds ();
            if (age >= halfRange || age > freshness.milliseconds ())
            {
                return false;
            }
            if (lastSequence == 0 || evidence.sequence == lastSequence)
            {
                return true;
            }
            const uint32_t distance = evidence.sequence - lastSequence;
            return distance != 0 && distance < halfRange;
        }
    } // namespace

    DualDisplayTimingDeskConfig::DualDisplayTimingDeskConfig (
        uint32_t ownerTokenValue, uint16_t configurationRevisionValue,
        const MultiplexedDigitConfig& digitConfigValue,
        const Max7219PresentationConfig& matrixConfigValue,
        const TimingDeskControlIdentity& startPauseSourceValue,
        const TimingDeskControlIdentity& lapSourceValue,
        const TimingDeskControlIdentity& resetSourceValue,
        Duration presentationGraceValue, Duration selfTestTimeoutValue,
        Duration controlFreshnessValue) noexcept
        : ownerToken            (ownerTokenValue),
          configurationRevision (configurationRevisionValue),
          digitConfig           (digitConfigValue),
          matrixConfig          (matrixConfigValue),
          startPauseSource      (startPauseSourceValue),
          lapSource             (lapSourceValue),
          resetSource           (resetSourceValue),
          presentationGrace     (presentationGraceValue),
          selfTestTimeout       (selfTestTimeoutValue),
          controlFreshness      (controlFreshnessValue)
    {
    }

    DualDisplayTimingDesk::DualDisplayTimingDesk (
        const DualDisplayTimingDeskConfig& config) noexcept
        : config_                       (config),
          digitPolicy_                  (config.digitConfig),
          matrixPolicy_                 (config.matrixConfig),
          digitPreview_                 (),
          matrixPreview_                (),
          digitSnapshot_                (digitPolicy_.snapshot ()),
          matrixSnapshot_               (matrixPolicy_.snapshot ()),
          digitService_                 (StatusCode::NotInitialized,
                                         noDigitTransaction ()),
          matrixService_                (StatusCode::NotInitialized,
                                         noMatrixCommand ()),
          expectedDigitFrame_           (blankDigitFrame ()),
          expectedMatrixFrame_          (blankMatrixFrame ()),
          logicalRows_                  {0, 0, 0, 0, 0, 0, 0, 0},
          lastUpdateAt_                 (),
          runStartedAt_                 (),
          lapUntil_                     (),
          presentationPublishedAt_      (),
          selfTestStageStartedAt_       (),
          materializedElapsed_          (),
          lapElapsed_                   (),
          lifecycleGeneration_          (0),
          snapshotSequence_             (0),
          presentationGeneration_       (0),
          digitDigest_                  (0),
          matrixDigest_                 (0),
          lastControlSequences_         {0, 0, 0},
          selfTestStage_                (0),
          stopwatchState_               (TimingDeskStopwatchState::Stopped),
          qualification_                (TimingDeskQualification::Configuring),
          presentationDisposition_      (
              TimingDeskPresentationDisposition::Configuring),
          faultOwner_                   (TimingDeskFaultOwner::None),
          status_                       (StatusCode::NotInitialized),
          haveLastUpdate_               (false),
          lapVisible_                   (false),
          digitAccepted_                (false),
          matrixAccepted_               (false),
          initialized_                  (false)
    {
    }

    Status DualDisplayTimingDesk::initialize (TimePoint now) noexcept
    {
        if (status_.error () == StatusCode::CapacityExceeded)
        {
            return status_;
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
        const Status digitStatus  = digitPolicy_.initialize  (now);
        const Status matrixStatus = matrixPolicy_.initialize ();
        if (!digitStatus.ok                                  () || !matrixStatus.ok     ())
        {
            status_ = !digitStatus.ok () ? digitStatus : matrixStatus;
            return status_;
        }
        initialized_ = true;
        reset (now);
        return status_;
    }

    void DualDisplayTimingDesk::reset (TimePoint now) noexcept
    {
        if (status_.error () == StatusCode::CapacityExceeded)
        {
            return;
        }
        if (initialized_)
        {
            ++lifecycleGeneration_;
            if (lifecycleGeneration_ == 0)
            {
                digitPolicy_.shutdown  ();
                matrixPolicy_.shutdown ();
                stopwatchState_  = TimingDeskStopwatchState::Faulted;
                qualification_   = TimingDeskQualification::Fault;
                faultOwner_      = TimingDeskFaultOwner::Coordinator;
                status_          = StatusCode::CapacityExceeded;
                initialized_     = false;
                return;
            }
        }
        digitPolicy_.reset                          (now);
        matrixPolicy_.reset                         ();
        expectedDigitFrame_      = blankDigitFrame  ();
        expectedMatrixFrame_     = blankMatrixFrame ();
        lastUpdateAt_            = now;
        runStartedAt_            = now;
        lapUntil_                = now;
        presentationPublishedAt_ = now;
        selfTestStageStartedAt_  = now;
        materializedElapsed_     = Duration ();
        lapElapsed_              = Duration ();
        snapshotSequence_        = 0;
        presentationGeneration_  = 0;
        digitDigest_             = 0;
        matrixDigest_            = 0;
        lastControlSequences_[0] = 0;
        lastControlSequences_[1] = 0;
        lastControlSequences_[2] = 0;
        selfTestStage_           = 0;
        stopwatchState_          = TimingDeskStopwatchState::Stopped;
        qualification_           = TimingDeskQualification::Configuring;
        presentationDisposition_ =
            TimingDeskPresentationDisposition::Configuring;
        faultOwner_       = TimingDeskFaultOwner::None;
        status_           = initialized_ ? StatusCode::Ok
                                         : StatusCode::NotInitialized;
        haveLastUpdate_   = initialized_;
        lapVisible_       = false;
        digitAccepted_    = false;
        matrixAccepted_   = false;
    }

    void DualDisplayTimingDesk::shutdown () noexcept
    {
        digitPolicy_.shutdown                       ();
        matrixPolicy_.shutdown                      ();
        expectedDigitFrame_      = blankDigitFrame  ();
        expectedMatrixFrame_     = blankMatrixFrame ();
        presentationDisposition_ = TimingDeskPresentationDisposition::Fault;
        qualification_           = TimingDeskQualification::Fault;
        stopwatchState_          = TimingDeskStopwatchState::Faulted;
        status_ = status_.error () == StatusCode::CapacityExceeded
                      ? status_
                      : StatusCode::NotInitialized;
        digitAccepted_  = false;
        matrixAccepted_ = false;
        initialized_    = false;
    }

    void DualDisplayTimingDesk::enterPresentationFault (
        TimingDeskFaultOwner owner, Status status) noexcept
    {
        faultOwner_              = owner;
        qualification_           = TimingDeskQualification::Fault;
        presentationDisposition_ = TimingDeskPresentationDisposition::Disagreement;
        status_                  = status;
        digitAccepted_           = false;
        matrixAccepted_          = false;
    }

    Status DualDisplayTimingDesk::publishPresentation (TimePoint now) noexcept
    {
        if (presentationGeneration_ != 0)
        {
            return StatusCode::ResourceBusy;
        }

        ++snapshotSequence_;
        if (snapshotSequence_ == 0)
        {
            enterPresentationFault (TimingDeskFaultOwner::Coordinator,
                                    StatusCode::CapacityExceeded);
            return status_;
        }

        for (uint8_t row = 0; row < 8; ++row)
        {
            logicalRows_[row] = 0;
        }
        Status digitStatus;
        uint8_t mode = 0;

        if (qualification_ == TimingDeskQualification::SelfTest)
        {
            mode = static_cast<uint8_t> (0x80U | selfTestStage_);
            if (selfTestStage_ == 0 || selfTestStage_ == 13)
            {
                digitStatus = digitPolicy_.previewDiagnostic (
                    MultiplexedDigitDiagnosticGlyph::AllOff, 0x0fU,
                    snapshotSequence_, digitPreview_);
            }
            else if (selfTestStage_ <= 8)
            {
                const MultiplexedDigitDiagnosticGlyph glyph =
                    static_cast<MultiplexedDigitDiagnosticGlyph> (
                        static_cast<uint8_t> (
                            MultiplexedDigitDiagnosticGlyph::SegmentA) +
                        selfTestStage_ - 1U);
                const uint8_t digit =
                    static_cast<uint8_t> ((selfTestStage_ - 1U) % 4U);
                digitStatus = digitPolicy_.previewDiagnostic (
                    glyph, static_cast<uint8_t> (1U << digit),
                    snapshotSequence_, digitPreview_);
                const uint8_t pixel =
                    static_cast<uint8_t> (selfTestStage_ - 1U);
                setPixel (logicalRows_, pixel, pixel);
            }
            else
            {
                const uint8_t digit =
                    static_cast<uint8_t> (selfTestStage_ - 9U);
                digitStatus = digitPolicy_.previewDiagnostic (
                    MultiplexedDigitDiagnosticGlyph::DigitIdentification,
                    static_cast<uint8_t> (1U << digit), snapshotSequence_,
                    digitPreview_);
                for (uint8_t row = 0; row < 8; ++row)
                {
                    setPixel (logicalRows_, row, digit);
                }
            }
        }
        else
        {
            const uint32_t elapsed =
                lapVisible_
                    ? lapElapsed_.milliseconds ()
                    : elapsedAt                (
                          stopwatchState_, materializedElapsed_, runStartedAt_,
                          now);
            mode = static_cast<uint8_t> (stopwatchState_);
            if (lapVisible_)
            {
                mode |= 0x40U;
            }
            digitStatus = digitPolicy_.preview (
                timingValue (elapsed), true, 0x0aU, snapshotSequence_,
                digitPreview_);
            ordinaryMatrix (elapsed, stopwatchState_, lapVisible_, logicalRows_);
        }

        const Status matrixStatus = matrixPolicy_.preview (
            logicalRows_, snapshotSequence_, matrixPreview_);
        if (!digitStatus.ok () || !matrixStatus.ok ())
        {
            --snapshotSequence_;
            return !digitStatus.ok () ? digitStatus : matrixStatus;
        }
        if (!digitPolicy_.canCommit (digitPreview_) ||
            !matrixPolicy_.canCommit (matrixPreview_))
        {
            --snapshotSequence_;
            return StatusCode::ResourceBusy;
        }

        const Status digitCommit  = digitPolicy_.commit  (digitPreview_);
        const Status matrixCommit = matrixPolicy_.commit (matrixPreview_);
        if (!digitCommit.ok                              () || !matrixCommit.ok ())
        {
            enterPresentationFault (TimingDeskFaultOwner::Coordinator,
                                    StatusCode::InternalInvariant);
            return status_;
        }

        digitSnapshot_        = digitPolicy_.snapshot  ();
        matrixSnapshot_       = matrixPolicy_.snapshot ();
        expectedDigitFrame_   = digitSnapshot_.pendingFrame;
        expectedMatrixFrame_  = matrixSnapshot_.desiredFrame;
        if (expectedDigitFrame_.generation == 0 ||
            expectedDigitFrame_.generation !=
                expectedMatrixFrame_.generation)
        {
            enterPresentationFault (TimingDeskFaultOwner::Coordinator,
                                    StatusCode::InternalInvariant);
            return status_;
        }
        presentationGeneration_  = expectedDigitFrame_.generation;
        digitDigest_             = digitDigest  (expectedDigitFrame_, mode);
        matrixDigest_            = matrixDigest (expectedMatrixFrame_, mode);
        presentationPublishedAt_ = now;
        selfTestStageStartedAt_  = now;
        digitAccepted_           = false;
        matrixAccepted_          = false;
        presentationDisposition_ =
            qualification_ == TimingDeskQualification::SelfTest
                ? TimingDeskPresentationDisposition::SelfTest
                : TimingDeskPresentationDisposition::Pending;
        return StatusCode::Ok;
    }

    Result<DualDisplayTimingDeskResult> DualDisplayTimingDesk::update (
        const DualDisplayEnvelope& envelope) noexcept
    {
        DualDisplayTimingDeskResult result =
            noResult (presentationDisposition_);
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, result};
        }

        if ((haveLastUpdate_ &&
             envelope.now.elapsedSince (lastUpdateAt_).milliseconds () >=
                 halfRange) ||
            !evidenceValid (envelope.startPause, config_.startPauseSource,
                            envelope.now, config_.controlFreshness,
                            lastControlSequences_[0]) ||
            !evidenceValid (envelope.lap, config_.lapSource, envelope.now,
                            config_.controlFreshness,
                            lastControlSequences_[1]) ||
            !evidenceValid (envelope.reset, config_.resetSource, envelope.now,
                            config_.controlFreshness,
                            lastControlSequences_[2]))
        {
            result.controlStatus = StatusCode::InvalidArgument;
            return {StatusCode::InvalidArgument, result};
        }

        digitSnapshot_  = digitPolicy_.snapshot  ();
        matrixSnapshot_ = matrixPolicy_.snapshot ();
        if (envelope.digitReceipt != nullptr)
        {
            const DigitFrameReceipt& receipt = *envelope.digitReceipt;
            const bool correlation =
                presentationGeneration_ != 0 &&
                receipt.ownerToken == config_.digitConfig.ownerToken &&
                receipt.lifecycleGeneration ==
                    digitSnapshot_.lifecycleGeneration &&
                receipt.configurationRevision ==
                    config_.digitConfig.configurationRevision &&
                receipt.requestedGeneration == presentationGeneration_;
            const bool accepted =
                correlation && receipt.status.ok () &&
                receipt.acceptedGeneration == presentationGeneration_ &&
                receipt.reportedDigest == digitDigest_ &&
                frameEqual (receipt.reportedFrame, expectedDigitFrame_) &&
                frameEqual (digitSnapshot_.activeFrame,
                            expectedDigitFrame_) &&
                !receipt.blankRequestAccepted &&
                envelope.now.elapsedSince (receipt.observedAt).milliseconds () <
                    halfRange &&
                receipt.observedAt.elapsedSince (
                    presentationPublishedAt_).milliseconds () < halfRange;
            if (!accepted)
            {
                enterPresentationFault (
                    TimingDeskFaultOwner::DigitDisplay,
                    correlation ? StatusCode::HardwareFailure
                                : StatusCode::InvalidArgument);
            }
            else
            {
                digitAccepted_ = true;
            }
        }
        if (envelope.matrixReceipt != nullptr)
        {
            const MatrixFrameReceipt& receipt = *envelope.matrixReceipt;
            const bool correlation =
                presentationGeneration_ != 0 &&
                receipt.ownerToken == config_.matrixConfig.ownerToken &&
                receipt.lifecycleGeneration ==
                    matrixSnapshot_.lifecycleGeneration &&
                receipt.configurationRevision ==
                    config_.matrixConfig.configurationRevision &&
                receipt.requestedGeneration == presentationGeneration_;
            const bool accepted =
                correlation && receipt.status.ok () &&
                receipt.acceptedGeneration == presentationGeneration_ &&
                receipt.reportedDigest == matrixDigest_ &&
                frameEqual (receipt.reportedFrame, expectedMatrixFrame_) &&
                frameEqual (matrixSnapshot_.submittedFrame,
                            expectedMatrixFrame_) &&
                !receipt.blankRequestAccepted &&
                envelope.now.elapsedSince (receipt.observedAt).milliseconds () <
                    halfRange &&
                receipt.observedAt.elapsedSince (
                    presentationPublishedAt_).milliseconds () < halfRange;
            if (!accepted)
            {
                enterPresentationFault (
                    faultOwner_ == TimingDeskFaultOwner::DigitDisplay
                        ? TimingDeskFaultOwner::BothDisplays
                        : TimingDeskFaultOwner::MatrixDisplay,
                    correlation ? StatusCode::HardwareFailure
                                : StatusCode::InvalidArgument);
            }
            else
            {
                matrixAccepted_ = true;
            }
        }

        digitService_                  = digitPolicy_.refresh (envelope.now);
        result.digitTransaction        = digitService_.value  ();
        result.digitTransactionPresent = digitService_.value  ().emitted;
        if (!digitService_.ok                                 () &&
            digitService_.error  () != StatusCode::ResourceBusy)
        {
            enterPresentationFault (TimingDeskFaultOwner::DigitDisplay,
                                    digitService_.status ());
        }

        matrixService_                  =
            matrixPolicy_.service (envelope.transportReceipt);
        result.matrixCommand        = matrixService_.value  ();
        result.matrixCommandPresent = matrixService_.value  ().emitted;
        if (!matrixService_.ok                              () &&
            matrixService_.error  () != StatusCode::ResourceBusy)
        {
            enterPresentationFault (
                faultOwner_ == TimingDeskFaultOwner::DigitDisplay
                    ? TimingDeskFaultOwner::BothDisplays
                    : TimingDeskFaultOwner::MatrixDisplay,
                matrixService_.status ());
        }

        if (presentationGeneration_ != 0 &&
            qualification_ != TimingDeskQualification::Fault)
        {
            if (digitAccepted_ && matrixAccepted_)
            {
                presentationGeneration_ = 0;
                digitAccepted_           = false;
                matrixAccepted_          = false;
                if (qualification_ == TimingDeskQualification::SelfTest)
                {
                    if (selfTestStage_ == 13)
                    {
                        qualification_ = TimingDeskQualification::Ready;
                        presentationDisposition_ =
                            TimingDeskPresentationDisposition::InSync;
                    }
                    else
                    {
                        ++selfTestStage_;
                    }
                }
                else
                {
                    presentationDisposition_ =
                        TimingDeskPresentationDisposition::InSync;
                }
            }
            else
            {
                const uint32_t age =
                    envelope.now.elapsedSince (presentationPublishedAt_)
                        .milliseconds ();
                const uint32_t timeout =
                    qualification_ == TimingDeskQualification::SelfTest
                        ? config_.selfTestTimeout.milliseconds    ()
                        : config_.presentationGrace.milliseconds  ();
                if (age >= halfRange)
                {
                    result.controlStatus = StatusCode::InvalidArgument;
                    return {StatusCode::InvalidArgument, result};
                }
                if (age > timeout)
                {
                    TimingDeskFaultOwner owner =
                        digitAccepted_ ? TimingDeskFaultOwner::MatrixDisplay
                                       : TimingDeskFaultOwner::DigitDisplay;
                    if (!digitAccepted_ && !matrixAccepted_)
                    {
                        owner = TimingDeskFaultOwner::BothDisplays;
                    }
                    enterPresentationFault (owner, StatusCode::Timeout);
                }
                else
                {
                    presentationDisposition_ =
                        qualification_ == TimingDeskQualification::SelfTest
                            ? TimingDeskPresentationDisposition::SelfTest
                            : TimingDeskPresentationDisposition::Pending;
                }
            }
        }

        const bool startEvent =
            envelope.startPause.sequence != lastControlSequences_[0] &&
            envelope.startPause.pressEvent;
        const bool lapEvent =
            envelope.lap.sequence != lastControlSequences_[1] &&
            envelope.lap.pressEvent;
        const bool resetEvent =
            envelope.reset.sequence != lastControlSequences_[2] &&
            envelope.reset.pressEvent;
        lastControlSequences_[0] = envelope.startPause.sequence;
        lastControlSequences_[1] = envelope.lap.sequence;
        lastControlSequences_[2] = envelope.reset.sequence;

        if (resetEvent)
        {
            reset (envelope.now);
            lastControlSequences_[0] = envelope.startPause.sequence;
            lastControlSequences_[1] = envelope.lap.sequence;
            lastControlSequences_[2] = envelope.reset.sequence;
        }
        else if (startEvent && lapEvent)
        {
            result.controlStatus = StatusCode::InvalidArgument;
        }
        else if (qualification_ == TimingDeskQualification::Ready)
        {
            const uint32_t current =
                elapsedAt (stopwatchState_, materializedElapsed_,
                           runStartedAt_, envelope.now);
            if (startEvent)
            {
                if (stopwatchState_ == TimingDeskStopwatchState::Running)
                {
                    materializedElapsed_ = Duration (current);
                    stopwatchState_      = TimingDeskStopwatchState::Paused;
                }
                else
                {
                    runStartedAt_   = envelope.now;
                    stopwatchState_ = TimingDeskStopwatchState::Running;
                }
            }
            if (lapEvent)
            {
                lapElapsed_ = Duration  (current);
                lapUntil_   = TimePoint (
                    envelope.now.milliseconds () + lapDuration);
                lapVisible_ = true;
            }
            if (current >= stopwatchLimit)
            {
                materializedElapsed_ = Duration (stopwatchLimit);
                stopwatchState_      = TimingDeskStopwatchState::Stopped;
            }
        }

        if (lapVisible_)
        {
            const uint32_t lapAge =
                envelope.now.elapsedSince (
                    TimePoint                                           (lapUntil_.milliseconds () - lapDuration))
                    .milliseconds                                       ();
            if (lapAge >= halfRange)
            {
                result.controlStatus = StatusCode::InvalidArgument;
                return {StatusCode::InvalidArgument, result};
            }
            if (lapAge > lapDuration)
            {
                lapVisible_ = false;
            }
        }

        lastUpdateAt_  = envelope.now;
        haveLastUpdate_ = true;

        matrixSnapshot_ = matrixPolicy_.snapshot ();
        if (qualification_ == TimingDeskQualification::Configuring &&
            matrixSnapshot_.configured)
        {
            qualification_           = TimingDeskQualification::SelfTest;
            selfTestStage_            = 0;
            presentationDisposition_ =
                TimingDeskPresentationDisposition::SelfTest;
        }

        if (presentationGeneration_ == 0 &&
            (qualification_ == TimingDeskQualification::SelfTest ||
             qualification_ == TimingDeskQualification::Ready))
        {
            const Status publish = publishPresentation (envelope.now);
            if (!publish.ok                            () &&
                publish.error  () != StatusCode::ResourceBusy)
            {
                enterPresentationFault (TimingDeskFaultOwner::Coordinator,
                                        publish);
            }
            else if (publish.error () == StatusCode::ResourceBusy)
            {
                presentationDisposition_ =
                    TimingDeskPresentationDisposition::ResourceBusy;
            }
        }

        result.controlStatus           = result.controlStatus.ok ()
                                             ? StatusCode::Ok
                                             : result.controlStatus;
        result.presentationDisposition = presentationDisposition_;
        result.presentationGeneration  = presentationGeneration_;
        result.digitDigest             = digitDigest_;
        result.matrixDigest            = matrixDigest_;
        result.digitBlankRequested =
            qualification_ == TimingDeskQualification::Fault;
        result.matrixBlankRequested =
            qualification_ == TimingDeskQualification::Fault;
        return {status_, result};
    }

    bool DualDisplayTimingDesk::initialized () const noexcept
    {
        return initialized_;
    }

    DualDisplayTimingDeskSnapshot DualDisplayTimingDesk::snapshot () const noexcept
    {
        return {expectedDigitFrame_,
                expectedMatrixFrame_,
                Duration (elapsedAt (stopwatchState_, materializedElapsed_,
                                     runStartedAt_, lastUpdateAt_)),
                lapElapsed_,
                presentationPublishedAt_,
                selfTestStageStartedAt_,
                lifecycleGeneration_,
                snapshotSequence_,
                presentationGeneration_,
                digitDigest_,
                matrixDigest_,
                selfTestStage_,
                stopwatchState_,
                qualification_,
                presentationDisposition_,
                faultOwner_,
                status_,
                lapVisible_,
                digitAccepted_,
                matrixAccepted_,
                initialized_};
    }
    // clang-format on
} // namespace adk
