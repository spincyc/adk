// E0 local-identity fixture. This sketch replays copied synthetic identifiers
// and stores registry outcomes in memory. It owns no endpoint, pin, bus,
// reader, keypad, clock, nonvolatile medium, or powered circuit.
#include <Adk.h>
#include <local_identity_registry.h>

namespace {

    enum struct ReplayAction : uint8_t
    {
        Observe,
        EnrollAndCommit,
        PreviewAndCancel
    };

    struct ReplayFrame
    {
        uint32_t     now;
        uint32_t     observedAt;
        uint32_t     sequence;
        uint8_t      identityIndex;
        ReplayAction action;
        uint8_t      binId;
    };

    struct RegistryCell
    {
        uint32_t acceptedSequence;
        uint32_t imageGeneration;
        uint16_t matchedBindingRevision;
        uint8_t  disposition;
        uint8_t  selectedBin;
        uint8_t  bindingCount;
        uint8_t  failedAttempts;
        uint8_t  enrollmentPending;
        uint8_t  externalCommitPending;
        uint8_t  operationStatus;
        uint8_t  registryStatus;
        uint8_t  predictionPass;
        uint8_t  available;
    };

    const adk::CarouselSource identitySource = {
        adk::CarouselSourceKind::SyntheticIdentity, 1, 1};

    const adk::LocalIdentity identities[] = {
        {4, {0x12, 0x34, 0x56, 0x78, 0, 0, 0, 0, 0, 0}},
        {7, {0x04, 0xa1, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6, 0, 0, 0}},
        {10, {0x91, 0x82, 0x73, 0x64, 0x55, 0x46, 0x37, 0x28, 0x19, 0x0a}}};

    const ReplayFrame replayFrames[] = {
        {10, 10, 1, 0, ReplayAction::Observe, 0},
        {20, 20, 2, 0, ReplayAction::EnrollAndCommit, 2},
        {30, 30, 3, 0, ReplayAction::Observe, 0},
        {40, 40, 4, 1, ReplayAction::Observe, 0},
        {40, 40, 4, 1, ReplayAction::Observe, 0},
        {50, 50, 5, 2, ReplayAction::PreviewAndCancel, 3},
        {60, 60, 6, 1, ReplayAction::Observe, 0},
        {70, 70, 7, 2, ReplayAction::Observe, 0}};

    constexpr uint8_t replayFrameCount =
        sizeof (replayFrames) / sizeof (replayFrames[0]);

    const adk::IdentityDisposition expectedDisposition[] = {
        adk::IdentityDisposition::Unknown,   adk::IdentityDisposition::None,
        adk::IdentityDisposition::Known,     adk::IdentityDisposition::Unknown,
        adk::IdentityDisposition::Duplicate, adk::IdentityDisposition::None,
        adk::IdentityDisposition::LockedOut, adk::IdentityDisposition::LockedOut};
    const uint8_t expectedSelectedBin[] = {0, 0, 2, 0, 0, 0, 0, 0};

    adk::IdentityBinding liveBindings[adk::maximumLocalIdentities];
    uint8_t imageSlots[adk::localIdentitySlotCount * adk::localIdentityImageBytes];
    uint8_t candidateImage[adk::localIdentityImageBytes];

    const adk::LocalIdentityRegistryConfig registryConfig = {
        0x04900001UL, 4, 2, adk::Duration (100), adk::Duration (50)};

    adk::LocalIdentityRegistry
        registry (registryConfig, liveBindings, adk::maximumLocalIdentities, imageSlots,
                  sizeof (imageSlots), adk::localIdentityImageBytes,
                  adk::localIdentitySlotCount, candidateImage, sizeof (candidateImage));

    volatile RegistryCell resultCells[replayFrameCount];
    volatile uint8_t      fixtureStatusCell;
    volatile uint8_t      initializeStatusCell;
    volatile uint8_t      recoveredFromCorruptSlotCell;
    volatile uint8_t      completedReplayFramesCell;
    volatile uint8_t      replayActiveCell;

    uint8_t replayIndex;
    bool    replayActive;

    // clang-format off
    uint16_t            calculateCrc            (const uint8_t* bytes,
                                                 uint16_t length);
    void                store16                 (uint8_t* bytes, uint16_t value);
    void                store32                 (uint8_t* bytes, uint32_t value);
    void                prepareEmptyImage       (uint8_t* bytes,
                                                 uint32_t generation);
    adk::Status         configureReplay         ();
    adk::IdentityEvidence observeIdentity       (const ReplayFrame& frame);
    adk::Status         decideRegistryAction    (const ReplayFrame& frame,
                                                 const adk::IdentityEvidence& evidence);
    adk::Status         commitCandidate         (
        const adk::EnrollmentCandidate& candidate);
    void                presentRegistrySnapshot (uint8_t index,
                                                 adk::Status operationStatus);
    // clang-format on

} // namespace

void setup ()
{
    const adk::Status fixtureStatus = configureReplay ();

    fixtureStatusCell = static_cast<uint8_t> (fixtureStatus.error ());

    if (!fixtureStatus.ok ())
    {
        return;
    }

    const adk::Status initialized = registry.initialize ();

    initializeStatusCell = static_cast<uint8_t> (initialized.error ());

    if (!initialized.ok ())
    {
        return;
    }

    recoveredFromCorruptSlotCell = registry.snapshot ().imageGeneration == 1 ? 1 : 0;
    replayIndex                  = 0;
    replayActive                 = true;
    replayActiveCell             = 1;
}

