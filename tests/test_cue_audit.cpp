#include <assert.h>
#include <string.h>
#include <type_traits>

#include "cue_audit.h"

namespace {

    static_assert (!std::is_copy_constructible<adk::CueAuditBuffer>::value,
                   "audit owns its lifecycle");
    static_assert (!std::is_move_constructible<adk::CueAuditBuffer>::value,
                   "audit has a stable address");

    adk::CueAuditEntry sampleEntry () noexcept
    {
        return {4294967295u,
                adk::TimePoint (4294967295u),
                adk::CueAuditEvent::ConfirmationRequested,
                31,
                31,
                adk::StatusCode::CapacityExceeded};
    }

    void testConfigurationAndLifecycle ()
    {
        adk::CueAuditEntry  storage[3];
        adk::CueAuditBuffer nullAudit  (nullptr, 3);
        adk::CueAuditBuffer smallAudit (storage, 2);
        adk::CueAuditBuffer audit      (storage, 3);

        assert (nullAudit.initialize ().error () == adk::StatusCode::InvalidArgument);
        assert (smallAudit.initialize ().error () == adk::StatusCode::InvalidArgument);
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

    void testEncoderGrammarAndCapacity ()
    {
        const adk::CueAuditEntry entry = sampleEntry ();
        adk::CueAuditEncoder     encoder;
        char                     output[adk::CueAuditEncoder::maximumLength];
        uint8_t                  length = 99;

        assert (encoder.encode (entry, output, sizeof (output), length).ok ());

        const char expected[] =
            "adk-cue,1,4294967295,4294967295,confirmation-requested,31,31,"
            "capacity exceeded\n";

        assert (length == strlen (expected));
        assert (memcmp (output, expected, length) == 0);

        char    shortOutput[8];
        uint8_t preservedLength = 17;

        assert (
            encoder.encode (entry, shortOutput, sizeof (shortOutput), preservedLength)
                .error () == adk::StatusCode::CapacityExceeded);
        assert (preservedLength == 17);
        assert (encoder.encode (entry, nullptr, 0, preservedLength).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testEveryEventName ()
    {
        adk::CueAuditEncoder encoder;
        char                 output[adk::CueAuditEncoder::maximumLength];

        for (uint8_t raw = 0;
             raw <= static_cast<uint8_t> (adk::CueAuditEvent::Shutdown); ++raw)
        {
            adk::CueAuditEntry entry = sampleEntry ();
            uint8_t            length;

            entry.event = static_cast<adk::CueAuditEvent> (raw);
            assert (encoder.encode (entry, output, sizeof (output), length).ok ());
            assert (length > 0);
            assert (output[length - 1] == '\n');
        }

        adk::CueAuditEntry invalid = sampleEntry ();
        uint8_t            length  = 0;

        invalid.event = static_cast<adk::CueAuditEvent> (255);
        assert (encoder.encode (invalid, output, sizeof (output), length).error () ==
                adk::StatusCode::InvalidArgument);
    }
} // namespace

int main ()
{
    testConfigurationAndLifecycle ();
    testEncoderGrammarAndCapacity ();
    testEveryEventName            ();
}
