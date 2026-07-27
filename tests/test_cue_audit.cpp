#include <assert.h>
#include <string.h>
#include <type_traits>

#include "cue_audit.h"

namespace {

    static_assert (!std::is_copy_constructible<adk::CueAuditBuffer>::value,
                   "audit owns its lifecycle");
    static_assert (!std::is_move_constructible<adk::CueAuditBuffer>::value,
                   "audit has a stable address");

    adk::CueAuditEntry sampleEntry (bool hasCue = true) noexcept
    {
        return {4294967295u,
                adk::TimePoint (4294967295u),
                adk::CueAuditEvent::ConfirmationRequested,
                31,
                31,
                adk::StatusCode::CapacityExceeded,
                hasCue};
    }

    void testConfigurationAndLifecycle ()
    {
        adk::CueAuditEntry  storage[3];
        adk::CueAuditBuffer nullAudit (nullptr, 3);

        adk::CueAuditBuffer smallAudit (storage, 2);

        adk::CueAuditBuffer audit (storage, 3);

        assert (nullAudit.initialize ().error () == adk::StatusCode::InvalidArgument);

        assert (smallAudit.initialize ().error () == adk::StatusCode::InvalidArgument);

        assert (audit.capacity () == 3);

        assert (audit.initialize ().ok ());

        assert (audit.initialize ().ok ());

        assert (audit.initialized ());

        assert (audit.count () == 0);

        assert (audit.entry (0).error () == adk::StatusCode::InvalidArgument);

        audit.shutdown ();

        audit.shutdown ();

        assert (!audit.initialized ());

        assert (audit.count () == 0);

        assert (audit.entry (0).error () == adk::StatusCode::NotInitialized);
    }

    void testEncoderGrammarAndAtomicCapacity ()
    {
        const adk::CueAuditEntry entry = sampleEntry ();
        adk::CueAuditEncoder     encoder;
        char                     output[adk::CueAuditEncoder::maximumLength];

        const char expected[] =
            "adk-cue,1,4294967295,4294967295,confirmation-requested,31,31,"
            "capacity exceeded\n";

        const adk::Result<uint8_t> required = encoder.requiredSize (entry);

        assert (required.ok ());

        assert (required.value () == strlen (expected));
        memset (output, 'x', sizeof (output));

        const adk::Result<uint8_t> encoded =
            encoder.encode (entry, output, required.value ());

        assert (encoded.ok ());

        assert (encoded.value () == required.value ());

        assert (memcmp (output, expected, encoded.value ()) == 0);

        assert (output[encoded.value ()] == 'x');

        char shortOutput[adk::CueAuditEncoder::maximumLength];
        memset (shortOutput, 'q', sizeof (shortOutput));

        assert (encoder
                    .encode (entry, shortOutput,
                             static_cast<uint8_t> (required.value () - 1U))
                    .error () == adk::StatusCode::CapacityExceeded);

        for (uint8_t index = 0; index < sizeof (shortOutput); ++index)
        {
            assert (shortOutput[index] == 'q');
        }

        assert (encoder.encode (entry, nullptr, 1).error () ==
                adk::StatusCode::InvalidArgument);
        assert (encoder.encode (entry, nullptr, 0).error () ==
                adk::StatusCode::CapacityExceeded);
    }

    void testAbsentCueAndEveryEvent ()
    {
        adk::CueAuditEncoder encoder;
        char                 output[adk::CueAuditEncoder::maximumLength];
        adk::CueAuditEntry   absent = sampleEntry (false);

        const adk::Result<uint8_t> encoded =
            encoder.encode (absent, output, sizeof (output));
        const char expected[] =
            "adk-cue,1,4294967295,4294967295,confirmation-requested,-,-,"
            "capacity exceeded\n";

        assert (encoded.ok ());

        assert (encoded.value () == strlen (expected));

        assert (memcmp (output, expected, encoded.value ()) == 0);

        for (uint8_t raw = 0;
             raw <= static_cast<uint8_t> (adk::CueAuditEvent::Shutdown); ++raw)
        {
            adk::CueAuditEntry entry = sampleEntry ();

            entry.event = static_cast<adk::CueAuditEvent> (raw);
            assert (encoder.encode (entry, output, sizeof (output)).ok ());
        }

        adk::CueAuditEntry invalidEvent  = sampleEntry ();

        adk::CueAuditEntry invalidStatus = sampleEntry ();

        invalidEvent.event =
            static_cast<adk::CueAuditEvent> (static_cast<uint8_t> (255));
        invalidStatus.status =
            static_cast<adk::StatusCode> (static_cast<uint8_t> (255));

        assert (encoder.requiredSize (invalidEvent).error () ==
                adk::StatusCode::InvalidArgument);
        assert (encoder.requiredSize (invalidStatus).error () ==
                adk::StatusCode::InvalidArgument);
    }
} // namespace

int main ()
{
    testConfigurationAndLifecycle ();

    testEncoderGrammarAndAtomicCapacity ();

    testAbsentCueAndEveryEvent ();
}
