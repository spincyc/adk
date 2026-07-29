#include <dual_display_timing_desk.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>

namespace adk {
    struct MultiplexedDigitPolicyTestAccess
    {
        static void setServiceTime (MultiplexedDigitPolicy& policy, TimePoint now)
        {
            policy.lastServiceAt_ = now;
        }

        static void exhaustFrameGeneration (MultiplexedDigitPolicy& policy)
        {
            policy.activeFrame_.generation = UINT32_MAX;
            policy.pending_                = false;
        }
    };

    struct Max7219PresentationPolicyTestAccess
    {
        static void exhaustFrameGeneration (Max7219PresentationPolicy& policy)
        {
            policy.desiredFrame_.generation   = UINT32_MAX;
            policy.submittedFrame_.generation = UINT32_MAX;
            policy.outstanding_               = false;
        }
    };

    struct DualDisplayTimingDeskTestAccess
    {
        static void setLifecycleGeneration (DualDisplayTimingDesk& desk,
                                            uint32_t               generation)
        {
            desk.lifecycleGeneration_ = generation;
        }

        static void setPresentationGeneration (DualDisplayTimingDesk& desk,
                                               uint32_t               generation)
        {
            desk.presentationGeneration_ = generation;
        }

        static void forceStopwatch (DualDisplayTimingDesk&   desk,
                                    TimingDeskStopwatchState state, uint32_t elapsed,
                                    TimePoint now)
        {
            desk.stopwatchState_      = state;
            desk.materializedElapsed_ = Duration (elapsed);
            desk.runStartedAt_        = now;
            desk.lastUpdateAt_        = now;
            desk.haveLastUpdate_      = true;
            desk.qualification_       = state == TimingDeskStopwatchState::Faulted
                                            ? TimingDeskQualification::Fault
                                            : TimingDeskQualification::Ready;
            desk.status_              = StatusCode::Ok;
        }

        static void forceLap (DualDisplayTimingDesk& desk, Duration elapsed,
                              TimePoint until, TimePoint now)
        {
            desk.lapElapsed_              = elapsed;
            desk.lapUntil_                = until;
            desk.lapVisible_              = true;
            desk.lastUpdateAt_            = now;
            desk.presentationPublishedAt_ = now;
            desk.haveLastUpdate_          = true;
            MultiplexedDigitPolicyTestAccess::setServiceTime (desk.digitPolicy_, now);
        }

        static void exhaustSnapshotSequence (DualDisplayTimingDesk& desk)
        {
            desk.snapshotSequence_       = UINT32_MAX;
            desk.presentationGeneration_ = 0;
        }

        static void exhaustChildFrameGenerations (DualDisplayTimingDesk& desk)
        {
            MultiplexedDigitPolicyTestAccess::exhaustFrameGeneration (
                desk.digitPolicy_);
            Max7219PresentationPolicyTestAccess::exhaustFrameGeneration (
                desk.matrixPolicy_);
            desk.presentationGeneration_ = 0;
        }

