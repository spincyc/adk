#include <inertial_observation.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>
#include <type_traits>

// clang-format off
namespace {

    void require (bool condition, const char* message)

    {

        if (!condition)

        {
            std::cerr << "FAIL: " << message << '\n';

            std::exit (EXIT_FAILURE);

        }
    }


    adk::InertialObservationConfig config (
        uint32_t maximumAge = 10, uint16_t freshnessRevision = 3)
    {

        return {adk::Duration (maximumAge), freshnessRevision};
    }


    adk::InertialSource source (
        adk::InertialSourceKind kind = adk::InertialSourceKind::SyntheticFixture,
        adk::InertialModel model = adk::InertialModel::Synthetic,
        uint8_t sourceId = 7, uint16_t configurationRevision = 11,
        uint16_t calibrationRevision = 13,
        uint32_t accelerationRange = 2000000,
        uint32_t angularRateRange = 250000000)
    {
        return {kind, model, sourceId, configurationRevision,
                calibrationRevision, accelerationRange, angularRateRange};
    }


    adk::InertialSample sample (
        uint32_t observedAt = 100, uint32_t sequence = 1,
        adk::Status status = adk::StatusCode::Ok)
    {

        return {source (),
                {0, 0, 1000000},
                {0, 0, 0},

                adk::TimePoint (observedAt),
                sequence,
                true,
                adk::InertialSaturation::None,
                status};
    }


    void assignSampleFields (adk::InertialSample& destination,
                             const adk::InertialSample& value)
    {
        destination.source.kind = value.source.kind;
        destination.source.model = value.source.model;
        destination.source.sourceId = value.source.sourceId;
        destination.source.configurationRevision =
            value.source.configurationRevision;
        destination.source.calibrationRevision =
            value.source.calibrationRevision;
        destination.source.accelerationRangeMicroG =
            value.source.accelerationRangeMicroG;
        destination.source.angularRateRangeMilliDegreesPerSecond =
            value.source.angularRateRangeMilliDegreesPerSecond;
        destination.accelerationMicroG.x = value.accelerationMicroG.x;
        destination.accelerationMicroG.y = value.accelerationMicroG.y;
        destination.accelerationMicroG.z = value.accelerationMicroG.z;
        destination.angularRateMilliDegreesPerSecond.x =
            value.angularRateMilliDegreesPerSecond.x;
        destination.angularRateMilliDegreesPerSecond.y =
            value.angularRateMilliDegreesPerSecond.y;
        destination.angularRateMilliDegreesPerSecond.z =
            value.angularRateMilliDegreesPerSecond.z;
        destination.observedAt = value.observedAt;
        destination.sequence = value.sequence;
        destination.dataReady = value.dataReady;
        destination.saturation = value.saturation;
        destination.status = value.status;
    }


    bool sourceEqual (const adk::InertialSource& left,
                      const adk::InertialSource& right)
    {
        return left.kind == right.kind && left.model == right.model &&
               left.sourceId == right.sourceId &&
               left.configurationRevision == right.configurationRevision &&
               left.calibrationRevision == right.calibrationRevision &&
               left.accelerationRangeMicroG ==
                   right.accelerationRangeMicroG &&
               left.angularRateRangeMilliDegreesPerSecond ==
                   right.angularRateRangeMilliDegreesPerSecond;
    }


    bool vectorEqual (const adk::InertialVector& left,
                      const adk::InertialVector& right)
    {
        return left.x == right.x && left.y == right.y && left.z == right.z;
    }


    bool sampleEqual (const adk::InertialSample& left,
                      const adk::InertialSample& right)
    {

        return sourceEqual (left.source, right.source) &&

               vectorEqual (left.accelerationMicroG,
                            right.accelerationMicroG) &&

               vectorEqual (left.angularRateMilliDegreesPerSecond,
                            right.angularRateMilliDegreesPerSecond) &&
               left.observedAt == right.observedAt &&
               left.sequence == right.sequence &&
               left.dataReady == right.dataReady &&
               left.saturation == right.saturation &&
               left.status == right.status;
    }


