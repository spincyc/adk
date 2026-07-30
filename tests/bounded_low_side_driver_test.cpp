#include <bounded_low_side_driver_policy.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>

namespace {

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::LowSideDriverDescriptor descriptor ()
    {
        return {1,
                0x10203040UL,
                0x11223344UL,
                2,
                3,
                0x55667788UL,
                0x22334455UL,
                4,
                adk::LowSideLoadEnergy::ResistiveIndicator,
                adk::LowSideFlybackRequirement::NotRequired,
                adk::LowSideFlybackDeclaration::Absent,
                true,
                0,
                0,
                0,
                0,
                0,
                0,
                {100000, 10000, 50000, 60000, 5000, 10, 1, 1000, 0, 4000, 5000, 1000,
                 5000, adk::Duration (100), adk::Duration (1000), 500}};
    }

    adk::LowSideDriverDescriptor inductiveDescriptor ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();
        value.loadEnergy                   = adk::LowSideLoadEnergy::InductiveInert;
        value.flybackRequirement           = adk::LowSideFlybackRequirement::Required;
        value.flybackDeclaration           = adk::LowSideFlybackDeclaration::Present;
        value.flybackDiodeIdentity         = 0x10293847UL;
        value.flybackDiodeRevision         = 1;
        value.flybackOrientationCode       = 1;
        value.flybackReturnCode            = 1;
        value.flybackRepetitiveReverseMv   = 50000;
        value.flybackForwardCurrentUa      = 100000;
        return value;
    }

    adk::LowSideDriveRequest request (uint32_t sequence, uint32_t observedAt,
                                      bool active = true)
    {
        return {7,
                9,
                11,
                sequence,
                13,
                4,
                sequence,
                adk::TimePoint (observedAt),
                1,
                active,
                static_cast<uint32_t> (active ? 30000U : 0U),
                adk::Duration (active ? 100UL : 0UL),
                adk::StatusCode::Ok};
    }

    adk::LowSideControl control (uint32_t sequence, uint32_t observedAt,
                                 bool        offConfirmed = true,
                                 adk::Status status       = adk::StatusCode::Ok)
    {
        return {1,
                7,
                9,
                11,
                sequence,
                13,
                4,
                sequence,
                adk::TimePoint (observedAt),
                offConfirmed,
                status};
    }

    bool equalIntent (const adk::LowSideDriveIntent& left,
                      const adk::LowSideDriveIntent& right)
    {
        return left.lifecycleGeneration == right.lifecycleGeneration &&
               left.driverDescriptorIdentityDigest ==
                   right.driverDescriptorIdentityDigest &&
               left.specimenReference == right.specimenReference &&
               left.specimenRevision == right.specimenRevision &&
               left.electricalEvidenceRevision == right.electricalEvidenceRevision &&
               left.policyConfigurationId == right.policyConfigurationId &&
               left.policyConfigurationRevision == right.policyConfigurationRevision &&
               left.sessionId == right.sessionId && left.runId == right.runId &&
               left.stepId == right.stepId && left.requestId == right.requestId &&
               left.state == right.state && left.reason == right.reason &&
               left.logicalActive == right.logicalActive &&
               left.outputLevelHigh == right.outputLevelHigh &&
               left.requiredBaseUa == right.requiredBaseUa &&
               left.admittedBaseUa == right.admittedBaseUa &&
               left.admittedLoadUa == right.admittedLoadUa &&
               left.expiresAt.milliseconds () == right.expiresAt.milliseconds () &&
               left.producerStatus == right.producerStatus;
    }

    adk::LowSideDriveIntent blankIntent ()
    {
        return {0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                adk::LowSideDriveState::Off,
                adk::LowSideDriveReason::None,
                false,
                false,
                0,
                0,
                0,
                adk::TimePoint (0),
                adk::StatusCode::Ok};
    }

    void requireUnchanged (adk::BoundedLowSideDriverPolicy& policy,
                           const adk::LowSideDriveIntent&   before,
                           const adk::LowSideDriveIntent&   canary,
                           const adk::LowSideDriveIntent& output, const char* message)
    {
        require (equalIntent (before, policy.snapshot ()), message);

        require (equalIntent (canary, output), "rejected call preserves output");
    }

    uint32_t crcByte (uint32_t crc, uint8_t byte)
    {
        crc ^= byte;
        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            crc = (crc & 1U) != 0U ? (crc >> 1U) ^ 0xEDB88320UL : crc >> 1U;
        }
        return crc;
    }

    void digestU8 (uint32_t& crc, uint8_t value)
    {
        crc = crcByte (crc, value);
    }

    void digestU16 (uint32_t& crc, uint16_t value)
    {
        digestU8 (crc, static_cast<uint8_t> (value));

        digestU8 (crc, static_cast<uint8_t> (value >> 8U));
    }

    void digestU32 (uint32_t& crc, uint32_t value)
    {
        digestU16 (crc, static_cast<uint16_t> (value));

        digestU16 (crc, static_cast<uint16_t> (value >> 16U));
    }

    uint32_t independentDigest (const adk::LowSideDriverDescriptor& value,
                                const uint8_t* tag, uint8_t tagLength)
    {
        uint32_t crc = 0xFFFFFFFFUL;
        for (uint8_t index = 0; index < tagLength; ++index)
        {
            digestU8 (crc, tag[index]);
        }

        digestU16 (crc, value.schemaRevision);

        digestU32 (crc, value.specimenFamilyReference);

        digestU32 (crc, value.specimenReference);

        digestU16 (crc, value.specimenRevision);

        digestU16 (crc, value.electricalEvidenceRevision);

        digestU32 (crc, value.sourcePacketDigest);

        digestU32 (crc, value.configurationId);

        digestU16 (crc, value.configurationRevision);

        digestU8 (crc, static_cast<uint8_t> (value.loadEnergy));

        digestU8 (crc, static_cast<uint8_t> (value.flybackRequirement));

        digestU8 (crc, static_cast<uint8_t> (value.flybackDeclaration));

        digestU8 (crc, value.sourceEligible ? 1U : 0U);

        digestU32 (crc, value.flybackDiodeIdentity);

        digestU16 (crc, value.flybackDiodeRevision);

        digestU8 (crc, value.flybackOrientationCode);

        digestU8 (crc, value.flybackReturnCode);

        digestU32 (crc, value.flybackRepetitiveReverseMv);

        digestU32 (crc, value.flybackForwardCurrentUa);

        digestU32 (crc, value.budget.supplyLimitUa);

        digestU32 (crc, value.budget.reservedSupplyUa);

        digestU32 (crc, value.budget.fixtureLoadCeilingUa);

        digestU32 (crc, value.budget.deviceContinuousUa);

        digestU32 (crc, value.budget.gpioSourceCeilingUa);

        digestU32 (crc, value.budget.forcedGainNumerator);

        digestU32 (crc, value.budget.forcedGainDenominator);

        digestU32 (crc, value.budget.baseResistanceOhms);

        digestU16 (crc, value.budget.baseResistanceTolerancePermille);

        digestU16 (crc, value.budget.logicHighMinimumMv);

        digestU16 (crc, value.budget.logicHighMaximumMv);

        digestU16 (crc, value.budget.baseEmitterMaximumMv);

        digestU16 (crc, value.budget.collectorEmitterOperatingMaximumMv);

        digestU32 (crc, value.budget.maximumActiveDuration.milliseconds ());

        digestU32 (crc, value.budget.dutyWindow.milliseconds ());

        digestU16 (crc, value.budget.maximumDutyPermille);
        return crc ^ 0xFFFFFFFFUL;
    }

    void testDescriptorAndDigest ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();

        require (adk::validateLowSideDriverDescriptor (value).ok (),
                 "canonical resistive descriptor valid");
        require (adk::validateLowSideDriverDescriptor (inductiveDescriptor ()).ok (),
                 "complete inductive descriptor valid");

        constexpr uint8_t tag[] = {0x41, 0x44, 0x4B, 0x37, 0x39, 0x44, 0x53, 0x43};
        require (adk::lowSideDriverDescriptorIdentityDigest (value) ==
                     independentDigest (value, tag, sizeof (tag)),
                 "production digest matches independent canonical encoder");
        const uint32_t golden = independentDigest (value, tag, sizeof (tag));

        require (golden == 0xD08F78B1UL, "descriptor digest fixed golden");

        constexpr uint8_t wrongTag[] = {0x41, 0x44, 0x4B, 0x37, 0x39, 0x44, 0x53, 0x44};
        constexpr uint8_t nulTag[]   = {0x41, 0x44, 0x4B, 0x37, 0x39,
                                        0x44, 0x53, 0x43, 0x00};
        require (golden != independentDigest (value, tag, sizeof (tag) - 1),
                 "omitted tag byte changes digest");
        require (golden != independentDigest (value, nulTag, sizeof (nulTag)),
                 "extra NUL changes digest");
        require (golden != independentDigest (value, wrongTag, sizeof (wrongTag)),
                 "wrong tag changes digest");

