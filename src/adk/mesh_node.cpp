#include "mesh_node.h"

#include <string.h>

namespace adk {

    namespace {

        const unsigned long Baud = 38400;
        const Millis        Gap  = 1500;        // the node sends after 1 s of quiet
    }

    MeshNode::MeshNode (HardwareSerial& port)
        : port_     (port)
        , sender_   {}
        , text_     {}
        , now_      (0)
        , sentAt_   (0)
        , sent_     (false)
        , received_ (false)
    {
    }

    void MeshNode::setup ()
    {
        if (claimSerial (port_))
        {
            port_.begin (Baud);
        }
    }

    bool MeshNode::send (const char* text)
    {
        if (!canSend () || strlen (text) > MaxLength)
        {
            return false;
        }

        // No newline: the node would send it as part of the message.
        port_.print (text);
        sentAt_ = now_;
        sent_   = true;
        return true;
    }

    bool MeshNode::canSend () const
    {
        return !sent_ || now_ - sentAt_ >= Gap;
    }

    bool MeshNode::wasReceived () const
    {
        return received_;
    }

    const char* MeshNode::sender () const
    {
        return sender_;
    }

    const char* MeshNode::text () const
    {
        return text_;
    }

    // The node puts a blank line before and after each message.
    void MeshNode::update (Millis now)
    {
        now_      = now;
        received_ = false;

        while (reader_.read (port_))
        {
            const char* line = reader_.line ();

            if (line[0] == '\0')
            {
                continue;
            }

            const char* colon = strstr (line, ": ");
            size_t      name  = colon ? static_cast<size_t> (colon - line) : 0;

            if (name >= sizeof sender_)
            {
                name  = 0;
                colon = nullptr;
            }

            memcpy (sender_, line, name);
            sender_[name] = '\0';
            strcpy (text_, colon ? colon + 2 : line);
            received_ = true;
            return;
        }
    }
}
