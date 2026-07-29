#include <known_ir_emission_policy.h>

#include <infrared_decoder.h>
#include <pulse_capture.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>

namespace {
    constexpr uint32_t envelopeDuration  = 67980UL;
    constexpr uint32_t catalogDigest     = 0xbc6b6e95UL;
    constexpr uint32_t leaderDuration    = 13500UL;
    constexpr uint32_t bitMarkDuration   = 560UL;
    constexpr uint32_t zeroSpaceDuration = 560UL;
    constexpr uint32_t oneSpaceDuration  = 1690UL;

    struct TestCatalogEntry
    {
        uint8_t  codeId;
        uint32_t payload;
        uint8_t  encodingRevision;
        uint8_t  repeatCount;
    };

    void require (bool condition, const char* message);

    uint32_t deriveCatalogDigest (const TestCatalogEntry* entries,
                                  uint8_t                 count) noexcept
    {
        uint32_t digest = UINT32_C (2166136261);
        for (uint8_t index = 0; index < count; ++index)
        {
            const TestCatalogEntry& entry = entries[index];
            const uint8_t bytes[]         = {entry.codeId,
                                             static_cast<uint8_t> (entry.payload),
                                             static_cast<uint8_t> (entry.payload >> 8),
                                             static_cast<uint8_t> (entry.payload >> 16),
                                             static_cast<uint8_t> (entry.payload >> 24),
                                             entry.encodingRevision,
                                             entry.repeatCount};
            for (const uint8_t byte : bytes)
            {
                digest = (digest ^ byte) * UINT32_C (16777619);
            }
        }
        return digest;
    }

    void testCatalogDigestCoverage ()
    {
        const TestCatalogEntry canonical[] = {{0, 0xef10ff00UL, 1, 1},
                                              {1, 0xee11ff00UL, 1, 1},
                                              {2, 0xed12ff00UL, 1, 1},
                                              {3, 0xec13ff00UL, 1, 1}};
        require (deriveCatalogDigest (canonical, 4) == catalogDigest,
                 "independent canonical byte-order digest matches");

        for (uint8_t entryIndex = 0; entryIndex < 4; ++entryIndex)
        {
            TestCatalogEntry changed[4] = {canonical[0], canonical[1], canonical[2],
                                           canonical[3]};
#define REQUIRE_CATALOG_FIELD_COVERED(field, mask, message)                            \
    do                                                                                 \
    {                                                                                  \
        changed[entryIndex] = canonical[entryIndex];                                   \
        changed[entryIndex].field ^= mask;                                             \
        require (deriveCatalogDigest (changed, 4) != catalogDigest, message);          \
    }                                                                                  \
    while (false)
            REQUIRE_CATALOG_FIELD_COVERED (codeId, UINT8_C (0x80),
                                           "digest covers code ID");
            REQUIRE_CATALOG_FIELD_COVERED (payload, UINT32_C (0x01000000),
                                           "digest covers payload");
            REQUIRE_CATALOG_FIELD_COVERED (encodingRevision, UINT8_C (0x01),
                                           "digest covers encoding revision");
            REQUIRE_CATALOG_FIELD_COVERED (repeatCount, UINT8_C (0x01),
                                           "digest covers repeat count");
#undef REQUIRE_CATALOG_FIELD_COVERED
        }
    }

    uint32_t payloadFor (adk::LocalIrCodeId codeId) noexcept
    {
        const uint32_t payloads[] = {0xef10ff00UL, 0xee11ff00UL, 0xed12ff00UL,
                                     0xec13ff00UL};
        return payloads[static_cast<uint8_t> (codeId)];
    }

    adk::IrEnvelopeIntent expectedIntent (adk::LocalIrCodeId codeId,
                                          uint32_t           offset) noexcept
    {
        if (offset < 9000)
        {
            return adk::IrEnvelopeIntent::CarrierOn;
        }
        if (offset < leaderDuration)
        {
            return adk::IrEnvelopeIntent::CarrierOff;
        }

        uint32_t       bodyOffset = offset - leaderDuration;
        const uint32_t payload    = payloadFor (codeId);
        for (uint8_t bit = 0; bit < 32; ++bit)
        {
            if (bodyOffset < bitMarkDuration)
            {
                return adk::IrEnvelopeIntent::CarrierOn;
            }
            bodyOffset -= bitMarkDuration;
            const uint32_t spaceDuration = (payload & (uint32_t (1) << bit)) != 0
                                               ? oneSpaceDuration
                                               : zeroSpaceDuration;
            if (bodyOffset < spaceDuration)
            {
                return adk::IrEnvelopeIntent::CarrierOff;
            }
            bodyOffset -= spaceDuration;
        }
        return adk::IrEnvelopeIntent::CarrierOn;
    }

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::KnownIrEmissionConfig
    config (uint16_t revision = 7, uint32_t epoch = 19,
            uint32_t maximumDuration = envelopeDuration) noexcept
    {
        return {revision, epoch, adk::MicrosecondDuration (maximumDuration)};
    }

