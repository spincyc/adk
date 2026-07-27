#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "packet_receiver.h"

namespace {

    void testLifecycleAndStableObservation ()
    {
        adk::PacketReceiver receiver;
        const uint8_t       packet[] = {1, 2, 3};

        assert (receiver.submit ({packet, sizeof (packet)}, adk::TimePoint (4), 5)
                    .error () == adk::StatusCode::NotInitialized);
        assert (receiver.update (adk::TimePoint ()).error () ==
                adk::StatusCode::NotInitialized);
        assert (!receiver.observation ());
        assert (receiver.latest ().packet.data == nullptr);

        assert (receiver.initialize ().ok ());
        assert (receiver.initialize ().ok ());
        assert (receiver.initialized ());
        assert (
            receiver.submit ({packet, sizeof (packet)}, adk::TimePoint (4), 5).ok ());

        const adk::PacketObservation observed = receiver.latest ();
        assert                                                  (observed.packet.size == sizeof (packet));
        assert                                                  (memcmp (observed.packet.data, packet, sizeof (packet)) == 0);
        assert                                                  (observed.receivedAt == adk::TimePoint (4));
        assert                                                  (observed.captureSequence == 5);

        receiver.shutdown ();
        receiver.shutdown ();
        assert            (!receiver.initialized ());
        assert            (!receiver.observation ());
        assert            (receiver.initialize ().ok ());
    }

    void testCapacityAndInputBoundaries ()
    {
        adk::PacketReceiver receiver;
        uint8_t             maximum[adk::PacketReceiver::capacity] = {};
        assert (receiver.initialize ().ok ());
        assert (receiver.submit ({nullptr, 0}, adk::TimePoint (), 1).ok ());
        assert (receiver.latest ().packet.size == 0);
        assert (receiver.acknowledge (1).ok ());
        assert (
            receiver.submit ({maximum, sizeof (maximum)}, adk::TimePoint (), 2).ok ());
        assert (receiver.acknowledge (2).ok ());
        assert (receiver.submit ({maximum, sizeof (maximum) + 1}, adk::TimePoint (), 3)
                    .error () == adk::StatusCode::InvalidArgument);
        assert (receiver.submit ({nullptr, 1}, adk::TimePoint (), 4).error () ==
                adk::StatusCode::InvalidArgument);
    }

    void testAcknowledgementAndOverrun ()
    {
        adk::PacketReceiver receiver;
        const uint8_t       first[]  = {1, 2};
        const uint8_t       second[] = {9};
        assert (receiver.initialize ().ok ());
        assert (receiver.submit ({first, sizeof (first)}, adk::TimePoint (7), 8).ok ());
        assert (receiver.submit ({second, sizeof (second)}, adk::TimePoint (9), 10)
                    .error () == adk::StatusCode::CapacityExceeded);
        assert (receiver.update (adk::TimePoint ()).error () ==
                adk::StatusCode::CapacityExceeded);
        assert (receiver.latest ().captureSequence == 8);
        assert (receiver.latest ().packet.size == sizeof (first));
        assert (receiver.acknowledge (7).error () == adk::StatusCode::InvalidArgument);
        assert (receiver.acknowledge (8).ok ());
        assert (!receiver.observation ());
        assert (receiver.acknowledge (8).error () == adk::StatusCode::InvalidArgument);
        assert (receiver.update (adk::TimePoint ()).ok ());
        assert (
            receiver.submit ({second, sizeof (second)}, adk::TimePoint (9), 10).ok ());
    }

    void testReceiverFaultAndRestart ()
    {
        adk::PacketReceiver receiver;
        const uint8_t       packet[] = {4};
        assert (receiver.initialize ().ok ());
        assert (receiver.observeFailure (adk::StatusCode::Ok).error () ==
                adk::StatusCode::InvalidArgument);
        assert (receiver.observeFailure (adk::StatusCode::HardwareFailure).ok ());
        assert (receiver.update (adk::TimePoint ()).error () ==
                adk::StatusCode::HardwareFailure);
        assert (
            receiver.submit ({packet, sizeof (packet)}, adk::TimePoint (), 1).ok ());
        assert            (receiver.update (adk::TimePoint ()).ok ());
        receiver.shutdown ();
        assert            (receiver.observeFailure (adk::StatusCode::HardwareFailure).error () ==
                adk::StatusCode::NotInitialized);
        assert (receiver.initialize ().ok ());
        assert (receiver.update (adk::TimePoint ()).ok ());
    }

    void testCopiesCallerStorage ()
    {
        adk::PacketReceiver receiver;
        uint8_t             packet[] = {1, 2, 3};
        assert (receiver.initialize ().ok ());
        assert (
            receiver.submit ({packet, sizeof (packet)}, adk::TimePoint (), 1).ok ());
        memset (packet, 0, sizeof (packet));
        assert (receiver.latest ().packet.data[0] == 1);
        assert (receiver.latest ().packet.data[1] == 2);
        assert (receiver.latest ().packet.data[2] == 3);
    }
} // namespace

int main ()
{
    testLifecycleAndStableObservation ();
    testCapacityAndInputBoundaries    ();
    testAcknowledgementAndOverrun     ();
    testReceiverFaultAndRestart       ();
    testCopiesCallerStorage           ();
}
