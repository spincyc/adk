#pragma once

namespace adk {

    // A radio that carries lines of text between two boards, which is all a
    // Bridge needs: a LoraModem, LoraLink or MeshNode both ways, or a
    // RadioTransmitter one way and a RadioReceiver the other.
    struct Link
    {
        // Send a line of text. False, and nothing sent, if the radio is busy
        // or can't send at all.
        virtual bool sendLine (const char* text) = 0;

        // The line that arrived in this update, or nullptr.
        virtual const char* heardLine () const = 0;

      protected:
        ~Link () = default;
    };
}
