#include <inert_parts_carousel.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <new>
#include <type_traits>

#ifndef ADK_INERT_PARTS_CAROUSEL_TEST_PART
#define ADK_INERT_PARTS_CAROUSEL_TEST_PART 0
#endif

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART < 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART > 2
#error "ADK_INERT_PARTS_CAROUSEL_TEST_PART must be 1 or 2 when defined"
#endif

namespace {
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
            const uint32_t mixed = static_cast<uint32_t> (crc) ^
                                   (static_cast<uint32_t> (bytes[index]) << 8U);
            crc                  = static_cast<uint16_t> (mixed);

            for (uint8_t bit = 0; bit < 8; ++bit)
            {
                const uint32_t shifted = static_cast<uint32_t> (crc) << 1U;
                const uint32_t reduced =
                    (crc & 0x8000U) != 0U ? shifted ^ 0x1021U : shifted;
                crc = static_cast<uint16_t> (reduced);
            }
        }
        return crc;
    }

    void put16 (uint8_t* bytes, uint16_t offset, uint16_t value)
    {
        bytes[offset] = static_cast<uint8_t> (value);

        bytes[offset + 1] = static_cast<uint8_t> (value >> 8U);
    }

    void put32 (uint8_t* bytes, uint16_t offset, uint32_t value)
    {
        bytes[offset] = static_cast<uint8_t> (value);

        bytes[offset + 1] = static_cast<uint8_t> (value >> 8U);

        bytes[offset + 2] = static_cast<uint8_t> (value >> 16U);

        bytes[offset + 3] = static_cast<uint8_t> (value >> 24U);
    }

    void encodeEmptyImage (uint8_t* bytes, uint32_t configurationId)
    {
        std::memset (bytes, 0, adk::localIdentityImageBytes);

        put16 (bytes, 0, adk::localIdentityImageMagic);
        bytes[2] = adk::localIdentityImageVersion;
        put16 (bytes, 4, adk::localIdentityImageBytes);

        put32 (bytes, 6, configurationId);

        put32 (bytes, 10, 1);

        put16 (bytes, 158, crc16 (bytes, 158));
    }

    adk::LocalIdentity knownIdentity ()
    {
        return {4, {0x10, 0x20, 0x30, 0x40, 0, 0, 0, 0, 0, 0}};
    }

    void encodeKnownImage (uint8_t* bytes, uint32_t configurationId, uint8_t bin = 1)
    {
        encodeEmptyImage (bytes, configurationId);
        uint8_t*   entry    = bytes + 16;
        const auto identity = knownIdentity ();
        entry[0]            = identity.length;
        std::memcpy (entry + 1, identity.bytes, adk::maximumLocalIdentityBytes);
        entry[11] = bin;
        put16 (entry, 12, 7);

        put16 (entry, 14, crc16 (entry, 14));
        bytes[14] = 1;
        put16 (bytes, 158, crc16 (bytes, 158));
    }

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 2
    void
    encodeAudit (uint8_t* bytes, uint8_t kind = 1,
                 adk::CarouselPhase       phase  = adk::CarouselPhase::Homing,
                 adk::CarouselAuditStatus status = adk::CarouselAuditStatus::Success)
    {
        std::memset (bytes, 0, adk::carouselAuditRecordBytes);

        put16 (bytes, 0, adk::carouselAuditMagic);
        bytes[2] = adk::carouselAuditVersion;
        bytes[3] = adk::carouselAuditRecordBytes;
        put32 (bytes, 4, 0x55667788UL);

        put32 (bytes, 8, 7);

        put16 (bytes, 12, 9);

        put16 (bytes, 14, 11);

        put32 (bytes, 16, 1234);
        bytes[20] = kind;
        bytes[21] = static_cast<uint8_t> (phase);
        bytes[22] = 1;
        bytes[23] = static_cast<uint8_t> (status);

        put16 (bytes, 24, 0x2233);

        put16 (bytes, 26, 4);

        put32 (bytes, 28, 5);

        put32 (bytes, 32, kind == 1 ? 0 : 6);

        put16 (bytes, 38, crc16 (bytes, 38));
    }