#define REQUIRE_DIGEST_FIELD(field, replacement)                                       \
    do                                                                                 \
    {                                                                                  \
        adk::LowSideDriverDescriptor changed = value;                                  \
        changed.field                        = replacement;                            \
        require (adk::lowSideDriverDescriptorIdentityDigest (changed) != golden,       \
                 "digest binds " #field);                                              \
    }                                                                                  \
    while (false)
        REQUIRE_DIGEST_FIELD (schemaRevision, 2);

        REQUIRE_DIGEST_FIELD (specimenFamilyReference, 1);

        REQUIRE_DIGEST_FIELD (specimenReference, 1);

        REQUIRE_DIGEST_FIELD (specimenRevision, 1);

        REQUIRE_DIGEST_FIELD (electricalEvidenceRevision, 1);

        REQUIRE_DIGEST_FIELD (sourcePacketDigest, 1);

        REQUIRE_DIGEST_FIELD (configurationId, 1);

        REQUIRE_DIGEST_FIELD (configurationRevision, 1);

        REQUIRE_DIGEST_FIELD (loadEnergy, adk::LowSideLoadEnergy::InductiveInert);

        REQUIRE_DIGEST_FIELD (flybackRequirement,
                              adk::LowSideFlybackRequirement::Required);
        REQUIRE_DIGEST_FIELD (flybackDeclaration,
                              adk::LowSideFlybackDeclaration::Present);
        REQUIRE_DIGEST_FIELD (sourceEligible, false);

        REQUIRE_DIGEST_FIELD (flybackDiodeIdentity, 1);

        REQUIRE_DIGEST_FIELD (flybackDiodeRevision, 1);

        REQUIRE_DIGEST_FIELD (flybackOrientationCode, 1);

        REQUIRE_DIGEST_FIELD (flybackReturnCode, 1);

        REQUIRE_DIGEST_FIELD (flybackRepetitiveReverseMv, 1);

        REQUIRE_DIGEST_FIELD (flybackForwardCurrentUa, 1);
#undef REQUIRE_DIGEST_FIELD

#define REQUIRE_BUDGET_DIGEST(field, replacement)                                      \
    do                                                                                 \
    {                                                                                  \
        adk::LowSideDriverDescriptor changed = value;                                  \
        changed.budget.field                 = replacement;                            \
        require (adk::lowSideDriverDescriptorIdentityDigest (changed) != golden,       \
                 "digest binds budget " #field);                                       \
    }                                                                                  \
    while (false)
        REQUIRE_BUDGET_DIGEST (supplyLimitUa, 1);

        REQUIRE_BUDGET_DIGEST (reservedSupplyUa, 1);

        REQUIRE_BUDGET_DIGEST (fixtureLoadCeilingUa, 1);

        REQUIRE_BUDGET_DIGEST (deviceContinuousUa, 1);

        REQUIRE_BUDGET_DIGEST (gpioSourceCeilingUa, 1);

        REQUIRE_BUDGET_DIGEST (forcedGainNumerator, 1);

        REQUIRE_BUDGET_DIGEST (forcedGainDenominator, 2);

        REQUIRE_BUDGET_DIGEST (baseResistanceOhms, 1);

        REQUIRE_BUDGET_DIGEST (baseResistanceTolerancePermille, 1);

        REQUIRE_BUDGET_DIGEST (logicHighMinimumMv, 1);

        REQUIRE_BUDGET_DIGEST (logicHighMaximumMv, 1);

        REQUIRE_BUDGET_DIGEST (baseEmitterMaximumMv, 1);

        REQUIRE_BUDGET_DIGEST (collectorEmitterOperatingMaximumMv, 1);

        REQUIRE_BUDGET_DIGEST (maximumActiveDuration, adk::Duration (99));

        REQUIRE_BUDGET_DIGEST (dutyWindow, adk::Duration (999));

        REQUIRE_BUDGET_DIGEST (maximumDutyPermille, 499);
#undef REQUIRE_BUDGET_DIGEST

        const adk::LowSideDriverDescriptor original = value;
        const uint8_t                      invalidLoad =
            static_cast<uint8_t> (adk::LowSideLoadEnergy::InductiveInert) + 1U;
        value.loadEnergy = static_cast<adk::LowSideLoadEnergy> (invalidLoad);
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "invalid load enum rejected");
        value                    = original;
        value.flybackRequirement = static_cast<adk::LowSideFlybackRequirement> (2);
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "invalid requirement enum rejected");
        value                    = original;
        value.flybackDeclaration = static_cast<adk::LowSideFlybackDeclaration> (2);
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "invalid declaration enum rejected");

        value                         = original;
        value.specimenFamilyReference = 0;
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "zero family rejected");
        value                    = original;
        value.sourcePacketDigest = 0;
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "zero source packet rejected");
        value                         = original;
        value.budget.reservedSupplyUa = value.budget.supplyLimitUa + 1;
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "supply subtraction underflow rejected");

        value                      = inductiveDescriptor ();
        value.flybackDiodeIdentity = 0;
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "inductive diode identity required");
        value                         = inductiveDescriptor ();
        value.flybackForwardCurrentUa = value.budget.fixtureLoadCeilingUa - 1;
        require (!adk::validateLowSideDriverDescriptor (value).ok (),
                 "under-rated inductive diode rejected");
    }

    void testArithmeticAndDeadline ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        require (!policy.initialized (), "construction inert");

        require (policy.initialize ().ok (), "initialize succeeds");

        require (policy.initialize ().ok (), "initialize idempotent");

        require (policy.beginSession (7, 9).ok (), "session begins");

        adk::LowSideDriveIntent output = blankIntent ();

        const adk::Status       status = policy.apply (request (1, 1000), output);

        require (status.ok (), "active request is domain-evaluated");

        require (output.state == adk::LowSideDriveState::Requested &&
                     output.reason == adk::LowSideDriveReason::None &&
                     output.logicalActive && output.outputLevelHigh,
                 "healthy active request admitted active-high");
        require (output.requiredBaseUa == 3000 && output.admittedBaseUa == 3000 &&
                     output.admittedLoadUa == 30000,
                 "conservative current arithmetic exact");
        require (output.expiresAt.milliseconds () == 1100,
                 "deadline derives from supplied observation");

        const adk::LowSideDriveIntent admitted = output;
        require (policy.update (adk::TimePoint (1099), output).ok (),
                 "one tick before deadline accepted");
        require (equalIntent (admitted, output), "one tick before stays active");

        require (policy.update (adk::TimePoint (1100), output).ok (),
                 "deadline equality accepted");
        require (output.state == adk::LowSideDriveState::Off &&
                     output.reason == adk::LowSideDriveReason::Expired &&
                     !output.logicalActive && !output.outputLevelHigh,
                 "deadline equality expires off");

        require (policy.reset ().ok (), "reset succeeds");

        require (policy.beginSession (7, 9).ok (), "session restarts after reset");

        adk::LowSideDriveRequest rounded = request (1, 2000);
        rounded.requestedLoadUa          = 30001;
        rounded.lifecycleGeneration      = 2;
        require (policy.apply (rounded, output).ok (), "rounded request evaluated");

        require (output.requiredBaseUa == 3001, "required base current rounds upward");

        adk::LowSideDriverDescriptor limited = descriptor ();
        limited.budget.baseResistanceOhms    = 2000;
        adk::BoundedLowSideDriverPolicy limitedPolicy (limited);

        require (limitedPolicy.initialize ().ok (), "limited policy initializes");

        require (limitedPolicy.beginSession (7, 9).ok (), "limited session begins");

        rounded.lifecycleGeneration = 1;
        require (limitedPolicy.apply (rounded, output).ok (),
                 "over-budget request is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::BaseBudgetInsufficient &&
                     !output.outputLevelHigh,
                 "one microamp base deficit rejects off");

        rounded.requestedActiveDuration      = adk::Duration (101);

        const adk::LowSideDriveIntent before = limitedPolicy.snapshot ();
        adk::LowSideDriveIntent       canary = output;
        require (limitedPolicy.apply (rounded, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "duration one over is API-invalid");
        requireUnchanged (limitedPolicy, before, canary, output,
                          "duration rejection atomic");
    }

    void testIdentitySequenceAndAtomicity ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.beginSession (7, 9).error () == adk::StatusCode::NotInitialized,
                 "session before initialization rejected");
        require (policy.initialize ().ok (), "initialize for correlation");

        require (policy.beginSession (0, 9).error () ==
                     adk::StatusCode::InvalidArgument,
                 "zero session rejected");
        require (policy.beginSession (7, 9).ok (), "correlated session begins");

        require (policy.apply (request (1, 100), output).ok (),
                 "first sequence accepted");

        const adk::LowSideDriveIntent before  = policy.snapshot ();
        adk::LowSideDriveIntent       canary  = output;
        adk::LowSideDriveRequest      changed = request (1, 100);
        changed.requestedLoadUa               = 20000;
        require (policy.apply (changed, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed duplicate rejected");
        requireUnchanged (policy, before, canary, output, "changed duplicate atomic");

        changed           = request (3, 101, false);
        changed.sequence  = 3;
        changed.requestId = 3;
        require (policy.apply (changed, output).ok (),
                 "sequence gap is attributable domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason ==
                         adk::LowSideDriveReason::SequenceDiscontinuity &&
                     !output.outputLevelHigh,
                 "sequence gap publishes off discontinuity");
        const adk::LowSideDriveIntent discontinuity = output;

        require (policy.apply (changed, output).ok (),
                 "identical sequence-gap replay is idempotent");
        require (equalIntent (discontinuity, output),
                 "identical gap replay preserves terminal result");

        adk::LowSideDriveRequest changedGap = changed;
        changedGap.producerStatus = adk::StatusCode::HardwareFailure;
        require (policy.apply (changedGap, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed replay at gap sequence rejects");
        requireUnchanged (policy, discontinuity, discontinuity, output,
                          "changed gap replay is atomic");

        changed           = request (2, 101, false);
        changed.sessionId = 8;
        require (policy.apply (changed, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "session drift rejected");
        requireUnchanged (policy, discontinuity, discontinuity, output,
                          "identity drift atomic");

        changed                     = request (2, 101, false);
        changed.lifecycleGeneration = 2;
        require (policy.apply (changed, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "lifecycle drift rejected");
        requireUnchanged (policy, discontinuity, discontinuity, output,
                          "lifecycle rejection atomic");

        require (policy.update (adk::TimePoint (0x80000065UL), output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "half-range update rejected");
        requireUnchanged (policy, discontinuity, discontinuity, output,
                          "half-range update atomic");
    }

    void testResistanceToleranceBounds ()
    {
        adk::LowSideDriverDescriptor exact = descriptor ();

        adk::BoundedLowSideDriverPolicy exactPolicy (exact);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (exactPolicy.initialize ().ok (), "exact resistor initializes");

        require (exactPolicy.beginSession (7, 9).ok (), "exact resistor session");

        require (exactPolicy.apply (request (1, 100), output).ok (),
                 "exact resistor request evaluates");
        require (output.state == adk::LowSideDriveState::Requested &&
                     output.admittedBaseUa == 3000,
                 "nominal resistance admits exact current");

        adk::LowSideDriverDescriptor tolerant = descriptor ();
        tolerant.budget.baseResistanceTolerancePermille = 50;
        adk::BoundedLowSideDriverPolicy tolerantPolicy (tolerant);

        require (!adk::validateLowSideDriverDescriptor (tolerant).ok (),
                 "unsafe five-percent lower resistance rejected");
        require (tolerantPolicy.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "unsafe resistor tolerance blocks initialization");

        tolerant.budget.baseResistanceTolerancePermille = 1001;
        require (!adk::validateLowSideDriverDescriptor (tolerant).ok (),
                 "tolerance beyond 100 percent rejected");
    }

    void testArithmeticInvalidBoundaries ()
    {
        adk::LowSideDriverDescriptor invalid = descriptor ();
        invalid.budget.forcedGainNumerator   = 0;
        require (!adk::validateLowSideDriverDescriptor (invalid).ok (),
                 "zero gain numerator rejected");
        invalid                              = descriptor ();
        invalid.budget.forcedGainDenominator = 0;
        require (!adk::validateLowSideDriverDescriptor (invalid).ok (),
                 "zero gain denominator rejected");
        invalid                           = descriptor ();
        invalid.budget.baseResistanceOhms = 0;
        require (!adk::validateLowSideDriverDescriptor (invalid).ok (),
                 "zero resistance rejected");
        invalid                                      = descriptor ();
        invalid.budget.fixtureLoadCeilingUa          = 0;
        require (!adk::validateLowSideDriverDescriptor (invalid).ok (),
                 "zero fixture ceiling rejected");
        invalid                                      = descriptor ();

        invalid.budget.maximumActiveDuration         = adk::Duration (0);

        require (!adk::validateLowSideDriverDescriptor (invalid).ok (),
                 "zero maximum duration rejected");
        invalid                              = descriptor ();
        invalid.budget.maximumDutyPermille   = 1001;
        require (!adk::validateLowSideDriverDescriptor (invalid).ok (),
                 "duty above 1000 permille rejected");

        adk::LowSideDriverDescriptor overflow = descriptor ();
        overflow.budget.supplyLimitUa          = UINT32_MAX;
        overflow.budget.fixtureLoadCeilingUa   = UINT32_MAX;
        overflow.budget.deviceContinuousUa     = UINT32_MAX;
        overflow.budget.forcedGainNumerator    = 1;
        overflow.budget.forcedGainDenominator  = UINT32_MAX;
        adk::BoundedLowSideDriverPolicy policy (overflow);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "overflow descriptor initializes");

        require (policy.beginSession (7, 9).ok (), "overflow session begins");

        const adk::LowSideDriveIntent before = policy.snapshot ();
        const adk::LowSideDriveIntent canary = output;
        adk::LowSideDriveRequest       huge   = request (1, 100);
        huge.requestedLoadUa                   = UINT32_MAX;
        require (policy.apply (huge, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "unrepresentable required-base result rejects");
        requireUnchanged (policy, before, canary, output,
                          "arithmetic overflow rejection atomic");
    }

    void testWorstCaseBaseCurrentCeiling ()
    {
        adk::LowSideDriverDescriptor equal = descriptor ();
        equal.budget.baseResistanceOhms    = 1001;
        equal.budget.gpioSourceCeilingUa   = 4996;
        adk::BoundedLowSideDriverPolicy equalPolicy (equal);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (equalPolicy.initialize ().ok (),
                 "non-divisible maximum-current descriptor initializes");
        require (equalPolicy.beginSession (7, 9).ok (),
                 "maximum-current equality session");
        adk::LowSideDriveRequest bounded = request (1, 100);
        bounded.requestedLoadUa          = 20000;
        require (equalPolicy.apply (bounded, output).ok (),
                 "maximum-current equality evaluates");
        require (output.state == adk::LowSideDriveState::Requested,
                 "ceil maximum current equal to GPIO ceiling passes");

        adk::LowSideDriverDescriptor oneOver = equal;
        oneOver.budget.gpioSourceCeilingUa   = 4995;
        adk::BoundedLowSideDriverPolicy oneOverPolicy (oneOver);

        require (!adk::validateLowSideDriverDescriptor (oneOver).ok (),
                 "maximum-current one-over configuration rejected");
        require (oneOverPolicy.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "zero-Vbe high-voltage current blocks initialization");

        adk::LowSideDriverDescriptor tolerance = descriptor ();
        tolerance.budget.baseResistanceTolerancePermille = 1;
        tolerance.budget.gpioSourceCeilingUa              = 5000;
        adk::BoundedLowSideDriverPolicy tolerancePolicy (tolerance);

        require (!adk::validateLowSideDriverDescriptor (tolerance).ok (),
                 "lower-resistance maximum current rejected");
        require (tolerancePolicy.initialize ().error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "lower resistance and zero Vbe block initialization");
    }

    void testProducerPrecedence ()
    {
        adk::LowSideDriverDescriptor ineligible = descriptor ();
        ineligible.sourceEligible               = false;
        adk::BoundedLowSideDriverPolicy policy (ineligible);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "ineligible source initializes");

        require (policy.beginSession (7, 9).ok (), "ineligible source session");

        adk::LowSideDriveRequest failed = request (1, 100);
        failed.producerStatus            = adk::StatusCode::HardwareFailure;
        require (policy.apply (failed, output).ok (),
                 "producer failure is attributable domain evidence");
        require (output.state == adk::LowSideDriveState::Fault &&
                     output.reason == adk::LowSideDriveReason::ProducerFault &&
                     output.producerStatus ==
                         adk::StatusCode::HardwareFailure &&
                     !output.outputLevelHigh,
                 "producer fault precedes source ineligibility");

        require (policy.reset ().ok (), "producer policy resets");

        require (policy.beginSession (7, 9).ok (), "producer session restarts");

        failed                      = request (1, 200, false);
        failed.lifecycleGeneration  = 2;
        failed.producerStatus       = adk::StatusCode::HardwareFailure;
        require (policy.apply (failed, output).ok (),
                 "off producer fault is attributable");
        require (output.state == adk::LowSideDriveState::Fault &&
                     output.reason == adk::LowSideDriveReason::ProducerFault &&
                     !output.logicalActive && !output.outputLevelHigh,
                 "off producer fault remains canonical off intent");
    }

    void testSourceAndFlybackDomainPaths ()
    {
        adk::LowSideDriverDescriptor ineligible = descriptor ();
        ineligible.sourceEligible               = false;
        adk::BoundedLowSideDriverPolicy ineligiblePolicy (ineligible);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (ineligiblePolicy.initialize ().ok (),
                 "source-ineligible descriptor initializes");
        require (ineligiblePolicy.beginSession (7, 9).ok (),
                 "source-ineligible session begins");
        require (ineligiblePolicy.apply (request (1, 100), output).ok (),
                 "source ineligibility is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::SourceIneligible &&
                     !output.logicalActive && !output.outputLevelHigh,
                 "healthy ineligible source rejects off");

        adk::LowSideDriverDescriptor missing = inductiveDescriptor ();
        missing.flybackDeclaration           = adk::LowSideFlybackDeclaration::Absent;
        missing.flybackDiodeIdentity         = 0;
        missing.flybackDiodeRevision         = 0;
        missing.flybackOrientationCode       = 0;
        missing.flybackReturnCode            = 0;
        missing.flybackRepetitiveReverseMv   = 0;
        missing.flybackForwardCurrentUa      = 0;
        adk::BoundedLowSideDriverPolicy missingPolicy (missing);

        require (missingPolicy.initialize ().ok (),
                 "canonical absent inductive flyback initializes");
        require (missingPolicy.beginSession (7, 9).ok (),
                 "missing-flyback session begins");
        require (missingPolicy.apply (request (1, 100), output).ok (),
                 "missing flyback is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::FlybackMissing &&
                     !output.outputLevelHigh,
                 "absent required flyback rejects off");
    }

    void testDescriptorCrossProductsAndIdentityBounds ()
    {
        for (uint8_t energy = 0; energy < 2; ++energy)
        {
            for (uint8_t requirement = 0; requirement < 2; ++requirement)
            {
                for (uint8_t declaration = 0; declaration < 2; ++declaration)
                {
                    adk::LowSideDriverDescriptor value = descriptor ();
                    value.loadEnergy =
                        static_cast<adk::LowSideLoadEnergy> (energy);
                    value.flybackRequirement =
                        static_cast<adk::LowSideFlybackRequirement> (requirement);
                    value.flybackDeclaration =
                        static_cast<adk::LowSideFlybackDeclaration> (declaration);
                    if (declaration == 1)
                    {
                        const adk::LowSideDriverDescriptor complete =
                            inductiveDescriptor ();
                        value.flybackDiodeIdentity = complete.flybackDiodeIdentity;
                        value.flybackDiodeRevision = complete.flybackDiodeRevision;
                        value.flybackOrientationCode =
                            complete.flybackOrientationCode;
                        value.flybackReturnCode = complete.flybackReturnCode;
                        value.flybackRepetitiveReverseMv =
                            complete.flybackRepetitiveReverseMv;
                        value.flybackForwardCurrentUa =
                            complete.flybackForwardCurrentUa;
                    }
                    const bool valid =
                        (energy == 0 && requirement == 0 && declaration == 0) ||
                        (energy == 1 && requirement == 1);
                    require (adk::validateLowSideDriverDescriptor (value).ok () ==
                                 valid,
                             "load/flyback cross-product canonicality");
                }
            }
        }

#define REQUIRE_ZERO_DESCRIPTOR_FIELD(field)                                     \
    do                                                                            \
    {                                                                             \
        adk::LowSideDriverDescriptor value = descriptor ();                       \
        value.field                         = 0;                                   \
        require (!adk::validateLowSideDriverDescriptor (value).ok (),             \
                 "zero descriptor " #field " rejected");                         \
    } while (false)
        REQUIRE_ZERO_DESCRIPTOR_FIELD (schemaRevision);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (specimenFamilyReference);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (specimenReference);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (specimenRevision);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (electricalEvidenceRevision);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (sourcePacketDigest);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (configurationId);

        REQUIRE_ZERO_DESCRIPTOR_FIELD (configurationRevision);
#undef REQUIRE_ZERO_DESCRIPTOR_FIELD

        adk::LowSideDriverDescriptor maximum = descriptor ();
        maximum.schemaRevision               = UINT16_MAX;
        maximum.specimenFamilyReference      = UINT32_MAX;
        maximum.specimenReference            = UINT32_MAX;
        maximum.specimenRevision             = UINT16_MAX;
        maximum.electricalEvidenceRevision   = UINT16_MAX;
        maximum.sourcePacketDigest           = UINT32_MAX;
        maximum.configurationId              = UINT32_MAX;
        maximum.configurationRevision        = UINT16_MAX;
        require (adk::validateLowSideDriverDescriptor (maximum).ok (),
                 "maximum descriptor identities and revisions accepted");
    }

    void testExhaustiveEncodedValues ()
    {
        for (uint16_t encoded = 0; encoded <= 255; ++encoded)
        {
            adk::LowSideDriverDescriptor value = descriptor ();
            value.loadEnergy =
                static_cast<adk::LowSideLoadEnergy> (encoded);
            require (adk::validateLowSideDriverDescriptor (value).ok () ==
                         (encoded == 0),
                     "every load-energy encoding classified");

            value = descriptor ();
            value.flybackRequirement =
                static_cast<adk::LowSideFlybackRequirement> (encoded);
            require (adk::validateLowSideDriverDescriptor (value).ok () ==
                         (encoded == 0),
                     "every flyback-requirement encoding classified");

            value = descriptor ();
            value.flybackDeclaration =
                static_cast<adk::LowSideFlybackDeclaration> (encoded);
            require (adk::validateLowSideDriverDescriptor (value).ok () ==
                         (encoded == 0),
                     "every flyback-declaration encoding classified");
        }

        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "status-encoding policy initializes");

        require (policy.beginSession (7, 9).ok (),
                 "status-encoding session begins");
        require (policy.apply (request (1, 100), output).ok (),
                 "status-encoding active baseline");
        const adk::LowSideDriveIntent baseline = output;

        for (uint16_t encoded = 11; encoded <= 255; ++encoded)
        {
            adk::LowSideDriveRequest invalidRequest = request (2, 101, false);
            invalidRequest.producerStatus =
                static_cast<adk::StatusCode> (encoded);
            require (policy.apply (invalidRequest, output).error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid request status encoding rejected");
            requireUnchanged (policy, baseline, baseline, output,
                              "invalid request status is atomic");

            adk::LowSideControl invalidControl = control (2, 101);
            invalidControl.producerStatus =
                static_cast<adk::StatusCode> (encoded);
            require (policy.cancel (invalidControl, output).error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid control status encoding rejected");
            requireUnchanged (policy, baseline, baseline, output,
                              "invalid control status is atomic");
        }
    }

    void testDutyEqualityAndOneOver ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();

        value.budget.maximumActiveDuration = adk::Duration (100);

        value.budget.dutyWindow             = adk::Duration (100);
        value.budget.maximumDutyPermille    = 500;

        adk::BoundedLowSideDriverPolicy equalPolicy (value);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (equalPolicy.initialize ().ok (), "duty equality initializes");

        require (equalPolicy.beginSession (7, 9).ok (), "duty equality session");

        adk::LowSideDriveRequest equal = request (1, 100);
        equal.requestedLoadUa           = 20000;
        equal.requestedActiveDuration   = adk::Duration (50);

        require (equalPolicy.apply (equal, output).ok (), "duty equality evaluates");

        require (output.state == adk::LowSideDriveState::Requested,
                 "duty inequality equality passes");

        adk::BoundedLowSideDriverPolicy overPolicy (value);

        require (overPolicy.initialize ().ok (), "duty one-over initializes");

        require (overPolicy.beginSession (7, 9).ok (), "duty one-over session");

        equal.requestedActiveDuration = adk::Duration (51);

        require (overPolicy.apply (equal, output).ok (), "duty one-over evaluates");

        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::BudgetExceeded,
                 "one tick over duty allowance rejects");
    }

    void testDutyHistorySurvivesLifecycleBoundaries ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();

        value.budget.maximumActiveDuration = adk::Duration (50);

        value.budget.dutyWindow             = adk::Duration (100);
        value.budget.maximumDutyPermille    = 500;

        adk::BoundedLowSideDriverPolicy resetPolicy (value);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (resetPolicy.initialize ().ok (), "reset-duty policy initializes");

        require (resetPolicy.beginSession (7, 9).ok (),
                 "reset-duty session begins");
        adk::LowSideDriveRequest active = request (1, 100);
        active.requestedLoadUa          = 20000;
        active.requestedActiveDuration  = adk::Duration (50);

        require (resetPolicy.apply (active, output).ok (),
                 "reset-duty reservation admitted");
        require (output.state == adk::LowSideDriveState::Requested &&
                     output.expiresAt.milliseconds () == 150,
                 "full duty reservation recorded");
        adk::LowSideDriveRequest off = request (2, 150, false);

        require (resetPolicy.apply (off, output).ok (),
                 "exact-expiry off closes reservation");
        require (output.state == adk::LowSideDriveState::Off,
                 "off publishes canonical safe intent");

        require (resetPolicy.reset ().ok (), "reset removes drive authority");

        require (resetPolicy.beginSession (8, 10).ok (),
                 "new session begins after reset");
        adk::LowSideDriveRequest stale = request (3, 151, false);

        const adk::LowSideDriveIntent resetSnapshot = resetPolicy.snapshot ();
        adk::LowSideDriveIntent       resetCanary   = output;
        require (resetPolicy.apply (stale, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "pre-reset request and session replay rejected");
        requireUnchanged (resetPolicy, resetSnapshot, resetCanary, output,
                          "stale pre-reset replay is atomic");
        active                     = request (1, 249);
        active.lifecycleGeneration = 2;
        active.sessionId           = 8;
        active.runId               = 10;
        active.requestedLoadUa     = 20000;
        active.requestedActiveDuration = adk::Duration (50);

        require (resetPolicy.apply (active, output).ok (),
                 "pre-prune reset request is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::BudgetExceeded &&
                     !output.logicalActive && !output.outputLevelHigh,
                 "reset cannot bypass retained duty history");

        require (resetPolicy.reset ().ok (), "reset after duty rejection");

        require (resetPolicy.beginSession (9, 11).ok (),
                 "exact-prune reset session begins");
        active           = request (1, 250);
        active.lifecycleGeneration = 3;
        active.sessionId = 9;
        active.runId     = 11;
        active.requestedLoadUa = 20000;
        active.requestedActiveDuration = adk::Duration (50);

        require (resetPolicy.apply (active, output).ok (),
                 "exact pruning boundary evaluates");
        require (output.state == adk::LowSideDriveState::Requested &&
                     output.expiresAt.milliseconds () == 300,
                 "exact expiry prunes history and restores admission");

        adk::BoundedLowSideDriverPolicy shutdownPolicy (value);

        require (shutdownPolicy.initialize ().ok (),
                 "shutdown-duty policy initializes");
        require (shutdownPolicy.beginSession (7, 9).ok (),
                 "shutdown-duty session begins");
        active = request (1, 100);
        active.requestedLoadUa = 20000;
        active.requestedActiveDuration = adk::Duration (50);

        require (shutdownPolicy.apply (active, output).ok (),
                 "shutdown-duty reservation admitted");
        off = request (2, 150, false);

        require (shutdownPolicy.apply (off, output).ok (),
                 "shutdown-duty reservation closes");
        shutdownPolicy.shutdown ();

        require (shutdownPolicy.initialize ().ok (),
                 "shutdown-duty policy reinitializes");
        require (shutdownPolicy.beginSession (8, 10).ok (),
                 "post-shutdown session begins");
        active                     = request (1, 249);
        active.lifecycleGeneration = 2;
        active.sessionId           = 8;
        active.runId               = 10;
        active.requestedLoadUa     = 20000;
        active.requestedActiveDuration = adk::Duration (50);

        require (shutdownPolicy.apply (active, output).ok (),
                 "post-shutdown pre-prune request evaluates");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::BudgetExceeded &&
                     !output.outputLevelHigh,
                 "shutdown/reinitialize cannot bypass duty history");

        shutdownPolicy.shutdown ();

        require (shutdownPolicy.initialize ().ok (),
                 "shutdown-duty exact-prune reinitializes");
        require (shutdownPolicy.beginSession (9, 11).ok (),
                 "shutdown-duty exact-prune session begins");
        active                     = request (1, 250);
        active.lifecycleGeneration = 3;
        active.sessionId           = 9;
        active.runId               = 11;
        active.requestedLoadUa     = 20000;
        active.requestedActiveDuration = adk::Duration (50);

        require (shutdownPolicy.apply (active, output).ok (),
                 "shutdown exact pruning boundary evaluates");
        require (output.state == adk::LowSideDriveState::Requested &&
                     output.expiresAt.milliseconds () == 300,
                 "shutdown history prunes only at exact supplied time");
    }

    void testEarlyCancelShortensDutyHistory ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();

        value.budget.maximumActiveDuration = adk::Duration (50);

        value.budget.dutyWindow             = adk::Duration (100);
        value.budget.maximumDutyPermille    = 500;
        adk::BoundedLowSideDriverPolicy policy (value);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "early-cancel policy initializes");

        require (policy.beginSession (7, 9).ok (), "early-cancel session begins");

        adk::LowSideDriveRequest active = request (1, 100);
        active.requestedLoadUa          = 20000;
        active.requestedActiveDuration  = adk::Duration (50);

        require (policy.apply (active, output).ok (),
                 "early-cancel reservation admitted");
        require (policy.cancel (control (2, 110), output).ok (),
                 "early cancellation closes reservation");
        require (output.state == adk::LowSideDriveState::Cancelled &&
                     !output.outputLevelHigh,
                 "early cancellation publishes safe intent");

        require (policy.reset ().ok (), "early-cancel policy resets");

        require (policy.beginSession (8, 10).ok (),
                 "early-cancel follow-up session begins");
        active                              = request (1, 150);
        active.lifecycleGeneration          = 2;
        active.sessionId                    = 8;
        active.runId                        = 10;
        active.requestedLoadUa              = 20000;
        active.requestedActiveDuration      = adk::Duration (40);

        require (policy.apply (active, output).ok (),
                 "shortened reservation participates in duty equality");
        require (output.state == adk::LowSideDriveState::Requested &&
                     output.expiresAt.milliseconds () == 190,
                 "ten prior ticks plus forty candidate ticks passes equality");
    }

    void testSequenceRolloverAndControlCanaries ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "rollover policy initializes");

        require (policy.beginSession (7, 9).ok (), "rollover session begins");

        adk::LowSideDriveRequest maximum = request (UINT32_MAX, 100, false);

        require (policy.apply (maximum, output).ok (),
                 "maximum nonzero sequence accepted");
        const adk::LowSideDriveIntent terminal = output;

        adk::LowSideDriveRequest zero = request (1, 101, false);
        zero.sequence                 = 0;
        require (policy.apply (zero, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "reserved zero rollover sequence rejected");
        requireUnchanged (policy, terminal, terminal, output,
                          "zero rollover rejection atomic");

        adk::LowSideControl invalidControl = control (1, 101);
        invalidControl.sessionId           = 8;
        require (policy.cancel (invalidControl, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "control identity mismatch rejected");
        requireUnchanged (policy, terminal, terminal, output,
                          "control correlation rejection atomic");
    }

#if defined(ADK_TESTING)
    void testLifecycleGenerationExhaustion ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        const adk::LowSideDriveIntent   before = policy.snapshot ();

        policy.seedLifecycleGenerationForTest (UINT32_MAX);

        require (policy.initialize ().error () ==
                     adk::StatusCode::CapacityExceeded,
                 "lifecycle generation exhaustion rejects initialization");
        require (!policy.initialized (),
                 "generation exhaustion preserves inert lifecycle");
        require (equalIntent (before, policy.snapshot ()),
                 "generation exhaustion preserves complete snapshot");

        adk::LowSideDriverDescriptor value = descriptor ();

        value.budget.maximumActiveDuration = adk::Duration (50);

        value.budget.dutyWindow             = adk::Duration (100);
        value.budget.maximumDutyPermille    = 500;
        adk::BoundedLowSideDriverPolicy activePolicy (value);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (activePolicy.initialize ().ok (),
                 "active exhaustion policy initializes");
        require (activePolicy.beginSession (7, 9).ok (),
                 "active exhaustion session begins");
        adk::LowSideDriveRequest active = request (1, 100);
        active.requestedLoadUa          = 20000;
        active.requestedActiveDuration  = adk::Duration (50);

        require (activePolicy.apply (active, output).ok (),
                 "active exhaustion reservation admitted");
        require (activePolicy.apply (request (2, 150, false), output).ok (),
                 "active exhaustion reservation closes");
        activePolicy.seedLifecycleGenerationForTest (UINT32_MAX);

        const adk::LowSideDriveIntent terminal = activePolicy.snapshot ();

        require (activePolicy.reset ().error () ==
                     adk::StatusCode::CapacityExceeded,
                 "reset at maximum generation rejects");
        require (equalIntent (terminal, activePolicy.snapshot ()),
                 "reset exhaustion preserves complete snapshot");

        activePolicy.seedLifecycleGenerationForTest (UINT32_MAX - 1U);

        require (activePolicy.reset ().ok (),
                 "pre-maximum reset advances to maximum generation");
        require (activePolicy.snapshot ().lifecycleGeneration == UINT32_MAX,
                 "reset advances lifecycle generation exactly once");
        require (activePolicy.beginSession (8, 10).ok (),
                 "post-exhaustion proof session begins");
        active                   = request (1, 249);
        active.lifecycleGeneration = UINT32_MAX;
        active.sessionId         = 8;
        active.runId             = 10;
        active.requestedLoadUa   = 20000;
        active.requestedActiveDuration = adk::Duration (50);

        require (activePolicy.apply (active, output).ok (),
                 "retained history after exhaustion is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::BudgetExceeded,
                 "failed reset preserves duty history");
    }
#endif

    void testUpdateAdvancesChronology ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "chronology policy initializes");

        require (policy.beginSession (7, 9).ok (), "chronology session begins");

        require (policy.apply (request (1, 100), output).ok (),
                 "chronology active request accepted");
        require (policy.update (adk::TimePoint (250), output).ok (),
                 "later update expires and prunes");
        require (output.reason == adk::LowSideDriveReason::Expired,
                 "update publishes expiry");

        const adk::LowSideDriveIntent before = policy.snapshot ();
        const adk::LowSideDriveIntent canary = output;
        adk::LowSideDriveRequest      stale  = request (2, 249, false);

        require (policy.apply (stale, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "apply before accepted update time rejects");
        requireUnchanged (policy, before, canary, output,
                          "post-update chronology rejection atomic");
    }

    void testRequestTimestampDiscontinuities ()
    {
        for (uint8_t halfRangeCase = 0; halfRangeCase < 2; ++halfRangeCase)
        {
            adk::BoundedLowSideDriverPolicy policy (descriptor ());

            adk::LowSideDriveIntent         output = blankIntent ();

            require (policy.initialize ().ok (), "timestamp policy initializes");

            require (policy.beginSession (7, 9).ok (), "timestamp session begins");

            require (policy.apply (request (1, 100, false), output).ok (),
                     "timestamp baseline accepted");

            const uint32_t observedAt =
                halfRangeCase == 0 ? 99U : 0x80000064UL;
            require (policy.apply (request (2, observedAt, false), output).ok (),
                     "timestamp discontinuity is domain evidence");
            require (output.state == adk::LowSideDriveState::Rejected &&
                         output.reason ==
                             adk::LowSideDriveReason::TimestampDiscontinuity &&
                         !output.outputLevelHigh,
                     "backward and half-range timestamps reject off");
        }
    }

    void testRejectedTimestampsPreserveChronologyFloor ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "chronology-floor policy initializes");
        require (policy.beginSession (7, 9).ok (),
                 "chronology-floor session begins");
        require (policy.apply (request (1, 100, false), output).ok (),
                 "chronology-floor baseline accepted");
        require (policy.apply (request (2, 90, false), output).ok (),
                 "backdated request remains domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason ==
                         adk::LowSideDriveReason::TimestampDiscontinuity,
                 "first backdated request rejects");
        require (policy.apply (request (3, 95, false), output).ok (),
                 "follow-up below original floor remains domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason ==
                         adk::LowSideDriveReason::TimestampDiscontinuity,
                 "rejection cannot lower chronology floor");
    }

    void testSequenceGapBackdatedCollisionPreservesChronologyFloor ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "collision policy initializes");
        require (policy.beginSession (7, 9).ok (), "collision session begins");
        require (policy.apply (request (1, 100, false), output).ok (),
                 "collision baseline accepted");
        require (policy.apply (request (3, 90, false), output).ok (),
                 "sequence-gap and backdated collision is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason ==
                         adk::LowSideDriveReason::SequenceDiscontinuity,
                 "sequence discontinuity wins collision precedence");
        require (policy.apply (request (4, 95, false), output).ok (),
                 "collision follow-up remains domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason ==
                         adk::LowSideDriveReason::TimestampDiscontinuity,
                 "sequence collision cannot lower chronology floor");
    }

    void testLateOffAndCancelCannotExtendReservations ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();

        value.budget.dutyWindow          = adk::Duration (1000);
        value.budget.maximumDutyPermille = 200;

        for (uint8_t closeWithCancel = 0; closeWithCancel < 2; ++closeWithCancel)
        {
            adk::BoundedLowSideDriverPolicy policy (value);

            adk::LowSideDriveIntent         output = blankIntent ();

            require (policy.initialize ().ok (), "late-close policy initializes");
            require (policy.beginSession (7, 9).ok (),
                     "late-close session begins");
            require (policy.apply (request (1, 100), output).ok (),
                     "late-close reservation admitted");

            if (closeWithCancel != 0)
            {
                require (policy.cancel (control (2, 250), output).ok (),
                         "late cancel accepted");
            }
            else
            {
                require (policy.apply (request (2, 250, false), output).ok (),
                         "late off accepted");
            }

            require (!output.outputLevelHigh, "late close publishes safe intent");
            require (policy.apply (request (3, 251), output).ok (),
                     "post-close equality request is domain evidence");
            require (output.state == adk::LowSideDriveState::Requested &&
                         output.outputLevelHigh,
                     "late close cannot extend reservation beyond original deadline");
        }
    }

    void testOffCancelFaultAndLifecycle ()
    {
        adk::BoundedLowSideDriverPolicy policy (descriptor ());

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "lifecycle initialize");

        require (policy.beginSession (7, 9).ok (), "lifecycle session");

        require (policy.apply (request (1, 100), output).ok (), "lifecycle active");

        adk::LowSideControl stop = control (2, 120);

        require (policy.cancel (stop, output).ok (), "cancel accepted");

        require (output.state == adk::LowSideDriveState::Cancelled &&
                     output.reason == adk::LowSideDriveReason::Cancelled &&
                     !output.outputLevelHigh,
                 "confirmed cancel wins and forces off");
        const adk::LowSideDriveIntent cancelled = output;
        require (policy.cancel (stop, output).ok (),
                 "identical cancel duplicate is idempotent");
        require (equalIntent (cancelled, output),
                 "identical cancel duplicate preserves result");

        adk::LowSideControl changedStop = stop;
        changedStop.offConfirmed        = false;
        require (policy.cancel (changedStop, output).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed cancel duplicate rejected");
        requireUnchanged (policy, cancelled, cancelled, output,
                          "changed cancel duplicate atomic");

        require (policy.reset ().ok (), "reset after cancel");

        require (policy.beginSession (7, 9).ok (), "restart after cancel");

        adk::LowSideDriveRequest restarted = request (1, 200);
        restarted.lifecycleGeneration      = 2;
        require (policy.apply (restarted, output).ok (),
                 "active before faulted cancel");
        stop = control (2, 220, false, adk::StatusCode::HardwareFailure);
        stop.lifecycleGeneration = 2;

        require (policy.cancel (stop, output).ok (),
                 "faulted cancel remains domain evidence");
        require (output.state == adk::LowSideDriveState::Fault &&
                     output.reason == adk::LowSideDriveReason::Cancelled &&
                     output.producerStatus == adk::StatusCode::HardwareFailure &&
                     !output.outputLevelHigh,
                 "failed off confirmation dominates cancellation state");

        require (policy.reset ().ok (), "reset after off-confirmation fault");

        require (policy.beginSession (8, 10).ok (),
                 "producer-collision session begins");
        adk::LowSideDriveRequest collision = request (1, 300);
        collision.lifecycleGeneration      = 3;
        collision.sessionId                = 8;
        collision.runId                    = 10;
        require (policy.apply (collision, output).ok (),
                 "active before producer-cancel collision");
        stop           = control (2, 320, true, adk::StatusCode::HardwareFailure);
        stop.lifecycleGeneration = 3;
        stop.sessionId = 8;
        stop.runId     = 10;
        require (policy.cancel (stop, output).ok (),
                 "cancel producer collision is domain evidence");
        require (output.state == adk::LowSideDriveState::Fault &&
                     output.reason == adk::LowSideDriveReason::Cancelled &&
                     output.producerStatus ==
                         adk::StatusCode::HardwareFailure &&
                     !output.outputLevelHigh,
                 "producer failure prevents confirmed cancel from hiding fault");

        policy.shutdown ();

        require (!policy.initialized (), "shutdown leaves uninitialized");

        require (policy.snapshot ().state == adk::LowSideDriveState::Shutdown &&
                     !policy.snapshot ().outputLevelHigh,
                 "shutdown publishes canonical off");
        policy.shutdown ();

        require (policy.snapshot ().state == adk::LowSideDriveState::Shutdown,
                 "shutdown idempotent");
        require (policy.initialize ().ok (), "restart after shutdown");

        require (policy.snapshot ().lifecycleGeneration == 4,
                 "restart increments lifecycle generation");
    }

    void testDutyHistoryAndRollover ()
    {
        adk::LowSideDriverDescriptor value = descriptor ();

        value.budget.maximumActiveDuration = adk::Duration (10);

        value.budget.dutyWindow            = adk::Duration (100);
        value.budget.maximumDutyPermille   = 1000;
        adk::BoundedLowSideDriverPolicy policy (value);

        adk::LowSideDriveIntent         output = blankIntent ();

        require (policy.initialize ().ok (), "history initializes");

        require (policy.beginSession (7, 9).ok (), "history session");

        for (uint32_t index = 0; index < 8; ++index)
        {
            adk::LowSideDriveRequest active =
                request (index * 2 + 1, 0xFFFFFF80UL + index * 10);
            active.requestedActiveDuration = adk::Duration (5);

            require (policy.apply (active, output).ok (), "history active admission");
            adk::LowSideDriveRequest off =
                request (index * 2 + 2, 0xFFFFFF83UL + index * 10, false);
            require (policy.apply (off, output).ok (), "early off closes reservation");
        }

        adk::LowSideDriveRequest ninth = request (17, 0xFFFFFFD0UL);

        ninth.requestedActiveDuration  = adk::Duration (5);

        require (policy.apply (ninth, output).ok (),
                 "ninth overlapping interval is domain evidence");
        require (output.state == adk::LowSideDriveState::Rejected &&
                     output.reason == adk::LowSideDriveReason::CapacityExceeded &&
                     !output.outputLevelHigh,
                 "ninth interval exhausts fixed history safely");

        require (policy.update (adk::TimePoint (0x00000020UL), output).ok (),
                 "ordinary modular rollover update accepted");
        require (!output.outputLevelHigh, "rollover cannot reactivate output");

        adk::LowSideDriveRequest afterPrune = request (18, 0x00000021UL);

        afterPrune.requestedActiveDuration  = adk::Duration (5);

        require (policy.apply (afterPrune, output).ok (),
                 "history accepts after rollover pruning");
        require (output.state == adk::LowSideDriveState::Requested,
                 "expired reservations release fixed capacity");
    }

} // namespace

int main ()
{
    testDescriptorAndDigest ();

    testArithmeticAndDeadline ();

    testIdentitySequenceAndAtomicity ();

    testResistanceToleranceBounds ();

    testArithmeticInvalidBoundaries ();

    testWorstCaseBaseCurrentCeiling ();

    testProducerPrecedence ();

    testSourceAndFlybackDomainPaths ();

    testDescriptorCrossProductsAndIdentityBounds ();

    testExhaustiveEncodedValues ();

    testDutyEqualityAndOneOver ();

    testDutyHistorySurvivesLifecycleBoundaries ();

    testEarlyCancelShortensDutyHistory ();

    testSequenceRolloverAndControlCanaries ();

#if defined(ADK_TESTING)
    testLifecycleGenerationExhaustion ();
#endif

    testUpdateAdvancesChronology ();

    testRequestTimestampDiscontinuities ();

    testRejectedTimestampsPreserveChronologyFloor ();

    testSequenceGapBackdatedCollisionPreservesChronologyFloor ();

    testLateOffAndCancelCannotExtendReservations ();

    testOffCancelFaultAndLifecycle ();

    testDutyHistoryAndRollover ();
    return EXIT_SUCCESS;
}
