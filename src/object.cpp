
#include <object.h>

namespace adk {

    static Object*& objectListHead ()
    {
        static Object* head = nullptr;
        return head;
    }

    static Object*& objectListTail ()
    {
        static Object* tail = nullptr;
        return tail;
    }

    void Object::initializeAll ()
    {
        for (auto* object = objectListHead (); object; object = object->next_)
        {
            object->initialize ();
        }
    }

    void Object::updateAll ()
    {
        for (auto* object = objectListHead (); object; object = object->next_)
        {
            object->update ();
        }
    }

    Object::Object ()
        : next_ (nullptr)
    {
        auto*& head = objectListHead ();
        auto*& tail = objectListTail ();

        if (!head)
        {
            head = this;
            tail = this;
        }
        else
        {
            tail->next_ = this;
            tail        = this;
        }
    }

    Object::~Object () noexcept
    {
        auto*& head = objectListHead ();
        auto*& tail = objectListTail ();

        Object* previous = nullptr;
        for (auto* current = head; current; current = current->next_)
        {
            if (current != this)
            {
                previous = current;
                continue;
            }

            if (previous)
            {
                previous->next_ = next_;
            }
            else
            {
                head = next_;
            }

            if (tail == this)
            {
                tail = previous;
            }

            next_ = nullptr;
            return;
        }
    }

    void Object::update ()
    {
    }
}
