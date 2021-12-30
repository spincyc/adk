
#include <object.h>

namespace adk {

    static object*& obj_list_head()
    {
        static object* ret = nullptr;
        return ret;
    }

    static object*& obj_list_tail()
    {
        static object* ret = nullptr;
        return ret;
    }

    void object::setup_all()
    {
        for(auto* obj = obj_list_head(); obj; obj = obj->_next)
        {
            obj->setup();
        }
    }

    void object::update_all()
    {
    }

    object::object()
        : _next(nullptr)
    {
        auto*& head = obj_list_head();
        auto*& tail = obj_list_tail();

        if(!head)
        {
            head = this;
            tail = this;
        }
        else
        {
            tail->_next = this;
            tail        = this;
        }
    }

}
