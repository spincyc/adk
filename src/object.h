#pragma once

namespace adk {

    struct Object
    {
        static void initializeAll ();
        static void updateAll     ();

        Object          ();
        virtual ~Object () noexcept;

        Object            (const Object&) = delete;
        Object& operator= (const Object&) = delete;
        Object            (Object&&)      = delete;
        Object& operator= (Object&&)      = delete;

      protected:
        virtual void initialize () = 0;
        virtual void update     ();
        
      private:
        Object* next_;
    };
}