    bool snapshotEqual (const adk::KnownIrEmissionSnapshot& left,
                        const adk::KnownIrEmissionSnapshot& right) noexcept
    {
        return left.configurationRevision == right.configurationRevision &&
               left.instanceEpoch == right.instanceEpoch &&
               left.policyGeneration == right.policyGeneration &&
               left.candidateGeneration == right.candidateGeneration &&
               left.codeId == right.codeId &&
               left.catalog.revision == right.catalog.revision &&
               left.catalog.digest == right.catalog.digest &&
               left.transactionId == right.transactionId &&
               left.startAt.microseconds    () == right.startAt.microseconds () &&
               left.completeAt.microseconds () == right.completeAt.microseconds () &&
               left.repeatIndex == right.repeatIndex && left.intent == right.intent &&
               left.disposition == right.disposition &&
               left.terminalCause == right.terminalCause &&
               left.terminalTransactionId == right.terminalTransactionId &&
               left.terminalAt.microseconds () == right.terminalAt.microseconds () &&
               left.status == right.status;
    }

    adk::KnownIrEmissionPreview prepare (adk::KnownIrEmissionPolicy& policy,
                                         adk::LocalIrCodeId          codeId,
                                         uint32_t transactionId, uint32_t now)
    {
        const adk::Result<adk::KnownIrEmissionPreview> result =
            policy.prepare (codeId, transactionId, adk::MicrosecondTimePoint (now));
        require             (result.ok (), "prepare succeeds");
        return result.value ();
    }

    adk::KnownIrEmissionPreview start (adk::KnownIrEmissionPolicy& policy,
                                       adk::LocalIrCodeId          codeId,
                                       uint32_t transactionId, uint32_t now)
    {
        const adk::KnownIrEmissionPreview preview =
            prepare (policy, codeId, transactionId, now);
        require (policy.canCommit (preview, adk::MicrosecondTimePoint (now)),
                 "fresh preview is committable");
        require (policy.commit (preview, adk::MicrosecondTimePoint (now)).ok (),
                 "fresh preview commits");
        return preview;
    }

    template <typename Value> struct HasRawPrepare
    {
      private:
        template <typename Candidate>
        static auto check (int) -> decltype (std::declval<Candidate&> ().prepare (
                                                 std::declval<Value> (), uint32_t (),
                                                 adk::MicrosecondTimePoint        ()),
                                             std::true_type ());

        template <typename> static std::false_type check (...);

      public:
        static constexpr bool value =
            decltype (check<adk::KnownIrEmissionPolicy> (0))::value;
    };

