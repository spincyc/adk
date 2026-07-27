#include <assert.h>
#include <stdint.h>
#include <type_traits>

#include "fixed_storage.h"
#include "rtc.h"

namespace {

    void assertBytes (const adk::FixedStorageMedium& medium, const uint8_t* expected,
                      uint16_t size)
    {
        assert (medium.durableSize () == size);

        for (uint16_t index = 0; index < size; ++index)
        {
            assert (medium.data ()[index] == expected[index]);
        }
    }

    void testRtcValueContract ()
    {
        const adk::ClockReading valid = {1700000000u, adk::ClockState::Valid};
        const adk::Result<adk::ClockReading> result (adk::Status (), valid);

        assert (result.ok ());
        assert (result.value ().unixSeconds == 1700000000u);
        assert (result.value ().state == adk::ClockState::Valid);

        const adk::ClockReading fault = {0, adk::ClockState::TransportFault};
        const adk::Result<adk::ClockReading> failed (adk::StatusCode::HardwareFailure,
                                                     fault);

        assert (!failed.ok ());
        assert (failed.transient ());

        const adk::ClockReading states[] = {{1, adk::ClockState::Valid},
                                            {0, adk::ClockState::NotSet},
                                            {2, adk::ClockState::OscillatorStopped},
                                            {0, adk::ClockState::TransportFault}};
        assert (states[0].state == adk::ClockState::Valid);
        assert (states[1].state == adk::ClockState::NotSet);
        assert (states[2].state == adk::ClockState::OscillatorStopped);
        assert (states[3].state == adk::ClockState::TransportFault);
    }

    void testStorageRequiresInitialization ()
    {
        adk::FixedStorageMedium medium  (8);
        adk::FixedStorage       storage (medium);
        const uint8_t           byte = 7;

        assert (storage.append (&byte, 1).error () == adk::StatusCode::NotInitialized);
        assert (storage.sync ().error () == adk::StatusCode::NotInitialized);
    }

    void testAppendAndSyncSeparateStagingFromDurability ()
    {
        adk::FixedStorageMedium medium  (8);
        adk::FixedStorage       storage (medium);
        const uint8_t           bytes[] = {1, 2, 3};

        assert (storage.initialize ().ok ());
        assert (storage.append (bytes, 3).ok ());
        assert (storage.stagedSize () == 3);
        assert (medium.durableSize () == 0);

        assert      (storage.sync ().ok ());
        assertBytes (medium, bytes, 3);
        assert      (storage.initialize ().ok ());
        assert      (storage.stagedSize () == 3);
    }

    void testRestartDiscardsUnsyncedSuffix ()
    {
        adk::FixedStorageMedium medium (8);
        const uint8_t           durable[] = {4, 5};
        const uint8_t           pending[] = {6, 7};

        {
            adk::FixedStorage storage (medium);

            assert (storage.initialize ().ok ());
            assert (storage.append (durable, 2).ok ());
            assert (storage.sync ().ok ());
            assert (storage.append (pending, 2).ok ());
        }

        adk::FixedStorage restarted (medium);

        assert      (restarted.initialize ().ok ());
        assert      (restarted.stagedSize () == 2);
        assertBytes (medium, durable, 2);
    }

    void testEveryAppendFailureOffsetPreservesExactPrefix ()
    {
        const uint8_t bytes[] = {10, 11, 12, 13};

        for (uint16_t failureOffset = 0; failureOffset < 4; ++failureOffset)
        {
            adk::FixedStorageMedium medium  (8);
            adk::FixedStorage       storage (medium);

            assert                        (storage.initialize ().ok ());
            storage.injectAppendFailureAt (failureOffset);
            assert                        (storage.append (bytes, 4).error () ==
                    adk::StatusCode::HardwareFailure);
            assert (storage.stagedSize () == failureOffset);
            assert (medium.durableSize () == 0);

            storage.injectAppendFailureAt (adk::FixedStorage::noAppendFailure);
            assert                        (storage.sync ().ok ());
            assertBytes                   (medium, bytes, failureOffset);
        }
    }

    void testPartialAppendRestartRecoversPriorDurablePrefix ()
    {
        adk::FixedStorageMedium medium  (8);
        adk::FixedStorage       storage (medium);
        const uint8_t           durable[] = {30, 31};
        const uint8_t           pending[] = {32, 33, 34};

        assert                        (storage.initialize ().ok ());
        assert                        (storage.append (durable, 2).ok ());
        assert                        (storage.sync ().ok ());
        storage.injectAppendFailureAt (1);
        assert                        (storage.append (pending, 3).error () ==
                adk::StatusCode::HardwareFailure);
        assert (storage.stagedSize () == 3);

        storage.shutdown ();
        assert           (storage.initialize ().ok ());
        assert           (storage.stagedSize () == 2);
        assertBytes      (medium, durable, 2);
    }

