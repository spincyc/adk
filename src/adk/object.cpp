#include "object.h"

#include <Arduino.h>

namespace adk {

    namespace {

        Object* first    = nullptr;
        Object* last     = nullptr;
        bool    updating = false;
    }

    Object::Object ()
        : next_ (nullptr)
    {
        (last ? last->next_ : first) = this;
        last = this;
    }

    Object::~Object ()
    {
        Object* previous = nullptr;

        for (Object* object = first; object; previous = object, object = object->next_)
        {
            if (object == this)
            {
                (previous ? previous->next_ : first) = next_;
                last = (last == this) ? previous : last;
                return;
            }
        }
    }

    void Object::setup ()
    {
    }

    void Object::update (Millis)
    {
    }

    void Object::stop ()
    {
    }

    bool start ()
    {
        releaseClaims ();

        for (Object* object = first; object; object = object->next_)
        {
            object->setup ();
        }

        return fault () == Fault::None;
    }

    void setup ()
    {
        if (!start ())
        {
            halt (fault (), faultPin ());
        }
    }

    void setup (Print& log)
    {
        if (!start ())
        {
            explain (log, fault (), faultPin ());
            halt    (fault (), faultPin ());
        }
    }

    void update ()
    {
        update (static_cast<Millis> (millis ()));
    }

    void update (Millis now)
    {
        updating = true;

        for (Object* object = first; object; object = object->next_)
        {
            object->update (now);
        }

        updating = false;
    }

    void wait (Millis duration)
    {
        // Waiting from inside an update would update that object again.
        if (updating)
        {
            delay (duration);
            return;
        }

        Millis began = static_cast<Millis> (millis ());

        for (;;)
        {
            update ();

            if (static_cast<Millis> (millis ()) - began >= duration)
            {
                return;
            }

            delay (1);
        }
    }

    void stop ()
    {
        for (Object* object = first; object; object = object->next_)
        {
            object->stop ();
        }
    }
}