    void testCompileSurfaceAndLifecycle ()
    {
        static_assert (!std::is_copy_constructible<adk::KnownIrEmissionPolicy>::value,
                       "emission policy does not copy");
        static_assert (!std::is_move_constructible<adk::KnownIrEmissionPolicy>::value,
                       "emission policy does not move");
        static_assert (sizeof (adk::KnownIrEmissionPolicy) <= 96,
                       "emission policy meets reusable-object target");
        static_assert (
            !std::is_constructible<adk::KnownIrEmissionPolicy, adk::PulseFrame>::value,
            "pulse frames cannot construct emission policy");
        static_assert (!std::is_constructible<adk::KnownIrEmissionPolicy,
                                              adk::InfraredFrame>::value,
                       "decoded frames cannot construct emission policy");
        static_assert (!HasRawPrepare<adk::PulseFrame>::value,
                       "prepare rejects captured pulse frames");
        static_assert (!HasRawPrepare<adk::InfraredFrame>::value,
                       "prepare rejects decoded infrared frames");
        static_assert (!HasRawPrepare<uint32_t*>::value,
                       "prepare rejects raw duration arrays");

        adk::KnownIrEmissionPolicy policy (config ());
        require                           (policy.snapshot ().disposition == adk::IrEmissionDisposition::Idle,
                 "construction is idle");
        require (policy.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                 "construction is inert");
        require (policy.snapshot ().status.error () == adk::StatusCode::NotInitialized,
                 "construction reports not initialized");
        require (policy.prepare (adk::LocalIrCodeId::StationPing, 1,
                                 adk::MicrosecondTimePoint (0))
                         .error () == adk::StatusCode::NotInitialized,
                 "prepare before initialize rejects");
        require (policy.update (adk::MicrosecondTimePoint (0)).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejects");
        require (policy.cancel (1, adk::MicrosecondTimePoint (0)).error () ==
                     adk::StatusCode::NotInitialized,
                 "cancel before initialize rejects");

        require                                          (policy.initialize ().ok (), "initialize succeeds");
        const uint32_t firstGeneration = policy.snapshot ().policyGeneration;
        require                                          (firstGeneration != 0, "initialize publishes generation");
        require                                          (policy.initialize ().ok (), "initialize is idempotent");
        require                                          (policy.snapshot ().policyGeneration == firstGeneration,
                 "idempotent initialize preserves generation");

        const adk::KnownIrEmissionPreview oldPreview =
            prepare (policy, adk::LocalIrCodeId::StationPing, 1, 4);
        policy.reset ();
        require      (policy.snapshot ().policyGeneration != firstGeneration,
                 "reset advances policy generation");
        require (!policy.canCommit (oldPreview, adk::MicrosecondTimePoint (4)),
                 "reset invalidates preview");
        require (policy.snapshot ().disposition == adk::IrEmissionDisposition::Idle,
                 "reset restores idle");

        policy.shutdown ();
        require         (policy.snapshot ().disposition == adk::IrEmissionDisposition::Shutdown,
                 "shutdown is retained");
        require (policy.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                 "shutdown is inert");
        require (policy.prepare (adk::LocalIrCodeId::StationPing, 2,
                                 adk::MicrosecondTimePoint (5))
                         .error () == adk::StatusCode::NotInitialized,
                 "shutdown disables prepare");
        require (policy.initialize ().ok (), "restart succeeds");
        require (policy.snapshot ().disposition == adk::IrEmissionDisposition::Idle,
                 "restart never resumes prior work");
    }

    void testConfigurationAndCatalog ()
    {
        const adk::KnownIrEmissionConfig invalid[] = {
            config (0, 19, envelopeDuration), config (7, 0, envelopeDuration),
            config (7, 19, envelopeDuration - 1), config (7, 19, 0x80000000UL),
            config (7, 19, 0xffffffffUL)};
        for (const auto& rejectedConfig : invalid)
        {
            adk::KnownIrEmissionPolicy rejected (rejectedConfig);
            require                             (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid configuration rejects");
            require (rejected.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                     "invalid configuration remains inert");
        }

        adk::KnownIrEmissionPolicy maximum (
            config (0xffffU, 0xffffffffUL, 0x7fffffffUL));
        require (maximum.initialize ().ok (),
                 "maximum wrap-safe configuration initializes");

        const adk::LocalIrCodeId codes[] = {
            adk::LocalIrCodeId::StationPing, adk::LocalIrCodeId::StationReady,
            adk::LocalIrCodeId::StationCancel, adk::LocalIrCodeId::StationAcknowledge};
        uint32_t candidateDigests[4] = {};
        for (uint8_t index = 0; index < 4; ++index)
        {
            adk::KnownIrEmissionPolicy policy (config ());
            require                           (policy.initialize ().ok (), "catalog policy initializes");
            const adk::KnownIrEmissionPreview preview =
                prepare (policy, codes[index], uint32_t (index) + 1, 100);
            require (preview.codeId == codes[index],
                     "preview retains symbolic catalog ID");
            require (preview.catalog.revision == 1 &&
                         preview.catalog.digest == catalogDigest,
                     "preview publishes fixed catalog identity");
            require (preview.candidateDigest != 0,
                     "preview publishes candidate digest");
            require (preview.completeAt.microseconds () == 100 + envelopeDuration,
                     "preview publishes exact envelope bound");
            require (preview.firstIntent == adk::IrEnvelopeIntent::CarrierOn,
                     "every local code starts with carrier mark");
            candidateDigests[index] = preview.candidateDigest;
            require (policy.snapshot ().catalog.digest == catalogDigest,
                     "snapshot retains catalog digest");
        }
        for (uint8_t index = 1; index < 4; ++index)
        {
            require (candidateDigests[index] != candidateDigests[index - 1],
                     "symbolic catalog IDs bind distinct candidates");
        }

        adk::KnownIrEmissionPolicy policy (config ());
        require                           (policy.initialize ().ok (), "validation policy initializes");
        for (uint16_t value = 4; value <= 255; ++value)
        {
            const adk::KnownIrEmissionSnapshot before = policy.snapshot ();
            require                                                     (policy.prepare (static_cast<adk::LocalIrCodeId> (value), 1,
                                     adk::MicrosecondTimePoint (0))
                             .error () == adk::StatusCode::InvalidArgument,
                     "every invalid code representation rejects");
            require (snapshotEqual (before, policy.snapshot ()),
                     "invalid code rejection is atomic");
        }
        require (policy.prepare (adk::LocalIrCodeId::StationPing, 0,
                                 adk::MicrosecondTimePoint (0))
                         .error () == adk::StatusCode::InvalidArgument,
                 "zero transaction ID rejects");
    }

    void testPreviewIdentityAndAtomicity ()
    {
        adk::KnownIrEmissionPolicy policy  (config ());
        adk::KnownIrEmissionPolicy foreign (config ());
        require                            (policy.initialize ().ok () && foreign.initialize ().ok (),
                 "identity policies initialize");
        const adk::KnownIrEmissionPreview preview =
            prepare (policy, adk::LocalIrCodeId::StationReady, 41, 1000);
        const adk::KnownIrEmissionPreview copiedPreview = preview;
        require (policy.canCommit (copiedPreview, adk::MicrosecondTimePoint (1000)),
                 "an exact copy of the live issued preview is valid");
        require (policy.prepare (adk::LocalIrCodeId::StationCancel, 42,
                                 adk::MicrosecondTimePoint (1000))
                         .error () == adk::StatusCode::ResourceBusy,
                 "one candidate reservation is enforced");

        adk::KnownIrEmissionPreview changed = preview;
#define REQUIRE_CHANGED_REJECTS(field, value, message)                                 \
    do                                                                                 \
    {                                                                                  \
        changed       = preview;                                                       \
        changed.field = value;                                                         \
        require (!policy.canCommit (changed, adk::MicrosecondTimePoint (1000)),        \
                 message);                                                             \
        require (policy.commit (changed, adk::MicrosecondTimePoint (1000)).error () == \
                     adk::StatusCode::InvalidArgument,                                 \
                 message);                                                             \
    }                                                                                  \
    while (false)
        REQUIRE_CHANGED_REJECTS (owner, &foreign, "foreign owner rejects");
        REQUIRE_CHANGED_REJECTS (configurationRevision, 8,
                                 "changed configuration rejects");
        REQUIRE_CHANGED_REJECTS (instanceEpoch, 20, "changed epoch rejects");
        REQUIRE_CHANGED_REJECTS (policyGeneration, preview.policyGeneration + 1,
                                 "changed policy generation rejects");
        REQUIRE_CHANGED_REJECTS (candidateGeneration, preview.candidateGeneration + 1,
                                 "changed candidate generation rejects");
        REQUIRE_CHANGED_REJECTS (transactionId, 42, "changed transaction rejects");
        REQUIRE_CHANGED_REJECTS (codeId, adk::LocalIrCodeId::StationCancel,
                                 "changed code rejects");
        REQUIRE_CHANGED_REJECTS (catalog.revision, 2,
                                 "changed catalog revision rejects");
        REQUIRE_CHANGED_REJECTS (catalog.digest, catalogDigest + 1,
                                 "changed catalog digest rejects");
        REQUIRE_CHANGED_REJECTS (candidateDigest, preview.candidateDigest + 1,
                                 "changed candidate digest rejects");
        REQUIRE_CHANGED_REJECTS (startAt, adk::MicrosecondTimePoint (1001),
                                 "changed start time rejects");
        REQUIRE_CHANGED_REJECTS (
            completeAt,
            adk::MicrosecondTimePoint (preview.completeAt.microseconds () + 1),
            "changed completion time rejects");
        REQUIRE_CHANGED_REJECTS (firstIntent, adk::IrEnvelopeIntent::CarrierOff,
                                 "changed first intent rejects");
#undef REQUIRE_CHANGED_REJECTS

        const adk::KnownIrEmissionSnapshot prepared = policy.snapshot ();
        require                                                       (!policy.canCommit (preview, adk::MicrosecondTimePoint (1001)),
                 "delayed commit rejects");
        require (policy.commit (preview, adk::MicrosecondTimePoint (1001)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "delayed commit cannot shift waveform");
        require (snapshotEqual (prepared, policy.snapshot ()),
                 "all changed and delayed commits reject atomically");
        require (policy.commit (copiedPreview, adk::MicrosecondTimePoint (1000)).ok (),
                 "exact copied capability commits at exact time");
        require (!policy.canCommit (preview, adk::MicrosecondTimePoint (1000)),
                 "consumed preview cannot recommit");
        require (policy.commit (preview, adk::MicrosecondTimePoint (1000)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "field-identical preview rejects after consumption");
    }

    void testEnvelopeBoundariesAndMissedService ()
    {
        const adk::LocalIrCodeId codes[] = {
            adk::LocalIrCodeId::StationPing, adk::LocalIrCodeId::StationReady,
            adk::LocalIrCodeId::StationCancel, adk::LocalIrCodeId::StationAcknowledge};
        for (const auto code : codes)
        {
            adk::KnownIrEmissionPolicy policy (config ());
            require                           (policy.initialize ().ok (), "waveform policy initializes");
            start                             (policy, code, 51, 100);

            require (policy.snapshot ().intent == adk::IrEnvelopeIntent::CarrierOn,
                     "commit publishes first mark");
            require (policy.update (adk::MicrosecondTimePoint (9099)).ok (),
                     "one tick before leader boundary updates");
            require (policy.snapshot ().intent == adk::IrEnvelopeIntent::CarrierOn,
                     "leader mark remains on before boundary");
            require (policy.update (adk::MicrosecondTimePoint (9100)).ok (),
                     "leader-space boundary updates");
            require (policy.snapshot ().intent == adk::IrEnvelopeIntent::CarrierOff,
                     "leader space turns carrier off");
            require (policy.update (adk::MicrosecondTimePoint (13599)).ok (),
                     "one tick before data boundary updates");
            require (policy.snapshot ().intent == adk::IrEnvelopeIntent::CarrierOff,
                     "leader space remains off before data");
            require (policy.update (adk::MicrosecondTimePoint (13600)).ok (),
                     "first data mark boundary updates");
            require (policy.snapshot ().intent == adk::IrEnvelopeIntent::CarrierOn,
                     "first data mark turns carrier on");
            require (policy.update (adk::MicrosecondTimePoint (68079)).ok (),
                     "last active tick updates");
            require (policy.snapshot ().disposition ==
                         adk::IrEmissionDisposition::Active,
                     "one tick before completion remains active");
            require (policy.update (adk::MicrosecondTimePoint (68080)).ok (),
                     "exact completion updates");
            require (policy.snapshot ().disposition ==
                             adk::IrEmissionDisposition::Complete &&
                         policy.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                     "completion is terminal and inactive");
            require (policy.snapshot ().terminalCause ==
                             adk::IrEmissionTerminalCause::Completed &&
                         policy.snapshot ().terminalTransactionId == 51,
                     "completion retains attribution");
        }

        adk::KnownIrEmissionPolicy late (config ());
        require                         (late.initialize ().ok (), "late policy initializes");
        start                           (late, adk::LocalIrCodeId::StationPing, 52, 7);
        require                         (
            late.update (adk::MicrosecondTimePoint (7 + envelopeDuration + 1000000UL))
                .ok (),
            "late update completes in one call");
        require (late.snapshot ().intent == adk::IrEnvelopeIntent::Inactive &&
                     late.snapshot ().disposition ==
                         adk::IrEmissionDisposition::Complete,
                 "missed service never catches up active bursts");
    }

    void testEveryEnvelopeMicrosecond ()
    {
        const adk::LocalIrCodeId codes[] = {
            adk::LocalIrCodeId::StationPing, adk::LocalIrCodeId::StationReady,
            adk::LocalIrCodeId::StationCancel, adk::LocalIrCodeId::StationAcknowledge};
        for (const auto code : codes)
        {
            adk::KnownIrEmissionPolicy policy (config ());
            require                           (policy.initialize ().ok (),
                     "exhaustive envelope policy initializes");
            start (policy, code, 91, 0);
            for (uint32_t offset = 0; offset < envelopeDuration; ++offset)
            {
                require (policy.update (adk::MicrosecondTimePoint (offset)).ok (),
                         "every active envelope microsecond updates");
                require (policy.snapshot ().intent == expectedIntent (code, offset),
                         "every envelope microsecond matches catalog vector");
            }
            require (policy.update (adk::MicrosecondTimePoint (envelopeDuration)).ok (),
                     "exhaustive envelope completes at exact terminal time");
            require (policy.snapshot ().disposition ==
                             adk::IrEmissionDisposition::Complete &&
                         policy.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                     "exhaustive envelope terminal state is inert");
        }
    }

    void testTimeOrderingAndRollover ()
    {
        adk::KnownIrEmissionPolicy policy (config ());
        require                           (policy.initialize ().ok (), "time policy initializes");
        start                             (policy, adk::LocalIrCodeId::StationAcknowledge, 61, 0xfffffff0UL);
        require                           (policy.update (adk::MicrosecondTimePoint (0xfffffff0UL)).ok (),
                 "repeated timestamp is accepted");
        const adk::KnownIrEmissionSnapshot repeated = policy.snapshot ();
        require                                                       (policy.update (adk::MicrosecondTimePoint (0xfffffff0UL)).ok (),
                 "second repeated timestamp is accepted");
        require (snapshotEqual (repeated, policy.snapshot ()),
                 "repeated timestamp is fieldwise stable");
        require (policy.update (adk::MicrosecondTimePoint (0)).ok (),
                 "time advances across rollover");

        const adk::KnownIrEmissionSnapshot before = policy.snapshot ();
        require                                                     (policy.update (adk::MicrosecondTimePoint (0xffffffffUL)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "regressing time rejects");
        require (snapshotEqual (before, policy.snapshot ()),
                 "regressing time rejection is atomic");
        require (policy.update (adk::MicrosecondTimePoint (0x80000000UL)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "exact half-range time rejects");
        require (snapshotEqual (before, policy.snapshot ()),
                 "half-range rejection is atomic");

        require (policy
                     .update (adk::MicrosecondTimePoint (
                         uint32_t (0xfffffff0UL + envelopeDuration)))
                     .ok (),
                 "completion across rollover succeeds");
        require (policy.snapshot ().disposition == adk::IrEmissionDisposition::Complete,
                 "rolled transaction completes");
    }

    void testDeterministicReplay ()
    {
        adk::KnownIrEmissionPolicy first  (config ());
        adk::KnownIrEmissionPolicy second (config ());
        require                           (first.initialize ().ok () && second.initialize ().ok (),
                 "replay policies initialize");

        const adk::KnownIrEmissionPreview firstPreview =
            prepare (first, adk::LocalIrCodeId::StationAcknowledge, 301, 0xfffff000UL);
        const adk::KnownIrEmissionPreview secondPreview =
            prepare (second, adk::LocalIrCodeId::StationAcknowledge, 301, 0xfffff000UL);
        require (firstPreview.candidateDigest == secondPreview.candidateDigest,
                 "identical inputs reproduce candidate digest");
        require (snapshotEqual (first.snapshot (), second.snapshot ()),
                 "prepared replay is fieldwise identical");
        require (first.commit (firstPreview, firstPreview.startAt).ok () &&
                     second.commit (secondPreview, secondPreview.startAt).ok (),
                 "replay previews commit");

        const uint32_t offsets[] = {0,     8999,  9000,  13499, 13500,
                                    14059, 14060, 67979, 67980};
        for (const uint32_t offset : offsets)
        {
            const adk::MicrosecondTimePoint now (uint32_t (0xfffff000UL + offset));
            require                             (first.update (now).ok () && second.update (now).ok (),
                     "replay timestamp updates");
            require (snapshotEqual (first.snapshot (), second.snapshot ()),
                     "replay snapshots remain fieldwise identical");
        }
    }

    void testCancellationAndShutdownAttribution ()
    {
        adk::KnownIrEmissionPolicy beforeCommit (config ());
        require                                 (beforeCommit.initialize ().ok (),
                 "precommit cancellation policy initializes");
        const adk::KnownIrEmissionPreview preview =
            prepare (beforeCommit, adk::LocalIrCodeId::StationCancel, 71, 30);
        require (beforeCommit.cancel (preview, adk::MicrosecondTimePoint (30)).ok (),
                 "exact candidate cancellation succeeds");
        require (beforeCommit.snapshot ().disposition ==
                         adk::IrEmissionDisposition::Cancelled &&
                     beforeCommit.snapshot ().terminalCause ==
                         adk::IrEmissionTerminalCause::CancelledBeforeCommit,
                 "precommit cancellation is attributed");
        require (beforeCommit.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                 "precommit cancellation never publishes carrier");
        const adk::KnownIrEmissionSnapshot cancelled = beforeCommit.snapshot ();
        require                                                              (beforeCommit.cancel (preview, adk::MicrosecondTimePoint (30)).ok (),
                 "repeated candidate cancellation is idempotent");
        require (snapshotEqual (cancelled, beforeCommit.snapshot ()),
                 "repeated candidate cancellation is stable");
        require (!beforeCommit.canCommit (preview, adk::MicrosecondTimePoint (30)),
                 "cancelled preview cannot commit");

        const uint32_t cancellationOffsets[] = {0,     8999,  9000,  13499,
                                                13500, 14059, 14060, 67979};
        for (const uint32_t offset : cancellationOffsets)
        {
            adk::KnownIrEmissionPolicy active (config ());
            require                           (active.initialize ().ok (),
                     "active cancellation policy initializes");
            start   (active, adk::LocalIrCodeId::StationReady, 72, 100);
            require (active.cancel (72, adk::MicrosecondTimePoint (100 + offset)).ok (),
                     "active cancellation succeeds at envelope phase");
            require (active.snapshot ().intent == adk::IrEnvelopeIntent::Inactive &&
                         active.snapshot ().terminalCause ==
                             adk::IrEmissionTerminalCause::CancelledActive,
                     "active cancellation dominates carrier intent");
            const adk::KnownIrEmissionSnapshot terminal = active.snapshot ();
            require                                                       (active.cancel (72, adk::MicrosecondTimePoint (100 + offset)).ok (),
                     "repeated active cancellation is idempotent");
            require (snapshotEqual (terminal, active.snapshot ()),
                     "repeated active cancellation is stable");
            require (
                active.cancel (73, adk::MicrosecondTimePoint (100 + offset)).error () ==
                    adk::StatusCode::InvalidArgument,
                "foreign transaction cancellation rejects");
            require (snapshotEqual (terminal, active.snapshot ()),
                     "foreign cancellation rejection is atomic");
        }

        adk::KnownIrEmissionPolicy prepared (config ());
        require                             (prepared.initialize ().ok (), "prepared shutdown initializes");
        prepare                             (prepared, adk::LocalIrCodeId::StationPing, 81, 400);
        prepared.shutdown                   ();
        require                             (prepared.snapshot ().terminalCause ==
                         adk::IrEmissionTerminalCause::ShutdownBeforeCommit &&
                     prepared.snapshot ().terminalTransactionId == 81 &&
                     prepared.snapshot ().terminalAt.microseconds () == 400,
                 "prepared shutdown retains candidate attribution");

        adk::KnownIrEmissionPolicy active (config ());
        require                           (active.initialize ().ok (), "active shutdown initializes");
        start                             (active, adk::LocalIrCodeId::StationPing, 82, 500);
        require                           (active.update (adk::MicrosecondTimePoint (600)).ok (),
                 "active shutdown accepts later time");
        active.shutdown ();
        require         (active.snapshot ().terminalCause ==
                         adk::IrEmissionTerminalCause::ShutdownActive &&
                     active.snapshot ().terminalTransactionId == 82 &&
                     active.snapshot ().terminalAt.microseconds () == 600 &&
                     active.snapshot ().intent == adk::IrEnvelopeIntent::Inactive,
                 "active shutdown retains latest accepted attribution");
    }
} // namespace

int main ()
{
    testCatalogDigestCoverage              ();
    testCompileSurfaceAndLifecycle         ();
    testConfigurationAndCatalog            ();
    testPreviewIdentityAndAtomicity        ();
    testEnvelopeBoundariesAndMissedService ();
    testEveryEnvelopeMicrosecond           ();
    testTimeOrderingAndRollover            ();
    testDeterministicReplay                ();
    testCancellationAndShutdownAttribution ();
    return EXIT_SUCCESS;
}
