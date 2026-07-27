#include "fixed_storage.h"
#include "record_sink.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace {

    using namespace adk;

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    StableRecord recordWith (const char* text)
    {
        StableRecord record = {};
        const size_t length = std::strlen (text);

        require     (length <= StableRecord::capacity, "test record capacity");
        std::memcpy (record.text, text, length);
        record.length = static_cast<uint8_t> (length);
        return record;
    }

    void testLifecycleAndDurability ()
    {
        FixedStorageMedium medium              (128);
        FixedStorage       storage             (medium);
        StorageRecordSink  sink                (storage);
        const StableRecord record = recordWith ("first\n");

        require (!sink.initialized (), "construction is inert");
        require (sink.append (record).error () == StatusCode::NotInitialized,
                 "inactive append rejected");
        require (sink.initialize ().ok (), "initialize");
        require (sink.initialize ().ok (), "repeat initialize");
        require (sink.append (record).ok (), "append and sync");
        require (medium.durableSize () == record.length,
                 "successful append is durable");
        require (std::memcmp (medium.data (), record.text, record.length) == 0,
                 "durable bytes exact");

        sink.shutdown ();
        require       (!sink.initialized (), "shutdown state");
        sink.shutdown ();

        require (sink.initialize ().ok (), "restart");
        require (medium.durableSize () == record.length,
                 "restart preserves durable prefix");
    }

    void testPartialAppendRollbackAndRetry ()
    {
        FixedStorageMedium medium              (128);
        FixedStorage       storage             (medium);
        StorageRecordSink  sink                (storage);
        const StableRecord first  = recordWith ("first\n");
        const StableRecord second = recordWith ("second\n");

        require (sink.initialize ().ok (), "initialize");
        require (sink.append (first).ok (), "seed durable prefix");

        storage.injectAppendFailureAt (3);
        require                       (sink.append (second).error () == StatusCode::HardwareFailure,
                 "partial append failure returned");
        require (sink.initialized (), "rollback restores active storage");
        require (storage.stagedSize () == medium.durableSize (),
                 "partial staged bytes discarded");
        require (medium.durableSize () == first.length,
                 "failure preserves durable prefix");

        storage.injectAppendFailureAt (FixedStorage::noAppendFailure);
        require                       (sink.append (second).ok (), "byte-identical retry");
        require                       (medium.durableSize () == first.length + second.length,
                 "retry extends durable prefix once");
        require (std::memcmp (medium.data () + first.length, second.text,
                              second.length) == 0,
                 "retry bytes exact");
    }

    void testSyncFailureRollbackAndCapacity ()
    {
        FixedStorageMedium medium                   (8);
        FixedStorage       storage                  (medium);
        StorageRecordSink  sink                     (storage);
        const StableRecord shortRecord = recordWith ("1234");
        const StableRecord longRecord  = recordWith ("123456789");

        require                   (sink.initialize ().ok (), "initialize");
        storage.injectSyncFailure (true);
        require                   (sink.append (shortRecord).error () == StatusCode::HardwareFailure,
                 "sync failure returned");
        require (medium.durableSize () == 0, "failed sync is not durable");
        require (storage.stagedSize () == 0, "failed sync staging discarded");

        storage.injectSyncFailure (false);
        require                   (sink.append (longRecord).error () == StatusCode::CapacityExceeded,
                 "capacity failure returned");
        require (medium.durableSize () == 0, "capacity failure changes no media");
        require (sink.append (shortRecord).ok (), "sink remains reusable");
    }
} // namespace

int main ()
{
    testLifecycleAndDurability         ();
    testPartialAppendRollbackAndRetry  ();
    testSyncFailureRollbackAndCapacity ();

    std::cout << "record sink tests passed\n";
    return EXIT_SUCCESS;
}
