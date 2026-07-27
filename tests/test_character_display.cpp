#include <character_display.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>

namespace {

    struct RecordingTransport final : adk::Hd44780Transport
    {
        struct Write
        {
            adk::PinId pin;
            adk::Level level;
        };

        adk::Status configureOutput (adk::PinId pin) noexcept override
        {
            if (configureCount == failConfigureAt)
            {
                return adk::StatusCode::HardwareFailure;
            }

            configured[configureCount++] = pin;
            return adk::StatusCode::Ok;
        }

        void release (adk::PinId pin) noexcept override
        {
            released[releaseCount++] = pin;
        }

        adk::Status write (adk::PinId pin, adk::Level level) noexcept override
        {
            if (writeCount == failWriteAt)
            {
                return adk::StatusCode::HardwareFailure;
            }

            writes[writeCount++] = {pin, level};
            return adk::StatusCode::Ok;
        }

        void waitMicroseconds (uint16_t duration) noexcept override
        {
            waitedMicroseconds += duration;
        }

        adk::PinId configured[32]     = {};
        adk::PinId released[32]       = {};
        Write      writes[512]        = {};
        size_t     configureCount     = 0;
        size_t     releaseCount       = 0;
        size_t     writeCount         = 0;
        size_t     failConfigureAt    = 99;
        size_t     failWriteAt        = 999;
        uint32_t   waitedMicroseconds = 0;
    };

    constexpr adk::Hd44780Pins pins = {22, 23, 24, 25, 26, 27};

    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void startDisplay (adk::Hd44780Display& display)
    {
        require (display.initialize ().ok (), "display initializes");
        require (display.update (adk::TimePoint (100)).ok (), "startup anchors");

        const uint32_t times[] = {115, 120, 121, 122, 123, 124, 125, 127, 128, 129};

        for (uint32_t time : times)
        {
            require (display.update (adk::TimePoint (time)).ok (),
                     "startup step succeeds");
        }

        require (display.ready (), "display becomes ready");
    }

    void testLifecycleAndStartup ()
    {
        adk::ResourceRegistry resources;
        RecordingTransport    transport;
        adk::Hd44780Display   display (resources, pins, transport);

        require (!display.initialized (), "display starts inert");
        require (display.update (adk::TimePoint (0)).error () ==
                     adk::StatusCode::NotInitialized,
                 "stopped display rejects updates");
        require (display.show (0, "ready").error () == adk::StatusCode::NotInitialized,
                 "stopped display rejects text");

        startDisplay (display);
        const size_t configured = transport.configureCount;

        require (display.initialize ().ok (), "repeated initialization succeeds");
        require (transport.configureCount == configured,
                 "repeated initialization is inert");

        display.shutdown ();

        require (!display.initialized (), "shutdown stops display");
        require (transport.releaseCount == 6, "shutdown releases every pin");

        display.shutdown ();

        require (transport.releaseCount == 6, "repeated shutdown is inert");
    }

    void testTextValidationAndIncrementalFlush ()
    {
        adk::ResourceRegistry resources;
        RecordingTransport    transport;
        adk::Hd44780Display   display (resources, pins, transport);

        startDisplay (display);

        require (display.show (2, "bad").error () == adk::StatusCode::InvalidArgument,
                 "unknown row is rejected");
        require (display.show (0, nullptr).error () == adk::StatusCode::InvalidArgument,
                 "null text is rejected");
        require (display.show (0, "12345678901234567").error () ==
                     adk::StatusCode::CapacityExceeded,
                 "long text is rejected");
        require (display.show (0, "room 21C").ok (), "short text is padded");
        require (std::strcmp (display.line (0), "room 21C        ") == 0,
                 "desired line remains observable");
        require (display.line (2) == nullptr, "unknown line has no snapshot");

        const size_t before = transport.writeCount;
        require (display.update (adk::TimePoint (200)).ok (), "one dirty cell flushes");
        require (transport.writeCount > before, "flush writes one cell");
        require (display.update (adk::TimePoint (201)).ok (),
                 "next dirty cell flushes separately");
    }

    void testValidationAndRollback ()
    {
        {
            adk::ResourceRegistry  resources;
            RecordingTransport     transport;
            const adk::Hd44780Pins duplicate = {22, 22, 24, 25, 26, 27};
            adk::Hd44780Display    display (resources, duplicate, transport);

            require (display.initialize ().error () == adk::StatusCode::InvalidArgument,
                     "duplicate pins are rejected");
            require (transport.configureCount == 0, "invalid pins touch no hardware");
        }

        {
            adk::ResourceRegistry resources;
            RecordingTransport    transport;
            transport.failConfigureAt = 3;
            adk::Hd44780Display display (resources, pins, transport);

            require (display.initialize ().error () == adk::StatusCode::HardwareFailure,
                     "configure failure is reported");
            require (!display.initialized (), "failed display remains stopped");
            require (transport.releaseCount == 6,
                     "configure failure releases every claim");

            transport.failConfigureAt = 99;
            require (display.initialize ().ok (), "released pins can be reacquired");
        }
    }

    void testDestructorRelease ()
    {
        adk::ResourceRegistry resources;
        RecordingTransport    transport;

        {
            adk::Hd44780Display display (resources, pins, transport);

            require (display.initialize ().ok (), "scoped display initializes");
        }

        require (transport.releaseCount == 6, "destructor releases every pin");

        adk::Hd44780Display replacement (resources, pins, transport);

        require (replacement.initialize ().ok (), "destructor permits resource reuse");
    }

    void testStableRecords ()
    {
        char                     record[128] = {};
        const adk::ClimateSample valid       = {-125, 456, adk::TimePoint (4294967290U),
                                                adk::ClimateSampleState::Valid};

        require (adk::formatClimateRecord (valid, 42, record, sizeof (record)).ok (),
                 "record fits");
        require (std::strcmp (record,
                              "seq=42,state=valid,temp_centi_c=-125,rh_permille=456,"
                              "at_ms=4294967290\n") == 0,
                 "record is exact and locale independent");

        char small[8] = {};
        require (adk::formatClimateRecord (valid, 42, small, sizeof (small)).error () ==
                     adk::StatusCode::CapacityExceeded,
                 "small buffer reports capacity");
        require (small[sizeof (small) - 1U] == '\0', "small buffer remains terminated");
        require (adk::formatClimateRecord (valid, 0, nullptr, 0).error () ==
                     adk::StatusCode::InvalidArgument,
                 "null record buffer is rejected");

        const adk::ClimateSample fault = {0, 0, adk::TimePoint (7),
                                          adk::ClimateSampleState::ChecksumFailure};
        require (adk::formatClimateRecord (fault, 1, record, sizeof (record)).ok (),
                 "fault record fits");
        require (std::strstr (record, "state=checksum_failure") != nullptr,
                 "fault state remains explicit");
    }
} // namespace

int main ()
{
    testLifecycleAndStartup               ();
    testTextValidationAndIncrementalFlush ();
    testValidationAndRollback             ();
    testDestructorRelease                 ();
    testStableRecords                     ();

    static_assert (!std::is_copy_constructible<adk::Hd44780Display>::value,
                   "display owns pin resources");
    static_assert (!std::is_move_constructible<adk::Hd44780Display>::value,
                   "display has a stable address");

    std::cout << "character display tests passed\n";
    return EXIT_SUCCESS;
}
