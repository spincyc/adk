// E0 parts-carousel fixture. This sketch replays copied synthetic evidence and
// retains semantic intent in memory. It owns no endpoint, pin, bus, clock,
// reader, keypad, nonvolatile medium, actuator, or physical-position evidence.
#include <Adk.h>
#include <inert_parts_carousel.h>

namespace {

    struct ReplayFrame
    {
        uint32_t now;
        bool     hasIdentity;
        bool     hasKey;
        bool     homeActive;
        bool     stopActive;
        uint8_t  digitCount;
        uint8_t  firstDigit;
        uint8_t  secondDigit;
        bool     confirm;
        bool     cancel;
    };

    struct CarouselResultCell
    {
        int32_t  logicalPosition;
        uint32_t operationId;
        uint8_t  phase;
        uint8_t  fault;
        uint8_t  requestedBin;
        uint8_t  authorizationCurrent;
        uint8_t  positionKnown;
        uint8_t  coilBits;
        uint8_t  gateIntent;
        uint8_t  auditStatus;
        uint8_t  durableAdmissionPending;
        uint8_t  terminalReconciliationPending;
        uint8_t  operationStatus;
        uint8_t  policyStatus;
        uint8_t  intentIsInert;
    };

    const adk::CarouselSource identitySource = {
        adk::CarouselSourceKind::SyntheticIdentity, 1, 1};
    const adk::CarouselSource keySource = {adk::CarouselSourceKind::SyntheticKey, 1, 1};
    const adk::CarouselSource homeSource = {adk::CarouselSourceKind::SyntheticHome, 1,
                                            1};
    const adk::CarouselSource stopSource = {adk::CarouselSourceKind::SyntheticStop, 1,
                                            1};

    const adk::LocalIdentity selectedIdentity = {
        4, {0x12, 0x34, 0x56, 0x78, 0, 0, 0, 0, 0, 0}};

    // clang-format off
    // A reconstructed registry lifetime uses a different nonzero synthetic epoch.
    const adk::LocalIdentityRegistryConfig registryConfig = {
        0x04905101UL, 0x05100001UL, 4, 2, adk::Duration (100), adk::Duration (50)};
    const adk::BoundedHomingConfig homingConfig = {
        -4, 4, 0, 1, 2, 4, adk::Duration (50), adk::Duration (100),

        adk::Duration (10), adk::Duration (30), adk::Duration (5)};
    const adk::CarouselConfig carouselConfig = {
        4,
        {-2, 0, 2, 4, 0, 0, 0, 0},
        0x05100001UL,
        2,
        {11, 23, 42, 74, 0, 0, 0, 0},
        10,
        11,
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        adk::Duration (80),
        adk::Duration (20),
        adk::Duration (10),
        adk::Duration (40),
        adk::Duration (30),
        adk::Duration (5)};