    void testFailedSyncDoesNotAdvanceDurablePrefix ()
    {
        adk::FixedStorageMedium medium  (8);
        adk::FixedStorage       storage (medium);
        const uint8_t           bytes[] = {20, 21, 22};

        assert                    (storage.initialize ().ok ());
        assert                    (storage.append (bytes, 3).ok ());
        storage.injectSyncFailure (true);
        assert                    (storage.sync ().error () == adk::StatusCode::HardwareFailure);
        assert                    (medium.durableSize () == 0);

        storage.injectSyncFailure (false);
        assert                    (storage.sync ().ok ());
        assertBytes               (medium, bytes, 3);
    }

    void testRepeatedInitializePreservesUnsyncedBytes ()
    {
        adk::FixedStorageMedium medium  (8);
        adk::FixedStorage       storage (medium);
        const uint8_t           bytes[] = {40, 41};

        assert (storage.initialize ().ok ());
        assert (storage.append (bytes, 2).ok ());
        assert (storage.initialize ().ok ());
        assert (storage.stagedSize () == 2);
        assert (medium.durableSize () == 0);
    }

    void testCapacityAndNullBufferRules ()
    {
        adk::FixedStorageMedium medium  (3);
        adk::FixedStorage       storage (medium);
        const uint8_t           bytes[] = {1, 2, 3, 4};

        assert (storage.initialize ().ok ());
        assert (storage.append (0, 0).ok ());
        assert (storage.append (0, 1).error () == adk::StatusCode::InvalidArgument);
        assert (storage.append (bytes, 4).error () ==
                adk::StatusCode::CapacityExceeded);
        assert      (storage.stagedSize () == 0);
        assert      (storage.append (bytes, 3).ok ());
        assert      (storage.sync ().ok ());
        assertBytes (medium, bytes, 3);
    }

    void testCapacityFailurePreservesDurablePrefix ()
    {
        adk::FixedStorageMedium medium  (4);
        adk::FixedStorage       storage (medium);
        const uint8_t           durable[]  = {50, 51, 52};
        const uint8_t           overflow[] = {53, 54};

        assert (storage.initialize ().ok ());
        assert (storage.append (durable, 3).ok ());
        assert (storage.sync ().ok ());
        assert (storage.append (overflow, 2).error () ==
                adk::StatusCode::CapacityExceeded);
        assert      (storage.stagedSize () == 3);
        assertBytes (medium, durable, 3);
    }

    void testInvalidCapacityAndShutdownIsIdempotent ()
    {
        adk::FixedStorageMedium emptyMedium  (0);
        adk::FixedStorage       emptyStorage (emptyMedium);
        adk::FixedStorageMedium largeMedium  (
            static_cast<uint16_t> (adk::FixedStorageMedium::maximumCapacity + 1));
        adk::FixedStorage largeStorage (largeMedium);

        assert (emptyStorage.initialize ().error () ==
                adk::StatusCode::InvalidArgument);
        assert (largeStorage.initialize ().error () ==
                adk::StatusCode::InvalidArgument);

        adk::FixedStorageMedium medium  (8);
        adk::FixedStorage       storage (medium);
        assert                          (storage.initialize ().ok ());
        storage.shutdown                ();
        storage.shutdown                ();
        assert                          (!storage.initialized ());
        assert                          (storage.initialize ().ok ());
    }
} // namespace

int main ()
{
    static_assert (!std::is_copy_constructible<adk::FixedStorage>::value,
                   "storage ownership must not copy");
    static_assert (!std::is_move_constructible<adk::FixedStorage>::value,
                   "storage ownership must not move");

    testRtcValueContract                               ();
    testStorageRequiresInitialization                  ();
    testAppendAndSyncSeparateStagingFromDurability     ();
    testRestartDiscardsUnsyncedSuffix                  ();
    testEveryAppendFailureOffsetPreservesExactPrefix   ();
    testPartialAppendRestartRecoversPriorDurablePrefix ();
    testFailedSyncDoesNotAdvanceDurablePrefix          ();
    testRepeatedInitializePreservesUnsyncedBytes       ();
    testCapacityAndNullBufferRules                     ();
    testCapacityFailurePreservesDurablePrefix          ();
    testInvalidCapacityAndShutdownIsIdempotent         ();
    return 0;
}
