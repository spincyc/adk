#include <orientation_presentation.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <type_traits>

// clang-format off
namespace {
    constexpr double pi = 3.14159265358979323846;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::OrientationConfig orientationConfig ()
    {
        return {{adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveY,
                 adk::SignedAxis::PositiveZ},
                1,
                std::numeric_limits<int32_t>::max (),
                1000,
                5000,
                120000};
    }

    adk::InertialObservation observation (
        int32_t right, int32_t forward, int32_t up,
        adk::InertialSampleQuality quality = adk::InertialSampleQuality::Current,
        adk::Status                status  = adk::StatusCode::Ok)
    {
        const adk::InertialSource source = {adk::InertialSourceKind::SyntheticFixture,
                                            adk::InertialModel::Synthetic,
                                            1,
                                            2,
                                            3,
                                            static_cast<uint32_t> (
                                                std::numeric_limits<int32_t>::max ()),
                                            static_cast<uint32_t> (
                                                std::numeric_limits<int32_t>::max ())};
        const adk::InertialSample sample = {
            source, {right, forward, up},          {0, 0, 0}, adk::TimePoint (10), 1,
            true,   adk::InertialSaturation::None, status};
        adk::InertialObservation result;
        result.sample                    = sample;
        result.quality                   = quality;
        result.latestDataReady           = true;
        result.age                       = adk::Duration (0);
        result.maximumAge                = adk::Duration (20);
        result.freshnessContractRevision = 4;
        result.sequenceGap               = 0;
        result.status                    = status;
        return result;
    }

    adk::BalancePresentationConfig presentationConfig ()
    {
        return {{0, 1000, 0, false},
                {1000, 0, 0, false},
                {0, 1000, 0, false},
                {0, 0, 1000, false},
                {1000, 1000, 0, false},
                {0, 0, 300, false},
                {0, 0, 700, false},
                {1000, 500, 0, true},
                {1000, 0, 1000, true},
                180000,
                100,
                1000,
                1000,
                20};
    }

    bool lightEqual (const adk::BalanceLightIntent& left,
                     const adk::BalanceLightIntent& right)
    {
        return left.redPermille == right.redPermille &&
               left.greenPermille == right.greenPermille &&
               left.bluePermille == right.bluePermille && left.fault == right.fault;
    }

    bool estimateEqual (const adk::OrientationEstimate& left,
                        const adk::OrientationEstimate& right)
    {
        return left.pitchMilliDegrees == right.pitchMilliDegrees &&
               left.rollMilliDegrees == right.rollMilliDegrees &&
               left.quality == right.quality && left.status == right.status;
    }

    int32_t angularDifference (int32_t left, int32_t right)
    {
        int32_t difference = std::abs (left - right);
        if (difference > 180000)
        {
            difference = 360000 - difference;
        }
        return difference;
    }

    bool presentationEqual (const adk::BalancePresentation& left,
                            const adk::BalancePresentation& right)
    {
        return left.direction == right.direction &&
               lightEqual (left.light, right.light) &&
               left.tone.enabled == right.tone.enabled &&
               left.tone.frequencyHertz == right.tone.frequencyHertz &&
               left.tone.durationMilliseconds == right.tone.durationMilliseconds &&
               left.status == right.status;
    }

    void requireInvalidEstimate (const adk::OrientationEstimate& estimate,
                                 adk::StatusCode status, const char* message)
    {
        require (estimate.pitchMilliDegrees == 0 && estimate.rollMilliDegrees == 0 &&
                     estimate.quality == adk::OrientationQuality::Invalid &&
                     estimate.status.error () == status,
                 message);
    }

    adk::OrientationEstimate
    estimateFor (const adk::InertialObservation& input,
                 adk::OrientationConfig          config = orientationConfig ())
    {
        adk::OrientationPolicy policy (config);

        require (policy.initialize ().ok (), "orientation fixture initializes");

        policy.update (input);

        return policy.snapshot ();
    }

    void assignMapped (adk::InertialVector& vector, adk::SignedAxis axis, int32_t value)
    {
        const uint8_t encoded     = static_cast<uint8_t> (axis);
        const int32_t sensorValue = (encoded & 1U) == 0U ? value : -value;
        switch (encoded / 2U)
        {
            case 0: vector.x = sensorValue; break;
            case 1: vector.y = sensorValue; break;
            default: vector.z = sensorValue; break;
        }
    }

    void testNeutralSignedAxisMapping ()
    {
        const adk::SignedAxis axes[] = {
            adk::SignedAxis::PositiveX, adk::SignedAxis::NegativeX,
            adk::SignedAxis::PositiveY, adk::SignedAxis::NegativeY,
            adk::SignedAxis::PositiveZ, adk::SignedAxis::NegativeZ};
        const int32_t input[] = {11, 22, 33};
        unsigned      validMappings = 0;

        for (adk::SignedAxis x : axes)
        {
            for (adk::SignedAxis y : axes)
            {
                for (adk::SignedAxis z : axes)
                {
                    const adk::SignedAxisMapping mapping = {x, y, z};
                    int32_t outputX = 101;
                    int32_t outputY = 102;
                    int32_t outputZ = 103;
                    const bool valid =
                        adk::validSignedAxisMapping (mapping);
                    require (adk::mapSignedAxes (
                                 mapping, input[0], input[1], input[2],
                                 outputX, outputY, outputZ) == valid,
                             "all 216 mappings agree on validity and application");
                    if (!valid)
                    {
                        require (outputX == 101 && outputY == 102 && outputZ == 103,
                                 "invalid mapping leaves output unchanged");
                        continue;
                    }

                    ++validMappings;
                    const adk::SignedAxis outputAxes[] = {x, y, z};
                    const int32_t output[]             = {outputX, outputY, outputZ};
                    for (uint8_t index = 0; index < 3; ++index)
                    {
                        const int32_t source =
                            input[adk::signedAxisIndex (outputAxes[index])];
                        const int32_t expected =
                            adk::signedAxisSign (outputAxes[index]) < 0 ? -source :
                                                                             source;
                        require (output[index] == expected,
                                 "valid mapping applies the selected sign and axis");
                    }

                    int32_t minimumInput[] = {11, 22, 33};
                    for (uint8_t index = 0; index < 3; ++index)
                    {
                        if (adk::signedAxisSign (outputAxes[index]) >= 0)
                        {
                            continue;
                        }
                        minimumInput[adk::signedAxisIndex (outputAxes[index])] =
                            INT32_MIN;
                        outputX = 201;
                        outputY = 202;
                        outputZ = 203;
                        require (!adk::mapSignedAxes (
                                     mapping, minimumInput[0], minimumInput[1],
                                     minimumInput[2], outputX, outputY, outputZ) &&
                                     outputX == 201 && outputY == 202 &&
                                     outputZ == 203,
                                 "negative INT32_MIN mapping fails transactionally");
                        minimumInput[adk::signedAxisIndex (outputAxes[index])] =
                            input[adk::signedAxisIndex (outputAxes[index])];
                    }
                }
            }
        }

        require (validMappings == 24,
                 "neutral mapping recognizes exactly 24 proper rotations");
    }

