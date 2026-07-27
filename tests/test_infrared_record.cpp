#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "infrared_record.h"

namespace {

    void testEncodesCanonicalGoldenRecord ()
    {
        const adk::InfraredFrame frame = {adk::InfraredProtocol::Nec,
                                          adk::FrameValidity::Valid, 0x12, 0xa5,
                                          4294967295u};
        constexpr char expected[]      = "IR1,4294967295,NEC,VALID,00000012,000000A5\n";
        char           storage[sizeof (expected) - 1] = {};

        const adk::Result<uint16_t> result = adk::InfraredRecordEncoder ().encode (
            frame, {storage, static_cast<uint16_t> (sizeof (storage))});

        assert (result.ok ());
        assert (result.value () == sizeof (storage));
        assert (memcmp (storage, expected, sizeof (storage)) == 0);
    }

    void testEncodesInvalidEvidenceWithoutInventedFields ()
    {
        const adk::InfraredFrame frame = {adk::InfraredProtocol::Unknown,
                                          adk::FrameValidity::UnknownProtocol, 0, 0, 7};
        constexpr char           expected[] =
            "IR1,7,UNKNOWN,UNKNOWN_PROTOCOL,00000000,00000000\n";
        char storage[sizeof (expected) - 1] = {};

        const adk::Result<uint16_t> result = adk::InfraredRecordEncoder ().encode (
            frame, {storage, static_cast<uint16_t> (sizeof (storage))});

        assert (result.ok ());
        assert (result.value () == sizeof (storage));
        assert (memcmp (storage, expected, sizeof (storage)) == 0);
    }

    void testOneByteShortBufferRemainsUnchanged ()
    {
        const adk::InfraredFrame frame      = {adk::InfraredProtocol::Nec,
                                               adk::FrameValidity::Repeat, 0, 0, 1};
        constexpr char           expected[] = "IR1,1,NEC,REPEAT,00000000,00000000\n";
        char                     storage[sizeof (expected) - 1];
        memset (storage, 'x', sizeof (storage));

        const adk::Result<uint16_t> result = adk::InfraredRecordEncoder ().encode (
            frame, {storage, static_cast<uint16_t> (sizeof (storage) - 1)});

        assert (result.error () == adk::StatusCode::CapacityExceeded);
        assert (result.value () == 0);

        for (uint8_t index = 0; index < sizeof (storage); ++index)
        {
            assert (storage[index] == 'x');
        }
    }

    void testNullStorageFailsWithoutWriting ()
    {
        const adk::InfraredFrame    frame = {adk::InfraredProtocol::Nec,
                                             adk::FrameValidity::Valid, 1, 2, 3};
        const adk::Result<uint16_t> result =
            adk::InfraredRecordEncoder ().encode (frame, {nullptr, 80});

        assert (result.error () == adk::StatusCode::CapacityExceeded);
        assert (result.value () == 0);
    }

    void testZeroCapacityStorageRemainsUnchanged ()
    {
        const adk::InfraredFrame frame   = {adk::InfraredProtocol::Nec,
                                            adk::FrameValidity::Valid, 1, 2, 3};
        char                     storage = 'x';

        const adk::Result<uint16_t> result =
            adk::InfraredRecordEncoder ().encode (frame, {&storage, 0});

        assert (result.error () == adk::StatusCode::CapacityExceeded);
        assert (result.value () == 0);
        assert (storage == 'x');
    }

    void testNormalizesFieldlessEvidence ()
    {
        const adk::InfraredFrame frame      = {adk::InfraredProtocol::Nec,
                                               adk::FrameValidity::Repeat, 0x12345678,
                                               0x90abcdef, 8};
        constexpr char           expected[] = "IR1,8,NEC,REPEAT,00000000,00000000\n";
        char                     storage[sizeof (expected) - 1] = {};

        const adk::Result<uint16_t> result = adk::InfraredRecordEncoder ().encode (
            frame, {storage, static_cast<uint16_t> (sizeof (storage))});

        assert (result.ok ());
        assert (memcmp (storage, expected, sizeof (storage)) == 0);
    }

    void testRejectsUnknownEnumsWithoutMutation ()
    {
        char storage[80];
        memset (storage, 'x', sizeof (storage));

        const adk::InfraredFrame unknownProtocol = {
            static_cast<adk::InfraredProtocol> (255),
            adk::FrameValidity::UnknownProtocol, 0, 0, 1};
        const adk::Result<uint16_t> protocolResult =
            adk::InfraredRecordEncoder ().encode (
                unknownProtocol, {storage, static_cast<uint16_t> (sizeof (storage))});
        assert (protocolResult.error () == adk::StatusCode::InvalidArgument);

        const adk::InfraredFrame unknownValidity = {
            adk::InfraredProtocol::Unknown, static_cast<adk::FrameValidity> (255), 0, 0,
            1};
        const adk::Result<uint16_t> validityResult =
            adk::InfraredRecordEncoder ().encode (
                unknownValidity, {storage, static_cast<uint16_t> (sizeof (storage))});
        assert (validityResult.error () == adk::StatusCode::InvalidArgument);

        for (uint8_t index = 0; index < sizeof (storage); ++index)
        {
            assert (storage[index] == 'x');
        }
    }

    void testRejectsIncoherentProtocols ()
    {
        char storage[80];
        memset (storage, 'x', sizeof (storage));

        const adk::InfraredFrame validUnknown  = {adk::InfraredProtocol::Unknown,
                                                  adk::FrameValidity::Valid, 1, 2, 3};
        const adk::InfraredFrame repeatUnknown = {adk::InfraredProtocol::Unknown,
                                                  adk::FrameValidity::Repeat, 1, 2, 3};

        const adk::Result<uint16_t> validResult =
            adk::InfraredRecordEncoder ().encode (
                validUnknown, {storage, sizeof (storage)});
        const adk::Result<uint16_t> repeatResult =
            adk::InfraredRecordEncoder ().encode (
                repeatUnknown, {storage, sizeof (storage)});

        assert (validResult.error () == adk::StatusCode::InvalidArgument);
        assert (repeatResult.error () == adk::StatusCode::InvalidArgument);
    }
} // namespace

int main ()
{
    testEncodesCanonicalGoldenRecord                ();
    testEncodesInvalidEvidenceWithoutInventedFields ();
    testOneByteShortBufferRemainsUnchanged          ();
    testNullStorageFailsWithoutWriting              ();
    testZeroCapacityStorageRemainsUnchanged         ();
    testNormalizesFieldlessEvidence                 ();
    testRejectsUnknownEnumsWithoutMutation          ();
    testRejectsIncoherentProtocols                  ();
}