    const ReplayFrame replayFrames[] = {
        {0, true, false, false, false, 0, 0, 0, false, false},
        {1, false, true, false, false, 2, 4, 2, true, false},
        {2, false, false, false, false, 0, 0, 0, false, false},
        {12, false, false, false, false, 0, 0, 0, false, false},
        {22, false, false, true, false, 0, 0, 0, false, false},
        {32, false, false, true, false, 0, 0, 0, false, false},
        {42, false, false, true, false, 0, 0, 0, false, false},
        {52, false, false, true, false, 0, 0, 0, false, false},
        {62, false, false, true, true, 0, 0, 0, false, false}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    adk::IdentityBinding liveBindings[adk::maximumLocalIdentities];
    uint8_t identitySlots[adk::localIdentitySlotCount * adk::localIdentityImageBytes];
    uint8_t identityCandidate[adk::localIdentityImageBytes];
    uint8_t auditSlots[4 * adk::carouselAuditRecordBytes];
    uint8_t auditCandidate[adk::carouselAuditRecordBytes];

    adk::LocalIdentityRegistry
        registry (registryConfig, liveBindings, adk::maximumLocalIdentities,
                  identitySlots, sizeof (identitySlots), adk::localIdentityImageBytes,
                  adk::localIdentitySlotCount, identityCandidate,
                  sizeof (identityCandidate));
    adk::BoundedHomingPolicy homingPolicy (homingConfig);

    adk::InertPartsCarousel carousel (
        carouselConfig, registry, homingPolicy, auditSlots, sizeof (auditSlots),
        adk::carouselAuditRecordBytes, 4, auditCandidate, sizeof (auditCandidate));
    // clang-format on

    volatile CarouselResultCell resultCells[replayFrameCount];
    volatile uint8_t            fixtureStatusCell;
    volatile uint8_t            initializeStatusCell;
    volatile uint8_t            replayActiveCell;
    volatile uint8_t            completedReplayFramesCell;

    uint8_t replayIndex;
    bool    replayActive;

    // clang-format off
    uint16_t                calculateCrc             (const uint8_t* bytes,
                                                     uint16_t length);
    void                    store16                  (uint8_t* bytes,
                                                     uint16_t value);
    void                    store32                  (uint8_t* bytes,
                                                     uint32_t value);
    void                    prepareIdentityImage     ();
    adk::Status             configureReplay          ();
    adk::CarouselInputFrame observeCopiedEvidence    (const ReplayFrame& frame,
                                                      uint32_t sequence);
    adk::Status             decideCarouselIntent     (
        const adk::CarouselInputFrame& input);
    adk::Status             reconcileAuditIntent     ();
    void                    presentCarouselIntent    (uint8_t index,
                                                      adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    // clang-format off
    const adk::Status fixtureStatus = configureReplay ();

    fixtureStatusCell               = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    const adk::Status initializationStatus = carousel.initialize ();

    initializeStatusCell = static_cast<uint8_t> (initializationStatus.error ());

    if (!initializationStatus.ok ())
    {
        return;
    }
    // clang-format on

    replayIndex      = 0;
    replayActive     = true;
    replayActiveCell = 1;
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame&            frame = replayFrames[replayIndex];
    const adk::CarouselInputFrame input =
        observeCopiedEvidence (frame, static_cast<uint32_t> (replayIndex) + 1);
    adk::Status operationStatus = decideCarouselIntent (input);

    if (operationStatus.ok ())
    {
        operationStatus = reconcileAuditIntent ();
    }

    presentCarouselIntent (replayIndex, operationStatus);

    ++replayIndex;
    completedReplayFramesCell = replayIndex;
    replayActive              = replayIndex < replayFrameCount;
    replayActiveCell          = replayActive ? 1 : 0;
}

namespace {

    uint16_t calculateCrc (const uint8_t* bytes, uint16_t length)
    {
        uint16_t crc = 0xffff;
        for (uint16_t index = 0; index < length; ++index)
        {
            crc ^= static_cast<uint16_t> (bytes[index]) << 8;
            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                crc = (crc & 0x8000) != 0 ? static_cast<uint16_t> ((crc << 1) ^ 0x1021)
                                          : static_cast<uint16_t> (crc << 1);
            }
        }
        return crc;
    }

    void store16 (uint8_t* bytes, uint16_t value)
    {
        bytes[0] = static_cast<uint8_t> (value);
        bytes[1] = static_cast<uint8_t> (value >> 8);
    }

    void store32 (uint8_t* bytes, uint32_t value)
    {
        for (uint8_t index = 0; index < 4; ++index)
        {
            bytes[index] = static_cast<uint8_t> (value >> (8 * index));
        }
    }

    void prepareIdentityImage ()
    {
        for (uint16_t index = 0; index < sizeof (identitySlots); ++index)
        {
            identitySlots[index] = 0xff;
        }
        for (uint16_t index = 0; index < adk::localIdentityImageBytes; ++index)
        {
            identitySlots[index] = 0;
        }

        store16 (&identitySlots[0], adk::localIdentityImageMagic);
        identitySlots[2] = adk::localIdentityImageVersion;
        store16 (&identitySlots[4], adk::localIdentityImageBytes);
        store32 (&identitySlots[6], registryConfig.registryConfigurationId);
        store32 (&identitySlots[10], 1);
        identitySlots[14] = 1;

        identitySlots[16] = selectedIdentity.length;
        for (uint8_t index = 0; index < adk::maximumLocalIdentityBytes; ++index)
        {
            identitySlots[17 + index] = selectedIdentity.bytes[index];
        }
        identitySlots[27] = 2;
        store16 (&identitySlots[28], 7);
        store16 (&identitySlots[30], calculateCrc (&identitySlots[16], 14));
        store16 (&identitySlots[158], calculateCrc (identitySlots, 158));
    }

    adk::Status configureReplay ()
    {
        prepareIdentityImage ();
        for (uint16_t index = 0; index < sizeof (auditSlots); ++index)
        {
            auditSlots[index] = 0xff;
        }
        for (uint8_t index = 0; index < sizeof (auditCandidate); ++index)
        {
            auditCandidate[index] = 0;
        }

        replayIndex               = 0;
        replayActive              = false;
        replayActiveCell          = 0;
        completedReplayFramesCell = 0;
        return replayFrameCount > 0 ? adk::StatusCode::Ok
                                    : adk::StatusCode::InvalidConfiguration;
    }

    adk::CarouselInputFrame observeCopiedEvidence (const ReplayFrame& frame,
                                                   uint32_t           sequence)
    {
        const adk::TimePoint observedAt (frame.now);
        // clang-format off
        adk::CarouselInputFrame input = {
            adk::TimePoint (),
            0,
            false,
            {identitySource, adk::TimePoint (), 0, adk::LocalIdentity {},
             adk::StatusCode::Ok},
            false,
            {keySource,
             adk::TimePoint (),
             0,
             0,
             {0, 0, 0, 0},
             false,
             false,
            adk::StatusCode::Ok},
            {homeSource, adk::TimePoint (), 0, false, false, 0, adk::StatusCode::Ok},

            {stopSource, adk::TimePoint (), 0, false, false, 0, adk::StatusCode::Ok},

            {adk::TimePoint (), 0, adk::StatusCode::Ok}};
        // clang-format on
        input.observedAt          = observedAt;
        input.sequence            = sequence;
        input.hasIdentity         = frame.hasIdentity;
        input.identity.source     = identitySource;
        input.identity.observedAt = observedAt;
        input.identity.sequence   = frame.hasIdentity ? sequence : 0;
        input.identity.identity =
            frame.hasIdentity ? selectedIdentity : adk::LocalIdentity{};
        input.identity.status = adk::StatusCode::Ok;
        input.hasKey          = frame.hasKey;
        input.key.source      = keySource;
        input.key.observedAt  = frame.hasKey ? observedAt : adk::TimePoint ();
        input.key.sequence    = frame.hasKey ? sequence : 0;
        input.key.digitCount  = frame.hasKey ? frame.digitCount : 0;
        input.key.digits[0]   = frame.hasKey ? frame.firstDigit : 0;
        input.key.digits[1]   = frame.hasKey ? frame.secondDigit : 0;
        input.key.confirm     = frame.hasKey && frame.confirm;
        input.key.cancel      = frame.hasKey && frame.cancel;
        input.key.status      = adk::StatusCode::Ok;
        input.home = {homeSource, observedAt, sequence,           frame.homeActive,
                      true,       1,          adk::StatusCode::Ok};
        input.stop = {stopSource, observedAt, sequence,           frame.stopActive,
                      true,       1,          adk::StatusCode::Ok};
        input.presentation = {observedAt, sequence, adk::StatusCode::Ok};
        return input;
    }

    adk::Status decideCarouselIntent (const adk::CarouselInputFrame& input)
    {
        return carousel.update (input.observedAt, input);
    }

    adk::Status reconcileAuditIntent ()
    {
        const adk::Result<adk::DurableAuditCandidate> candidateResult =
            carousel.previewAuditWrite ();
        if (!candidateResult.ok ())
        {
            return candidateResult.status ().error () == adk::StatusCode::NotInitialized
                       ? adk::StatusCode::Ok
                       : candidateResult.status ();
        }

        const adk::DurableAuditCandidate        candidate = candidateResult.value ();
        const adk::Result<adk::AuditRecordView> exportResult =
            carousel.previewAuditExport (candidate);
        if (!exportResult.ok ())
        {
            return exportResult.status ();
        }

        const adk::AuditRecordView source = exportResult.value ();
        uint8_t* const             destination =
            &auditSlots[static_cast<uint16_t> (candidate.slot) *
                        adk::carouselAuditRecordBytes];
        for (uint8_t index = 0; index < source.length; ++index)
        {
            destination[index] = source.bytes[index];
        }

        const adk::AuditRecordView reconciled = {destination, source.length,
                                                 candidate.slot, candidate.operationId};
        const adk::AuditDurableCommitEvidence evidence = {candidate.owner,
                                                          candidate.generation,
                                                          candidate.operationId,
                                                          candidate.slot,
                                                          reconciled,
                                                          true,
                                                          true,
                                                          adk::StatusCode::Ok};
        return carousel.acknowledgeAuditWrite (candidate, evidence);
    }

    void presentCarouselIntent (uint8_t index, adk::Status operationStatus)
    {
        const adk::CarouselSnapshot snapshot    = carousel.snapshot ();
        resultCells[index].logicalPosition      = snapshot.logicalPosition;
        resultCells[index].operationId          = snapshot.operationId;
        resultCells[index].phase                = static_cast<uint8_t> (snapshot.phase);
        resultCells[index].fault                = static_cast<uint8_t> (snapshot.fault);
        resultCells[index].requestedBin         = snapshot.requestedBin;
        resultCells[index].authorizationCurrent = snapshot.authorizationCurrent ? 1 : 0;
        resultCells[index].positionKnown        = snapshot.positionKnown ? 1 : 0;
        resultCells[index].coilBits             = snapshot.intent.coilBits;
        resultCells[index].gateIntent = static_cast<uint8_t> (snapshot.intent.gate);
        resultCells[index].auditStatus =
            static_cast<uint8_t> (snapshot.intent.statusCode);
        resultCells[index].durableAdmissionPending =
            snapshot.durableAdmissionPending ? 1 : 0;
        resultCells[index].terminalReconciliationPending =
            snapshot.terminalReconciliationPending ? 1 : 0;
        resultCells[index].operationStatus =
            static_cast<uint8_t> (operationStatus.error ());
        resultCells[index].policyStatus =
            static_cast<uint8_t> (snapshot.status.error ());
        resultCells[index].intentIsInert =
            (snapshot.intent.coilBits & 0xf0U) == 0 ? 1 : 0;
    }

} // namespace