    void testLifecycleAndConfigurations ()
    {
        static_assert (!std::is_copy_constructible<adk::OrientationPolicy>::value,
                       "orientation policy does not copy");
        static_assert (
            !std::is_move_constructible<adk::BalancePresentationPolicy>::value,
            "presentation policy does not move");

        adk::OrientationPolicy policy (orientationConfig ());

        require (!policy.initialized (), "orientation starts inert");

        requireInvalidEstimate (policy.snapshot (), adk::StatusCode::NotInitialized,
                                "construction snapshot is canonical");
        require (policy.update (observation (0, 0, 1000000)).error () ==
                     adk::StatusCode::NotInitialized,
                 "orientation rejects pre-initialize update");
        require (policy.initialize ().ok () && policy.initialize ().ok (),
                 "orientation initialization is idempotent");
        require (policy.update (observation (0, 0, 1000000)).ok (),
                 "orientation accepts current level sample");
        policy.reset ();

        requireInvalidEstimate (policy.snapshot (), adk::StatusCode::Ok,
                                "orientation reset is canonical");

        const adk::SignedAxis axes[] = {
            adk::SignedAxis::PositiveX, adk::SignedAxis::NegativeX,
            adk::SignedAxis::PositiveY, adk::SignedAxis::NegativeY,
            adk::SignedAxis::PositiveZ, adk::SignedAxis::NegativeZ};
        unsigned validFrames = 0;
        for (adk::SignedAxis right : axes)
        {
            for (adk::SignedAxis forward : axes)
            {
                for (adk::SignedAxis up : axes)
                {
                    adk::OrientationConfig config = orientationConfig ();
                    config.boardFrame             = {right, forward, up};
                    adk::OrientationPolicy candidate (config);

                    if (candidate.initialize ().ok ())
                    {
                        ++validFrames;
                        adk::InertialObservation mapped = observation (0, 0, 0);

                        assignMapped (mapped.sample.accelerationMicroG, right, 300000);

                        assignMapped (mapped.sample.accelerationMicroG, forward,
                                      -400000);
                        assignMapped (mapped.sample.accelerationMicroG, up, 1000000);

                        require (candidate.update (mapped).ok (),
                                 "valid frame accepts mapped pose");
                        const adk::OrientationEstimate estimate = candidate.snapshot ();

                        require (std::abs (estimate.pitchMilliDegrees + 20963) <= 4 &&
                                     std::abs (estimate.rollMilliDegrees - 16699) <= 4,
                                 "all valid frames preserve board geometry");
                    }
                }
            }
        }
        require (validFrames == 24, "exactly 24 signed right-handed frames");

        adk::OrientationConfig invalid[] = {
            {{static_cast<adk::SignedAxis> (6), adk::SignedAxis::PositiveY,
              adk::SignedAxis::PositiveZ},
             1,
             2,
             1,
             0,
             1},
            {{adk::SignedAxis::PositiveX, adk::SignedAxis::NegativeX,
              adk::SignedAxis::PositiveZ},
             1,
             2,
             1,
             0,
             1},
            {{adk::SignedAxis::PositiveX, adk::SignedAxis::PositiveZ,
              adk::SignedAxis::PositiveY},
             1,
             2,
             1,
             0,
             1},
            {orientationConfig ().boardFrame, 0, 2, 1, 0, 1},
            {orientationConfig ().boardFrame, 2, 1, 1, 0, 1},
            {orientationConfig ().boardFrame, 1, 2, 0, 0, 1},
            {orientationConfig ().boardFrame, 1, 2, 1, -1, 1},
            {orientationConfig ().boardFrame, 1, 2, 1, 1, 1},
            {orientationConfig ().boardFrame, 1, 2, 1, 1, 180001}};
        for (const adk::OrientationConfig& config : invalid)
        {
            adk::OrientationPolicy candidate (config);

            require (candidate.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid orientation configuration rejected");
        }
    }

    void testMappingAndCordicOracle ()
    {
        struct Pose
        {
            int32_t right;
            int32_t forward;
            int32_t up;
            int32_t pitch;
            int32_t roll;
        };
        const Pose poses[] = {{0, 0, 1000000, 0, 0},
                              {0, 1000000, 0, 90000, 0},
                              {0, -1000000, 0, -90000, 0},
                              {1000000, 0, 0, 0, 90000},
                              {-1000000, 0, 0, 0, -90000},
                              {0, 0, -1000000, 0, 180000},
                              {707107, 707107, 1000000, 30000, 35264},
                              {-707107, 707107, 1000000, 30000, -35264},
                              {707107, -707107, 1000000, -30000, 35264},
                              {-707107, -707107, 1000000, -30000, -35264}};
        for (const Pose& pose : poses)
        {
            const adk::OrientationEstimate estimate =
                estimateFor (observation (pose.right, pose.forward, pose.up));
            require (std::abs (estimate.pitchMilliDegrees - pose.pitch) <= 4 &&
                         std::abs (estimate.rollMilliDegrees - pose.roll) <= 4,
                     "canonical pose remains within four millidegrees");
        }

        const int32_t          values[] = {-std::numeric_limits<int32_t>::max () + 1,
                                           -1000000000,
                                           -1000000,
                                           -4,
                                           -3,
                                           -1,
                                           0,
                                           1,
                                           3,
                                           4,
                                           1000000,
                                           1000000000,
                                           std::numeric_limits<int32_t>::max () - 1};
        adk::OrientationConfig config   = orientationConfig ();
        config.maximumPresentationAngleMilliDegrees = 180000;
        for (int32_t y : values)
        {
            for (int32_t x : values)
            {
                if (x == 0 && y == 0)
                {
                    continue;
                }
                const int64_t squared =
                    static_cast<int64_t> (x) * x + static_cast<int64_t> (y) * y;
                if (squared >
                    static_cast<int64_t> (std::numeric_limits<int32_t>::max ()) *
                        std::numeric_limits<int32_t>::max ())
                {
                    continue;
                }
                const adk::OrientationEstimate estimate =
                    estimateFor (observation (y, 0, x), config);
                const double oracle =
                    std::atan2 (static_cast<double> (y), static_cast<double> (x)) *
                    180000.0 / pi;
                require (
                    angularDifference (estimate.rollMilliDegrees,
                                       static_cast<int32_t> (std::round (oracle))) <= 4,
                    "CORDIC grid stays within oracle bound");
            }
        }

        uint32_t state = 0x31415926U;
        for (unsigned index = 0; index < 2000; ++index)
        {
            state           = state * 1664525U + 1013904223U;
            const int32_t x = static_cast<int32_t> (state % 2000001U) - 1000000;
            state           = state * 1664525U + 1013904223U;
            const int32_t y = static_cast<int32_t> (state % 2000001U) - 1000000;
            if (x == 0 && y == 0)
            {
                continue;
            }
            const adk::OrientationEstimate estimate =
                estimateFor (observation (y, 0, x), config);
            const int32_t oracle = static_cast<int32_t> (std::round (
                std::atan2 (static_cast<double> (y), static_cast<double> (x)) *
                180000.0 / pi));
            require (angularDifference (estimate.rollMilliDegrees, oracle) <= 4,
                     "randomized CORDIC oracle bound");
        }
    }

    void testGuardsThresholdsAndFaults ()
    {
        adk::OrientationConfig config = orientationConfig ();
        config.minimumGravityMicroG   = 999999;
        config.maximumGravityMicroG   = 1000000;
        require (estimateFor (observation (0, 0, 999999), config).quality ==
                     adk::OrientationQuality::Level,
                 "minimum gravity is inclusive");
        require (estimateFor (observation (0, 0, 1000000), config).quality ==
                     adk::OrientationQuality::Level,
                 "maximum gravity is inclusive");
        require (estimateFor (observation (0, 0, 999998), config).quality ==
                     adk::OrientationQuality::Unsteady,
                 "one below gravity band is unsteady");
        require (estimateFor (observation (0, 0, 1000001), config).quality ==
                     adk::OrientationQuality::Unsteady,
                 "one above gravity band is unsteady");
        requireInvalidEstimate (
            estimateFor (observation (
                std::numeric_limits<int32_t>::min (), 0, 0)),
            adk::StatusCode::InvalidArgument,
            "minimum signed component outside declared range is safe");
        require (estimateFor (observation (
                     std::numeric_limits<int32_t>::max () - 1, 0, 0))
                         .rollMilliDegrees == 90000,
                 "largest admissible component squares without overflow");

        for (unsigned axis = 0; axis < 3; ++axis)
        {
            for (int sign : {-1, 1})
            {
                adk::InertialObservation input = observation (0, 0, 1000000);
                int32_t* rate = &input.sample.angularRateMilliDegreesPerSecond.x;
                rate[axis]    = sign * 1000;
                require (estimateFor (input).quality == adk::OrientationQuality::Level,
                         "stationary rate edge is inclusive");
                rate[axis] = sign * 1001;
                require (estimateFor (input).quality ==
                             adk::OrientationQuality::Unsteady,
                         "rate one beyond edge is unsteady");
            }
        }

        for (adk::InertialSampleQuality quality :
             {adk::InertialSampleQuality::Invalid, adk::InertialSampleQuality::Stale,
              adk::InertialSampleQuality::Saturated})
        {
            requireInvalidEstimate (estimateFor (observation (100, 200, 300, quality)),
                                    adk::StatusCode::InvalidArgument,
                                    "ineligible input never leaks angles");
        }
        adk::InertialObservation failed =
            observation (100, 200, 300, adk::InertialSampleQuality::Current,
                         adk::StatusCode::HardwareFailure);
        requireInvalidEstimate (estimateFor (failed), adk::StatusCode::HardwareFailure,
                                "producer failure is preserved");
        adk::InertialObservation malformed = observation (1, 2, 3);
        malformed.quality = static_cast<adk::InertialSampleQuality> (99);
        requireInvalidEstimate (estimateFor (malformed),
                                adk::StatusCode::InvalidArgument,
                                "malformed quality rejected");

        adk::InertialObservation freshnessFaults[6] = {
            observation (0, 0, 1000000), observation (0, 0, 1000000),
            observation (0, 0, 1000000), observation (0, 0, 1000000),
            observation (0, 0, 1000000), observation (0, 0, 1000000)};
        freshnessFaults[0].latestDataReady                    = false;
        freshnessFaults[1].maximumAge                        = adk::Duration (0);
        freshnessFaults[2].freshnessContractRevision         = 0;
        freshnessFaults[3].sample.source.configurationRevision = 0;
        freshnessFaults[4].sample.source.calibrationRevision = 0;
        freshnessFaults[5].sample.dataReady                   = false;
        for (const adk::InertialObservation& input : freshnessFaults)
        {
            requireInvalidEstimate (estimateFor (input),
                                    adk::StatusCode::InvalidArgument,
                                    "freshness and revision faults rejected");
        }
        adk::InertialObservation saturated = observation (1, 2, 3);
        saturated.sample.saturation = adk::InertialSaturation::Acceleration;
        requireInvalidEstimate (estimateFor (saturated),
                                adk::StatusCode::InvalidArgument,
                                "saturation metadata rejects numeric use");

        adk::InertialObservation sourceFaults[10] = {
            observation (0, 0, 1000000), observation (0, 0, 1000000),
            observation (0, 0, 1000000), observation (0, 0, 1000000),
            observation (0, 0, 1000000), observation (0, 0, 1000000),
            observation (0, 0, 1000000), observation (0, 0, 1000000),
            observation (0, 0, 1000000), observation (0, 0, 1000000)};
        sourceFaults[0].sample.source.kind =
            adk::InertialSourceKind::Mpu6050Adapter;
        sourceFaults[1].sample.source.model = adk::InertialModel::Mpu6050;
        sourceFaults[2].sample.source.kind =
            static_cast<adk::InertialSourceKind> (99);
        sourceFaults[3].sample.source.model =
            static_cast<adk::InertialModel> (99);
        sourceFaults[4].sample.source.sourceId = 0;
        sourceFaults[5].sample.source.accelerationRangeMicroG = 0;
        sourceFaults[6].sample.source.angularRateRangeMilliDegreesPerSecond = 0;
        sourceFaults[7].sample.saturation =
            static_cast<adk::InertialSaturation> (99);
        sourceFaults[8].sample.source.accelerationRangeMicroG = 1000000;
        sourceFaults[8].sample.accelerationMicroG.z           = 1000000;
        sourceFaults[9].sample.source.angularRateRangeMilliDegreesPerSecond =
            1000;
        sourceFaults[9].sample.angularRateMilliDegreesPerSecond.x = 1000;
        for (const adk::InertialObservation& input : sourceFaults)
        {
            requireInvalidEstimate (estimateFor (input),
                                    adk::StatusCode::InvalidArgument,
                                    "forged source structure rejected");
        }

        config                                      = orientationConfig ();
        config.levelThresholdMilliDegrees           = 10000;
        config.maximumPresentationAngleMilliDegrees = 30000;
        const int32_t targetAngles[] = {9995,  9996,  9997,  9998,  9999,
                                        10000, 10001, 29995, 29996, 29997,
                                        29998, 29999, 30000, 30001};
        for (int32_t target : targetAngles)
        {
            const double  radians = static_cast<double> (target) * pi / 180000.0;
            const int32_t forward =
                static_cast<int32_t> (std::round (std::sin (radians) * 1000000.0));
            const int32_t up =
                static_cast<int32_t> (std::round (std::cos (radians) * 1000000.0));
            const adk::OrientationEstimate estimate =
                estimateFor (observation (0, forward, up), config);
            require (std::abs (estimate.pitchMilliDegrees - target) <= 4,
                     "threshold fixture remains within oracle bound");
            if (estimate.pitchMilliDegrees + 4 <= config.levelThresholdMilliDegrees)
            {
                require (estimate.quality == adk::OrientationQuality::Level,
                         "conservative level edge accepted");
            }
            else if (estimate.pitchMilliDegrees + 4 >
                     config.maximumPresentationAngleMilliDegrees)
            {
                require (estimate.quality ==
                             adk::OrientationQuality::BeyondPresentationRange,
                         "conservative presentation edge rejected");
            }
            else
            {
                require (estimate.quality == adk::OrientationQuality::Tilted,
                         "threshold error band resolves to tilted");
            }
        }

        struct OracleClass
        {
            int32_t                 target;
            adk::OrientationQuality expected;
        };
        const OracleClass oracleClasses[] = {
            {9991, adk::OrientationQuality::Level},
            {-9991, adk::OrientationQuality::Level},
            {10001, adk::OrientationQuality::Tilted},
            {-10001, adk::OrientationQuality::Tilted},
            {29991, adk::OrientationQuality::Tilted},
            {-29991, adk::OrientationQuality::Tilted},
            {30001, adk::OrientationQuality::BeyondPresentationRange},
            {-30001, adk::OrientationQuality::BeyondPresentationRange}};
        for (const OracleClass& expected : oracleClasses)
        {
            const double radians =
                static_cast<double> (expected.target) * pi / 180000.0;
            const int32_t forward =
                static_cast<int32_t> (std::round (std::sin (radians) *
                                                  1000000.0));
            const int32_t up =
                static_cast<int32_t> (std::round (std::cos (radians) *
                                                  1000000.0));
            require (estimateFor (observation (0, forward, up), config).quality ==
                         expected.expected,
                     "oracle-separated threshold class is exact");
        }
    }

    void testPresentation ()
    {
        adk::BalancePresentationPolicy policy (presentationConfig ());

        require (!policy.initialized (), "presentation starts inert");

        require (
            policy.update ({0, 0, adk::OrientationQuality::Level, adk::StatusCode::Ok},
                           1000, false)
                    .error () == adk::StatusCode::NotInitialized,
            "presentation rejects pre-initialize update");
        require (policy.initialize ().ok () && policy.initialize ().ok (),
                 "presentation initializes idempotently");

        const adk::OrientationEstimate level = {0, 0, adk::OrientationQuality::Level,
                                                adk::StatusCode::Ok};
        require (
            policy.update (level, 1000, false).ok () &&
                lightEqual (policy.snapshot ().light, presentationConfig ().level) &&

                !policy.snapshot ().tone.enabled,
            "level maps to configured light without tone");

        const adk::OrientationEstimate directions[] = {
            {30000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {-30000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {0, -30000, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {0, 30000, adk::OrientationQuality::Tilted, adk::StatusCode::Ok}};
        const adk::BalanceDirection expected[] = {
            adk::BalanceDirection::Forward, adk::BalanceDirection::Backward,
            adk::BalanceDirection::Left, adk::BalanceDirection::Right};
        policy.reset ();
        for (unsigned index = 0; index < 4; ++index)
        {
            require (policy.update (directions[index], 1000, false).ok (),
                     "direction intent accepts tilt");
            require (policy.snapshot ().direction == expected[index],
                     "cardinal direction maps correctly");
            require (policy.snapshot ().tone.enabled == (index != 0),
                     "tone occurs only on consecutive direction change");
        }
        require (policy.update (directions[3], 1000, false).ok () &&
                     !policy.snapshot ().tone.enabled,
                 "repeated direction is silent");

        policy.reset ();
        const adk::OrientationEstimate pitchTie = {
            -10000, 10008, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (policy.update (pitchTie, 1, false).ok () &&
                     policy.snapshot ().direction == adk::BalanceDirection::Backward,
                 "eight millidegree tie resolves pitch first");
        const adk::OrientationEstimate rollWins = {
            -10000, 10009, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (policy.update (rollWins, 1, false).ok () &&
                     policy.snapshot ().direction == adk::BalanceDirection::Right,
                 "roll wins beyond tie error band");

        policy.reset ();

        require (policy.update (directions[0], 1, false).ok (),
                 "minimum sensitivity accepted");
        const adk::BalanceLightIntent minimum = policy.snapshot ().light;

        require (minimum.redPermille == 100, "minimum intensity clamps dynamically");

        require (policy.update (directions[1], 1000, false).ok (),
                 "maximum sensitivity accepted");
        require (policy.snapshot ().light.greenPermille > minimum.redPermille,
                 "dynamic sensitivity changes scaled intent");

        adk::BalancePresentationConfig responsiveConfig = presentationConfig ();
        responsiveConfig.fullScaleAngleMilliDegrees = 30000;
        adk::BalancePresentationPolicy responsive (responsiveConfig);

        require (responsive.initialize ().ok (), "responsive scale initializes");
        const adk::OrientationEstimate halfScale = {
            15000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (responsive.update (halfScale, 1000, false).ok () &&
                     responsive.snapshot ().light.redPermille == 500,
                 "configured half-scale angle produces half intensity");
        const adk::OrientationEstimate overScale = {
            60000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (responsive.update (overScale, 1000, false).ok () &&
                     responsive.snapshot ().light.redPermille == 1000,
                 "configured full scale clamps rather than wraps");

        adk::BalancePresentationConfig unitScaleConfig = presentationConfig ();
        unitScaleConfig.fullScaleAngleMilliDegrees = 1;
        unitScaleConfig.minimumTiltIntensityPermille = 1;
        adk::BalancePresentationPolicy unitScale (unitScaleConfig);

        require (unitScale.initialize ().ok (), "unit denominator initializes");
        const adk::OrientationEstimate unitAngles[] = {
            {0, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {1, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {2, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok}};
        const uint16_t unitIntensities[] = {1, 1000, 1000};
        for (unsigned index = 0; index < 3; ++index)
        {
            require (unitScale.update (unitAngles[index], 1000, false).ok () &&
                         unitScale.snapshot ().light.redPermille ==
                             unitIntensities[index],
                     "unit denominator below/at/above behavior is bounded");
        }

        adk::BalancePresentationPolicy largestScale (presentationConfig ());

        require (largestScale.initialize ().ok (),
                 "largest denominator initializes");
        const adk::OrientationEstimate largestAngles[] = {
            {179999, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {180000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {180001, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok}};
        require (largestScale.update (largestAngles[0], 1000, false).ok () &&
                     largestScale.snapshot ().light.redPermille == 999,
                 "largest denominator just below stays proportional");
        require (largestScale.update (largestAngles[1], 1000, false).ok () &&
                     largestScale.snapshot ().light.redPermille == 1000,
                 "largest denominator exact edge reaches full intensity");
        require (largestScale.update (largestAngles[2], 1000, false).error () ==
                         adk::StatusCode::InvalidArgument &&
                     !largestScale.snapshot ().tone.enabled,
                 "largest denominator just above is rejected");

        const adk::OrientationEstimate unsteady = {
            7, 9, adk::OrientationQuality::Unsteady, adk::StatusCode::Ok};
        require (policy.update (unsteady, 1000, false).ok () &&
                     lightEqual (policy.snapshot ().light,
                                 presentationConfig ().unsteadyPhaseA) &&
                     !policy.snapshot ().tone.enabled,
                 "unsteady phase A is complete and silent");
        require (policy.update (unsteady, 1000, true).ok () &&
                     lightEqual (policy.snapshot ().light,
                                 presentationConfig ().unsteadyPhaseB) &&
                     !policy.snapshot ().tone.enabled,
                 "unsteady phase B is caller-selected and silent");

        const adk::OrientationEstimate beyond = {
            90000, 0, adk::OrientationQuality::BeyondPresentationRange,
            adk::StatusCode::Ok};
        require (policy.update (beyond, 1000, false).ok () &&
                     lightEqual (policy.snapshot ().light,
                                 presentationConfig ().beyondRange) &&
                     !policy.snapshot ().tone.enabled,
                 "beyond range is fault-safe and silent");
        const adk::OrientationEstimate invalid = {999, 888,
                                                  adk::OrientationQuality::Invalid,
                                                  adk::StatusCode::HardwareFailure};
        require (
            policy.update (invalid, 1000, false).error () ==
                    adk::StatusCode::HardwareFailure &&
                lightEqual (policy.snapshot ().light, presentationConfig ().invalid) &&

                !policy.snapshot ().tone.enabled,
            "invalid input preserves fault and never tones");

        require (policy.update (level, 0, false).error () ==
                         adk::StatusCode::InvalidArgument &&
                     !policy.snapshot ().tone.enabled,
                 "zero sensitivity rejected safely");
        require (policy.update (level, 1001, false).error () ==
                         adk::StatusCode::InvalidArgument &&
                     !policy.snapshot ().tone.enabled,
                 "excess sensitivity rejected safely");

        const adk::OrientationEstimate malformedAngles[] = {
            {std::numeric_limits<int32_t>::min (), 0,
             adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {std::numeric_limits<int32_t>::max (), 0,
             adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {0, std::numeric_limits<int32_t>::min (),
             adk::OrientationQuality::Tilted, adk::StatusCode::Ok},
            {0, std::numeric_limits<int32_t>::max (),
             adk::OrientationQuality::Tilted, adk::StatusCode::Ok}};
        for (const adk::OrientationEstimate& malformed : malformedAngles)
        {
            require (policy.update (malformed, 1000, false).error () ==
                         adk::StatusCode::InvalidArgument &&
                         !policy.snapshot ().tone.enabled,
                     "out-of-domain tilted angle is rejected safely");
        }
    }

    void testPresentationConfigurationsAndReplay ()
    {
        adk::BalancePresentationConfig invalid[11];
        for (adk::BalancePresentationConfig& config : invalid)
        {
            config = presentationConfig ();
        }
        invalid[0].level.redPermille                   = 1001;
        invalid[1].fullScaleAngleMilliDegrees          = 0;
        invalid[2].fullScaleAngleMilliDegrees          = 180001;
        invalid[3].fullScaleAngleMilliDegrees          = -1;
        invalid[4].minimumTiltIntensityPermille        = 0;
        invalid[5].minimumTiltIntensityPermille        = 501;
        invalid[5].maximumTiltIntensityPermille        = 500;
        invalid[6].maximumTiltIntensityPermille        = 1001;
        invalid[7].directionChangeFrequencyHertz       = 0;
        invalid[8].directionChangeDurationMilliseconds = 0;
        invalid[9].directionChangeFrequencyHertz       = 30;
        invalid[10].directionChangeDurationMilliseconds = 60001;
        for (const adk::BalancePresentationConfig& config : invalid)
        {
            adk::BalancePresentationPolicy policy (config);

            require (policy.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid presentation configuration rejected");
        }

        for (unsigned frame = 0; frame < 9; ++frame)
        {
            adk::BalancePresentationConfig config = presentationConfig ();
            adk::BalanceLightIntent* frames[] = {
                &config.level,          &config.forward,      &config.backward,
                &config.left,           &config.right,        &config.unsteadyPhaseA,
                &config.unsteadyPhaseB, &config.beyondRange,  &config.invalid};
            frames[frame]->bluePermille = 1001;
            adk::BalancePresentationPolicy policy (config);

            require (policy.initialize ().error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "every independently malformed light frame is rejected");
        }

        adk::BalancePresentationConfig scaledConfig = presentationConfig ();
        scaledConfig.minimumTiltIntensityPermille    = 1;
        scaledConfig.fullScaleAngleMilliDegrees      = 40000;
        adk::BalancePresentationPolicy scaled (scaledConfig);

        require (scaled.initialize ().ok (),
                 "explicit presentation full scale initializes");

        const adk::OrientationEstimate quarterScale = {
            10000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (scaled.update (quarterScale, 1000, false).ok () &&
                     scaled.snapshot ().light.redPermille == 250,
                 "quarter full-scale angle maps to quarter intensity");
        require (scaled.update (quarterScale, 500, false).ok () &&
                     scaled.snapshot ().light.redPermille == 125,
                 "dynamic sensitivity rescales the same angle");

        const adk::OrientationEstimate fullScale = {
            40000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (scaled.update (fullScale, 1000, false).ok () &&
                     scaled.snapshot ().light.redPermille == 1000,
                 "configured full-scale angle reaches maximum intensity");

        const adk::OrientationEstimate beyondScale = {
            80000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        require (scaled.update (beyondScale, 1000, false).ok () &&
                     scaled.snapshot ().light.redPermille == 1000,
                 "angle beyond configured full scale clamps safely");

        scaledConfig.fullScaleAngleMilliDegrees = 80000;
        adk::BalancePresentationPolicy widerScale (scaledConfig);

        require (widerScale.initialize ().ok () &&
                     widerScale.update (fullScale, 1000, false).ok () &&

                     widerScale.snapshot ().light.redPermille == 500,
                 "full-scale configuration changes the angle mapping");

        const adk::InertialObservation input = observation (250000, -400000, 1000000);

        adk::OrientationPolicy         firstOrientation (orientationConfig ());

        adk::OrientationPolicy         secondOrientation (orientationConfig ());

        require (firstOrientation.initialize ().ok () &&
                     secondOrientation.initialize ().ok (),
                 "replay orientations initialize");
        require (firstOrientation.update (input).ok () &&
                     secondOrientation.update (input).ok () &&

                     estimateEqual (firstOrientation.snapshot (),
                                    secondOrientation.snapshot ()),
                 "identical copied observation replays byte-stable semantics");

        adk::BalancePresentationPolicy firstPresentation (presentationConfig ());

        adk::BalancePresentationPolicy secondPresentation (presentationConfig ());

        require (firstPresentation.initialize ().ok () &&
                     secondPresentation.initialize ().ok (),
                 "replay presentations initialize");
        require (
            firstPresentation.update (firstOrientation.snapshot (), 777, true).ok () &&
                secondPresentation.update (secondOrientation.snapshot (), 777, true)
                    .ok () &&
                presentationEqual (firstPresentation.snapshot (),
                                   secondPresentation.snapshot ()),
            "identical estimate and controls replay byte-stable semantics");

        firstOrientation.reset ();
        requireInvalidEstimate (firstOrientation.snapshot (), adk::StatusCode::Ok,
                                "orientation recovers to safe reset state");
        firstPresentation.reset ();

        require (firstPresentation.snapshot ().direction ==
                         adk::BalanceDirection::None &&
                     !firstPresentation.snapshot ().tone.enabled,
                 "presentation reset clears history and tone");
    }

    void testTransactionalPreviewAndConfigurationPreflight ()
    {
        const adk::OrientationConfig validOrientation = orientationConfig ();

        require (adk::validateOrientationConfig (validOrientation).ok (),
                 "orientation configuration preflight accepts valid config");

        adk::OrientationConfig invalidOrientation = validOrientation;
        invalidOrientation.minimumGravityMicroG = 0;
        require (adk::validateOrientationConfig (invalidOrientation).error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "orientation configuration preflight rejects invalid config");

        const adk::BalancePresentationConfig validPresentation = presentationConfig ();

        require (adk::validateBalancePresentationConfig (validPresentation).ok (),
                 "presentation configuration preflight accepts valid config");

        adk::BalancePresentationConfig invalidPresentation = validPresentation;
        invalidPresentation.fullScaleAngleMilliDegrees = 0;
        require (adk::validateBalancePresentationConfig (invalidPresentation).error () ==
                     adk::StatusCode::InvalidConfiguration,
                 "presentation configuration preflight rejects invalid config");

        const adk::InertialObservation levelInput = observation (0, 0, 1000000);
        const adk::InertialObservation forwardInput =
            observation (0, 500000, 1000000);
        const adk::InertialObservation rightInput =
            observation (500000, 0, 1000000);

        adk::OrientationPolicy orientation (validOrientation);

        static_assert (
            !std::is_aggregate<adk::PreparedOrientationEstimate>::value,
            "prepared orientation candidates hide binding state");
        static_assert (
            !std::is_aggregate<adk::PreparedBalancePresentation>::value,
            "prepared presentation candidates hide binding state");

        adk::PreparedOrientationEstimate candidate;

        require (orientation.preview (levelInput, candidate).error () ==
                     adk::StatusCode::NotInitialized &&
                     !orientation.canCommit (candidate) &&

                     orientation.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "orientation preinitialize preview cannot mutate state");
        require (orientation.initialize ().ok () && orientation.update (levelInput).ok (),
                 "orientation transaction fixture initializes");
        const adk::OrientationEstimate level = orientation.snapshot ();

        adk::PreparedOrientationEstimate forward;
        adk::PreparedOrientationEstimate repeatedForward;
        adk::PreparedOrientationEstimate right;
        require (orientation.preview (forwardInput, forward).ok () &&
                     orientation.preview (forwardInput, repeatedForward).ok () &&

                     estimateEqual (forward.result (), repeatedForward.result ()) &&

                     orientation.canCommit (forward) &&
                     orientation.canCommit (repeatedForward) &&

                     estimateEqual (orientation.snapshot (), level),
                 "repeated orientation preview is deterministic and nonmutating");
        require (orientation.preview (rightInput, right).ok () &&
                     estimateEqual (orientation.snapshot (), level),
                 "a second orientation preview does not replace state");

        adk::InertialObservation malformed = forwardInput;
        malformed.freshnessContractRevision = 0;
        adk::PreparedOrientationEstimate rejected;
        require (orientation.preview (malformed, rejected).error () ==
                     adk::StatusCode::InvalidArgument &&
                     !orientation.canCommit (rejected) &&

                     estimateEqual (orientation.snapshot (), level),
                 "rejected orientation preview preserves committed state");

        require (orientation.commit (forward).ok (),
                 "orientation commits the selected earlier candidate");

        require (estimateEqual (orientation.snapshot (), forward.result ()) &&
                     !orientation.canCommit (forward) &&
                     !orientation.canCommit (repeatedForward),
                 "orientation commit invalidates same-generation candidates");
        const adk::OrientationEstimate committedForward = orientation.snapshot ();

        require (orientation.commit (forward).error () ==
                     adk::StatusCode::InvalidArgument &&
                     estimateEqual (orientation.snapshot (), committedForward),
                 "orientation rejects candidate reuse without mutation");

        adk::OrientationConfig foreignOrientationConfig = validOrientation;
        foreignOrientationConfig.levelThresholdMilliDegrees = 6000;
        adk::OrientationPolicy legacyOrientation        (validOrientation);
        adk::OrientationPolicy transactionalOrientation (validOrientation);
        adk::OrientationPolicy foreignOrientation       (foreignOrientationConfig);

        require (legacyOrientation.initialize ().ok () &&
                     transactionalOrientation.initialize ().ok () &&

                     foreignOrientation.initialize ().ok (),
                 "orientation equivalence fixtures initialize");
        adk::PreparedOrientationEstimate transactionalEstimate;
        require (legacyOrientation.update (rightInput).ok () &&
                     transactionalOrientation.preview (rightInput,
                                                       transactionalEstimate)
                         .ok () &&
                     transactionalOrientation.canCommit (transactionalEstimate) &&

                     !foreignOrientation.canCommit (transactionalEstimate),
                 "orientation legacy and transactional paths accept input");
        const adk::OrientationEstimate foreignBefore = foreignOrientation.snapshot ();

        require (foreignOrientation.commit (transactionalEstimate).error () ==
                     adk::StatusCode::InvalidArgument &&
                     estimateEqual (foreignOrientation.snapshot (), foreignBefore),
                 "orientation rejects candidate from another policy");
        require (transactionalOrientation.commit (transactionalEstimate).ok (),
                 "orientation accepts its fresh prepared candidate");

        require (estimateEqual (legacyOrientation.snapshot (),
                                transactionalOrientation.snapshot ()),
                 "orientation update equals preview then commit");

        adk::BalancePresentationPolicy presentation (validPresentation);
        adk::PreparedBalancePresentation presentationCandidate;
        require (presentation.preview (forward.result (), 1000, false,
                                       presentationCandidate)
                         .error () == adk::StatusCode::NotInitialized &&
                     !presentation.canCommit (presentationCandidate) &&

                     presentation.snapshot ().status.error () ==
                         adk::StatusCode::NotInitialized,
                 "presentation preinitialize preview cannot mutate state");
        require (presentation.initialize ().ok (),
                 "presentation transaction fixture initializes");
        const adk::BalancePresentation empty = presentation.snapshot ();

        const adk::OrientationEstimate forwardEstimate = {
            30000, 0, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        const adk::OrientationEstimate rightEstimate = {
            0, 30000, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        adk::PreparedBalancePresentation forwardPresentation;
        adk::PreparedBalancePresentation repeatedPresentation;
        adk::PreparedBalancePresentation rightBeforeCommit;
        require (presentation.preview (forwardEstimate, 1000, false,
                                       forwardPresentation)
                         .ok () &&
                     presentation.preview (forwardEstimate, 1000, false,
                                           repeatedPresentation)
                         .ok () &&
                     presentationEqual (forwardPresentation.result (),
                                        repeatedPresentation.result ()) &&
                     presentation.canCommit (forwardPresentation) &&
                     presentation.canCommit (repeatedPresentation) &&

                     presentationEqual (presentation.snapshot (), empty),
                 "repeated presentation preview is deterministic and nonmutating");
        require (presentation.preview (rightEstimate, 1000, false,
                                       rightBeforeCommit)
                         .ok () &&
                     !rightBeforeCommit.result ().tone.enabled &&

                     presentationEqual (presentation.snapshot (), empty),
                 "uncommitted preview does not advance direction history");

        adk::PreparedBalancePresentation rejectedPresentation;
        require (presentation.preview (rightEstimate, 0, false,
                                       rejectedPresentation)
                         .error () == adk::StatusCode::InvalidArgument &&
                     !presentation.canCommit (rejectedPresentation) &&

                     presentationEqual (presentation.snapshot (), empty),
                 "rejected presentation preview preserves state and history");

        require (presentation.commit (forwardPresentation).ok (),
                 "presentation commits selected candidate");
        require (!presentation.canCommit (forwardPresentation) &&
                     !presentation.canCommit (repeatedPresentation),
                 "presentation commit invalidates same-generation candidates");
        const adk::BalancePresentation committedPresentation =
            presentation.snapshot ();
        require (presentation.commit (forwardPresentation).error () ==
                     adk::StatusCode::InvalidArgument &&
                     presentationEqual (presentation.snapshot (),
                                        committedPresentation),
                 "presentation rejects candidate reuse without mutation");

        adk::PreparedBalancePresentation rightAfterCommit;
        require (presentation.preview (rightEstimate, 1000, false, rightAfterCommit)
                         .ok () &&
                     rightAfterCommit.result ().tone.enabled,
                 "direction history advances only after successful commit");

        adk::BalancePresentationConfig foreignPresentationConfig = validPresentation;
        foreignPresentationConfig.fullScaleAngleMilliDegrees = 90000;
        adk::BalancePresentationPolicy legacyPresentation        (validPresentation);
        adk::BalancePresentationPolicy transactionalPresentation (validPresentation);
        adk::BalancePresentationPolicy foreignPresentation       (
            foreignPresentationConfig);

        require (legacyPresentation.initialize ().ok () &&
                     transactionalPresentation.initialize ().ok () &&

                     foreignPresentation.initialize ().ok (),
                 "presentation equivalence fixtures initialize");
        adk::PreparedBalancePresentation transactionalIntent;
        require (legacyPresentation.update (rightEstimate, 800, true).ok () &&
                     transactionalPresentation.preview (rightEstimate, 800, true,
                                                        transactionalIntent)
                         .ok () &&
                     transactionalPresentation.canCommit (transactionalIntent) &&

                     !foreignPresentation.canCommit (transactionalIntent),
                 "presentation legacy and transactional paths accept input");
        const adk::BalancePresentation foreignPresentationBefore =
            foreignPresentation.snapshot ();
        require (foreignPresentation.commit (transactionalIntent).error () ==
                     adk::StatusCode::InvalidArgument &&
                     presentationEqual (foreignPresentation.snapshot (),
                                        foreignPresentationBefore),
                 "presentation rejects candidate from another policy");
        require (transactionalPresentation.commit (transactionalIntent).ok (),
                 "presentation accepts its fresh prepared candidate");

        require (presentationEqual (legacyPresentation.snapshot (),
                                    transactionalPresentation.snapshot ()),
                 "presentation update equals preview then commit");

        presentation.reset ();

        require (!presentation.canCommit (rightAfterCommit) &&
                     presentation.commit (rightAfterCommit).error () ==
                         adk::StatusCode::InvalidArgument &&
                     presentationEqual (presentation.snapshot (), empty),
                 "presentation reset invalidates prepared history candidate");
        require (presentation.preview (rightEstimate, 1000, false,
                                       rightBeforeCommit)
                         .ok () &&
                     !rightBeforeCommit.result ().tone.enabled,
                 "presentation reset clears committed direction history");
    }

    void testCommittableSafeStatesResetDirectionHistory ()
    {
        adk::OrientationPolicy orientation (orientationConfig ());

        require (orientation.initialize ().ok (),
                 "safe-state orientation fixture initializes");

        adk::InertialObservation staleInput = observation (0, 0, 1000000);
        staleInput.quality                   = adk::InertialSampleQuality::Stale;
        staleInput.latestDataReady           = false;
        adk::PreparedOrientationEstimate stale;
        require (orientation.preview (staleInput, stale).error () ==
                     adk::StatusCode::InvalidArgument &&
                     orientation.canCommit (stale) &&

                     orientation.commit (stale).ok () &&

                     orientation.snapshot ().quality ==
                         adk::OrientationQuality::Invalid,
                 "structurally valid stale input commits a safe estimate");

        adk::InertialObservation saturatedInput = observation (0, 0, 1000000);
        saturatedInput.quality = adk::InertialSampleQuality::Saturated;
        saturatedInput.sample.saturation = adk::InertialSaturation::Acceleration;
        adk::PreparedOrientationEstimate saturated;
        require (orientation.preview (saturatedInput, saturated).error () ==
                     adk::StatusCode::InvalidArgument &&
                     orientation.canCommit (saturated) &&

                     orientation.commit (saturated).ok (),
                 "structurally valid saturation commits a safe estimate");

        adk::PreparedOrientationEstimate producerFault;
        require (orientation
                     .preview (observation (0, 0, 1000000,
                                            adk::InertialSampleQuality::Current,
                                            adk::StatusCode::HardwareFailure),
                               producerFault)
                     .error () == adk::StatusCode::HardwareFailure &&

                     orientation.canCommit (producerFault) &&

                     orientation.commit (producerFault).ok () &&

                     orientation.snapshot ().status.error () ==
                         adk::StatusCode::HardwareFailure,
                 "producer fault remains a committable safe-state candidate");

        adk::PreparedOrientationEstimate unsteady;
        require (orientation.preview (observation (0, 0, 0), unsteady).ok () &&
                     unsteady.result ().quality ==
                         adk::OrientationQuality::Unsteady &&
                     orientation.canCommit (unsteady) &&

                     orientation.commit (unsteady).ok (),
                 "unsteady estimate remains committable");

        adk::PreparedOrientationEstimate beyond;
        require (orientation.preview (observation (0, 0, -1000000), beyond).ok () &&
                     beyond.result ().quality ==
                         adk::OrientationQuality::BeyondPresentationRange &&
                     orientation.canCommit (beyond) &&

                     orientation.commit (beyond).ok (),
                 "beyond-range estimate remains committable");

        adk::BalancePresentationPolicy presentation (presentationConfig ());

        require (presentation.initialize ().ok (),
                 "safe-state presentation fixture initializes");

        const adk::OrientationEstimate right = {
            0, 30000, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        const adk::OrientationEstimate left = {
            0, -30000, adk::OrientationQuality::Tilted, adk::StatusCode::Ok};
        const adk::OrientationEstimate unsteadyEstimate = {
            0, 0, adk::OrientationQuality::Unsteady, adk::StatusCode::Ok};

        adk::PreparedBalancePresentation preparedRight;
        adk::PreparedBalancePresentation preparedSafe;
        adk::PreparedBalancePresentation preparedLeft;
        require (presentation.preview (right, 1000, false, preparedRight).ok () &&
                     presentation.commit (preparedRight).ok (),
                 "right direction commits");
        require (
            presentation.preview (unsteadyEstimate, 1000, false, preparedSafe).ok () &&
                presentation.canCommit (preparedSafe) &&

                presentation.commit (preparedSafe).ok () &&

                preparedSafe.result ().direction == adk::BalanceDirection::None,
            "unsteady safe-state intent commits and clears direction");
        require (presentation.preview (left, 1000, false, preparedLeft).ok () &&
                     !preparedLeft.result ().tone.enabled,
                 "right to unsteady to left emits no spurious change tone");

        require (presentation.commit (preparedLeft).ok (),
                 "left direction commits after unsteady recovery");
        adk::PreparedBalancePresentation invalidSafe;
        require (presentation.preview (stale.result (), 1000, false, invalidSafe)
                         .error () == adk::StatusCode::InvalidArgument &&
                     presentation.canCommit (invalidSafe) &&

                     presentation.commit (invalidSafe).ok () &&

                     invalidSafe.result ().direction == adk::BalanceDirection::None,
                 "stale-derived invalid safe-state intent commits");
        require (presentation.preview (right, 1000, false, preparedRight).ok () &&
                     !preparedRight.result ().tone.enabled,
                 "left to stale to right emits no spurious change tone");
    }
} // namespace

int main ()
{
    testNeutralSignedAxisMapping ();

    testLifecycleAndConfigurations ();

    testMappingAndCordicOracle ();

    testGuardsThresholdsAndFaults ();

    testPresentation ();

    testPresentationConfigurationsAndReplay ();

    testTransactionalPreviewAndConfigurationPreflight ();

    testCommittableSafeStatesResetDirectionHistory ();
    std::cout << "orientation presentation tests passed\n";
    return EXIT_SUCCESS;
}
// clang-format on
