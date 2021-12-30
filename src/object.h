#pragma once

#include <defines.h>

namespace adk {

    struct object
    {
        static void setup_all();
        static void update_all();

        object();

      private:
        virtual void setup() = 0;
        
        object* _next;
    };

}