        static void prepareTimeJump (DualDisplayTimingDesk& desk, TimePoint now)
        {
            MultiplexedDigitPolicyTestAccess::setServiceTime (desk.digitPolicy_, now);
            desk.presentationPublishedAt_ = now;
        }
    };
} // namespace adk

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::TimingDeskControlIdentity identity (uint16_t sourceId)
    {
        return {sourceId, static_cast<uint16_t> (sourceId + 10U),
                static_cast<uint32_t> (sourceId + 100U)};
    }

    adk::DualDisplayTimingDeskConfig
    config (adk::Duration presentationGrace = adk::Duration (100),
            adk::Duration selfTestTimeout   = adk::Duration (100),
            adk::Duration controlFreshness  = adk::Duration (100))
    {
        return {91,
                12,
                adk::MultiplexedDigitConfig (92, 13,
                                             adk::SevenSegmentPolarity::CommonCathode,
                                             adk::DigitSelectPolarity::ActiveHigh),
                adk::Max7219PresentationConfig (93, 14,
                                                adk::Max7219Orientation::Identity, 1),
                identity (1),
                identity (2),
                identity (3),
                presentationGrace,
                selfTestTimeout,
                controlFreshness};
    }

    adk::TimingDeskControlEvidence evidence (adk::TimingDeskControlIdentity source,
                                             uint32_t sequence, uint32_t observedAt,
                                             bool        pressed    = false,
                                             bool        pressEvent = false,
                                             adk::Status status     = adk::Status ())
    {
        return {source, sequence, adk::TimePoint (observedAt),
                status, pressed,  pressEvent};
    }

    adk::DualDisplayEnvelope
    envelope (uint32_t now, uint32_t sequence,
              const adk::DigitFrameReceipt*  digitReceipt     = nullptr,
              const adk::MatrixFrameReceipt* matrixReceipt    = nullptr,
              const adk::Max7219Receipt*     transportReceipt = nullptr,
              bool startPause = false, bool lap = false, bool reset = false)
    {
        return {adk::TimePoint (now),
                evidence (identity (1), sequence, now, startPause, startPause),
                evidence (identity (2), sequence, now, lap, lap),
                evidence (identity (3), sequence, now, reset, reset),
                digitReceipt,
                matrixReceipt,
                transportReceipt};
    }

    adk::Max7219Receipt transportReceipt (const adk::Max7219Command& command,
                                          uint32_t                   observedAt,
                                          adk::Status status = adk::Status (),
                                          uint8_t     acceptedByteCount  = 2,
                                          bool        chipSelectInactive = true)
    {
        return {command.ownerToken,
                command.lifecycleGeneration,
                command.configurationRevision,
                command.presentationGeneration,
                command.operationIndex,
                command.registerAddress,
                command.data,
                command.operation,
                acceptedByteCount,
                chipSelectInactive,
                adk::TimePoint (observedAt),
                status};
    }

    void digestByte (uint32_t& digest, uint8_t value)
    {
        digest ^= value;
        digest *= UINT32_C (0x01000193);
    }

    void digestUint32 (uint32_t& digest, uint32_t value)
    {
        for (uint8_t shift = 0; shift < 32; shift = static_cast<uint8_t> (shift + 8U))
        {
            digestByte (digest, static_cast<uint8_t> (value >> shift));
        }
    }

    uint32_t expectedDigitDigest (const adk::MultiplexedDigitFrame& frame, uint8_t mode)
    {
        static const char domain[] = "ADK.DIGIT.FRAME.V1";
        uint32_t          digest   = UINT32_C (0x811c9dc5);
        for (uint8_t index = 0; index < sizeof domain - 1U; ++index)
        {
            digestByte (digest, static_cast<uint8_t> (domain[index]));
        }
        for (uint8_t index = 0; index < 4; ++index)
        {
            digestByte (digest, static_cast<uint8_t> (frame.glyphs[index]));
            digestByte (digest, static_cast<uint8_t> (frame.diagnosticGlyphs[index]));
        }
        digestByte (digest, frame.decimalMask);

        digestByte (digest, frame.overflow ? 1U : 0U);

        digestUint32 (digest, frame.sourceSnapshotSequence);

        digestUint32 (digest, frame.generation);

        digestByte (digest, mode);
        return digest;
    }

    uint32_t expectedMatrixDigest (const adk::Max7219Frame& frame, uint8_t mode)
    {
        static const char domain[] = "ADK.MATRIX.FRAME.V1";
        uint32_t          digest   = UINT32_C (0x811c9dc5);
        for (uint8_t index = 0; index < sizeof domain - 1U; ++index)
        {
            digestByte (digest, static_cast<uint8_t> (domain[index]));
        }
        for (const auto row : frame.rows)
        {
            digestByte (digest, row);
        }
        digestUint32 (digest, frame.sourceSnapshotSequence);

        digestUint32 (digest, frame.generation);

        digestByte (digest, mode);
        return digest;
    }

    void setExpectedPixel (uint8_t rows[8], uint8_t row, uint8_t column)
    {
        rows[row] |= static_cast<uint8_t> (0x80U >> column);
    }

    void expectedOrdinaryRows (uint32_t elapsed, adk::TimingDeskStopwatchState state,
                               bool lapVisible, uint8_t rows[8])
    {
        std::memset (rows, 0, 8);
        const uint32_t milliseconds = elapsed % 60000U;
        uint8_t        litCount =
            milliseconds == 0 ? 0
                              : static_cast<uint8_t> (1U + milliseconds * 28U / 60000U);
        if (litCount > 28)
        {
            litCount = 28;
        }
        for (uint8_t index = 0; index < litCount; ++index)
        {
            if (index < 8)
            {
                setExpectedPixel (rows, 0, index);
            }
            else if (index < 15)
            {
                setExpectedPixel (rows, static_cast<uint8_t> (index - 7U), 7);
            }
            else if (index < 22)
            {
                setExpectedPixel (rows, 7, static_cast<uint8_t> (21U - index));
            }
            else
            {
                setExpectedPixel (rows, static_cast<uint8_t> (28U - index), 0);
            }
        }

        static const uint8_t stopped[4] = {0x0f, 0x09, 0x09, 0x0f};
        static const uint8_t running[4] = {0x06, 0x03, 0x0f, 0x02};
        static const uint8_t paused[4]  = {0x0a, 0x0a, 0x0a, 0x0a};
        static const uint8_t lap[4]     = {0x08, 0x08, 0x08, 0x0f};
        const uint8_t*       glyph      = stopped;
        if (lapVisible)
        {
            glyph = lap;
        }
        else if (state == adk::TimingDeskStopwatchState::Running)
        {
            glyph = running;
        }
        else if (state == adk::TimingDeskStopwatchState::Paused)
        {
            glyph = paused;
        }
        for (uint8_t row = 0; row < 4; ++row)
        {
            rows[row + 2U] |= static_cast<uint8_t> (glyph[row] << 2U);
        }
    }

    bool sameDigitFrame (const adk::MultiplexedDigitFrame& left,
                         const adk::MultiplexedDigitFrame& right)
    {
        return std::memcmp (left.glyphs, right.glyphs, sizeof left.glyphs) == 0 &&
               std::memcmp (left.diagnosticGlyphs, right.diagnosticGlyphs,
                            sizeof left.diagnosticGlyphs) == 0 &&
               left.decimalMask == right.decimalMask &&
               left.sourceSnapshotSequence == right.sourceSnapshotSequence &&
               left.generation == right.generation && left.overflow == right.overflow;
    }

    bool sameMatrixFrame (const adk::Max7219Frame& left, const adk::Max7219Frame& right)
    {
        return std::memcmp (left.rows, right.rows, sizeof left.rows) == 0 &&
               left.sourceSnapshotSequence == right.sourceSnapshotSequence &&
               left.generation == right.generation;
    }

    bool sameSnapshot (const adk::DualDisplayTimingDeskSnapshot& left,
                       const adk::DualDisplayTimingDeskSnapshot& right)
    {
        return sameDigitFrame (left.digitFrame, right.digitFrame) &&
               sameMatrixFrame (left.matrixFrame, right.matrixFrame) &&
               left.elapsed == right.elapsed && left.lapElapsed == right.lapElapsed &&
               left.presentationPublishedAt == right.presentationPublishedAt &&
               left.selfTestStageStartedAt == right.selfTestStageStartedAt &&
               left.lifecycleGeneration == right.lifecycleGeneration &&
               left.snapshotSequence == right.snapshotSequence &&
               left.presentationGeneration == right.presentationGeneration &&
               left.digitDigest == right.digitDigest &&
               left.matrixDigest == right.matrixDigest &&
               left.selfTestStage == right.selfTestStage &&
               left.stopwatchState == right.stopwatchState &&
               left.qualification == right.qualification &&
               left.presentationDisposition == right.presentationDisposition &&
               left.faultOwner == right.faultOwner && left.status == right.status &&
               left.lapVisible == right.lapVisible &&
               left.digitAccepted == right.digitAccepted &&
               left.matrixAccepted == right.matrixAccepted &&
               left.initialized == right.initialized;
    }

    bool sameResult (const adk::DualDisplayTimingDeskResult& left,
                     const adk::DualDisplayTimingDeskResult& right)
    {
        const auto& leftDigit   = left.digitTransaction;
        const auto& rightDigit  = right.digitTransaction;
        const auto& leftMatrix  = left.matrixCommand;
        const auto& rightMatrix = right.matrixCommand;
        return left.controlStatus == right.controlStatus &&
               left.presentationDisposition == right.presentationDisposition &&
               left.presentationGeneration == right.presentationGeneration &&
               left.digitDigest == right.digitDigest &&
               left.matrixDigest == right.matrixDigest &&
               left.digitTransactionPresent == right.digitTransactionPresent &&
               left.matrixCommandPresent == right.matrixCommandPresent &&
               left.digitBlankRequested == right.digitBlankRequested &&
               left.matrixBlankRequested == right.matrixBlankRequested &&
               leftDigit.ownerToken == rightDigit.ownerToken &&
               leftDigit.configurationRevision == rightDigit.configurationRevision &&
               leftDigit.lifecycleGeneration == rightDigit.lifecycleGeneration &&
               leftDigit.frameGeneration == rightDigit.frameGeneration &&
               leftDigit.sourceSnapshotSequence == rightDigit.sourceSnapshotSequence &&
               leftDigit.emittedAt == rightDigit.emittedAt &&
               leftDigit.digitIndex == rightDigit.digitIndex &&
               std::memcmp (leftDigit.stages, rightDigit.stages,
                            sizeof leftDigit.stages) == 0 &&
               std::memcmp (leftDigit.segmentLevels, rightDigit.segmentLevels,
                            sizeof leftDigit.segmentLevels) == 0 &&
               std::memcmp (leftDigit.digitSelectLevels, rightDigit.digitSelectLevels,
                            sizeof leftDigit.digitSelectLevels) == 0 &&
               leftDigit.fault == rightDigit.fault &&
               leftDigit.emitted == rightDigit.emitted &&
               leftMatrix.ownerToken == rightMatrix.ownerToken &&
               leftMatrix.lifecycleGeneration == rightMatrix.lifecycleGeneration &&
               leftMatrix.configurationRevision == rightMatrix.configurationRevision &&
               leftMatrix.presentationGeneration ==
                   rightMatrix.presentationGeneration &&
               leftMatrix.operationIndex == rightMatrix.operationIndex &&
               leftMatrix.registerAddress == rightMatrix.registerAddress &&
               leftMatrix.data == rightMatrix.data &&
               leftMatrix.operation == rightMatrix.operation &&
               leftMatrix.emitted == rightMatrix.emitted;
    }

    struct DeskDriver
    {
        explicit DeskDriver (adk::DualDisplayTimingDesk& deskValue,
                             uint32_t                    startAt = 0)
            : desk{deskValue}, now{startAt}, sequence{1}, digitLifecycle{0},
              matrixLifecycle{0}, haveTransportReceipt{false},
              pendingTransportReceipt{0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      0,
                                      adk::Max7219Operation::Configure,
                                      0,
                                      false,
                                      adk::TimePoint{0},
                                      adk::Status{}}
        {
        }

        adk::Result<adk::DualDisplayTimingDeskResult>
        step (const adk::DigitFrameReceipt*  digitReceipt  = nullptr,
              const adk::MatrixFrameReceipt* matrixReceipt = nullptr,
              bool startPause = false, bool lap = false, bool reset = false)
        {
            const adk::Max7219Receipt* receipt =
                haveTransportReceipt ? &pendingTransportReceipt : nullptr;
            const auto result =
                desk.update (envelope (now, sequence, digitReceipt, matrixReceipt,
                                       receipt, startPause, lap, reset));
            haveTransportReceipt = false;
            if (result.value ().matrixCommandPresent)
            {
                const auto& command = result.value ().matrixCommand;

                pendingTransportReceipt = transportReceipt (command, now + 1U);
                haveTransportReceipt    = true;
                matrixLifecycle         = command.lifecycleGeneration;
            }
            if (result.value ().digitTransactionPresent)
            {
                digitLifecycle = result.value ().digitTransaction.lifecycleGeneration;
            }
            ++now;
            ++sequence;
            return result;
        }

        void configure ()
        {
            for (uint8_t attempts = 0; attempts < 40; ++attempts)
            {
                const auto result = step ();

                require (result.ok (), "configuration service succeeds");

                if (desk.snapshot ().qualification !=
                    adk::TimingDeskQualification::Configuring)
                {
                    require (desk.snapshot ().qualification ==
                                 adk::TimingDeskQualification::SelfTest,
                             "configuration enters self-test");
                    return;
                }
            }
            require (false, "configuration completes in bounded calls");
        }

        adk::DigitFrameReceipt
        digitReceiptFor (const adk::DualDisplayTimingDeskSnapshot& snapshot,
                         adk::Status status = adk::Status ()) const
        {
            return {92,
                    digitLifecycle,
                    13,
                    snapshot.presentationGeneration,
                    snapshot.presentationGeneration,
                    snapshot.digitFrame,
                    snapshot.digitDigest,
                    adk::TimePoint (now),
                    status,
                    false};
        }

        adk::MatrixFrameReceipt
        matrixReceiptFor (const adk::DualDisplayTimingDeskSnapshot& snapshot,
                          adk::Status status = adk::Status ()) const
        {
            return {93,
                    matrixLifecycle,
                    14,
                    snapshot.presentationGeneration,
                    snapshot.presentationGeneration,
                    snapshot.matrixFrame,
                    snapshot.matrixDigest,
                    adk::TimePoint (now),
                    status,
                    false};
        }

        adk::DualDisplayTimingDeskSnapshot makeCurrentPresentationReady ()
        {
            const auto target = desk.snapshot ();

            require (target.presentationGeneration != 0, "presentation is outstanding");

            bool digitReady  = false;
            bool matrixReady = false;
            for (uint8_t attempts = 0; attempts < 40; ++attempts)
            {
                const auto result = step ();

                require (result.ok (), "presentation service succeeds");

                if (result.value ().digitTransactionPresent &&
                    result.value ().digitTransaction.frameGeneration ==
                        target.digitFrame.generation)
                {
                    digitReady = true;
                }
                const auto matrix = result.value ().matrixCommand;

                if (result.value ().matrixCommandPresent &&
                    matrix.operation == adk::Max7219Operation::SubmitRow &&
                    matrix.operationIndex == 7)
                {
                    matrixReady = true;
                }
                if (digitReady && matrixReady && !haveTransportReceipt)
                {
                    break;
                }
            }

            if (haveTransportReceipt)
            {
                const auto accepted = step ();

                require (accepted.ok (), "final row receipt succeeds");
            }
            return target;
        }

        void acceptCurrentPresentation ()
        {
            const auto target = makeCurrentPresentationReady ();

            const auto digit = digitReceiptFor (target);

            const auto matrix = matrixReceiptFor (target);

            const auto result = step (&digit, &matrix);

            require (result.ok (), "endpoint receipts succeed");

            require (desk.snapshot ().presentationGeneration !=
                         target.presentationGeneration,
                     "accepted presentation generation retires");
        }

        void qualify ()
        {
            configure ();
            for (uint8_t stage = 0; stage < 14; ++stage)
            {
                acceptCurrentPresentation ();
            }
            require (desk.snapshot ().qualification ==
                         adk::TimingDeskQualification::Ready,
                     "all self-test stages establish ready state");
        }

        adk::DualDisplayTimingDesk& desk;
        uint32_t                    now;
        uint32_t                    sequence;
        uint32_t                    digitLifecycle;
        uint32_t                    matrixLifecycle;
        bool                        haveTransportReceipt;
        adk::Max7219Receipt         pendingTransportReceipt;
    };

    void testConfigurationAndLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::DualDisplayTimingDesk>::value,
                       "desk is not copy constructible");
        static_assert (!std::is_move_constructible<adk::DualDisplayTimingDesk>::value,
                       "desk is not move constructible");

        adk::DualDisplayTimingDesk desk (config ());

        require (!desk.initialized (), "construction is inert");

        require (desk.update (envelope (0, 1)).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialization rejects");
        require (desk.initialize (adk::TimePoint (10)).ok (),
                 "initialization succeeds");
        require (desk.initialize (adk::TimePoint (10)).ok (),
                 "initialization is idempotent");

        const auto snapshot = desk.snapshot ();

        require (
            snapshot.initialized &&
                snapshot.stopwatchState == adk::TimingDeskStopwatchState::Stopped &&
                snapshot.qualification == adk::TimingDeskQualification::Configuring &&
                snapshot.elapsed == adk::Duration (0),
            "initial state is stopped zero and configuring");

        desk.shutdown ();

        desk.shutdown ();

        require (!desk.initialized (), "shutdown is idempotent");

        require (desk.update (envelope (11, 2)).error () ==
                     adk::StatusCode::NotInitialized,
                 "shutdown makes update inert");

        require (desk.initialize (adk::TimePoint (20)).ok (),
                 "restart performs full initialization");
        require (desk.snapshot ().lifecycleGeneration > snapshot.lifecycleGeneration,
                 "restart advances lifecycle");
    }

    void testInvalidConfiguration ()
    {
        const adk::Duration invalidDurations[] = {
            adk::Duration (0), adk::Duration (1001),
            adk::Duration (UINT32_C (0x80000000))};
        for (const auto duration : invalidDurations)
        {
            adk::DualDisplayTimingDesk presentationInvalid (
                config (duration, adk::Duration (100), adk::Duration (100)));
            require (presentationInvalid.initialize (adk::TimePoint (0)).error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid presentation grace rejects");

            adk::DualDisplayTimingDesk selfTestInvalid (
                config (adk::Duration (100), duration, adk::Duration (100)));
            require (selfTestInvalid.initialize (adk::TimePoint (0)).error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid self-test timeout rejects");
        }

        auto duplicateSource      = config ();
        duplicateSource.lapSource = duplicateSource.startPauseSource;
        adk::DualDisplayTimingDesk duplicate (duplicateSource);

        require (duplicate.initialize (adk::TimePoint (0)).error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "duplicate control identities reject");
    }

    void testStructuralEvidenceRejectsAtomically ()
    {
        adk::DualDisplayTimingDesk desk (config ());

        require (desk.initialize (adk::TimePoint (100)).ok (),
                 "desk initializes for evidence checks");

        auto malformed = envelope (101, 1);
        malformed.startPause.source.sourceId++;
        const auto before = desk.snapshot ();

        require (desk.update (malformed).error () == adk::StatusCode::InvalidArgument,
                 "foreign control source rejects");
        require (sameSnapshot (before, desk.snapshot ()),
                 "foreign source rejection is atomic");

        malformed                = envelope (101, 1);
        malformed.lap.pressEvent = true;
        malformed.lap.pressed    = false;
        require (desk.update (malformed).error () == adk::StatusCode::InvalidArgument,
                 "inconsistent edge and level reject");
        require (sameSnapshot (before, desk.snapshot ()), "edge rejection is atomic");

        malformed = envelope (UINT32_C (0x80000064), 1);

        require (desk.update (malformed).error () == adk::StatusCode::InvalidArgument,
                 "exact half-range time rejects");
        require (sameSnapshot (before, desk.snapshot ()),
                 "half-range rejection is atomic");
    }

    void testCompleteSelfTestGoldenFrames ()
    {
        adk::DualDisplayTimingDesk desk (config ());

        require (desk.initialize (adk::TimePoint (0)).ok (),
                 "self-test desk initializes");

        DeskDriver driver (desk);

        driver.configure ();

        for (uint8_t stage = 0; stage < 14; ++stage)
        {
            const auto snapshot = desk.snapshot ();

            require (snapshot.qualification == adk::TimingDeskQualification::SelfTest &&
                         snapshot.selfTestStage == stage &&
                         snapshot.presentationGeneration != 0,
                     "expected self-test stage is outstanding");

            uint8_t expectedRows[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            if (stage >= 1 && stage <= 8)
            {
                const uint8_t pixel = static_cast<uint8_t> (stage - 1U);
                expectedRows[pixel] = static_cast<uint8_t> (0x80U >> pixel);
            }
            else if (stage >= 9 && stage <= 12)
            {
                const uint8_t column = static_cast<uint8_t> (stage - 9U);
                for (auto& row : expectedRows)
                {
                    row = static_cast<uint8_t> (0x80U >> column);
                }
            }
            require (std::memcmp (snapshot.matrixFrame.rows, expectedRows,
                                  sizeof expectedRows) == 0,
                     "self-test matrix frame matches golden stage");
            const uint8_t mode = static_cast<uint8_t> (0x80U | stage);
            require (snapshot.digitDigest ==
                         expectedDigitDigest (snapshot.digitFrame, mode),
                     "digit digest has exact domain and field encoding");
            require (snapshot.matrixDigest ==
                         expectedMatrixDigest (snapshot.matrixFrame, mode),
                     "matrix digest has exact domain and field encoding");

            for (uint8_t digit = 0; digit < 4; ++digit)
            {
                adk::MultiplexedDigitDiagnosticGlyph expected =
                    adk::MultiplexedDigitDiagnosticGlyph::None;
                if (stage >= 1 && stage <= 8 &&
                    digit == static_cast<uint8_t> ((stage - 1U) % 4U))
                {
                    expected = static_cast<adk::MultiplexedDigitDiagnosticGlyph> (
                        static_cast<uint8_t> (
                            adk::MultiplexedDigitDiagnosticGlyph::SegmentA) +
                        stage - 1U);
                }
                else if (stage >= 9 && stage <= 12 &&
                         digit == static_cast<uint8_t> (stage - 9U))
                {
                    expected =
                        adk::MultiplexedDigitDiagnosticGlyph::DigitIdentification;
                }
                require (snapshot.digitFrame.diagnosticGlyphs[digit] == expected,
                         "self-test digit frame matches bounded glyph");
            }
            driver.acceptCurrentPresentation ();
        }

        const auto ready = desk.snapshot ();

        require (ready.qualification == adk::TimingDeskQualification::Ready &&
                     ready.stopwatchState == adk::TimingDeskStopwatchState::Stopped &&
                     ready.presentationGeneration != 0,
                 "complete self-test publishes ready zero");
    }

    void testReceiptFaultBlankingAndReset ()
    {
        adk::DualDisplayTimingDesk prematureDesk (config ());

        require (prematureDesk.initialize (adk::TimePoint (0)).ok (),
                 "premature-receipt desk initializes");

        DeskDriver prematureDriver (prematureDesk);

        prematureDriver.configure ();

        const auto prematureTarget = prematureDesk.snapshot ();

        const auto prematureDigit = prematureDriver.digitReceiptFor (prematureTarget);

        const auto prematureMatrix = prematureDriver.matrixReceiptFor (prematureTarget);

        const auto premature = prematureDriver.step (&prematureDigit, &prematureMatrix);

        require (!premature.ok () &&
                     prematureDesk.snapshot ().qualification ==
                         adk::TimingDeskQualification::Fault &&
                     premature.value ().digitBlankRequested &&
                     premature.value ().matrixBlankRequested,
                 "premature endpoint receipts fault and request both blanks");

        const auto elapsedBeforeReset = prematureDesk.snapshot ().elapsed;
        const auto oldLifecycle       = prematureDesk.snapshot ().lifecycleGeneration;
        const auto resetResult =
            prematureDriver.step (nullptr, nullptr, true, true, true);
        require (resetResult.ok () &&
                     prematureDesk.snapshot ().qualification ==
                         adk::TimingDeskQualification::Configuring &&
                     prematureDesk.snapshot ().elapsed == adk::Duration (0) &&
                     prematureDesk.snapshot ().lifecycleGeneration > oldLifecycle,
                 "reset dominates controls and enters full requalification");
        require (elapsedBeforeReset == adk::Duration (0),
                 "presentation fault preserves stopwatch history");

        adk::DualDisplayTimingDesk digestDesk (config ());

        require (digestDesk.initialize (adk::TimePoint (0)).ok (),
                 "wrong-digest desk initializes");

        DeskDriver digestDriver (digestDesk);

        digestDriver.configure ();

        const auto digestTarget = digestDriver.makeCurrentPresentationReady ();

        auto wrongDigit = digestDriver.digitReceiptFor (digestTarget);

        const auto validMatrix = digestDriver.matrixReceiptFor (digestTarget);

        wrongDigit.reportedDigest ^= UINT32_C (0x00000001);

        const auto digestResult = digestDriver.step (&wrongDigit, &validMatrix);

        require (!digestResult.ok () &&
                     digestDesk.snapshot ().faultOwner ==
                         adk::TimingDeskFaultOwner::DigitDisplay &&
                     digestResult.value ().digitBlankRequested &&
                     digestResult.value ().matrixBlankRequested,
                 "one-sided digest mismatch attributes fault and blanks both");
    }

    void testGraceDeadlineAndMalformedResetCompanion ()
    {
        adk::DualDisplayTimingDesk desk (
            config (adk::Duration (100), adk::Duration (100), adk::Duration (100)));
        require (desk.initialize (adk::TimePoint (0)).ok (), "grace desk initializes");

        DeskDriver driver (desk);

        driver.configure ();

        const auto published = desk.snapshot ().presentationPublishedAt;

        while (driver.now <= published.milliseconds () + 100U)
        {
            const auto result = driver.step ();

            require (result.ok () && desk.snapshot ().qualification ==
                                         adk::TimingDeskQualification::SelfTest,
                     "self-test deadline is inclusive");
        }
        const auto expired = driver.step ();

        require (!expired.ok () &&
                     desk.snapshot ().status.error () == adk::StatusCode::Timeout &&
                     desk.snapshot ().faultOwner ==
                         adk::TimingDeskFaultOwner::BothDisplays &&
                     expired.value ().digitBlankRequested &&
                     expired.value ().matrixBlankRequested,
                 "tick after self-test deadline faults both sides");

        adk::DualDisplayTimingDesk malformedDesk (config ());

        require (malformedDesk.initialize (adk::TimePoint (10)).ok (),
                 "malformed-reset desk initializes");
        auto malformed =
            envelope (10, 1, nullptr, nullptr, nullptr, false, false, true);
        malformed.lap.source.sourceId++;
        const auto before = malformedDesk.snapshot ();

        require (malformedDesk.update (malformed).error () ==
                         adk::StatusCode::InvalidArgument &&
                     sameSnapshot (before, malformedDesk.snapshot ()),
                 "malformed companion rejects before reset precedence");
    }

    void testEveryControlMaskInEveryStopwatchState ()
    {
        const adk::TimingDeskStopwatchState states[] = {
            adk::TimingDeskStopwatchState::Stopped,
            adk::TimingDeskStopwatchState::Running,
            adk::TimingDeskStopwatchState::Paused,
            adk::TimingDeskStopwatchState::Faulted};
        for (const auto initialState : states)
        {
            for (uint8_t mask = 0; mask < 8; ++mask)
            {
                adk::DualDisplayTimingDesk desk (config ());

                require (desk.initialize (adk::TimePoint (100)).ok (),
                         "control-mask desk initializes");
                adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
                    desk, initialState, 1234, adk::TimePoint (100));

                const auto result = desk.update (
                    envelope (100, 1, nullptr, nullptr, nullptr, (mask & 0x01U) != 0,
                              (mask & 0x02U) != 0, (mask & 0x04U) != 0));
                const auto snapshot = desk.snapshot ();
                if ((mask & 0x04U) != 0)
                {
                    require (result.ok () &&
                                 snapshot.stopwatchState ==
                                     adk::TimingDeskStopwatchState::Stopped &&
                                 snapshot.elapsed == adk::Duration (0) &&
                                 snapshot.qualification ==
                                     adk::TimingDeskQualification::Configuring,
                             "reset dominates every companion control mask");
                }
                else if ((mask & 0x03U) == 0x03U)
                {
                    require (result.ok () &&
                                 result.value ().controlStatus.error () ==
                                     adk::StatusCode::InvalidArgument &&
                                 snapshot.stopwatchState == initialState &&
                                 snapshot.elapsed == adk::Duration (1234) &&
                                 !snapshot.lapVisible,
                             "simultaneous non-reset controls reject without mutation");
                }
                else if (initialState == adk::TimingDeskStopwatchState::Faulted)
                {
                    require (snapshot.stopwatchState == initialState &&
                                 snapshot.elapsed == adk::Duration (1234),
                             "faulted stopwatch ignores non-reset controls");
                }
                else if ((mask & 0x01U) != 0)
                {
                    const auto expected =
                        initialState == adk::TimingDeskStopwatchState::Running
                            ? adk::TimingDeskStopwatchState::Paused
                            : adk::TimingDeskStopwatchState::Running;
                    require (snapshot.stopwatchState == expected,
                             "start-pause edge toggles eligible stopwatch state");
                }
                else if ((mask & 0x02U) != 0)
                {
                    require (snapshot.stopwatchState == initialState &&
                                 snapshot.lapVisible &&
                                 snapshot.lapElapsed == adk::Duration (1234),
                             "lap edge freezes snapshot without stopping");
                }
                else
                {
                    require (snapshot.stopwatchState == initialState &&
                                 !snapshot.lapVisible,
                             "empty control mask preserves state");
                }
            }
        }
    }

    void testOrdinaryMappingBucketsAndCapacity ()
    {
        adk::DualDisplayTimingDesk desk (config ());

        require (desk.initialize (adk::TimePoint (0)).ok (),
                 "ordinary-mapping desk initializes");

        DeskDriver driver (desk);

        driver.qualify ();

        uint32_t samples[60] = {0, 1, 59999, 60000, 599900};
        uint8_t  sampleCount = 5;
        for (uint8_t bucket = 1; bucket < 28; ++bucket)
        {
            const uint32_t boundary =
                (static_cast<uint32_t> (bucket) * 60000U + 27U) / 28U;
            samples[sampleCount++] = boundary - 1U;
            samples[sampleCount++] = boundary;
        }

        for (uint8_t index = 0; index < sampleCount; ++index)
        {
            const uint32_t elapsed = samples[index];
            adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
                desk, adk::TimingDeskStopwatchState::Stopped, elapsed,
                adk::TimePoint (driver.now));
            driver.acceptCurrentPresentation ();

            const auto snapshot = desk.snapshot ();

            uint8_t expectedRows[8];
            expectedOrdinaryRows (elapsed, adk::TimingDeskStopwatchState::Stopped,
                                  false, expectedRows);
            require (std::memcmp (snapshot.matrixFrame.rows, expectedRows,
                                  sizeof expectedRows) == 0,
                     "perimeter bucket and stopped glyph match golden rows");

            const uint32_t minute    = elapsed / 60000U;
            const uint32_t seconds   = (elapsed / 1000U) % 60U;
            const uint32_t tenths    = (elapsed / 100U) % 10U;
            const uint32_t value     = minute * 1000U + seconds * 10U + tenths;
            uint32_t       remaining = value;
            for (uint8_t offset = 0; offset < 4; ++offset)
            {
                const uint8_t digit = static_cast<uint8_t> (3U - offset);
                require (
                    snapshot.digitFrame.glyphs[digit] ==
                            static_cast<adk::SevenSegmentGlyph> (remaining % 10U) &&
                        snapshot.digitFrame.diagnosticGlyphs[digit] ==
                            adk::MultiplexedDigitDiagnosticGlyph::None,
                    "ordinary digit value uses four decimal glyphs");
                remaining /= 10U;
            }
            require (snapshot.digitFrame.decimalMask == 0x0aU &&
                         snapshot.digitDigest ==
                             expectedDigitDigest (snapshot.digitFrame, 0) &&
                         snapshot.matrixDigest ==
                             expectedMatrixDigest (snapshot.matrixFrame, 0),
                     "ordinary stopped presentation has exact separators and digests");
        }

        adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
            desk, adk::TimingDeskStopwatchState::Running, 599899,
            adk::TimePoint (driver.now));
        ++driver.now;
        const auto saturated = driver.step ();

        require (saturated.ok () &&
                     desk.snapshot ().stopwatchState ==
                         adk::TimingDeskStopwatchState::Stopped &&
                     desk.snapshot ().elapsed == adk::Duration (599900),
                 "capacity saturates visibly and stops without wrapping");
    }

    void testReceiptCorrelationCollisionMatrix ()
    {
        for (uint8_t side = 0; side < 2; ++side)
        {
            for (uint8_t collision = 0; collision < 9; ++collision)
            {
                adk::DualDisplayTimingDesk desk (config ());

                require (desk.initialize (adk::TimePoint (0)).ok (),
                         "receipt-collision desk initializes");

                DeskDriver driver (desk);

                driver.configure ();

                const auto target = driver.makeCurrentPresentationReady ();

                auto digit = driver.digitReceiptFor (target);

                auto matrix = driver.matrixReceiptFor (target);

                if (side == 0)
                {
                    switch (collision)
                    {
                        case 0: ++digit.ownerToken; break;
                        case 1: ++digit.lifecycleGeneration; break;
                        case 2: ++digit.configurationRevision; break;
                        case 3: ++digit.requestedGeneration; break;
                        case 4: ++digit.acceptedGeneration; break;
                        case 5: ++digit.reportedFrame.generation; break;
                        case 6: digit.reportedDigest ^= 1U; break;
                        case 7: digit.blankRequestAccepted = true; break;
                        case 8:
                            digit.observedAt = adk::TimePoint (driver.now + 1U);
                            break;
                    }
                }
                else
                {
                    switch (collision)
                    {
                        case 0: ++matrix.ownerToken; break;
                        case 1: ++matrix.lifecycleGeneration; break;
                        case 2: ++matrix.configurationRevision; break;
                        case 3: ++matrix.requestedGeneration; break;
                        case 4: ++matrix.acceptedGeneration; break;
                        case 5: matrix.reportedFrame.rows[3] ^= 0x04U; break;
                        case 6: matrix.reportedDigest ^= 1U; break;
                        case 7: matrix.blankRequestAccepted = true; break;
                        case 8:
                            matrix.observedAt = adk::TimePoint (driver.now + 1U);
                            break;
                    }
                }

                const auto result = driver.step (&digit, &matrix);

                const auto expectedOwner =
                    side == 0 ? adk::TimingDeskFaultOwner::DigitDisplay
                              : adk::TimingDeskFaultOwner::MatrixDisplay;
                const auto faultSnapshot = desk.snapshot ();

                const auto faultResult = result.value ();

                require (!result.ok () && faultSnapshot.faultOwner == expectedOwner &&
                             faultResult.digitBlankRequested &&
                             faultResult.matrixBlankRequested,
                         "receipt collision is side-attributed and blanks both");
            }
        }
    }

    void testLapDeadlineRolloverAndGenerationExhaustion ()
    {
        adk::DualDisplayTimingDesk lapDesk (config ());

        require (lapDesk.initialize (adk::TimePoint (0)).ok (),
                 "lap rollover desk initializes");

        DeskDriver lapDriver (lapDesk);

        lapDriver.qualify ();

        const uint32_t anchor   = UINT32_MAX - 500U;
        const uint32_t deadline = anchor + 2000U;
        adk::DualDisplayTimingDeskTestAccess::forceLap (lapDesk, adk::Duration (4321),
                                                        adk::TimePoint (deadline),
                                                        adk::TimePoint (deadline));
        lapDriver.now        = deadline;
        const auto inclusive = lapDriver.step ();

        const auto inclusiveSnapshot = lapDesk.snapshot ();

        require (inclusive.ok () && inclusiveSnapshot.lapVisible &&
                     inclusiveSnapshot.lapElapsed == adk::Duration (4321),
                 "lap remains visible at inclusive rollover deadline");
        const auto expired = lapDriver.step ();

        const auto expiredSnapshot = lapDesk.snapshot ();

        require (expired.ok () && !expiredSnapshot.lapVisible,
                 "lap expires on first rollover-safe tick after deadline");

        adk::DualDisplayTimingDesk lifecycleDesk (config ());

        require (lifecycleDesk.initialize (adk::TimePoint (0)).ok (),
                 "lifecycle exhaustion desk initializes");
        adk::DualDisplayTimingDeskTestAccess::setLifecycleGeneration (lifecycleDesk,
                                                                      UINT32_MAX);
        lifecycleDesk.reset (adk::TimePoint (1));

        const auto lifecycleSnapshot = lifecycleDesk.snapshot ();

        require (!lifecycleDesk.initialized () &&
                     lifecycleSnapshot.status.error () ==
                         adk::StatusCode::CapacityExceeded &&
                     lifecycleSnapshot.faultOwner ==
                         adk::TimingDeskFaultOwner::Coordinator,
                 "lifecycle generation faults before wrapping to zero");

        adk::DualDisplayTimingDesk snapshotDesk (config ());

        require (snapshotDesk.initialize (adk::TimePoint (0)).ok (),
                 "snapshot exhaustion desk initializes");
        DeskDriver snapshotDriver (snapshotDesk);

        snapshotDriver.qualify ();

        adk::DualDisplayTimingDeskTestAccess::exhaustSnapshotSequence (snapshotDesk);

        const auto snapshotExhausted = snapshotDriver.step ();

        const auto exhaustedSnapshot = snapshotDesk.snapshot ();

        require (!snapshotExhausted.ok () &&
                     exhaustedSnapshot.status.error () ==
                         adk::StatusCode::CapacityExceeded &&
                     exhaustedSnapshot.faultOwner ==
                         adk::TimingDeskFaultOwner::Coordinator,
                 "snapshot sequence faults before wrapping to zero");

        adk::DualDisplayTimingDesk childDesk (config ());

        require (childDesk.initialize (adk::TimePoint (0)).ok (),
                 "child generation exhaustion desk initializes");
        DeskDriver childDriver (childDesk);

        childDriver.qualify ();

        adk::DualDisplayTimingDeskTestAccess::exhaustChildFrameGenerations (childDesk);

        const auto childExhausted = childDriver.step ();

        const auto childSnapshot = childDesk.snapshot ();

        require (!childExhausted.ok () &&
                     childSnapshot.status.error () ==
                         adk::StatusCode::CapacityExceeded &&
                     childSnapshot.faultOwner == adk::TimingDeskFaultOwner::Coordinator,
                 "child generation exhaustion faults coordinator");
    }

    void testByteIdenticalFullTraceReplay ()
    {
        adk::DualDisplayTimingDesk left (config ());

        adk::DualDisplayTimingDesk right (config ());

        require (left.initialize (adk::TimePoint (0)).ok () &&
                     right.initialize (adk::TimePoint (0)).ok (),
                 "replay desks initialize identically");

        DeskDriver leftDriver (left);

        DeskDriver rightDriver (right);

        leftDriver.configure ();

        rightDriver.configure ();

        const auto leftConfigured = left.snapshot ();

        const auto rightConfigured = right.snapshot ();

        require (sameSnapshot (leftConfigured, rightConfigured),
                 "configuration replay is field-identical");

        for (uint8_t stage = 0; stage < 14; ++stage)
        {
            const auto leftTarget = leftDriver.makeCurrentPresentationReady ();

            const auto rightTarget = rightDriver.makeCurrentPresentationReady ();

            const auto leftServiced = left.snapshot ();

            const auto rightServiced = right.snapshot ();

            require (sameSnapshot (leftTarget, rightTarget) &&
                         sameSnapshot (leftServiced, rightServiced),
                     "self-test service replay is field-identical");

            const auto leftDigit = leftDriver.digitReceiptFor (leftTarget);

            const auto leftMatrix = leftDriver.matrixReceiptFor (leftTarget);

            const auto rightDigit = rightDriver.digitReceiptFor (rightTarget);

            const auto rightMatrix = rightDriver.matrixReceiptFor (rightTarget);

            const auto leftResult = leftDriver.step (&leftDigit, &leftMatrix);

            const auto rightResult = rightDriver.step (&rightDigit, &rightMatrix);

            const auto leftAcknowledged = left.snapshot ();

            const auto rightAcknowledged = right.snapshot ();

            const auto leftValue = leftResult.value ();

            const auto rightValue = rightResult.value ();

            const bool resultsSame = sameResult (leftValue, rightValue);

            const bool snapshotsSame =
                sameSnapshot (leftAcknowledged, rightAcknowledged);

            require (leftResult.status () == rightResult.status () && resultsSame &&
                         snapshotsSame,
                     "self-test acknowledgement replay is field-identical");
        }

        const auto leftStart = leftDriver.step (nullptr, nullptr, true);

        const auto rightStart = rightDriver.step (nullptr, nullptr, true);

        const auto leftStarted = left.snapshot ();

        const auto rightStarted = right.snapshot ();

        const auto leftStartValue = leftStart.value ();

        const auto rightStartValue = rightStart.value ();

        const bool startResultsSame = sameResult (leftStartValue, rightStartValue);

        const bool startSnapshotsSame = sameSnapshot (leftStarted, rightStarted);

        require (leftStart.status () == rightStart.status () && startResultsSame &&
                     startSnapshotsSame,
                 "running control replay is field-identical");
    }

    void testStopwatchRolloverHeldLevelsAndLargeIncrement ()
    {
        adk::DualDisplayTimingDesk rolloverDesk (config ());

        require (rolloverDesk.initialize (adk::TimePoint (0)).ok (),
                 "rollover stopwatch desk initializes");
        adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
            rolloverDesk, adk::TimingDeskStopwatchState::Running, 1000,
            adk::TimePoint (UINT32_MAX - 5U));
        adk::DualDisplayTimingDeskTestAccess::prepareTimeJump (rolloverDesk,
                                                               adk::TimePoint (4));
        const auto rollover = rolloverDesk.update (envelope (4, 1));

        const auto rolloverSnapshot = rolloverDesk.snapshot ();

        require (rollover.ok () &&
                     rolloverSnapshot.stopwatchState ==
                         adk::TimingDeskStopwatchState::Running &&
                     rolloverSnapshot.elapsed == adk::Duration (1010),
                 "running stopwatch advances exactly across uint32 rollover");

        adk::DualDisplayTimingDesk heldStartDesk (config ());

        require (heldStartDesk.initialize (adk::TimePoint (100)).ok (),
                 "held-start desk initializes");
        adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
            heldStartDesk, adk::TimingDeskStopwatchState::Stopped, 0,
            adk::TimePoint (100));
        require (
            heldStartDesk.update (envelope (100, 1, nullptr, nullptr, nullptr, true))
                .ok (),
            "first start edge is admitted");
        auto heldStart = envelope (101, 1, nullptr, nullptr, nullptr, false);
        heldStart.startPause.pressed    = true;
        heldStart.startPause.pressEvent = false;

        const auto heldStartResult = heldStartDesk.update (heldStart);

        const auto heldStartSnapshot = heldStartDesk.snapshot ();

        require (heldStartResult.ok () && heldStartSnapshot.stopwatchState ==
                                              adk::TimingDeskStopwatchState::Running,
                 "held start level at same sequence does not pause again");

        adk::DualDisplayTimingDesk heldLapDesk (config ());

        require (heldLapDesk.initialize (adk::TimePoint (200)).ok (),
                 "held-lap desk initializes");
        adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
            heldLapDesk, adk::TimingDeskStopwatchState::Paused, 1234,
            adk::TimePoint (200));
        const auto firstLap = heldLapDesk.update (
            envelope (200, 1, nullptr, nullptr, nullptr, false, true));

        require (firstLap.ok (), "first lap edge is admitted");

        adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
            heldLapDesk, adk::TimingDeskStopwatchState::Paused, 4321,
            adk::TimePoint (201));
        auto heldLap           = envelope (201, 1);
        heldLap.lap.pressed    = true;
        heldLap.lap.pressEvent = false;

        const auto heldLapResult = heldLapDesk.update (heldLap);

        const auto heldLapSnapshot = heldLapDesk.snapshot ();

        require (heldLapResult.ok () &&
                     heldLapSnapshot.lapElapsed == adk::Duration (1234),
                 "held lap level at same sequence does not recapture elapsed");

        adk::DualDisplayTimingDesk largeDesk (config ());

        require (largeDesk.initialize (adk::TimePoint (100)).ok (),
                 "large-increment desk initializes");
        adk::DualDisplayTimingDeskTestAccess::forceStopwatch (
            largeDesk, adk::TimingDeskStopwatchState::Running, 1000,
            adk::TimePoint (100));
        const uint32_t largeNow = 100U + 700000U;
        adk::DualDisplayTimingDeskTestAccess::prepareTimeJump (
            largeDesk, adk::TimePoint (largeNow));
        const auto large = largeDesk.update (envelope (largeNow, 1));

        const auto largeSnapshot = largeDesk.snapshot ();

        require (large.ok () &&
                     largeSnapshot.stopwatchState ==
                         adk::TimingDeskStopwatchState::Stopped &&
                     largeSnapshot.elapsed == adk::Duration (599900),
                 "large valid forward increment saturates without underflow");
    }

    void testLiteralDigestWitnesses ()
    {
        const adk::MultiplexedDigitFrame digit = {
            {static_cast<adk::SevenSegmentGlyph> (0),
             static_cast<adk::SevenSegmentGlyph> (1),
             static_cast<adk::SevenSegmentGlyph> (2),
             static_cast<adk::SevenSegmentGlyph> (3)},
            {static_cast<adk::MultiplexedDigitDiagnosticGlyph> (1),
             static_cast<adk::MultiplexedDigitDiagnosticGlyph> (2),
             static_cast<adk::MultiplexedDigitDiagnosticGlyph> (3),
             static_cast<adk::MultiplexedDigitDiagnosticGlyph> (4)},
            0x0a,
            UINT32_C (0x01020304),
            UINT32_C (0x11223344),
            true};
        const adk::Max7219Frame matrix = {
            {0, 1, 2, 3, 4, 5, 6, 7}, UINT32_C (0x01020304), UINT32_C (0x11223344)};

        require (expectedDigitDigest (digit, 0x80) == UINT32_C (0x80dc784f),
                 "digit digest literal fixes exact serialized witness");
        require (expectedMatrixDigest (matrix, 0x80) == UINT32_C (0x2887577e),
                 "matrix digest literal fixes exact serialized witness");
    }
} // namespace

int main ()
{
    testConfigurationAndLifecycle ();

    testInvalidConfiguration ();

    testStructuralEvidenceRejectsAtomically ();

    testCompleteSelfTestGoldenFrames ();

    testReceiptFaultBlankingAndReset ();

    testGraceDeadlineAndMalformedResetCompanion ();

    testEveryControlMaskInEveryStopwatchState ();

    testOrdinaryMappingBucketsAndCapacity ();

    testReceiptCorrelationCollisionMatrix ();

    testLapDeadlineRolloverAndGenerationExhaustion ();

    testByteIdenticalFullTraceReplay ();

    testStopwatchRolloverHeldLevelsAndLargeIncrement ();

    testLiteralDigestWitnesses ();
    return EXIT_SUCCESS;
}