    bool observationEqual (const adk::InertialObservation& left,
                           const adk::InertialObservation& right)
    {

        return sampleEqual (left.sample, right.sample) &&
               left.quality == right.quality && left.age == right.age &&
               left.maximumAge == right.maximumAge &&
               left.freshnessContractRevision ==
                   right.freshnessContractRevision &&
               left.sequenceGap == right.sequenceGap &&
               left.latestDataReady == right.latestDataReady &&
               left.status == right.status;
    }


    void requireCanonicalInvalid (const adk::InertialObservation& observation,
                                  const char* message)
    {

        require (observation.quality == adk::InertialSampleQuality::Invalid &&

                     observation.age == adk::Duration (0) &&
                     observation.sequenceGap == 0 &&
                     !observation.latestDataReady &&

                     observation.status.error () ==
                         adk::StatusCode::NotInitialized,
                 message);
    }


    void testLifecycleAndConfiguration ()
    {

        static_assert (
            !std::is_copy_constructible<adk::InertialObservationPolicy>::value,
            "inertial policy must not copy");

        static_assert (
            !std::is_move_constructible<adk::InertialObservationPolicy>::value,
            "inertial policy must not move");

        static_assert (
            std::is_trivially_destructible<adk::InertialObservationPolicy>::value,
            "inertial policy destruction must be side-effect free");


        adk::InertialObservationPolicy policy (config ());


        require (!policy.initialized (), "policy starts inert");

        requireCanonicalInvalid (policy.snapshot (),
                                 "construction is canonical invalid");

        require (policy.update (adk::TimePoint (100), sample ()).error () ==
                     adk::StatusCode::NotInitialized,
                 "update before initialize rejected");

        require (policy.initialize ().ok (), "valid policy initializes");

        require (policy.initialize ().ok (), "initialize is idempotent");

        require (policy.initialized (), "initialized state visible");


        adk::InertialObservation initial = policy.snapshot ();


        require (initial.maximumAge == adk::Duration (10) &&
                     initial.freshnessContractRevision == 3,
                 "initialized snapshot preserves freshness contract");


        require (policy.update (adk::TimePoint (100), sample ()).ok (),
                 "initialized policy accepts sample");

        policy.reset ();

        require (policy.initialized (),
                 "reset preserves valid initialized configuration");

        requireCanonicalInvalid (policy.snapshot (),
                                 "reset returns canonical invalid snapshot");

        require (policy.snapshot ().maximumAge == adk::Duration (10) &&

                     policy.snapshot ().freshnessContractRevision == 3,
                 "reset preserves configured freshness evidence");

        require (policy.initialize ().ok (),
                 "initialize remains idempotent after reset");

        require (policy.update (adk::TimePoint (100), sample ()).ok (),
                 "reset clears sequence and policy-time history");

        adk::InertialObservationPolicy largestValid (
            config (0x7fffffffUL, 1));

        require (largestValid.initialize ().ok (),
                 "largest wrap-safe maximum age initializes");

        const adk::InertialObservationConfig invalidConfigs[] = {

            config (0, 3), config (10, 0), config (0x80000000UL, 3),

            config (0xffffffffUL, 3)};


        for (const auto& invalid : invalidConfigs)

        {

            adk::InertialObservationPolicy rejected (invalid);


            require (rejected.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid freshness configuration rejected");

            require (!rejected.initialized (),
                     "invalid configuration leaves policy inert");
        }
    }


    void testE0AllowListAndProvenanceValidation ()
    {
        const struct {
            adk::InertialSourceKind kind;
            adk::InertialModel      model;
            adk::StatusCode         expected;
        } cases[] = {
            {adk::InertialSourceKind::SyntheticFixture,
             adk::InertialModel::Synthetic, adk::StatusCode::Ok},
            {adk::InertialSourceKind::Mpu6050Adapter,
             adk::InertialModel::Mpu6050, adk::StatusCode::Unsupported},
            {adk::InertialSourceKind::Qmi8658Adapter,
             adk::InertialModel::Qmi8658UnknownRevision,
             adk::StatusCode::Unsupported},
            {adk::InertialSourceKind::SyntheticFixture,
             adk::InertialModel::Mpu6050, adk::StatusCode::InvalidArgument},
            {adk::InertialSourceKind::SyntheticFixture,
             adk::InertialModel::Qmi8658UnknownRevision,
             adk::StatusCode::InvalidArgument},
            {adk::InertialSourceKind::Mpu6050Adapter,
             adk::InertialModel::Synthetic, adk::StatusCode::InvalidArgument},
            {adk::InertialSourceKind::Qmi8658Adapter,
             adk::InertialModel::Synthetic, adk::StatusCode::InvalidArgument},
            {static_cast<adk::InertialSourceKind> (0xff),
             adk::InertialModel::Synthetic, adk::StatusCode::InvalidArgument},
            {adk::InertialSourceKind::SyntheticFixture,
             static_cast<adk::InertialModel> (0xff),
             adk::StatusCode::InvalidArgument}};


        for (const auto& fixture : cases)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample ();


            require (policy.initialize ().ok (), "allow-list policy initializes");
            input.source.kind  = fixture.kind;
            input.source.model = fixture.model;


            require (policy.update (adk::TimePoint (100), input).error () ==
                         fixture.expected,
                     "kind/model allow-list result exact");

            require (policy.snapshot ().status.error () == fixture.expected,
                     "kind/model result copied to snapshot");
        }


        using Mutator = void (*) (adk::InertialSample&);
        const Mutator invalid[] = {
            [] (adk::InertialSample& value) { value.source.sourceId = 0; },
            [] (adk::InertialSample& value) {
                value.source.configurationRevision = 0;
            },
            [] (adk::InertialSample& value) {
                value.source.calibrationRevision = 0;
            },
            [] (adk::InertialSample& value) {
                value.source.accelerationRangeMicroG = 0;
            },
            [] (adk::InertialSample& value) {
                value.source.angularRateRangeMilliDegreesPerSecond = 0;
            },
            [] (adk::InertialSample& value) {
                value.source.accelerationRangeMicroG = 0x80000000UL;
            },
            [] (adk::InertialSample& value) {
                value.source.angularRateRangeMilliDegreesPerSecond =
                    0x80000000UL;
            },
            [] (adk::InertialSample& value) {
                value.saturation =
                    static_cast<adk::InertialSaturation> (0xff);
            }};


        for (Mutator mutate : invalid)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample ();


            require (policy.initialize ().ok (), "validation policy initializes");

            mutate (input);

            require (policy.update (adk::TimePoint (100), input).error () ==
                         adk::StatusCode::InvalidArgument,
                     "invalid provenance or saturation rejected");
        }
    }


    void testAxesAndSaturation ()
    {
        const int32_t values[] = {0, 1, -1, 99, -99};


        for (int32_t value : values)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample ();

            input.source.accelerationRangeMicroG = 100;
            input.accelerationMicroG              = {value, value, value};

            require (policy.initialize ().ok (), "value policy initializes");

            require (policy.update (adk::TimePoint (100), input).ok (),
                     "in-range signed axes accepted");

            require (policy.snapshot ().quality ==
                         adk::InertialSampleQuality::Current,
                     "interior axes remain current");
        }

        const int32_t invalidValues[] = {

            101, -101, std::numeric_limits<int32_t>::min ()};


        for (int32_t value : invalidValues)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample ();

            input.source.accelerationRangeMicroG = 100;
            input.accelerationMicroG.x            = value;

            require (policy.initialize ().ok (), "range policy initializes");

            require (policy.update (adk::TimePoint (100), input).error () ==
                         adk::StatusCode::InvalidArgument,
                     "out-of-range axis rejected without signed abs overflow");

            adk::InertialObservationPolicy ratePolicy (config ());

            adk::InertialSample            rateInput = sample ();

            rateInput.source.angularRateRangeMilliDegreesPerSecond = 100;
            rateInput.angularRateMilliDegreesPerSecond.x = value;

            require (ratePolicy.initialize ().ok (),
                     "angular-rate range policy initializes");

            require (
                ratePolicy.update (adk::TimePoint (100), rateInput).error () ==
                    adk::StatusCode::InvalidArgument,
                "out-of-range angular-rate axis rejected safely");
        }

        const struct {
            adk::InertialVector     acceleration;
            adk::InertialVector     rate;
            adk::InertialSaturation saturation;
        } saturated[] = {
            {{100, 0, 0}, {0, 0, 0},
             adk::InertialSaturation::Acceleration},
            {{-100, 0, 0}, {0, 0, 0},
             adk::InertialSaturation::Acceleration},
            {{0, 0, 0}, {200, 0, 0},
             adk::InertialSaturation::AngularRate},
            {{0, 0, 0}, {-200, 0, 0},
             adk::InertialSaturation::AngularRate},
            {{0, 100, 0}, {0, 0, -200}, adk::InertialSaturation::Both}};


        for (const auto& fixture : saturated)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample (100);

            input.source.accelerationRangeMicroG = 100;
            input.source.angularRateRangeMilliDegreesPerSecond = 200;
            input.accelerationMicroG = fixture.acceleration;
            input.angularRateMilliDegreesPerSecond = fixture.rate;
            input.saturation = fixture.saturation;

            require (policy.initialize ().ok (), "saturation policy initializes");

            require (policy.update (adk::TimePoint (111), input).ok (),
                     "agreed saturation accepted");

            require (policy.snapshot ().quality ==
                         adk::InertialSampleQuality::Saturated &&

                         policy.snapshot ().status.ok (),
                     "saturation dominates stale with OK status");
        }

        const struct {
            adk::InertialVector     acceleration;
            adk::InertialVector     rate;
            adk::InertialSaturation saturation;
        } disagreement[] = {
            {{100, 0, 0}, {0, 0, 0}, adk::InertialSaturation::None},
            {{0, 0, 0}, {200, 0, 0}, adk::InertialSaturation::None},
            {{99, 0, 0}, {0, 0, 0},
             adk::InertialSaturation::Acceleration},
            {{100, 0, 0}, {0, 0, 0}, adk::InertialSaturation::Both}};


        for (const auto& fixture : disagreement)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample ();

            input.source.accelerationRangeMicroG = 100;
            input.source.angularRateRangeMilliDegreesPerSecond = 200;
            input.accelerationMicroG = fixture.acceleration;
            input.angularRateMilliDegreesPerSecond = fixture.rate;
            input.saturation = fixture.saturation;

            require (policy.initialize ().ok (), "agreement policy initializes");

            require (policy.update (adk::TimePoint (100), input).error () ==
                         adk::StatusCode::InvalidArgument,
                     "saturation declaration must exactly agree with axes");
        }
    }


    void testEveryProducerStatusAndPrecedence ()
    {
        const adk::StatusCode codes[] = {
            adk::StatusCode::Ok,
            adk::StatusCode::InvalidArgument,
            adk::StatusCode::InvalidConfiguration,
            adk::StatusCode::InvalidPin,
            adk::StatusCode::Unsupported,
            adk::StatusCode::ResourceBusy,
            adk::StatusCode::NotInitialized,
            adk::StatusCode::CapacityExceeded,
            adk::StatusCode::Timeout,
            adk::StatusCode::InternalInvariant,
            adk::StatusCode::HardwareFailure};


        for (adk::StatusCode code : codes)

        {

            adk::InertialObservationPolicy policy (config ());

            adk::InertialSample            input = sample (100, 1, code);


            require (policy.initialize ().ok (), "status policy initializes");
            input.source.sourceId = 0;
            input.accelerationMicroG.x =

                std::numeric_limits<int32_t>::min ();
            input.saturation = adk::InertialSaturation::Both;

            const adk::Status result =

                policy.update (adk::TimePoint (100), input);


            if (code == adk::StatusCode::Ok)

            {

                require (result.error () == adk::StatusCode::InvalidArgument,
                         "structural invalidity reported for OK producer");
            }
            else

            {

                require (result.error () == code &&

                             policy.snapshot ().status.error () == code &&

                             policy.snapshot ().quality ==
                                 adk::InertialSampleQuality::Invalid,
                         "producer failure dominates untrusted numeric payload");
            }
        }

        adk::InertialObservationPolicy invalidStatus (config ());

        adk::InertialSample            invalid = sample ();

        invalid.status = static_cast<adk::StatusCode> (0xff);

        require (invalidStatus.initialize ().ok (),
                 "invalid-status policy initializes");

        require (invalidStatus.update (adk::TimePoint (100), invalid).error () ==
                     adk::StatusCode::InvalidArgument,
                 "unrecognized producer status rejected exactly");
    }


    void testFreshnessTimeAndReplayAging ()
    {

        adk::InertialObservationPolicy policy (config (10));

        adk::InertialSample            input = sample (100, 1);


        require (policy.initialize ().ok (), "freshness policy initializes");

        require (policy.update (adk::TimePoint (100), input).ok (),
                 "zero-age sample accepted");

        require (policy.snapshot ().quality ==
                     adk::InertialSampleQuality::Current &&

                     policy.snapshot ().age == adk::Duration (0),
                 "zero age is current");

        require (policy.snapshot ().latestDataReady,
                 "accepted ready sample records latest readiness");

        require (policy.update (adk::TimePoint (110), input).ok (),
                 "semantic replay at freshness boundary accepted");

        require (policy.snapshot ().quality ==
                     adk::InertialSampleQuality::Current &&

                     policy.snapshot ().age == adk::Duration (10) &&

                     policy.snapshot ().sequenceGap == 0 &&

                     policy.snapshot ().latestDataReady,
                 "maximum age is inclusive and replay emits no gap");

        require (policy.update (adk::TimePoint (111), input).ok (),
                 "same sample can age without a new sequence");

        require (policy.snapshot ().quality ==
                     adk::InertialSampleQuality::Stale &&

                     policy.snapshot ().age == adk::Duration (11),
                 "same-sample replay becomes stale one tick over");

        adk::InertialSample conflict = input;

        conflict.accelerationMicroG.x = 1;

        require (policy.update (adk::TimePoint (112), conflict).error () ==
                     adk::StatusCode::InvalidArgument,
                 "changed same-sequence payload rejected");

        require (policy.snapshot ().age == adk::Duration (11),
                 "invalid conflicting replay preserves prior snapshot");

        require (policy.update (adk::TimePoint (99), input).error () ==
                     adk::StatusCode::InvalidArgument,
                 "future sample and regressed policy time rejected");


        adk::InertialObservationPolicy futurePolicy (config (10));

        adk::InertialSample            future = sample (101, 1);


        require (futurePolicy.initialize ().ok (), "future policy initializes");

        require (futurePolicy.update (adk::TimePoint (100), future).error () ==
                     adk::StatusCode::InvalidArgument,
                 "sample one tick in the future rejected");


        adk::InertialObservationPolicy timeHalfRange (config (10));


        require (timeHalfRange.initialize ().ok (),
                 "time half-range policy initializes");

        require (timeHalfRange.update (adk::TimePoint (100), sample (100)).ok (),
                 "time half-range base accepted");

        require (timeHalfRange

                     .update (adk::TimePoint (100 + 0x80000000UL), sample (100))

                     .error () == adk::StatusCode::InvalidArgument,
                 "ambiguous policy-time half range rejected");


        adk::InertialObservationPolicy wrapped (config (10));
        adk::InertialSample            nearWrap =

            sample (0xfffffffcUL, 0xfffffffeUL);


        require (wrapped.initialize ().ok (), "wrap policy initializes");

        require (wrapped.update (adk::TimePoint (3), nearWrap).ok (),
                 "sample age crosses TimePoint wrap");

        require (wrapped.snapshot ().age == adk::Duration (7) &&

                     wrapped.snapshot ().quality ==
                         adk::InertialSampleQuality::Current,
                 "wrapped age computed exactly");

        require (wrapped.update (adk::TimePoint (4), nearWrap).ok (),
                 "wrapped exact replay advances policy time");


        adk::InertialSample wrappedSequence = sample (4, 1);


        require (wrapped.update (adk::TimePoint (4), wrappedSequence).ok (),
                 "sequence wraps forward through zero");

        require (wrapped.snapshot ().sequenceGap == 2,
                 "wrapped sequence gap is explicit");
    }


    void testSequenceDomainsAndBoundaries ()
    {

        adk::InertialObservationPolicy policy (config (100));

        adk::InertialSample            first = sample (100, 20);


        require (policy.initialize ().ok (), "sequence policy initializes");

        require (policy.update (adk::TimePoint (100), first).ok (),
                 "first sequence accepted");


        adk::InertialSample next = sample (101, 21);


        require (policy.update (adk::TimePoint (101), next).ok (),
                 "delta one accepted");

        require (policy.snapshot ().sequenceGap == 0,
                 "delta one has no gap");


        next = sample (102, 25);

        require (policy.update (adk::TimePoint (102), next).ok (),
                 "forward gap accepted");

        require (policy.snapshot ().sequenceGap == 3,
                 "forward gap counted exactly");


        next = sample (103, 25 + 0x7fffffffUL);

        require (policy.update (adk::TimePoint (103), next).ok (),
                 "largest forward delta accepted");

        require (policy.snapshot ().sequenceGap == 0x7ffffffeUL,
                 "largest forward gap counted");

        adk::InertialSample ambiguous = next;

        ambiguous.sequence += 0x80000000UL;

        ambiguous.observedAt = adk::TimePoint (104);

        require (policy.update (adk::TimePoint (104), ambiguous).error () ==
                     adk::StatusCode::InvalidArgument,
                 "ambiguous sequence half range rejected");

        adk::InertialSample regression = next;

        --regression.sequence;

        regression.observedAt = adk::TimePoint (104);

        require (policy.update (adk::TimePoint (104), regression).error () ==
                     adk::StatusCode::InvalidArgument,
                 "sequence regression rejected");


        using DomainMutator = void (*) (adk::InertialSample&);
        const DomainMutator domains[] = {
            [] (adk::InertialSample& value) { ++value.source.sourceId; },
            [] (adk::InertialSample& value) {
                ++value.source.configurationRevision;
            },
            [] (adk::InertialSample& value) {
                ++value.source.calibrationRevision;
            },
            [] (adk::InertialSample& value) {
                ++value.source.accelerationRangeMicroG;
            },
            [] (adk::InertialSample& value) {
                ++value.source.angularRateRangeMilliDegreesPerSecond;
            }};


        for (DomainMutator mutate : domains)

        {

            adk::InertialObservationPolicy domainPolicy (config (100));

            adk::InertialSample            base = sample (200, 100);


            require (domainPolicy.initialize ().ok (),
                     "domain policy initializes");

            require (domainPolicy.update (adk::TimePoint (200), base).ok (),
                     "domain base accepted");

            mutate (base);

            base.observedAt = adk::TimePoint (201);
            base.sequence   = 1;

            require (domainPolicy.update (adk::TimePoint (201), base).ok (),
                     "identity change starts a fresh sequence domain");

            require (domainPolicy.snapshot ().sequenceGap == 0,
                     "new domain has no inherited gap");
        }
    }


    void testDataReadyAndRecovery ()
    {

        adk::InertialObservationPolicy policy (config (10));

        adk::InertialSample            current = sample (100, 1);


        require (policy.initialize ().ok (), "recovery policy initializes");

        require (policy.update (adk::TimePoint (100), current).ok (),
                 "recovery base accepted");

        require (policy.snapshot ().latestDataReady,
                 "ready base publishes explicit readiness");

        adk::InertialSample notReady = current;

        notReady.dataReady = false;

        adk::InertialSample prematureNotReady = notReady;

        prematureNotReady.sequence = 2;

        require (
            policy.update (adk::TimePoint (101), prematureNotReady).error () ==
                adk::StatusCode::InvalidArgument,
            "not-ready evidence cannot advance the accepted sequence");

        require (!policy.snapshot ().latestDataReady &&
                     sampleEqual (policy.snapshot ().sample, current),
                 "rejected readiness clears latest evidence but retains payload");

        require (policy.update (adk::TimePoint (101), current).ok (),
                 "exact ready replay restores readiness");

        require (policy.snapshot ().latestDataReady,
                 "accepted ready replay publishes readiness");

        require (policy.update (adk::TimePoint (101), notReady).ok (),
                 "not-ready input is stale evidence");

        require (policy.snapshot ().quality ==
                         adk::InertialSampleQuality::Stale &&
                     !policy.snapshot ().latestDataReady,
                 "not-ready input records explicit stale readiness evidence");

        require (policy.snapshot ().age == adk::Duration (1),
                 "not-ready age derives from retained accepted observation time");

        require (sampleEqual (policy.snapshot ().sample, current),
                 "not-ready evidence retains last accepted sample");

        const adk::InertialObservation retainedNotReady = policy.snapshot ();


        adk::InertialSample foreignNotReady = notReady;

        ++foreignNotReady.source.sourceId;

        require (
            policy.update (adk::TimePoint (102), foreignNotReady).error () ==
                adk::StatusCode::InvalidArgument,
            "different-domain not-ready evidence is rejected");

        const adk::InertialObservation foreignRejected = policy.snapshot ();

        require (policy.snapshot ().quality ==
                         adk::InertialSampleQuality::Invalid &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::InvalidArgument,
                 "different-domain not-ready rejection cannot relabel history");

        require (!foreignRejected.latestDataReady,
                 "invalid domain preserves latest readiness evidence");

        require (sampleEqual (foreignRejected.sample, current),
                 "invalid domain preserves accepted payload");

        require (policy.snapshot ().age == retainedNotReady.age,
                 "invalid not-ready domain preserves retained age");

        adk::InertialSample changedTimestamp = notReady;

        changedTimestamp.observedAt = adk::TimePoint (101);

        require (
            policy.update (adk::TimePoint (102), changedTimestamp).error () ==
                adk::StatusCode::InvalidArgument,
            "not-ready evidence cannot replace the accepted timestamp");

        adk::InertialSample changedAxis = notReady;

        changedAxis.accelerationMicroG.x = 1;

        require (policy.update (adk::TimePoint (102), changedAxis).error () ==
                     adk::StatusCode::InvalidArgument,
                 "not-ready evidence cannot replace accepted axes");


        const uint32_t invalidSequences[] = {0, 2, 0x80000001UL};


        for (uint32_t sequence : invalidSequences)

        {

            adk::InertialSample wrongSequence = notReady;

            wrongSequence.sequence = sequence;


            require (
                policy.update (adk::TimePoint (102), wrongSequence).error () ==
                    adk::StatusCode::InvalidArgument,
                "not-ready evidence requires exact accepted sequence");

            require (!policy.snapshot ().latestDataReady &&
                         sampleEqual (policy.snapshot ().sample, current),
                     "invalid not-ready sequence preserves retained evidence");
        }


        adk::InertialSample malformed = sample (102, 2);

        malformed.source.sourceId = 0;

        require (policy.update (adk::TimePoint (102), malformed).error () ==
                     adk::StatusCode::InvalidArgument,
                 "malformed frame rejected");

        require (sampleEqual (policy.snapshot ().sample, current),
                 "malformed frame retains accepted sample");

        require (!policy.snapshot ().latestDataReady,
                 "malformed frame preserves latest no-ready evidence");

        adk::InertialSample producerFault =

            sample (103, 2, adk::StatusCode::HardwareFailure);


        require (policy.update (adk::TimePoint (103), producerFault).error () ==
                     adk::StatusCode::HardwareFailure,
                 "producer fault remains attributable");

        require (sampleEqual (policy.snapshot ().sample, current),
                 "producer fault retains accepted sample");

        require (!policy.snapshot ().latestDataReady,
                 "producer fault preserves latest no-ready evidence");


        adk::InertialSample recovered = sample (104, 2);


        require (policy.update (adk::TimePoint (104), recovered).ok (),
                 "valid sample recovers after faults");

        require (policy.snapshot ().quality ==
                     adk::InertialSampleQuality::Current &&

                     policy.snapshot ().sample.sequence == 2 &&

                     policy.snapshot ().latestDataReady,
                 "recovery advances from last accepted history");


        adk::InertialSample agedNoReady = recovered;

        agedNoReady.dataReady = false;

        require (policy.update (adk::TimePoint (115), agedNoReady).ok (),
                 "same-sequence no-ready evidence can age retained sample");

        const adk::InertialObservation aged = policy.snapshot ();

        require (policy.snapshot ().quality ==
                         adk::InertialSampleQuality::Stale &&
                     policy.snapshot ().age == adk::Duration (11),
                 "aged no-ready evidence is explicitly stale");

        require (!aged.latestDataReady,
                 "aged no-ready evidence records readiness");

        require (sampleEqual (aged.sample, recovered),
                 "aged no-ready evidence retains payload");

        policy.reset ();

        requireCanonicalInvalid (policy.snapshot (),
                                 "reset clears latest readiness evidence");
    }


    void testPaddingIndependentIdentityAndStableSnapshots ()
    {

        alignas (adk::InertialSample)

            unsigned char leftBytes[sizeof (adk::InertialSample)];

        alignas (adk::InertialSample)

            unsigned char rightBytes[sizeof (adk::InertialSample)];


        std::memset (leftBytes, 0xaa, sizeof (leftBytes));

        std::memset (rightBytes, 0x55, sizeof (rightBytes));

        adk::InertialSample* left = new (leftBytes) adk::InertialSample;

        adk::InertialSample* right = new (rightBytes) adk::InertialSample;


        const adk::InertialSample value = sample (100, 1);


        assignSampleFields (*left, value);

        assignSampleFields (*right, value);

        require (sampleEqual (*left, *right),
                 "fixtures are semantically identical");


        adk::InertialObservationPolicy policy (config ());


        require (policy.initialize ().ok (), "padding policy initializes");

        require (policy.update (adk::TimePoint (100), *left).ok (),
                 "first padded sample accepted");

        require (policy.update (adk::TimePoint (105), *right).ok (),
                 "different padding does not make delta-zero conflict");

        require (policy.snapshot ().age == adk::Duration (5),
                 "semantic padded replay ages existing sample");


        const adk::InertialObservation first = policy.snapshot ();

        const adk::InertialObservation second = policy.snapshot ();


        require (observationEqual (first, second),
                 "snapshot reads are stable and non-consuming");
    }


    void testDeterministicReplay ()
    {

        adk::InertialObservationPolicy left (config (10, 9));

        adk::InertialObservationPolicy right (config (10, 9));
        const adk::InertialSample fixtures[] = {

            sample (0, 0), sample (5, 1), sample (5, 1), sample (16, 4),

            sample (17, 5, adk::StatusCode::Timeout), sample (18, 5)};
        const uint32_t times[] = {0, 5, 10, 16, 17, 18};


        require (left.initialize ().ok () && right.initialize ().ok (),
                 "replay policies initialize");


        for (size_t index = 0; index < sizeof (fixtures) / sizeof (fixtures[0]);
             ++index)

        {
            const adk::Status leftStatus =

                left.update (adk::TimePoint (times[index]), fixtures[index]);
            const adk::Status rightStatus =

                right.update (adk::TimePoint (times[index]), fixtures[index]);


            require (leftStatus == rightStatus,
                     "identical replay returns identical status");

            require (observationEqual (left.snapshot (), right.snapshot ()),
                     "identical replay returns field-identical observation");

        }
    }
}


int main ()
{

    testLifecycleAndConfiguration ();

    testE0AllowListAndProvenanceValidation ();

    testAxesAndSaturation ();

    testEveryProducerStatusAndPrecedence ();

    testFreshnessTimeAndReplayAging ();

    testSequenceDomainsAndBoundaries ();

    testDataReadyAndRecovery ();

    testPaddingIndependentIdentityAndStableSnapshots ();

    testDeterministicReplay ();
    return EXIT_SUCCESS;
}
// clang-format on