void loop ()
{
    if (!replayActive)
    {
        return;
    }

    const ReplayFrame& frame = replayFrames[replayIndex];

    const adk::IdentityEvidence evidence = observeIdentity (frame);

    const adk::Status operationStatus = decideRegistryAction (frame, evidence);

    presentRegistrySnapshot (replayIndex, operationStatus);

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
        bytes[0] = static_cast<uint8_t> (value);
        bytes[1] = static_cast<uint8_t> (value >> 8);
        bytes[2] = static_cast<uint8_t> (value >> 16);
        bytes[3] = static_cast<uint8_t> (value >> 24);
    }

    void prepareEmptyImage (uint8_t* bytes, uint32_t generation)
    {
        for (uint16_t index = 0; index < adk::localIdentityImageBytes; ++index)
        {
            bytes[index] = 0;
        }
        store16 (&bytes[0], adk::localIdentityImageMagic);
        bytes[2] = adk::localIdentityImageVersion;
        store16 (&bytes[4], adk::localIdentityImageBytes);
        store32 (&bytes[6], registryConfig.registryConfigurationId);
        store32 (&bytes[10], generation);
        store16 (&bytes[158], calculateCrc (bytes, 158));
    }

    adk::Status configureReplay ()
    {
        if (sizeof (expectedDisposition) / sizeof (expectedDisposition[0]) !=
                replayFrameCount ||
            sizeof (expectedSelectedBin) / sizeof (expectedSelectedBin[0]) !=
                replayFrameCount)
        {
            return adk::StatusCode::InvalidConfiguration;
        }

        fixtureStatusCell            = 0xff;
        initializeStatusCell         = 0xff;
        recoveredFromCorruptSlotCell = 0;
        completedReplayFramesCell    = 0;
        replayActiveCell             = 0;
        replayIndex                  = 0;
        replayActive                 = false;

        prepareEmptyImage (&imageSlots[0], 1);
        for (uint16_t index = adk::localIdentityImageBytes; index < sizeof (imageSlots);
             ++index)
        {
            imageSlots[index] = 0xff;
        }

        for (uint16_t index = 0; index < sizeof (candidateImage); ++index)
        {
            candidateImage[index] = 0;
        }
        for (uint8_t index = 0; index < replayFrameCount; ++index)
        {
            resultCells[index] = {0,    0,    0,    0xff, 0xff, 0xff, 0xff,
                                  0xff, 0xff, 0xff, 0xff, 0xff, 0};
            if (replayFrames[index].identityIndex >=
                    sizeof (identities) / sizeof (identities[0]) ||
                replayFrames[index].sequence == 0 ||
                replayFrames[index].observedAt > replayFrames[index].now)
            {
                return adk::StatusCode::InvalidConfiguration;
            }
        }
        return adk::StatusCode::Ok;
    }

    adk::IdentityEvidence observeIdentity (const ReplayFrame& frame)
    {
        return {identitySource, adk::TimePoint (frame.observedAt), frame.sequence,
                identities[frame.identityIndex], adk::StatusCode::Ok};
    }

    adk::Status decideRegistryAction (const ReplayFrame&           frame,
                                      const adk::IdentityEvidence& evidence)
    {
        if (frame.action == ReplayAction::Observe)
        {
            return registry.observe (adk::TimePoint (frame.now), evidence);
        }

        const adk::Result<adk::EnrollmentCandidate> previewed =
            registry.previewEnrollment (adk::TimePoint (frame.now), evidence,
                                        frame.binId);
        if (!previewed.ok ())
        {
            return previewed.status ();
        }
        if (frame.action == ReplayAction::PreviewAndCancel)
        {
            return registry.cancelEnrollment ();
        }
        return commitCandidate (previewed.value ());
    }

    adk::Status commitCandidate (const adk::EnrollmentCandidate& candidate)
    {
        const adk::Result<adk::IdentityImageView> exported =
            registry.previewExport (candidate);
        if (!exported.ok ())
        {
            return exported.status ();
        }

        const adk::IdentityImageView view = exported.value ();
        uint8_t* const               durableSlot =
            &imageSlots[view.slot * adk::localIdentityImageBytes];
        for (uint16_t index = 0; index < view.length; ++index)
        {
            durableSlot[index] = view.bytes[index];
        }

        const adk::IdentityImageView reconciled = {durableSlot, view.length, view.slot,
                                                   view.generation};
        const adk::IdentityDurableCommitEvidence durable = {
            candidate.owner,
            candidate.candidateGeneration,
            candidate.operationId,
            view.slot,
            reconciled,
            true,
            true,
            adk::StatusCode::Ok};
        return registry.acknowledgeExternalCommit (candidate, durable);
    }

    void presentRegistrySnapshot (uint8_t index, adk::Status operationStatus)
    {
        const adk::IdentityRegistrySnapshot snapshot = registry.snapshot ();

        const uint8_t operationCode = static_cast<uint8_t> (operationStatus.error ());

        const uint8_t registryCode = static_cast<uint8_t> (snapshot.status.error ());

        const bool predictionPass =
            operationStatus.ok () && snapshot.status.ok () &&
            snapshot.disposition == expectedDisposition[index] &&
            snapshot.selectedBin == expectedSelectedBin[index];

        resultCells[index] = {snapshot.acceptedSequence,
                              snapshot.imageGeneration,
                              snapshot.matchedBindingRevision,
                              static_cast<uint8_t> (snapshot.disposition),
                              snapshot.selectedBin,
                              snapshot.bindingCount,
                              snapshot.failedAttempts,
                              snapshot.enrollmentPending ? 1 : 0,
                              snapshot.externalCommitPending ? 1 : 0,
                              operationCode,
                              registryCode,
                              predictionPass ? 1 : 0,
                              1};
    }

} // namespace
