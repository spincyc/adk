#include <local_identity_registry.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>

// clang-format off
namespace {
    constexpr uint16_t imageBytes = adk::localIdentityImageBytes;

    void require (bool condition, const char* message)


    {
        if (!condition)


        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);


        }
    }

    uint16_t crc16 (const uint8_t* bytes, uint16_t length)


    {
        uint16_t crc = 0xffffU;
        for (uint16_t index = 0; index < length; ++index)


        {
            crc ^= static_cast<uint16_t> (bytes[index]) << 8U;


            for (uint8_t bit = 0; bit < 8; ++bit)


            {
                crc = static_cast<uint16_t> (


                    (crc & 0x8000U) != 0U ? (crc << 1U) ^ 0x1021U : crc << 1U);
            }
        }
        return crc;
    }

    void put16 (uint8_t* bytes, uint16_t offset, uint16_t value)


    {
        bytes[offset]     = static_cast<uint8_t> (value);


        bytes[offset + 1] = static_cast<uint8_t> (value >> 8U);


    }

    void put32 (uint8_t* bytes, uint16_t offset, uint32_t value)


    {
        bytes[offset]     = static_cast<uint8_t> (value);


        bytes[offset + 1] = static_cast<uint8_t> (value >> 8U);


        bytes[offset + 2] = static_cast<uint8_t> (value >> 16U);


        bytes[offset + 3] = static_cast<uint8_t> (value >> 24U);


    }

    adk::LocalIdentity identity (uint8_t first, uint8_t length = 4)


    {
        adk::LocalIdentity result = {};
        result.length             = length;
        for (uint8_t index = 0;


             index < length && index < adk::maximumLocalIdentityBytes; ++index)
        {
            result.bytes[index] = static_cast<uint8_t> (first + index);


        }
        return result;
    }

    bool sameIdentity (const adk::LocalIdentity& left, const adk::LocalIdentity& right)


    {
        if (left.length != right.length)


        {
            return false;
        }
        for (uint8_t index = 0; index < adk::maximumLocalIdentityBytes; ++index)


        {
            if (left.bytes[index] != right.bytes[index])


            {
                return false;
            }
        }
        return true;
    }

    bool sameSnapshot (const adk::IdentityRegistrySnapshot& left,
                       const adk::IdentityRegistrySnapshot& right)
    {
        return left.disposition == right.disposition &&
               left.selectedBin == right.selectedBin &&
               left.bindingCount == right.bindingCount &&
               left.failedAttempts == right.failedAttempts &&
               left.enrollmentPending == right.enrollmentPending &&
               left.externalCommitPending == right.externalCommitPending &&
               left.imageGeneration == right.imageGeneration &&
               left.matchedBindingRevision == right.matchedBindingRevision &&
               left.acceptedSequence == right.acceptedSequence &&
               left.status == right.status;
    }

    adk::CarouselSource
    source (uint8_t sourceId = 1, uint16_t revision = 1,


            adk::CarouselSourceKind kind = adk::CarouselSourceKind::SyntheticIdentity)
    {
        return {kind, sourceId, revision};
    }

    adk::IdentityEvidence evidence (uint32_t observedAt, uint32_t sequence,


                                    adk::LocalIdentity  value,
                                    adk::Status         status = adk::StatusCode::Ok,
                                    adk::CarouselSource identitySource = source ())


    {
        return {identitySource, adk::TimePoint (observedAt), sequence, value, status};


    }

    void encodeEntry (uint8_t* entry, const adk::LocalIdentity& value, uint8_t bin,


                      uint16_t revision)
    {
        std::memset (entry, 0, 16);


        entry[0] = value.length;
        std::memcpy (entry + 1, value.bytes, adk::maximumLocalIdentityBytes);


        entry[11] = bin;
        put16 (entry, 12, revision);


        put16 (entry, 14, crc16 (entry, 14));


    }

    void encodeImage (uint8_t* bytes, uint32_t configurationId, uint32_t generation,


                      const adk::IdentityBinding* bindings, uint8_t count)
    {
        std::memset (bytes, 0, imageBytes);


        put16 (bytes, 0, adk::localIdentityImageMagic);


        bytes[2] = adk::localIdentityImageVersion;
        put16 (bytes, 4, imageBytes);


        put32 (bytes, 6, configurationId);


        put32 (bytes, 10, generation);


        bytes[14] = count;
        for (uint8_t index = 0; index < count; ++index)


        {
            encodeEntry (bytes + 16U + static_cast<uint16_t> (index) * 16U,


                         bindings[index].identity, bindings[index].binId,
                         bindings[index].revision);
        }
        put16 (bytes, 158, crc16 (bytes, 158));


    }

    struct Fixture
    {
        static constexpr uint32_t configurationId = 0x10203040UL;

        adk::IdentityBinding bindings[adk::maximumLocalIdentities]           = {};
        uint8_t              slots[adk::localIdentitySlotCount * imageBytes] = {};
        uint8_t              candidate[imageBytes]                           = {};

        Fixture ()


        {
            encodeImage (slots, configurationId, 1, nullptr, 0);


            std::memcpy (slots + imageBytes, slots, imageBytes);


        }

        adk::LocalIdentityRegistry make (adk::LocalIdentityRegistryConfig config = {


                                             configurationId, adk::maximumCarouselBins,
                                             3, adk::Duration (10), adk::Duration (20)})


        {
            return adk::LocalIdentityRegistry (


                config, bindings, adk::maximumLocalIdentities, slots, sizeof (slots),


                imageBytes, adk::localIdentitySlotCount, candidate, sizeof (candidate));


        }
    };

    void testTraitsLifecycleAndRecovery ()


    {
        static_assert (!std::is_copy_constructible<adk::LocalIdentityRegistry>::value,


                       "registry is not copy constructible");
        static_assert (!std::is_copy_assignable<adk::LocalIdentityRegistry>::value,


                       "registry is not copy assignable");
        static_assert (!std::is_move_constructible<adk::LocalIdentityRegistry>::value,


                       "registry is not move constructible");
        static_assert (!std::is_move_assignable<adk::LocalIdentityRegistry>::value,


                       "registry is not move assignable");

        Fixture fixture;
        auto    registry = fixture.make ();


        require (registry.snapshot ().status.error () ==


                     adk::StatusCode::NotInitialized,
                 "construction is inert");
        require (registry.initialize ().ok (), "canonical equal slots initialize");


        require (registry.snapshot ().bindingCount == 0 &&


                     registry.snapshot ().imageGeneration == 1 &&


                     registry.snapshot ().disposition == adk::IdentityDisposition::None,


                 "empty image recovery is exact");
        require (registry.initialize ().ok (), "initialize is idempotent");


        require (registry.reset ().ok (), "reset succeeds");


        registry.shutdown ();


        registry.shutdown ();


        require (registry.snapshot ().status.error () ==


                     adk::StatusCode::NotInitialized,
                 "shutdown is idempotent and inactive");
        require (registry.initialize ().ok (), "restart recovers supplied images");



        adk::IdentityBinding initial = {identity (0x10), 3, 17, 0};


        encodeImage (fixture.slots, Fixture::configurationId, 9, &initial, 1);


        std::memset (fixture.slots + imageBytes, 0xff, imageBytes);


        auto recovered = fixture.make ();


        require (recovered.initialize ().ok (), "one valid slot dominates erased slot");


        require (recovered.snapshot ().bindingCount == 1 &&


                     recovered.snapshot ().imageGeneration == 9,


                 "new valid slot is recovered");
        require (sameIdentity (fixture.bindings[0].identity, initial.identity) &&


                     fixture.bindings[0].binId == 3 &&
                     fixture.bindings[0].revision == 17,
                 "recovery copies the validated binding");
    }

    void testStructuralImageRejection ()


    {
        const uint8_t knownCrcVector[] = {'1', '2', '3', '4', '5',
                                          '6', '7', '8', '9'};
        require (crc16 (knownCrcVector, sizeof (knownCrcVector)) == 0x29b1U,

                 "CRC-16/CCITT-FALSE matches its canonical check vector");

        for (uint16_t byte = 0; byte < imageBytes; ++byte)


        {
            Fixture fixture;
            fixture.slots[byte] ^= 0x01U;
            fixture.slots[imageBytes + byte] ^= 0x01U;
            auto registry = fixture.make ();


            require (!registry.initialize ().ok (),


                     "matching every-byte corruption rejects");
            require (registry.snapshot ().status.error () != adk::StatusCode::Ok,


                     "corrupt recovery publishes failure");
        }

        Fixture foreign;
        encodeImage (foreign.slots, Fixture::configurationId + 1, 1, nullptr, 0);


        std::memcpy (foreign.slots + imageBytes, foreign.slots, imageBytes);


        auto foreignRegistry = foreign.make ();


        require (!foreignRegistry.initialize ().ok (), "foreign configuration rejects");



        Fixture ambiguous;
        encodeImage (ambiguous.slots, Fixture::configurationId, 1, nullptr, 0);


        encodeImage (ambiguous.slots + imageBytes, Fixture::configurationId,


                     0x80000001UL, nullptr, 0);
        auto ambiguousRegistry = ambiguous.make ();


        require (!ambiguousRegistry.initialize ().ok (),


                 "half-range generation ambiguity rejects");

        Fixture              unequal;
        adk::IdentityBinding binding = {identity (0x21), 1, 1, 0};


        encodeImage (unequal.slots, Fixture::configurationId, 1, nullptr, 0);


        encodeImage (unequal.slots + imageBytes, Fixture::configurationId, 1, &binding,


                     1);
        auto unequalRegistry = unequal.make ();


        require (!unequalRegistry.initialize ().ok (),


                 "equal generations with unequal bytes reject");

        Fixture duplicate;
        adk::IdentityBinding repeated[2] = {
            {identity (0x31), 1, 1, 0}, {identity (0x31), 2, 2, 0}};

        encodeImage (duplicate.slots, Fixture::configurationId, 2, repeated, 2);

        std::memcpy (duplicate.slots + imageBytes, duplicate.slots, imageBytes);

        auto duplicateRegistry = duplicate.make ();

        require (!duplicateRegistry.initialize ().ok (),

                 "duplicate identity bytes in one image reject");

        Fixture newer;
        adk::IdentityBinding oldBinding = {identity (0x41), 1, 1, 0};

        adk::IdentityBinding newBinding = {identity (0x42), 2, 2, 0};

        encodeImage (newer.slots, Fixture::configurationId, 0xffffffffUL,

                     &oldBinding, 1);
        encodeImage (newer.slots + imageBytes, Fixture::configurationId, 0,

                     &newBinding, 1);
        auto newerRegistry = newer.make ();

        require (newerRegistry.initialize ().ok () &&

                     newerRegistry.snapshot ().imageGeneration == 0 &&

                     sameIdentity (newer.bindings[0].identity, newBinding.identity),

                 "generation rollover selects the newer valid peer");
    }

    void testConfigurationAndStorageGeometry ()


    {
        {
            Fixture fixture;
            auto    registry =
                fixture.make ({0, 8, 3, adk::Duration (10), adk::Duration (20)});


            require (registry.initialize ().error () ==


                         adk::StatusCode::InvalidConfiguration,
                     "zero configuration identity rejects");
        }
        {
            Fixture fixture;
            auto    registry = fixture.make ({Fixture::configurationId, 0, 3,


                                              adk::Duration (10), adk::Duration (20)});


            require (!registry.initialize ().ok (), "zero bins reject");


        }
        {
            Fixture fixture;
            auto    registry = fixture.make ({Fixture::configurationId, 8, 0,


                                              adk::Duration (10), adk::Duration (20)});


            require (!registry.initialize ().ok (), "zero failure threshold rejects");


        }
        {
            Fixture                    fixture;
            adk::LocalIdentityRegistry registry (


                {Fixture::configurationId, 8, 3, adk::Duration (10),


                 adk::Duration (20)},


                fixture.bindings, 8, fixture.slots,
                static_cast<uint16_t> (sizeof (fixture.slots) - 1), imageBytes, 2,


                fixture.candidate, imageBytes);
            require (!registry.initialize ().ok (), "undersized slot extent rejects");


        }
        {
            Fixture                    fixture;
            adk::LocalIdentityRegistry registry (


                {Fixture::configurationId, 8, 3, adk::Duration (10),


                 adk::Duration (20)},


                fixture.bindings, 8, fixture.slots, sizeof (fixture.slots),


                static_cast<uint16_t> (imageBytes - 1), 2, fixture.candidate,


                imageBytes);
            require (!registry.initialize ().ok (), "noncanonical slot stride rejects");


        }
        {
            Fixture                    fixture;
            adk::LocalIdentityRegistry registry (


                {Fixture::configurationId, 8, 3, adk::Duration (10),


                 adk::Duration (20)},


                fixture.bindings, 8, fixture.slots, sizeof (fixture.slots), imageBytes,


                2, fixture.slots, imageBytes);
            require (!registry.initialize ().ok (),


                     "overlapping candidate storage rejects");
        }
        {
            Fixture                    fixture;
            adk::LocalIdentityRegistry registry (


                {Fixture::configurationId, 8, 3, adk::Duration (10),


                 adk::Duration (20)},


                nullptr, 8, fixture.slots, sizeof (fixture.slots), imageBytes, 2,


                fixture.candidate, imageBytes);
            require (!registry.initialize ().ok (), "null live table rejects");


        }
    }

    void testIdentityWidthsAndSequenceRollover ()


    {
        Fixture fixture;
        auto    registry = fixture.make ();


        require (registry.initialize ().ok (), "width fixture initializes");


        uint32_t sequence = 1;
        for (uint8_t length = 4; length <= adk::maximumLocalIdentityBytes; ++length)


        {
            require (


                registry
                    .observe (adk::TimePoint (sequence),


                              evidence (sequence, sequence, identity (length, length)))


                    .ok (),


                "every supported identity width is admitted");
            require (registry.snapshot ().acceptedSequence == sequence,


                     "supported width advances exact sequence");
            ++sequence;
        }

        Fixture rolloverFixture;
        auto    rollover = rolloverFixture.make ();


        require (rollover.initialize ().ok (), "rollover fixture initializes");


        require (rollover


                     .observe (adk::TimePoint (0xfffffffeUL),


                               evidence (0xfffffffeUL, 0xfffffffeUL, identity (0x20)))


                     .ok (),


                 "pre-rollover observation succeeds");
        require (rollover.observe (adk::TimePoint (1), evidence (1, 1, identity (0x30)))


                     .ok (),


                 "time and sequence rollover move forward");
        const auto before = rollover.snapshot ();


        require (!rollover


                      .observe (adk::TimePoint (1),


                                evidence (1, 0x80000001UL, identity (0x40)))


                      .ok (),


                 "half-range sequence is ambiguous");
        require (rollover.snapshot ().acceptedSequence == before.acceptedSequence,


                 "ambiguous sequence preserves history");
    }

    void testKnownUnknownReplayAndLockout ()


    {
        Fixture              fixture;
        adk::IdentityBinding known = {identity (0x30), 4, 23, 0};


        encodeImage (fixture.slots, Fixture::configurationId, 7, &known, 1);


        std::memcpy (fixture.slots + imageBytes, fixture.slots, imageBytes);


        auto registry = fixture.make ();


        require (registry.initialize ().ok (), "lookup fixture initializes");



        require (registry.observe (adk::TimePoint (5), evidence (5, 1, known.identity))


                     .ok (),


                 "known observation succeeds");
        auto snapshot = registry.snapshot ();


        require (snapshot.disposition == adk::IdentityDisposition::Known &&


                     snapshot.selectedBin == 4 &&
                     snapshot.matchedBindingRevision == 23 &&
                     snapshot.imageGeneration == 7 && snapshot.failedAttempts == 0,
                 "known snapshot carries admitted provenance");

        const auto unknown = identity (0x50);


        require (registry.observe (adk::TimePoint (6), evidence (6, 2, unknown)).ok (),


                 "first unknown is admitted");
        require (registry.snapshot ().disposition ==


                         adk::IdentityDisposition::Unknown &&
                     registry.snapshot ().failedAttempts == 1 &&


                     registry.snapshot ().matchedBindingRevision == 0,


                 "unknown increments once and clears revision");
        require (registry.observe (adk::TimePoint (7), evidence (6, 2, unknown)).ok (),


                 "exact replay is harmless");
        require (registry.snapshot ().failedAttempts == 1,


                 "exact replay does not increment failures");

        auto changed = unknown;
        changed.bytes[0] ^= 1U;
        const auto beforeChanged = registry.snapshot ();


        require (!registry.observe (adk::TimePoint (7), evidence (6, 2, changed)).ok (),


                 "changed same-sequence payload rejects");
        require (registry.snapshot ().failedAttempts == beforeChanged.failedAttempts &&


                     registry.snapshot ().acceptedSequence ==


                         beforeChanged.acceptedSequence,
                 "changed replay is atomic");

        require (registry.observe (adk::TimePoint (8), evidence (8, 3, identity (0x60)))


                     .ok (),


                 "second unknown is admitted");
        require (registry.observe (adk::TimePoint (9), evidence (9, 4, identity (0x70)))


                     .ok (),


                 "third unknown starts lockout");
        require (registry.snapshot ().disposition ==


                         adk::IdentityDisposition::LockedOut &&
                     registry.snapshot ().failedAttempts == 3,


                 "lockout starts at configured saturation");
        require (


            registry.observe (adk::TimePoint (18), evidence (18, 5, known.identity))


                .ok (),


            "known before expiry remains a policy outcome");
        require (registry.snapshot ().disposition ==


                     adk::IdentityDisposition::LockedOut,
                 "one tick before expiry is locked");
        require (


            registry.observe (adk::TimePoint (19), evidence (19, 6, known.identity))


                .ok (),


            "known at exact expiry is admitted");
        require (registry.snapshot ().disposition == adk::IdentityDisposition::Known &&


                     registry.snapshot ().failedAttempts == 0,


                 "exact expiry clears attempts on known identity");
    }

    void testEvidenceValidationAndAtomicity ()


    {
        Fixture fixture;
        auto    registry = fixture.make ();


        require (registry.initialize ().ok (), "validation fixture initializes");


        require (registry.observe (adk::TimePoint (10), evidence (10, 1, identity (1)))


                     .ok (),


                 "baseline observation succeeds");
        const auto baseline = registry.snapshot ();



        adk::LocalIdentity allZero = {};
        allZero.length             = 4;
        adk::LocalIdentity allOnes = {};
        allOnes.length             = 4;
        std::memset (allOnes.bytes, 0xff, sizeof (allOnes.bytes));


        const adk::IdentityEvidence invalid[] = {
            evidence (10, 2, identity (1, 3)),


            evidence (10, 2, identity (1, 11)),


            evidence (10, 2, allZero),


            evidence (10, 2, allOnes),


            evidence (10, 2, identity (1), adk::StatusCode::HardwareFailure),


            evidence (10, 2, identity (1), adk::StatusCode::Ok, source (0)),


            evidence (10, 2, identity (1), adk::StatusCode::Ok, source (1, 0)),


            evidence (10, 2, identity (1), adk::StatusCode::Ok,


                      source (1, 1, adk::CarouselSourceKind::SyntheticKey)),


            evidence (11, 2, identity (1)),


            evidence (0x8000000aUL, 2, identity (1))};


        for (const auto& malformed : invalid)


        {
            require (!registry.observe (adk::TimePoint (10), malformed).ok (),


                     "malformed evidence rejects");
            require (registry.snapshot ().acceptedSequence ==


                             baseline.acceptedSequence &&
                         registry.snapshot ().failedAttempts == baseline.failedAttempts,


                     "malformed evidence preserves accepted history");
        }

        auto padded     = identity (1);


        padded.bytes[9] = 1;
        require (


            !registry.observe (adk::TimePoint (10), evidence (10, 2, padded)).ok (),


            "noncanonical identity tail rejects");
        require (!registry.observe (adk::TimePoint (40), evidence (10, 2, identity (1)))


                      .ok (),


                 "stale evidence rejects");

        Fixture replayFixture;
        auto replay = replayFixture.make ();

        const auto original = evidence (100, 7, identity (0x44));

        require (replay.initialize ().ok () &&
                     replay.observe (adk::TimePoint (100), original).ok (),
                 "same-sequence fixture establishes accepted evidence");
        const auto replayBaseline = replay.snapshot ();
        auto changedTime = original;
        changedTime.observedAt = adk::TimePoint (99);

        require (!replay.observe (adk::TimePoint (100), changedTime).ok (),
                 "changed occurrence time at one sequence rejects");
        require (replay.snapshot ().acceptedSequence ==
                     replayBaseline.acceptedSequence,
                 "changed occurrence time preserves replay identity");
        auto changedSource = original;
        changedSource.source.sourceId = 2;
        require (!replay.observe (adk::TimePoint (100), changedSource).ok (),
                 "changed source at one sequence rejects");
        auto changedLength = original;
        changedLength.identity.length = 5;
        changedLength.identity.bytes[4] = 0x55;
        require (!replay.observe (adk::TimePoint (100), changedLength).ok (),
                 "changed identity length at one sequence rejects");
    }

    void testEnrollmentCommitAndReconciliation ()


    {
        Fixture fixture;
        auto    registry = fixture.make ();


        require (registry.initialize ().ok (), "enrollment fixture initializes");


        const auto newIdentity = identity (0x80);


        const auto observed    = evidence (5, 1, newIdentity);


        require (registry.observe (adk::TimePoint (5), observed).ok (),

                 "candidate identity is first admitted as unknown");

        const auto candidate =
            registry.previewEnrollment (adk::TimePoint (5), observed, 2);


        require (candidate.ok (), "unknown identity enrollment previews");


        require (registry.snapshot ().disposition ==


                         adk::IdentityDisposition::EnrollmentPending &&
                     registry.snapshot ().enrollmentPending,


                 "preview publishes pending state");
        const auto exported = registry.previewExport (candidate.value ());


        require (exported.ok () && exported.value ().bytes == fixture.candidate &&


                     exported.value ().length == imageBytes &&


                     exported.value ().slot == 1 && exported.value ().generation == 2,


                 "candidate export identifies complete inactive-slot image");
        require (registry.previewExport (candidate.value ()).ok (),


                 "candidate export is retryable");

        auto badEvidence =
            adk::IdentityDurableCommitEvidence{candidate.value ().owner,


                                               candidate.value ().candidateGeneration,


                                               candidate.value ().operationId,


                                               exported.value ().slot,


                                               exported.value (),


                                               false,
                                               true,
                                               adk::StatusCode::Ok};
        require (


            !registry.acknowledgeExternalCommit (candidate.value (), badEvidence).ok (),


            "unsynchronized acknowledgement rejects");
        require (registry.snapshot ().externalCommitPending &&


                     registry.snapshot ().bindingCount == 0,


                 "failed acknowledgement latches reconciliation without install");
        require (


            !registry.observe (adk::TimePoint (6), evidence (6, 2, identity (0x90)))


                 .ok (),


            "reconciliation latch blocks observation");
        require (!registry.cancelEnrollment ().ok (),


                 "reconciliation latch blocks cancellation");
        require (!registry.reset ().ok (), "reconciliation latch blocks reset");


        auto candidatePointerEvidence         = badEvidence;
        candidatePointerEvidence.synchronized = true;
        require (

            !registry
                 .acknowledgeExternalCommit (candidate.value (),

                                             candidatePointerEvidence)
                 .ok (),

            "candidate scratch cannot masquerade as durable reread");

        uint8_t equalTemporary[imageBytes] = {};
        std::memcpy (equalTemporary, exported.value ().bytes, imageBytes);
        auto temporaryPointerEvidence = candidatePointerEvidence;
        temporaryPointerEvidence.reconciledImage.bytes = equalTemporary;
        require (!registry
                      .acknowledgeExternalCommit (candidate.value (),
                                                  temporaryPointerEvidence)
                      .ok (),
                 "byte-equal temporary cannot masquerade as supplied slot");

        uint8_t* durableSlot =
            fixture.slots +
            static_cast<uint16_t> (exported.value ().slot) * imageBytes;

        std::memcpy (durableSlot, exported.value ().bytes, imageBytes);

        auto goodEvidence                  = badEvidence;
        goodEvidence.reconciledImage.bytes = durableSlot;
        goodEvidence.synchronized          = true;
        goodEvidence.rereadValidated = true;
        goodEvidence.durableStatus   = adk::StatusCode::Ok;
        require (


            registry.acknowledgeExternalCommit (candidate.value (), goodEvidence).ok (),


            "matching durable acknowledgement installs");
        const auto installed = registry.snapshot ();


        require (installed.bindingCount == 1 && installed.imageGeneration == 2 &&


                     installed.acceptedSequence == 1 &&
                     !installed.enrollmentPending && !installed.externalCommitPending,
                 "install is one atomic mutation");
        require (


            registry.acknowledgeExternalCommit (candidate.value (), goodEvidence).ok (),


            "exact duplicate successful acknowledgement is idempotent");
        require (registry.snapshot ().bindingCount == installed.bindingCount &&


                     registry.snapshot ().imageGeneration == installed.imageGeneration,


                 "duplicate acknowledgement does not mutate");

        auto wrongDuplicate = goodEvidence;
        wrongDuplicate.operationId += 1;
        require (!registry
                      .acknowledgeExternalCommit (candidate.value (),
                                                  wrongDuplicate)
                      .ok (),
                 "changed duplicate metadata rejects");
        require (sameSnapshot (registry.snapshot (), installed),
                 "rejected duplicate preserves installed snapshot");

        require (


            registry.observe (adk::TimePoint (7), evidence (7, 2, newIdentity)).ok (),


            "installed identity can be observed");
        require (registry.snapshot ().disposition == adk::IdentityDisposition::Known &&


                     registry.snapshot ().selectedBin == 2 &&


                     registry.snapshot ().matchedBindingRevision != 0,


                 "installed binding publishes bin and revision");
    }

    void testEnrollmentRejectionsAndCancel ()


    {
        Fixture              fixture;
        adk::IdentityBinding existing = {identity (0x11), 1, 3, 0};


        encodeImage (fixture.slots, Fixture::configurationId, 4, &existing, 1);


        std::memcpy (fixture.slots + imageBytes, fixture.slots, imageBytes);


        auto registry = fixture.make ();


        require (registry.initialize ().ok (), "rejection fixture initializes");

        const auto directEvidence = evidence (0, 1, identity (0x20));
        const auto directCandidate =
            registry.previewEnrollment (adk::TimePoint (0), directEvidence, 2);
        require (directCandidate.ok () &&
                     registry.snapshot ().acceptedSequence == 0,
                 "direct preview validates forward evidence without consuming it");
        require (registry.cancelEnrollment ().ok (),
                 "direct side-effect-free preview remains cancellable");

        const auto beforeKnownReject = registry.snapshot ();

        require (!registry


                      .previewEnrollment (adk::TimePoint (1),


                                          evidence (1, 1, existing.identity), 2)


                      .ok (),


                 "known identity cannot enroll");
        require (sameSnapshot (registry.snapshot (), beforeKnownReject),
                 "known preview rejection preserves every snapshot field");

        const auto occupiedEvidence = evidence (1, 2, identity (0x22));

        const auto beforeOccupiedReject = registry.snapshot ();

        require (!registry


                      .previewEnrollment (adk::TimePoint (1),


                                          occupiedEvidence, 1)


                      .ok (),


                 "occupied bin cannot enroll");
        require (sameSnapshot (registry.snapshot (), beforeOccupiedReject),
                 "occupied-bin rejection preserves every snapshot field");
        const auto corrected =
            registry.previewEnrollment (adk::TimePoint (1), occupiedEvidence, 2);
        require (corrected.ok (), "same evidence succeeds after correcting bin");

        const auto pendingSnapshot = registry.snapshot ();

        require (pendingSnapshot.selectedBin == 0 &&
                     pendingSnapshot.matchedBindingRevision == 0,
                 "pending unknown clears prior match provenance");
        require (registry.observe (adk::TimePoint (1), occupiedEvidence).error () ==
                         adk::StatusCode::ResourceBusy &&
                     sameSnapshot (registry.snapshot (), pendingSnapshot),
                 "observation during enrollment is atomic and blocked");
        require (registry.cancelEnrollment ().ok (),
                 "corrected retry can be cancelled");

        const auto beforeRangeReject = registry.snapshot ();

        require (!registry


                      .previewEnrollment (adk::TimePoint (1),


                                          evidence (1, 3, identity (0x22)), 8)


                      .ok (),


                 "out-of-range bin cannot enroll");
        require (sameSnapshot (registry.snapshot (), beforeRangeReject),
                 "range rejection preserves every snapshot field");

        const auto candidateEvidence = evidence (2, 4, identity (0x33));


        require (registry.observe (adk::TimePoint (2), candidateEvidence).ok (),

                 "candidate identity is admitted before preview");

        const auto candidate = registry.previewEnrollment (


            adk::TimePoint (2), candidateEvidence, 2);


        require (candidate.ok (), "valid candidate previews");


        require (registry.cancelEnrollment ().ok (), "pending enrollment cancels");


        require (!registry.previewExport (candidate.value ()).ok (),


                 "cancel expires prior handle and view");
        require (registry.snapshot ().bindingCount == 1 &&


                     !registry.snapshot ().enrollmentPending,


                 "cancel preserves committed directory");
    }
} // namespace

int main ()


{
    testTraitsLifecycleAndRecovery ();


    testStructuralImageRejection ();


    testConfigurationAndStorageGeometry ();


    testIdentityWidthsAndSequenceRollover ();


    testKnownUnknownReplayAndLockout ();


    testEvidenceValidationAndAtomicity ();


    testEnrollmentCommitAndReconciliation ();


    testEnrollmentRejectionsAndCancel ();


    std::cout << "local identity registry tests passed\n";
}