#endif

    adk::CarouselConfig config ()
    {
        adk::CarouselConfig value = {3,
                                     {-4, 0, 5, 0, 0, 0, 0, 0},
                                     0x55667788UL,
                                     2,
                                     {12, 34, 56, 0, 0, 0, 0, 0},
                                     10,
                                     11,
                                     {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
                                     adk::Duration (50),

                                     adk::Duration (20),

                                     adk::Duration (5),

                                     adk::Duration (20),

                                     adk::Duration (30),

                                     adk::Duration (3)};
        return value;
    }

    adk::BoundedHomingConfig homingConfig ()
    {
        return {-8,
                8,
                0,
                -1,
                3,
                8,
                adk::Duration (30),

                adk::Duration (80),

                adk::Duration (5),

                adk::Duration (30),

                adk::Duration (3)};
    }

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 2
    bool sameSnapshot (const adk::CarouselSnapshot& left,
                       const adk::CarouselSnapshot& right)
    {
        return left.phase == right.phase && left.fault == right.fault &&
               left.identityDisposition == right.identityDisposition &&
               left.homingFault == right.homingFault &&
               left.requestedBin == right.requestedBin &&
               left.authorizationCurrent == right.authorizationCurrent &&
               left.positionKnown == right.positionKnown &&
               left.logicalPosition == right.logicalPosition &&
               left.intent.coilBits == right.intent.coilBits &&
               left.intent.gate == right.intent.gate &&
               left.intent.gateExpiresAt == right.intent.gateExpiresAt &&
               left.intent.selectedBin == right.intent.selectedBin &&
               left.intent.statusCode == right.intent.statusCode &&
               left.hasAuditRecord == right.hasAuditRecord &&
               left.durableAdmissionPending == right.durableAdmissionPending &&
               left.terminalReconciliationPending ==
                   right.terminalReconciliationPending &&
               left.operationId == right.operationId && left.status == right.status;
    }
#endif

    struct Fixture
    {
        static constexpr uint32_t registryId = 0x10203040UL;

        adk::CarouselConfig        configValue                           = config ();
        adk::IdentityBinding       bindings[adk::maximumLocalIdentities] = {};
        uint8_t                    identitySlots[adk::localIdentitySlotCount *
                                                 adk::localIdentityImageBytes]     = {};
        uint8_t                    identityCandidate[adk::localIdentityImageBytes] = {};
        adk::LocalIdentityRegistry identity;
        adk::BoundedHomingPolicy   homing;
        uint8_t                    auditSlots[4 * adk::carouselAuditRecordBytes] = {};
        uint8_t                    auditCandidate[adk::carouselAuditRecordBytes] = {};
        adk::InertPartsCarousel    carousel;

        Fixture ()
            : identity ({registryId, 0x11223344UL, 3, 3, adk::Duration (20),
                         adk::Duration (30)},
                        bindings, adk::maximumLocalIdentities, identitySlots,
                        sizeof (identitySlots), adk::localIdentityImageBytes,
                        adk::localIdentitySlotCount, identityCandidate,
                        sizeof (identityCandidate)),
              homing (homingConfig ()),

              carousel (configValue, identity, homing, auditSlots, sizeof (auditSlots),
                        adk::carouselAuditRecordBytes, 4, auditCandidate,
                        sizeof (auditCandidate))
        {
            encodeEmptyImage (identitySlots, registryId);

            std::memcpy (identitySlots + adk::localIdentityImageBytes, identitySlots,
                         adk::localIdentityImageBytes);
            std::memset (auditSlots, 0xff, sizeof (auditSlots));
        }
    };

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 1
    void testTraitsLifecycleAndSafeState ()
    {
        static_assert (!std::is_copy_constructible<adk::InertPartsCarousel>::value,
                       "carousel is not copy constructible");
        static_assert (!std::is_copy_assignable<adk::InertPartsCarousel>::value,
                       "carousel is not copy assignable");
        static_assert (!std::is_move_constructible<adk::InertPartsCarousel>::value,
                       "carousel is not move constructible");
        static_assert (!std::is_move_assignable<adk::InertPartsCarousel>::value,
                       "carousel is not move assignable");

        Fixture fixture;
        auto    snapshot = fixture.carousel.snapshot ();

        require (snapshot.phase == adk::CarouselPhase::Uninitialized,
                 "construction is inert");
        require (snapshot.intent.coilBits == 0 &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Closed,
                 "construction publishes closed and off");
        require (fixture.carousel.initialize ().ok (), "initialize succeeds");

        require (fixture.carousel.snapshot ().phase == adk::CarouselPhase::Idle,
                 "initialize enters idle");
        require (fixture.carousel.initialize ().ok (), "initialize is idempotent");

        fixture.carousel.shutdown ();

        fixture.carousel.shutdown ();

        snapshot = fixture.carousel.snapshot ();

        require (snapshot.phase == adk::CarouselPhase::Uninitialized,
                 "shutdown is idempotent");
        require (!snapshot.authorizationCurrent && !snapshot.positionKnown &&
                     snapshot.intent.coilBits == 0 &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Closed,
                 "shutdown invalidates authority and publishes safe intent");
        require (fixture.carousel.initialize ().ok (), "restart succeeds");
    }

    void testConfigurationAndStorageBounds ()
    {
        const uint8_t capacities[] = {2, 4, 6, 8};
        for (uint8_t capacity : capacities)
        {
            Fixture fixture;
            uint8_t slots[8 * adk::carouselAuditRecordBytes];
            uint8_t candidate[adk::carouselAuditRecordBytes];
            std::memset (slots, 0xff, sizeof (slots));

            adk::InertPartsCarousel carousel (
                fixture.configValue, fixture.identity, fixture.homing, slots,
                static_cast<uint16_t> (capacity * adk::carouselAuditRecordBytes),

                adk::carouselAuditRecordBytes, capacity, candidate, sizeof (candidate));
            require (carousel.initialize ().ok (), "every even capacity initializes");

            carousel.shutdown ();
        }

        for (uint8_t capacity : {uint8_t (0), uint8_t (1), uint8_t (3), uint8_t (5),
                                 uint8_t (7), uint8_t (9)})
        {
            Fixture                 fixture;
            uint8_t                 slots[9 * adk::carouselAuditRecordBytes] = {};
            uint8_t                 candidate[adk::carouselAuditRecordBytes] = {};
            adk::InertPartsCarousel carousel (
                fixture.configValue, fixture.identity, fixture.homing, slots,
                static_cast<uint16_t> (capacity * adk::carouselAuditRecordBytes),

                adk::carouselAuditRecordBytes, capacity, candidate, sizeof (candidate));
            require (!carousel.initialize ().ok (),
                     "odd/out-of-range capacity rejects");
        }

        Fixture fixture;
        auto    invalid                = fixture.configValue;
        invalid.projectConfigurationId = 0;
        adk::InertPartsCarousel bad (
            invalid, fixture.identity, fixture.homing, fixture.auditSlots,
            sizeof (fixture.auditSlots), adk::carouselAuditRecordBytes, 4,

            fixture.auditCandidate, sizeof (fixture.auditCandidate));
        require (!bad.initialize ().ok (), "zero project id rejects");

        Fixture                 extentFixture;
        adk::InertPartsCarousel shortExtent (
            extentFixture.configValue, extentFixture.identity, extentFixture.homing,
            extentFixture.auditSlots, sizeof (extentFixture.auditSlots) - 1,
            adk::carouselAuditRecordBytes, 4, extentFixture.auditCandidate,
            sizeof (extentFixture.auditCandidate));
        require (!shortExtent.initialize ().ok (), "short audit extent rejects");

        Fixture                 strideFixture;
        adk::InertPartsCarousel wrongStride (
            strideFixture.configValue, strideFixture.identity, strideFixture.homing,
            strideFixture.auditSlots, sizeof (strideFixture.auditSlots),
            adk::carouselAuditRecordBytes - 1, 4, strideFixture.auditCandidate,
            sizeof (strideFixture.auditCandidate));
        require (!wrongStride.initialize ().ok (), "wrong audit stride rejects");

        Fixture                 nullFixture;
        adk::InertPartsCarousel nullSlots (
            nullFixture.configValue, nullFixture.identity, nullFixture.homing, nullptr,
            sizeof (nullFixture.auditSlots), adk::carouselAuditRecordBytes, 4,

            nullFixture.auditCandidate, sizeof (nullFixture.auditCandidate));
        require (!nullSlots.initialize ().ok (), "null audit storage rejects");

        Fixture                 overlapFixture;
        adk::InertPartsCarousel overlap (
            overlapFixture.configValue, overlapFixture.identity, overlapFixture.homing,
            overlapFixture.auditSlots, sizeof (overlapFixture.auditSlots),
            adk::carouselAuditRecordBytes, 4,
            overlapFixture.auditSlots + adk::carouselAuditRecordBytes,
            adk::carouselAuditRecordBytes);
        require (!overlap.initialize ().ok (), "overlapping candidate rejects");
    }
#endif

    adk::CarouselInputFrame frame (uint32_t at, uint32_t sequence, bool stop)
    {
        return {adk::TimePoint (at),
                sequence,
                false,
                {{adk::CarouselSourceKind::SyntheticIdentity, 0, 0},
                 adk::TimePoint (),
                 0,
                 {0, {}},
                 adk::StatusCode::Ok},
                false,
                {{adk::CarouselSourceKind::SyntheticKey, 0, 0},
                 adk::TimePoint (),
                 0,
                 0,
                 {},
                 false,
                 false,
                 adk::StatusCode::Ok},
                {{adk::CarouselSourceKind::SyntheticHome, 1, 1},
                 adk::TimePoint (at),
                 sequence,
                 false,
                 true,
                 3,
                 adk::StatusCode::Ok},
                {{adk::CarouselSourceKind::SyntheticStop, 1, 1},
                 adk::TimePoint (at),
                 sequence,
                 stop,
                 true,
                 4,
                 adk::StatusCode::Ok},
                {adk::TimePoint (at), sequence, adk::StatusCode::Ok}};
    }

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 1
    void testStopPrecedenceAndReplay ()
    {
        Fixture fixture;
        require (fixture.carousel.initialize ().ok (), "stop fixture initializes");

        auto input                 = frame (10, 1, true);
        input.hasIdentity          = true;
        input.identity.source.kind = adk::CarouselSourceKind::SyntheticKey;
        require (fixture.carousel.update (adk::TimePoint (10), input).ok (),
                 "valid stop dominates malformed unrelated identity");
        const auto stopped = fixture.carousel.snapshot ();

        require (stopped.phase == adk::CarouselPhase::Stopped &&
                     !stopped.authorizationCurrent && stopped.intent.coilBits == 0 &&
                     stopped.intent.gate == adk::CarouselGateIntent::Closed,
                 "stop publishes closed off and clears authority");

        input.stop.active = false;
        require (!fixture.carousel.update (adk::TimePoint (10), input).ok (),
                 "malformed ordinary frame rejects");
        require (fixture.carousel.snapshot ().phase == stopped.phase,
                 "malformed rejection preserves stopped state");
    }
#endif

    void installKnownDirectory (Fixture& fixture, uint8_t bin = 1)
    {
        encodeKnownImage (fixture.identitySlots, Fixture::registryId, bin);

        std::memcpy (fixture.identitySlots + adk::localIdentityImageBytes,
                     fixture.identitySlots, adk::localIdentityImageBytes);
    }

    adk::DurableAuditCandidate stageStart (Fixture& fixture, uint8_t bin = 1)
    {
        installKnownDirectory (fixture, bin);

        require (fixture.carousel.initialize ().ok (), "start fixture initializes");

        auto identityFrame        = frame (10, 1, false);
        identityFrame.hasIdentity = true;
        identityFrame.identity    = {{adk::CarouselSourceKind::SyntheticIdentity, 1, 1},
                                     adk::TimePoint (10),
                                     1,
                                     knownIdentity (),
                                     adk::StatusCode::Ok};
        require (fixture.carousel.update (adk::TimePoint (10), identityFrame).ok (),
                 "start fixture admits identity");
        const uint16_t code     = fixture.configValue.binConfirmationCodes[bin];
        auto           keyFrame = frame (11, 2, false);
        keyFrame.hasKey         = true;
        keyFrame.key            = {{adk::CarouselSourceKind::SyntheticKey, 1, 1},
                                   adk::TimePoint (11),
                                   2,
                                   2,
                                   {static_cast<uint8_t> (code / 10U),
                                    static_cast<uint8_t> (code % 10U), 0, 0},
                                   true,
                                   false,
                                   adk::StatusCode::Ok};
        require (fixture.carousel.update (adk::TimePoint (11), keyFrame).ok (),
                 "start fixture stages admission");
        return fixture.carousel.previewAuditWrite ().value ();
    }

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 1
    void testStopAfterExternalStartBeforeAcknowledgement ()
    {
        Fixture    fixture;
        const auto candidate = stageStart (fixture);

        const auto view = fixture.carousel.previewAuditExport (candidate);

        std::memcpy (fixture.auditSlots, view.value ().bytes, view.value ().length);
        const adk::AuditDurableCommitEvidence evidence = {
            candidate.owner,
            candidate.generation,
            candidate.operationId,
            candidate.slot,
            {fixture.auditSlots, adk::carouselAuditRecordBytes, candidate.slot,
             candidate.operationId},
            true,
            true,
            adk::StatusCode::Ok};

        const auto stop = frame (12, 3, true);

        require (fixture.carousel.update (adk::TimePoint (12), stop).ok (),
                 "stop remains available after external start write");
        const auto stopped = fixture.carousel.snapshot ();

        require (stopped.phase == adk::CarouselPhase::Stopped &&
                     !stopped.authorizationCurrent && stopped.intent.coilBits == 0 &&
                     stopped.intent.gate == adk::CarouselGateIntent::Closed,
                 "stop collision remains closed off and unauthorized");
        require (fixture.carousel.acknowledgeAuditWrite (candidate, evidence).ok (),
                 "late exact start acknowledgement reconciles");
        const auto terminal = fixture.carousel.previewAuditWrite ();

        require (terminal.ok () && terminal.value ().recordKind == 2 &&
                     terminal.value ().slot == 1,
                 "late installed start stages adjacent stopped terminal");
        const auto terminalView =
            fixture.carousel.previewAuditExport (terminal.value ());
        require (terminalView.ok () &&
                     terminalView.value ().bytes[21] ==
                         static_cast<uint8_t> (adk::CarouselPhase::Stopped) &&
                     terminalView.value ().bytes[23] ==
                         static_cast<uint8_t> (adk::CarouselAuditStatus::Stopped) &&
                     terminalView.value ().bytes[16] == 12 &&

                     terminalView.value ().bytes[17] == 0 &&

                     terminalView.value ().bytes[18] == 0 &&

                     terminalView.value ().bytes[19] == 0,
                 "late reconciliation terminal preserves stop attribution");
        uint8_t terminalBytes[adk::carouselAuditRecordBytes];
        std::memcpy (terminalBytes, terminalView.value ().bytes,
                     sizeof (terminalBytes));
        require (fixture.carousel.acknowledgeAuditWrite (candidate, evidence).ok (),
                 "exact repeated late-start acknowledgement is idempotent");
        const auto terminalAfterRepeat = fixture.carousel.previewAuditWrite ();
        const auto terminalViewAfterRepeat =
            fixture.carousel.previewAuditExport (terminalAfterRepeat.value ());

        require (terminalAfterRepeat.ok () &&
                     terminalAfterRepeat.value ().owner == terminal.value ().owner &&

                     terminalAfterRepeat.value ().generation ==
                         terminal.value ().generation &&
                     terminalAfterRepeat.value ().checksum ==
                         terminal.value ().checksum &&
                     terminalViewAfterRepeat.ok () &&

                     std::memcmp (terminalViewAfterRepeat.value ().bytes, terminalBytes,
                                  sizeof (terminalBytes)) == 0,
                 "repeated start acknowledgement preserves terminal candidate");
        auto changedLateRepeat         = evidence;
        changedLateRepeat.synchronized = false;
        require (!fixture.carousel.acknowledgeAuditWrite (candidate, changedLateRepeat)
                      .ok (),
                 "changed repeated late-start acknowledgement rejects");
        const auto terminalAfterChanged = fixture.carousel.previewAuditWrite ();
        const auto terminalViewAfterChanged =
            fixture.carousel.previewAuditExport (terminalAfterChanged.value ());
        require (terminalAfterChanged.ok () &&
                     terminalAfterChanged.value ().owner == terminal.value ().owner &&

                     terminalAfterChanged.value ().generation ==
                         terminal.value ().generation &&
                     terminalAfterChanged.value ().checksum ==
                         terminal.value ().checksum &&
                     terminalViewAfterChanged.ok () &&

                     std::memcmp (terminalViewAfterChanged.value ().bytes,
                                  terminalBytes, sizeof (terminalBytes)) == 0,
                 "changed repeat preserves terminal identity and bytes");
        const uint8_t installedIdentityByte = fixture.auditSlots[24];
        fixture.auditSlots[24] ^= 0x01U;
        require (!fixture.carousel.acknowledgeAuditWrite (candidate, evidence).ok (),
                 "repeat rejects changed installed start bytes");
        const auto terminalAfterInstalledMutation =
            fixture.carousel.previewAuditWrite ();
        const auto terminalViewAfterInstalledMutation =
            fixture.carousel.previewAuditExport (
                terminalAfterInstalledMutation.value ());
        require (terminalAfterInstalledMutation.ok () &&
                     terminalAfterInstalledMutation.value ().owner ==
                         terminal.value ().owner &&
                     terminalAfterInstalledMutation.value ().generation ==
                         terminal.value ().generation &&
                     terminalViewAfterInstalledMutation.ok () &&

                     std::memcmp (terminalViewAfterInstalledMutation.value ().bytes,
                                  terminalBytes, sizeof (terminalBytes)) == 0,
                 "changed installed start cannot mutate pending terminal");
        fixture.auditSlots[24] = installedIdentityByte;
        require (!fixture.carousel.snapshot ().authorizationCurrent &&
                     fixture.carousel.snapshot ().phase == adk::CarouselPhase::Stopped,
                 "late installed start never resurrects authority");

        fixture.carousel.shutdown ();

        require (fixture.carousel.snapshot ().phase ==
                         adk::CarouselPhase::Uninitialized &&
                     fixture.carousel.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "shutdown with terminal pending remains uninitialized");
        const auto retainedTerminal = fixture.carousel.previewAuditWrite ();

        require (retainedTerminal.ok () &&
                     retainedTerminal.value ().owner == terminal.value ().owner &&

                     retainedTerminal.value ().generation ==
                         terminal.value ().generation &&
                     retainedTerminal.value ().operationId ==
                         terminal.value ().operationId &&
                     retainedTerminal.value ().slot == terminal.value ().slot &&

                     retainedTerminal.value ().checksum == terminal.value ().checksum,
                 "shutdown preserves exact stopped terminal identity");
        const auto retainedView =
            fixture.carousel.previewAuditExport (retainedTerminal.value ());
        require (retainedView.ok () &&
                     std::memcmp (retainedView.value ().bytes, terminalBytes,
                                  sizeof (terminalBytes)) == 0,
                 "shutdown preserves exact stopped terminal bytes");
        require (!fixture.carousel.initialize ().ok (),
                 "pending stopped terminal blocks reinitialize");
        std::memcpy (fixture.auditSlots + adk::carouselAuditRecordBytes,
                     retainedView.value ().bytes, retainedView.value ().length);
        const adk::AuditDurableCommitEvidence terminalEvidence = {
            retainedTerminal.value ().owner,

            retainedTerminal.value ().generation,

            retainedTerminal.value ().operationId,

            retainedTerminal.value ().slot,
            {fixture.auditSlots + adk::carouselAuditRecordBytes,
             adk::carouselAuditRecordBytes, retainedTerminal.value ().slot,

             retainedTerminal.value ().operationId},
            true,
            true,
            adk::StatusCode::Ok};
        require (
            fixture.carousel
                .acknowledgeAuditWrite (retainedTerminal.value (), terminalEvidence)

                .ok (),
            "stopped terminal remains retryable after shutdown");
        require (fixture.carousel.snapshot ().phase ==
                         adk::CarouselPhase::Uninitialized &&
                     fixture.carousel.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "terminal acknowledgement cannot reactivate shutdown object");
        require (fixture.carousel.initialize ().ok (),
                 "reinitialize accepts reconciled stopped pair");
    }

    void testDurableAuthorizationAndHomeInvariant ()
    {
        Fixture fixture;
        installKnownDirectory (fixture);

        require (fixture.carousel.initialize ().ok (), "known fixture initializes");

        auto identityFrame        = frame (10, 1, false);
        identityFrame.hasIdentity = true;
        identityFrame.identity    = {{adk::CarouselSourceKind::SyntheticIdentity, 1, 1},
                                     adk::TimePoint (10),
                                     1,
                                     knownIdentity (),
                                     adk::StatusCode::Ok};
        require (fixture.carousel.update (adk::TimePoint (10), identityFrame).ok (),
                 "known identity is admitted");
        require (fixture.carousel.snapshot ().phase ==
                     adk::CarouselPhase::AwaitingConfirmation,
                 "known identity awaits exact confirmation");

        auto keyFrame   = frame (11, 2, false);
        keyFrame.hasKey = true;
        keyFrame.key    = {{adk::CarouselSourceKind::SyntheticKey, 1, 1},
                           adk::TimePoint (11),
                           2,
                           2,
                           {3, 4, 0, 0},
                           true,
                           false,
                           adk::StatusCode::Ok};
        require (fixture.carousel.update (adk::TimePoint (11), keyFrame).ok (),
                 "matching confirmation stages durable start");
        auto snapshot = fixture.carousel.snapshot ();

        require (snapshot.durableAdmissionPending && !snapshot.authorizationCurrent &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Closed,
                 "motion and gate remain inhibited before durable start");

        const auto candidate = fixture.carousel.previewAuditWrite ();

        const auto view = fixture.carousel.previewAuditExport (candidate.value ());

        std::memcpy (fixture.auditSlots, view.value ().bytes, view.value ().length);
        const adk::AuditDurableCommitEvidence evidence = {
            candidate.value ().owner,

            candidate.value ().generation,

            candidate.value ().operationId,

            candidate.value ().slot,

            {fixture.auditSlots, adk::carouselAuditRecordBytes, candidate.value ().slot,
             candidate.value ().operationId},
            true,
            true,
            adk::StatusCode::Ok};
        require (
            fixture.carousel.acknowledgeAuditWrite (candidate.value (), evidence).ok (),
            "exact externally installed start is acknowledged");
        snapshot = fixture.carousel.snapshot ();

        require (snapshot.authorizationCurrent &&
                     snapshot.phase == adk::CarouselPhase::Homing &&
                     !snapshot.positionKnown &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Closed,
                 "durable admission alone cannot open gate");

        auto seek = frame (16, 3, false);

        require (fixture.carousel.update (adk::TimePoint (16), seek).ok (),
                 "homing search advances bounded policy");
        snapshot = fixture.carousel.snapshot ();

        require (snapshot.intent.gate == adk::CarouselGateIntent::Closed,
                 "search never opens gate");

        auto home        = frame (21, 4, false);
        home.home.active = true;
        require (fixture.carousel.update (adk::TimePoint (21), home).ok (),
                 "qualified home edge synchronizes logical zero");
        snapshot = fixture.carousel.snapshot ();

        require (snapshot.positionKnown && snapshot.logicalPosition == 0 &&
                     snapshot.authorizationCurrent &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Open,
                 "gate opens only at exact target after durable authority and home");

        auto expiry = frame (41, 5, false);

        require (fixture.carousel.update (adk::TimePoint (41), expiry).ok (),
                 "gate expiry frame advances");
        snapshot = fixture.carousel.snapshot ();

        require (snapshot.phase == adk::CarouselPhase::Complete &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Closed &&
                     !snapshot.authorizationCurrent,
                 "expiry closes gate and clears authority");
    }

    void testEveryBinAndDirection ()
    {
        for (uint8_t bin = 0; bin < 3; ++bin)
        {
            Fixture fixture;
            installKnownDirectory (fixture, bin);

            require (fixture.carousel.initialize ().ok (),
                     "bin traversal fixture initializes");

            auto identityFrame        = frame (10, 1, false);
            identityFrame.hasIdentity = true;
            identityFrame.identity    = {
                {adk::CarouselSourceKind::SyntheticIdentity, 1, 1},
                adk::TimePoint (10),
                1,
                knownIdentity (),
                adk::StatusCode::Ok};
            require (fixture.carousel.update (adk::TimePoint (10), identityFrame).ok (),
                     "bin traversal admits identity");

            const uint16_t code     = fixture.configValue.binConfirmationCodes[bin];
            auto           keyFrame = frame (11, 2, false);
            keyFrame.hasKey         = true;
            keyFrame.key            = {{adk::CarouselSourceKind::SyntheticKey, 1, 1},
                                       adk::TimePoint (11),
                                       2,
                                       2,
                                       {static_cast<uint8_t> (code / 10U),
                                        static_cast<uint8_t> (code % 10U), 0, 0},
                                       true,
                                       false,
                                       adk::StatusCode::Ok};
            require (fixture.carousel.update (adk::TimePoint (11), keyFrame).ok (),
                     "bin traversal confirms exact code");
            const auto candidate = fixture.carousel.previewAuditWrite ();

            const auto view = fixture.carousel.previewAuditExport (candidate.value ());

            std::memcpy (fixture.auditSlots, view.value ().bytes, view.value ().length);
            const adk::AuditDurableCommitEvidence evidence = {
                candidate.value ().owner,

                candidate.value ().generation,

                candidate.value ().operationId,

                candidate.value ().slot,
                {fixture.auditSlots, adk::carouselAuditRecordBytes,
                 candidate.value ().slot, candidate.value ().operationId},
                true,
                true,
                adk::StatusCode::Ok};
            require (
                fixture.carousel.acknowledgeAuditWrite (candidate.value (), evidence)
                    .ok (),
                "bin traversal acknowledges start");

            auto release = frame (16, 3, false);

            require (fixture.carousel.update (adk::TimePoint (16), release).ok (),
                     "bin traversal begins home");
            auto home        = frame (21, 4, false);
            home.home.active = true;
            require (fixture.carousel.update (adk::TimePoint (21), home).ok (),
                     "bin traversal synchronizes home");

            uint32_t at       = 26;
            uint32_t sequence = 5;
            for (uint8_t attempt = 0;
                 attempt < 48 &&
                 fixture.carousel.snapshot ().phase != adk::CarouselPhase::GateIntent;
                 ++attempt)
            {
                auto progress        = frame (at, sequence, false);
                progress.home.active = false;
                require (fixture.carousel.update (adk::TimePoint (at), progress).ok (),
                         "bounded bin traversal advances");
                const auto state = fixture.carousel.snapshot ();

                require (state.intent.coilBits == 0,
                         "atomic one-step composition publishes no held coil frame");

                require (state.intent.gate == adk::CarouselGateIntent::Closed ||
                             (state.positionKnown &&
                              state.logicalPosition ==
                                  fixture.configValue.binPositions[bin]),
                         "gate never opens away from exact selected bin");
                require (state.phase != adk::CarouselPhase::Fault,
                         "early pre-deadline replay remains bounded progress");
                at += attempt % 2U == 0 ? 2U : 3U;
                ++sequence;
            }
            const auto reached = fixture.carousel.snapshot ();

            require (reached.phase == adk::CarouselPhase::GateIntent &&
                         reached.positionKnown &&
                         reached.logicalPosition ==
                             fixture.configValue.binPositions[bin],
                     "each negative zero and positive bin reaches exact coordinate");

            auto close = frame (at + 20, sequence, false);

            require (fixture.carousel.update (adk::TimePoint (at + 20), close).ok (),
                     "completed traversal closes semantic gate");
            const auto terminal = fixture.carousel.previewAuditWrite ();

            require (terminal.ok () && terminal.value ().recordKind == 2 &&
                         terminal.value ().slot == 1,
                     "completion prepares adjacent terminal audit");
            const auto terminalView =
                fixture.carousel.previewAuditExport (terminal.value ());
            require (terminalView.ok () && terminalView.value ().bytes[20] == 2 &&
                         terminalView.value ().bytes[21] ==
                             static_cast<uint8_t> (adk::CarouselPhase::Complete) &&
                         terminalView.value ().bytes[23] ==
                             static_cast<uint8_t> (adk::CarouselAuditStatus::Success),
                     "completion terminal has exact kind phase and status");
        }
    }
#endif

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 2
    void testAuditGoldenCutsCorruptionAndRecovery ()
    {
        uint8_t golden[adk::carouselAuditRecordBytes];
        encodeAudit (golden);

        require (golden[0] == 0x41 && golden[1] == 0x43 && golden[2] == 1 &&
                     golden[3] == 40,
                 "audit header is exact little endian");
        require (golden[4] == 0x88 && golden[7] == 0x55 && golden[8] == 7 &&
                     golden[12] == 9 && golden[14] == 11 && golden[16] == 0xd2 &&
                     golden[17] == 0x04,
                 "audit scalar offsets are exact");
        require (golden[20] == 1 &&
                     golden[21] == static_cast<uint8_t> (adk::CarouselPhase::Homing) &&
                     golden[22] == 1 && golden[23] == 0 && golden[24] == 0x33 &&
                     golden[25] == 0x22 && golden[26] == 4 && golden[28] == 5 &&
                     golden[32] == 0 && golden[36] == 0 && golden[37] == 0,
                 "audit typed and reserved offsets are exact");
        require (static_cast<uint16_t> (golden[38] | golden[39] << 8U) ==
                     crc16 (golden, 38),
                 "audit checksum covers bytes zero through thirty-seven");

        for (uint8_t cut = 1; cut < adk::carouselAuditRecordBytes; ++cut)
        {
            Fixture fixture;
            std::memcpy (fixture.auditSlots, golden, cut);

            require (!fixture.carousel.initialize ().ok (),
                     "every interrupted record prefix rejects");
            require (fixture.carousel.snapshot ().fault ==
                         adk::CarouselFault::AuditCorrupt,
                     "interrupted record is attributed as corrupt");
        }

        for (uint8_t byte = 0; byte < adk::carouselAuditRecordBytes; ++byte)
        {
            Fixture fixture;
            std::memcpy (fixture.auditSlots, golden, sizeof (golden));
            fixture.auditSlots[byte] ^= 0x01U;
            require (!fixture.carousel.initialize ().ok (),
                     "every single-byte record corruption rejects");
        }

        Fixture fixture;
        std::memcpy (fixture.auditSlots, golden, sizeof (golden));

        require (!fixture.carousel.initialize ().ok (),
                 "interrupted durable start initializes inhibited");
        auto snapshot = fixture.carousel.snapshot ();

        require (snapshot.phase == adk::CarouselPhase::Fault &&
                     snapshot.fault == adk::CarouselFault::AuditIndeterminate &&
                     snapshot.terminalReconciliationPending &&
                     !snapshot.authorizationCurrent && !snapshot.positionKnown &&
                     snapshot.intent.coilBits == 0 &&
                     snapshot.intent.gate == adk::CarouselGateIntent::Closed,
                 "recovery remains inhibited and offers terminal reconciliation");
        const auto candidate = fixture.carousel.previewAuditWrite ();

        require (candidate.ok () && candidate.value ().recordKind == 3 &&
                     candidate.value ().slot == 1,
                 "recovery uses adjacent reserved terminal slot");
        const auto view = fixture.carousel.previewAuditExport (candidate.value ());

        require (view.ok () && view.value ().length == adk::carouselAuditRecordBytes,
                 "recovery candidate exports canonical record");
        auto foreignCandidate = candidate.value ();
        foreignCandidate.owner ^= 0x100U;
        require (!fixture.carousel.previewAuditExport (foreignCandidate).ok (),
                 "foreign candidate owner rejects");
        foreignCandidate = candidate.value ();
        foreignCandidate.generation += 1;
        require (!fixture.carousel.previewAuditExport (foreignCandidate).ok (),
                 "stale candidate generation rejects");
        uint8_t recoveryBytes[adk::carouselAuditRecordBytes];
        std::memcpy (recoveryBytes, view.value ().bytes, sizeof (recoveryBytes));

        fixture.carousel.shutdown ();

        require (fixture.carousel.snapshot ().phase ==
                         adk::CarouselPhase::Uninitialized &&
                     fixture.carousel.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "shutdown with recovered terminal pending stays inactive");
        require (!fixture.carousel.initialize ().ok (),
                 "pending recovered terminal blocks reinitialize");
        const auto retainedRecovery = fixture.carousel.previewAuditWrite ();
        const auto retainedRecoveryView =
            fixture.carousel.previewAuditExport (retainedRecovery.value ());
        require (
            retainedRecovery.ok () &&
                retainedRecovery.value ().owner == candidate.value ().owner &&

                retainedRecovery.value ().generation == candidate.value ().generation &&

                retainedRecovery.value ().operationId ==
                    candidate.value ().operationId &&
                retainedRecovery.value ().slot == candidate.value ().slot &&

                retainedRecovery.value ().checksum == candidate.value ().checksum &&

                retainedRecoveryView.ok () &&

                std::memcmp (retainedRecoveryView.value ().bytes, recoveryBytes,
                             sizeof (recoveryBytes)) == 0,
            "shutdown preserves exact recovered-terminal retry");
        std::memcpy (fixture.auditSlots + adk::carouselAuditRecordBytes,
                     view.value ().bytes, view.value ().length);
        adk::AuditDurableCommitEvidence evidence = {
            candidate.value ().owner,

            candidate.value ().generation,

            candidate.value ().operationId,

            candidate.value ().slot,
            {fixture.auditSlots + adk::carouselAuditRecordBytes,
             adk::carouselAuditRecordBytes, candidate.value ().slot,

             candidate.value ().operationId},
            true,
            true,
            adk::StatusCode::Ok};
        uint8_t temporary[adk::carouselAuditRecordBytes];
        std::memcpy (temporary, view.value ().bytes, sizeof (temporary));
        auto temporaryEvidence                   = evidence;
        temporaryEvidence.reconciledRecord.bytes = temporary;
        require (!fixture.carousel
                      .acknowledgeAuditWrite (candidate.value (), temporaryEvidence)

                      .ok (),
                 "candidate workspace or temporary reread pointer rejects");
        auto indeterminate         = evidence;
        indeterminate.synchronized = false;
        require (
            !fixture.carousel.acknowledgeAuditWrite (candidate.value (), indeterminate)
                 .ok (),
            "unsynchronized durable evidence is indeterminate");
        require (fixture.carousel.snapshot ().fault ==
                     adk::CarouselFault::AuditIndeterminate,
                 "indeterminate acknowledgement retains typed attribution");
        require (
            fixture.carousel.acknowledgeAuditWrite (candidate.value (), evidence).ok (),
            "exact reread acknowledges recovered terminal");
        require (fixture.carousel.snapshot ().phase ==
                         adk::CarouselPhase::Uninitialized &&
                     fixture.carousel.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "recovered-terminal ack cannot reactivate shutdown object");
        require (
            fixture.carousel.acknowledgeAuditWrite (candidate.value (), evidence).ok (),
            "exact repeated acknowledgement is idempotent");
        auto changedRepeat         = evidence;
        changedRepeat.synchronized = false;
        require (
            !fixture.carousel.acknowledgeAuditWrite (candidate.value (), changedRepeat)
                 .ok (),
            "changed repeated acknowledgement rejects");
        require (fixture.carousel.initialize ().ok (),
                 "reinitialize accepts reconciled recovered pair");
    }

    void testAuditPairSequenceMatrix ()
    {
        const uint16_t deltas[] = {1, 0x7fffU};
        for (uint16_t delta : deltas)
        {
            Fixture fixture;
            encodeAudit (fixture.auditSlots);

            encodeAudit (fixture.auditSlots + adk::carouselAuditRecordBytes, 2,
                         adk::CarouselPhase::Complete,
                         adk::CarouselAuditStatus::Success);
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 14,
                   static_cast<uint16_t> (11U + delta));
            put32 (fixture.auditSlots + adk::carouselAuditRecordBytes, 32, 6);

            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38,
                   crc16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38));
            require (fixture.carousel.initialize ().ok (),
                     "forward audit sequence delta initializes");
        }

        const uint16_t rejected[] = {0, 0x8000U, 0x8001U, 0xffffU};
        for (uint16_t delta : rejected)
        {
            Fixture fixture;
            encodeAudit (fixture.auditSlots);

            encodeAudit (fixture.auditSlots + adk::carouselAuditRecordBytes, 2,
                         adk::CarouselPhase::Complete,
                         adk::CarouselAuditStatus::Success);
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 14,
                   static_cast<uint16_t> (11U + delta));
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38,
                   crc16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38));
            require (!fixture.carousel.initialize ().ok (),
                     "duplicate ambiguous or regressed audit sequence rejects");
        }

        struct TerminalCase
        {
            adk::CarouselPhase       phase;
            adk::CarouselAuditStatus status;
            uint32_t                 homeEpoch;
            bool                     accepted;
        };
        const TerminalCase cases[] = {
            {adk::CarouselPhase::Complete, adk::CarouselAuditStatus::Success, 4, true},
            {adk::CarouselPhase::Stopped, adk::CarouselAuditStatus::Stopped, 4, true},
            {adk::CarouselPhase::Cancelled, adk::CarouselAuditStatus::Cancelled, 0,
             true},
            {adk::CarouselPhase::Cancelled,
             adk::CarouselAuditStatus::AuthorizationExpired, 0, true},
            {adk::CarouselPhase::Homing, adk::CarouselAuditStatus::Success, 4, false},
            {adk::CarouselPhase::Complete, adk::CarouselAuditStatus::Stopped, 4, false},
            {adk::CarouselPhase::Stopped, adk::CarouselAuditStatus::Success, 4, false},
            {adk::CarouselPhase::Uninitialized, adk::CarouselAuditStatus::Success, 4,
             false}};
        for (const auto& item : cases)
        {
            Fixture fixture;
            encodeAudit (fixture.auditSlots);

            encodeAudit (fixture.auditSlots + adk::carouselAuditRecordBytes, 2,
                         item.phase, item.status);
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 14, 12);

            put32 (fixture.auditSlots + adk::carouselAuditRecordBytes, 32,
                   item.homeEpoch);
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38,
                   crc16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38));
            require (fixture.carousel.initialize ().ok () == item.accepted,
                     "terminal phase/status/home tuple is classified exactly");
        }
    }

    void testAuditProvenanceCapacityAndFaultVocabulary ()
    {
        {
            Fixture fixture;
            encodeAudit (fixture.auditSlots);

            put32 (fixture.auditSlots, 28, 0);

            put16 (fixture.auditSlots, 38, crc16 (fixture.auditSlots, 38));

            require (!fixture.carousel.initialize ().ok (),
                     "zero identity image generation rejects");
        }

        for (uint8_t capacity : {uint8_t (2), uint8_t (4), uint8_t (6), uint8_t (8)})
        {
            Fixture fixture;
            uint8_t slots[8 * adk::carouselAuditRecordBytes];
            uint8_t candidate[adk::carouselAuditRecordBytes];
            std::memset (slots, 0xff, sizeof (slots));

            for (uint8_t pair = 0; pair < capacity / 2U; ++pair)
            {
                uint8_t* start    = slots + static_cast<uint16_t> (pair * 2U) *
                                                adk::carouselAuditRecordBytes;
                uint8_t* terminal = start + adk::carouselAuditRecordBytes;
                encodeAudit (start);

                encodeAudit (terminal, 2, adk::CarouselPhase::Complete,
                             adk::CarouselAuditStatus::Success);
                for (uint8_t* record : {start, terminal})
                {
                    put32 (record, 8, static_cast<uint32_t> (pair + 1U));

                    put16 (record, 12, static_cast<uint16_t> (pair + 1U));
                }
                put16 (start, 14, static_cast<uint16_t> (pair * 2U + 1U));

                put16 (terminal, 14, static_cast<uint16_t> (pair * 2U + 2U));

                put32 (terminal, 32, static_cast<uint32_t> (pair + 1U));

                put16 (start, 38, crc16 (start, 38));

                put16 (terminal, 38, crc16 (terminal, 38));
            }
            adk::InertPartsCarousel full (
                fixture.configValue, fixture.identity, fixture.homing, slots,
                static_cast<uint16_t> (capacity * adk::carouselAuditRecordBytes),

                adk::carouselAuditRecordBytes, capacity, candidate, sizeof (candidate));
            require (full.initialize ().ok (),
                     "every legal completely full paired journal initializes");
        }

        {
            Fixture fixture;
            encodeAudit (fixture.auditSlots);

            encodeAudit (fixture.auditSlots + adk::carouselAuditRecordBytes, 2,
                         adk::CarouselPhase::Complete,
                         adk::CarouselAuditStatus::Success);
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 14, 12);

            put32 (fixture.auditSlots + adk::carouselAuditRecordBytes, 32, 1);

            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38,
                   crc16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38));
            encodeAudit (fixture.auditSlots + 3U * adk::carouselAuditRecordBytes);

            require (!fixture.carousel.initialize ().ok (),
                     "non-erased record after erased tail rejects");
        }

        const adk::CarouselAuditStatus faults[] = {
            adk::CarouselAuditStatus::EvidenceFault,
            adk::CarouselAuditStatus::TimingFault,
            adk::CarouselAuditStatus::IdentityFault,
            adk::CarouselAuditStatus::ConfirmationConflict,
            adk::CarouselAuditStatus::HomingFault,
            adk::CarouselAuditStatus::PositionFault,
            adk::CarouselAuditStatus::GateFault,
            adk::CarouselAuditStatus::AuditCorrupt,
            adk::CarouselAuditStatus::AuditUnsupported,
            adk::CarouselAuditStatus::AuditIndeterminate,
            adk::CarouselAuditStatus::AuditStorageFault,
            adk::CarouselAuditStatus::PresentationFault};
        for (auto fault : faults)
        {
            Fixture fixture;
            encodeAudit (fixture.auditSlots);

            encodeAudit (fixture.auditSlots + adk::carouselAuditRecordBytes, 2,
                         adk::CarouselPhase::Fault, fault);
            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 14, 12);

            put16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38,
                   crc16 (fixture.auditSlots + adk::carouselAuditRecordBytes, 38));
            require (fixture.carousel.initialize ().ok (),
                     "every typed fault terminal decodes canonically");
        }
    }

    void testSameAddressCandidateAba ()
    {
        Fixture    fixture;
        const auto stale = stageStart (fixture);

        fixture.carousel.shutdown ();

        fixture.carousel.~InertPartsCarousel ();

        new (&fixture.carousel) adk::InertPartsCarousel (
            fixture.configValue, fixture.identity, fixture.homing, fixture.auditSlots,
            sizeof (fixture.auditSlots), adk::carouselAuditRecordBytes, 4,

            fixture.auditCandidate, sizeof (fixture.auditCandidate));
        const auto fresh = stageStart (fixture);

        require (fresh.owner != stale.owner,
                 "same-address reconstruction receives monotonic owner");
        require (!fixture.carousel.previewAuditExport (stale).ok (),
                 "same-address stale candidate cannot alias fresh generation");
        require (fixture.carousel.previewAuditExport (fresh).ok (),
                 "fresh reconstructed candidate remains valid");
    }

    void testTwoInstanceReplay ()
    {
        Fixture left;
        Fixture right;
        installKnownDirectory (left, 2);

        installKnownDirectory (right, 2);

        require (left.carousel.initialize ().ok () &&
                     right.carousel.initialize ().ok (),
                 "replay fixtures initialize");
        require (sameSnapshot (left.carousel.snapshot (), right.carousel.snapshot ()),
                 "initialized snapshots replay fieldwise");

        auto identityFrame        = frame (100, 1, false);
        identityFrame.hasIdentity = true;
        identityFrame.identity    = {{adk::CarouselSourceKind::SyntheticIdentity, 2, 3},
                                     adk::TimePoint (100),
                                     1,
                                     knownIdentity (),
                                     adk::StatusCode::Ok};
        require (left.carousel.update (adk::TimePoint (100), identityFrame).ok () &&
                     right.carousel.update (adk::TimePoint (100), identityFrame).ok (),
                 "identical copied identity replays");
        require (sameSnapshot (left.carousel.snapshot (), right.carousel.snapshot ()),
                 "identity snapshots replay fieldwise");

        auto cancel   = frame (101, 2, false);
        cancel.hasKey = true;
        cancel.key    = {{adk::CarouselSourceKind::SyntheticKey, 2, 3},
                         adk::TimePoint (101),
                         2,
                         0,
                         {},
                         false,
                         true,
                         adk::StatusCode::Ok};
        require (left.carousel.update (adk::TimePoint (101), cancel).ok () &&
                     right.carousel.update (adk::TimePoint (101), cancel).ok (),
                 "identical cancellation replays");
        require (sameSnapshot (left.carousel.snapshot (), right.carousel.snapshot ()),
                 "terminal cancellation snapshots replay fieldwise");
    }
#endif
} // namespace

int main ()
{
#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 1
    testTraitsLifecycleAndSafeState ();

    testConfigurationAndStorageBounds ();

    testStopPrecedenceAndReplay ();

    testStopAfterExternalStartBeforeAcknowledgement ();

    testDurableAuthorizationAndHomeInvariant ();

    testEveryBinAndDirection ();
#endif

#if ADK_INERT_PARTS_CAROUSEL_TEST_PART == 0 || ADK_INERT_PARTS_CAROUSEL_TEST_PART == 2
    testAuditGoldenCutsCorruptionAndRecovery ();

    testAuditPairSequenceMatrix ();

    testAuditProvenanceCapacityAndFaultVocabulary ();

    testSameAddressCandidateAba ();

    testTwoInstanceReplay ();
#endif
    std::cout << "inert parts carousel tests passed\n";
    return EXIT_SUCCESS;
}
